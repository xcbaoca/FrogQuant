#include "market/binance_ws_client.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <iostream>

namespace fq::market {
namespace {
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;
} // namespace

BinanceWsClient::BinanceWsClient(std::string symbol,
                                 TickHandler on_tick,
                                 std::string host,
                                 std::string port)
    : symbol_(std::move(symbol)),
      on_tick_(std::move(on_tick)),
      host_(std::move(host)),
      port_(std::move(port)) {}

BinanceWsClient::~BinanceWsClient() {
    stop();
}

void BinanceWsClient::start() {
    if (running_.exchange(true)) return;
    worker_ = std::thread(&BinanceWsClient::run, this);
}

void BinanceWsClient::stop() {
    if (!running_.exchange(false)) return;
    if (worker_.joinable()) worker_.join();
}

void BinanceWsClient::run() {
    // Binance stream path 约定：/ws/<symbol_lower>@trade
    const auto target = "/ws/" + to_lower(symbol_) + "@trade";

    // 持续运行，异常时自动重连。
    while (running_.load(std::memory_order_relaxed)) {
        try {
            // 1) 准备 IO 上下文 + TLS
            net::io_context ioc;
            ssl::context ctx{ssl::context::tlsv12_client};
            ctx.set_default_verify_paths();

            // 2) 构造 WSS 流
            websocket::stream<beast::ssl_stream<tcp::socket>> ws{ioc, ctx};

            // 3) DNS 解析 + TCP 连接
            tcp::resolver resolver{ioc};
            auto const results = resolver.resolve(host_, port_);
            // 注意：这里要使用 asio 的 free function connect，
            // 不能直接把 resolver results 传给 socket::connect。
            net::connect(beast::get_lowest_layer(ws), results);

            // 4) TLS 握手 + WS 握手
            ws.next_layer().handshake(ssl::stream_base::client);
            ws.handshake(host_, target);
            std::cout << "[ws] connected " << host_ << target << "\n";

            // 5) 读循环：每次 read 一条消息，解析并回调
            while (running_.load(std::memory_order_relaxed)) {
                beast::flat_buffer buffer;
                ws.read(buffer);
                const auto text = beast::buffers_to_string(buffer.data());

                MarketTick tick;
                if (parse_trade_json(text, tick) && on_tick_) {
                    on_tick_(tick);
                }
            }

            // 6) 优雅关闭（stop 触发时）
            beast::error_code ec;
            ws.close(websocket::close_code::normal, ec);
        } catch (const std::exception& ex) {
            // 网络抖动、握手失败、读异常等都走这里，做重连。
            std::cout << "[ws] error: " << ex.what() << ", reconnect in 1s\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

bool BinanceWsClient::parse_trade_json(const std::string& json, MarketTick& out) {
    // Binance trade payload 常用字段：
    // s = symbol, p = price, E = event time
    const auto sym = extract_json_value(json, "s");
    const auto p = extract_json_value(json, "p");
    const auto e = extract_json_value(json, "E");

    if (sym.empty() || p.empty()) return false;

    try {
        out.symbol = sym;
        out.price = std::stod(p);
        out.exchange_ts = e.empty() ? 0 : static_cast<uint64_t>(std::stoull(e));
        out.recv_ts = now_ms();
        out.source = TickSource::WebSocket;
        return true;
    } catch (...) {
        return false;
    }
}

uint64_t BinanceWsClient::now_ms() {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

std::string BinanceWsClient::to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string BinanceWsClient::extract_json_value(const std::string& json, const std::string& key) {
    // 轻量键值提取："key":<value>
    // 支持字符串和数字两种形式。
    const std::string token = "\"" + key + "\":";
    auto pos = json.find(token);
    if (pos == std::string::npos) return {};
    pos += token.size();

    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) ++pos;

    // 字符串值
    if (pos < json.size() && json[pos] == '"') {
        ++pos;
        auto end = json.find('"', pos);
        if (end == std::string::npos) return {};
        return json.substr(pos, end - pos);
    }

    // 数字值
    auto end = json.find_first_of(",}", pos);
    if (end == std::string::npos) return {};
    return json.substr(pos, end - pos);
}

} // namespace fq::market
