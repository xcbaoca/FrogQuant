#include "app/app_support.hpp"
#include "app/bootstrap.hpp"
#include "app/control_api_router.hpp"
#include "app/runtime_loop.hpp"
#include "app/run_guard.hpp"
#include "control/local_http_server.hpp"
#include "core/trading_core.hpp"
#include "execution/binance_testnet_execution.hpp"
#include "logger/async_jsonl_logger.hpp"
#include "market/market_data_service.hpp"
#include "strategy/long_grid_strategy.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <deque>

namespace {
std::atomic<bool> g_stop{false};
void signal_handler(int) { g_stop.store(true); }
} // namespace

using namespace fq::app;
int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    BootstrapResult bs;
    std::string bootstrap_err;
    if (!bootstrap(bs, bootstrap_err)) {
        std::cerr << "[bootstrap] " << bootstrap_err << "\n";
        return 2;
    }
    AppConfig& cfg = bs.cfg;

    std::atomic<uint64_t> tick_count{0};
    std::atomic<uint64_t> last_tick_ms{0};
    std::atomic<bool> strategy_enabled{cfg.strategy_start_enabled};
    const uint64_t start_ms = now_ms();

    fq::logger::AsyncJsonlLogger logger(cfg.log_path, 8192);
    logger.start();

    // 读取运行态覆盖（策略启停 + 风控参数）
    bool strategy_enabled_init = strategy_enabled.load();
    load_runtime_state(cfg.runtime_state_path, strategy_enabled_init, bs.risk_cfg);
    strategy_enabled.store(strategy_enabled_init);

    fq::core::TradingCore core(bs.risk_cfg, logger);
    fq::execution::ExecConfig exec_cfg = bs.exec_cfg;
    fq::execution::BinanceTestnetExecution exec(exec_cfg);

    fq::strategy::LongGridStrategy grid(cfg.grid_default);
    std::mutex grid_mu;

    std::deque<std::string> events;
    std::vector<ApiOrderItem> orders;
    std::mutex state_mu;

    const size_t kMaxEvents = std::max<size_t>(1, cfg.max_events_cache);
    const size_t kMaxOrders = std::max<size_t>(100, cfg.max_orders_cache);
    const size_t kOrderTrimBatch = std::max<size_t>(1, cfg.order_trim_batch);

    auto push_event = [&](const std::string& e) {
        std::scoped_lock lk(state_mu);
        events.push_back(e);
        if (events.size() > kMaxEvents) events.pop_front();
    };

        ControlApiContext api_ctx{
        cfg,
        exec_cfg,
        start_ms,
        tick_count,
        last_tick_ms,
        strategy_enabled,
        grid,
        grid_mu,
        core,
        bs.risk_cfg,
        exec,
        events,
        orders,
        state_mu,
        kMaxOrders,
        kOrderTrimBatch,
        push_event,
    };

    fq::control::LocalHttpServer control(static_cast<unsigned short>(cfg.control_port), make_control_api_handler(api_ctx));

    control.start();
    std::cout << "[control] listening on http://127.0.0.1:" << cfg.control_port << "\n";

    RuntimeLoopContext loop_ctx{cfg, tick_count, last_tick_ms, strategy_enabled, core, grid, grid_mu, push_event};
    fq::market::MarketDataService service(
        cfg.symbol,
        make_tick_handler(loop_ctx),
        std::chrono::milliseconds(cfg.market_poll_ms));

    service.start();

    run_until_stop(g_stop, cfg.run_seconds);

    std::cout << "[runtime] shutdown signal received, stopping services...\n";
    service.stop();
    control.stop();
    logger.stop();

    std::cout << "[summary] total_ticks=" << tick_count.load() << " dropped_logs=" << logger.dropped_count() << "\n";
    return 0;
}

