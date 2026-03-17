#pragma once

#include "market/binance_rest_client.hpp"
#include "market/binance_ws_client.hpp"

#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <thread>

namespace fq::market {

// MarketDataService:
// 行情模块编排层，负责把 WS 主通道 + REST fallback 统一成一个输出回调。
//
// 线程模型：
// - WS 客户端内部自带 1 个线程
// - Service 内部维护 1 个 REST fallback 线程
//
// 对下游来说，统一只看到 MarketTick，不关心来源细节。
class MarketDataService {
public:
    using TickHandler = std::function<void(const MarketTick&)>;

    MarketDataService(std::string symbol,
                      TickHandler on_tick,
                      std::chrono::milliseconds rest_interval = std::chrono::milliseconds(1000));
    ~MarketDataService();

    // 启动 WS + REST fallback。
    void start();

    // 停止所有后台线程。
    void stop();

private:
    // REST fallback 循环：定时拉价并回调。
    void rest_loop();

    std::string symbol_;
    TickHandler on_tick_;
    std::chrono::milliseconds rest_interval_;

    BinanceWsClient ws_client_;
    BinanceRestClient rest_client_;

    std::atomic<bool> running_{false};
    std::thread rest_thread_;
};

} // namespace fq::market
