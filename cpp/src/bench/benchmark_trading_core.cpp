#include "core/trading_core.hpp"
#include "logger/async_jsonl_logger.hpp"
#include "market/market_types.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    // 用法：
    //   ./frogquant_benchmark_trading_core [iterations]
    const int iterations = (argc > 1) ? std::max(1000, std::atoi(argv[1])) : 200000;

    const std::string log_path = "benchmark_logs.jsonl";
    std::remove(log_path.c_str());

    fq::logger::AsyncJsonlLogger logger(log_path, 1 << 15);
    logger.start();

    fq::core::RiskConfig risk{};
    risk.max_single_order_notional_usd = 1e9; // benchmark 期间避免因风控拒单影响测量
    risk.max_position_ratio = 1.0;
    risk.daily_loss_stop_ratio = 1.0;

    fq::core::TradingCore core(risk, logger);

    fq::market::MarketTick tick;
    tick.symbol = "BTCUSDT";
    tick.price = 60000.0;
    tick.recv_ts = 1;
    tick.source = fq::market::TickSource::RestFallback;
    core.on_tick(tick);

    fq::core::OrderRequest req;
    req.symbol = "BTCUSDT";
    req.side = fq::core::OrderSide::Buy;
    req.type = fq::core::OrderType::Market;
    req.qty = 0.001;

    std::vector<uint64_t> ns;
    ns.reserve(static_cast<size_t>(iterations));

    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i) {
        const auto s = std::chrono::steady_clock::now();
        (void)core.submit_manual_order(req);
        const auto e = std::chrono::steady_clock::now();
        ns.push_back(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()));
    }
    auto t1 = std::chrono::steady_clock::now();

    std::sort(ns.begin(), ns.end());
    auto pct = [&](double p) -> uint64_t {
        const size_t idx = static_cast<size_t>(p * (ns.size() - 1));
        return ns[idx];
    };

    const auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    const double sec = static_cast<double>(total_ns) / 1e9;
    const double ops = static_cast<double>(iterations) / sec;

    logger.stop();

    std::cout << "[benchmark] module=trading_core_submit_order\n";
    std::cout << "[benchmark] iterations=" << iterations << " total_sec=" << sec << " ops_per_sec=" << ops << "\n";
    std::cout << "[benchmark] latency_ns p50=" << pct(0.50)
              << " p95=" << pct(0.95)
              << " p99=" << pct(0.99)
              << " max=" << ns.back() << "\n";
    std::cout << "[benchmark] dropped_logs=" << logger.dropped_count() << "\n";

    return 0;
}
