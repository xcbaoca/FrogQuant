#include "core/trading_core.hpp"
#include "logger/async_jsonl_logger.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>

static void expect(bool cond, const char* msg) {
    if (!cond) {
        std::cerr << "[FAIL] " << msg << "\n";
        std::exit(1);
    }
}

int main() {
    const std::string log_path = "test_core_logs.jsonl";
    std::filesystem::remove(log_path);

    fq::logger::AsyncJsonlLogger logger(log_path, 1024);
    logger.start();

    fq::core::RiskConfig cfg;
    cfg.max_single_order_notional_usd = 10.0;
    cfg.max_position_ratio = 0.80;
    cfg.daily_loss_stop_ratio = 0.05;

    fq::core::TradingCore core(cfg, logger);

    // 先喂一条行情，确保 ref_price 可用
    fq::market::MarketTick tick;
    tick.symbol = "BTCUSDT";
    tick.price = 60000.0;
    tick.recv_ts = 1;
    tick.source = fq::market::TickSource::RestFallback;
    core.on_tick(tick);

    fq::core::OrderRequest small;
    small.symbol = "BTCUSDT";
    small.side = fq::core::OrderSide::Buy;
    small.type = fq::core::OrderType::Market;
    small.qty = 0.0001; // ~6 usd

    fq::core::OrderRequest big = small;
    big.qty = 0.001; // ~60 usd

    const auto id1 = core.submit_manual_order(small);
    const auto id2 = core.submit_manual_order(big);

    auto o1 = core.get_order(id1);
    auto o2 = core.get_order(id2);

    expect(o1.has_value(), "order1 should exist");
    expect(o2.has_value(), "order2 should exist");
    expect(o1->status == fq::core::OrderStatus::Accepted, "small order should be accepted");
    expect(o2->status == fq::core::OrderStatus::Rejected, "big order should be rejected");

    const bool cancel_ok = core.cancel_order(id1);
    expect(cancel_ok, "accepted order should be cancellable");

    auto o1_after = core.get_order(id1);
    expect(o1_after.has_value(), "order1 should still exist");
    expect(o1_after->status == fq::core::OrderStatus::Cancelled, "order1 should be cancelled");

    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    logger.stop();

    expect(std::filesystem::exists(log_path), "core log file should exist");
    expect(std::filesystem::file_size(log_path) > 0, "core log file should not be empty");
    std::filesystem::remove(log_path);

    std::cout << "[PASS] frogquant_risk_oms_core_tests\n";
    return 0;
}
