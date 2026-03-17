#include "market/market_data_service.hpp"

#include <iostream>

namespace fq::market {

MarketDataService::MarketDataService(std::string symbol,
                                     TickHandler on_tick,
                                     std::chrono::milliseconds rest_interval)
    : symbol_(std::move(symbol)),
      on_tick_(std::move(on_tick)),
      rest_interval_(rest_interval),
      // WS 收到数据后直接透传给统一回调。
      ws_client_(symbol_, [this](const MarketTick& t) {
          if (on_tick_) on_tick_(t);
      }),
      rest_client_() {}

MarketDataService::~MarketDataService() {
    stop();
}

void MarketDataService::start() {
    if (running_.exchange(true)) return;

    ws_client_.start();
    rest_thread_ = std::thread(&MarketDataService::rest_loop, this);

    std::cout << "[market-service] started\n";
}

void MarketDataService::stop() {
    if (!running_.exchange(false)) return;

    // 停止顺序：先停 WS，再等 REST 线程退出
    ws_client_.stop();
    if (rest_thread_.joinable()) rest_thread_.join();

    std::cout << "[market-service] stopped\n";
}

void MarketDataService::rest_loop() {
    while (running_.load(std::memory_order_relaxed)) {
        const auto tick = rest_client_.fetch_last_price(symbol_);
        if (tick.has_value()) {
            if (on_tick_) on_tick_(*tick);
        }

        // 固定周期轮询，简单稳定。
        std::this_thread::sleep_for(rest_interval_);
    }
}

} // namespace fq::market
