#pragma once

#include "app/app_support.hpp"
#include "control/local_http_server.hpp"
#include "core/trading_core.hpp"
#include "execution/binance_testnet_execution.hpp"
#include "strategy/long_grid_strategy.hpp"

#include <atomic>
#include <deque>
#include <functional>
#include <mutex>
#include <vector>

namespace fq::app {

struct ControlApiContext {
    const AppConfig& cfg;
    const execution::ExecConfig& exec_cfg;
    const uint64_t start_ms;

    std::atomic<uint64_t>& tick_count;
    std::atomic<uint64_t>& last_tick_ms;
    std::atomic<bool>& strategy_enabled;

    strategy::LongGridStrategy& grid;
    std::mutex& grid_mu;

    core::TradingCore& core;
    core::RiskConfig& risk_cfg;
    execution::BinanceTestnetExecution& exec;

    std::deque<std::string>& events;
    std::vector<ApiOrderItem>& orders;
    std::mutex& state_mu;

    size_t max_orders;
    size_t order_trim_batch;

    std::function<void(const std::string&)> push_event;
};

control::LocalHttpServer::RequestHandler make_control_api_handler(ControlApiContext ctx);

} // namespace fq::app
