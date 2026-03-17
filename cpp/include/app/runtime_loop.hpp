#pragma once

#include "app/app_support.hpp"
#include "core/trading_core.hpp"
#include "market/market_data_service.hpp"
#include "strategy/long_grid_strategy.hpp"

#include <atomic>
#include <functional>
#include <mutex>
#include <string>

namespace fq::app {

struct RuntimeLoopContext {
    const AppConfig& cfg;
    std::atomic<uint64_t>& tick_count;
    std::atomic<uint64_t>& last_tick_ms;
    std::atomic<bool>& strategy_enabled;

    core::TradingCore& core;
    strategy::LongGridStrategy& grid;
    std::mutex& grid_mu;

    std::function<void(const std::string&)> push_event;
};

std::function<void(const market::MarketTick&)> make_tick_handler(RuntimeLoopContext ctx);

} // namespace fq::app
