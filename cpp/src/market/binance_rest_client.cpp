#include "market/binance_rest_client.hpp"

#include <chrono>
#include <curl/curl.h>

namespace fq::market {
namespace {

// libcurl 回调：把响应体追加到 string。
size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    const size_t total = size * nmemb;
    auto* out = static_cast<std::string*>(userp);
    out->append(static_cast<char*>(contents), total);
    return total;
}

} // namespace

BinanceRestClient::BinanceRestClient(std::string base_url)
    : base_url_(std::move(base_url)) {}

std::optional<MarketTick> BinanceRestClient::fetch_last_price(const std::string& symbol) const {
    // Binance Testnet ticker/price endpoint
    const auto url = base_url_ + "/api/v3/ticker/price?symbol=" + symbol;

    const auto body = http_get(url);
    if (!body.has_value()) return std::nullopt;

    const auto price = parse_price_json(*body);
    if (!price.has_value()) return std::nullopt;

    // 标准化输出为 MarketTick
    MarketTick t;
    t.symbol = symbol;
    t.price = *price;
    t.exchange_ts = 0; // 该接口不返回事件时间
    t.recv_ts = now_ms();
    t.source = TickSource::RestFallback;
    return t;
}

std::optional<double> BinanceRestClient::parse_price_json(const std::string& json) {
    // 轻量解析：查找 "price":"..."
    const std::string key = "\"price\":\"";
    auto pos = json.find(key);
    if (pos == std::string::npos) return std::nullopt;

    pos += key.size();
    const auto end = json.find('"', pos);
    if (end == std::string::npos || end <= pos) return std::nullopt;

    try {
        return std::stod(json.substr(pos, end - pos));
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::string> BinanceRestClient::http_get(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return std::nullopt;

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 3000L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    const CURLcode code = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);

    // 非 200 或网络错误都视为失败，交给上层处理。
    if (code != CURLE_OK || http_code != 200) return std::nullopt;
    return response;
}

uint64_t BinanceRestClient::now_ms() {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

} // namespace fq::market
