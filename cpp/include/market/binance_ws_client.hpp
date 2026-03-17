#pragma once

#include "market/market_types.hpp"

#include <atomic>
#include <functional>
#include <string>
#include <thread>

namespace fq::market {

// BinanceWsClient:
// 通过 Binance Testnet WebSocket 持续接收 trade 数据（主行情通道）。
//
// 设计要点：
// - 独立后台线程，避免阻塞主线程
// - 网络异常自动重连
// - 输出统一 MarketTick，屏蔽上游协议细节
class BinanceWsClient {
public:
    using TickHandler = std::function<void(const MarketTick&)>;

    BinanceWsClient(std::string symbol,
                    TickHandler on_tick,
                    std::string host = "stream.testnet.binance.vision",
                    std::string port = "9443");
    ~BinanceWsClient();

    // start/stop 线程生命周期管理。
    void start();
    void stop();

    // parse_trade_json:
    // 解析 Binance trade payload，提取 s/p/E 字段。
    // static 暴露用于单元测试。
    static bool parse_trade_json(const std::string& json, MarketTick& out);

private:
    // 后台主循环：连接 -> 读取 -> 解析 -> 回调 -> 异常重连。
    void run();

    static uint64_t now_ms();
    static std::string to_lower(std::string s);

    // extract_json_value:
    // 轻量 JSON 提取工具（当前阶段避免引入 JSON 库依赖）。
    static std::string extract_json_value(const std::string& json, const std::string& key);

    std::string symbol_;
    TickHandler on_tick_;
    std::string host_;
    std::string port_;

    std::atomic<bool> running_{false};
    std::thread worker_;
};

} // namespace fq::market
