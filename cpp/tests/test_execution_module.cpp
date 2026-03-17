#include "execution/binance_testnet_execution.hpp"

#include <cstdlib>
#include <iostream>

static void expect(bool cond, const char* msg) {
    if (!cond) {
        std::cerr << "[FAIL] " << msg << "\n";
        std::exit(1);
    }
}

int main() {
    // 1) orderId 解析测试
    const std::string ack = R"({"orderId":123456789,"status":"NEW"})";
    const auto id = fq::execution::BinanceTestnetExecution::parse_order_id_from_json(ack);
    expect(id.has_value(), "orderId should be parsed");
    expect(*id == "123456789", "orderId should match");

    // 2) dry-run 下单测试（需要给非空 key，避免空凭证被前置拦截）
    fq::execution::ExecConfig cfg;
    cfg.api_key = "demo";
    cfg.secret_key = "demo";
    cfg.dry_run = true;

    fq::execution::BinanceTestnetExecution exec(cfg);
    const auto r = exec.place_order("BTCUSDT",
                                    fq::core::OrderSide::Buy,
                                    fq::core::OrderType::Market,
                                    0.001,
                                    std::nullopt);

    expect(r.success, "dry-run place_order should succeed");
    expect(r.http_status == 200, "dry-run http status should be 200");

    std::cout << "[PASS] frogquant_execution_tests\n";
    return 0;
}
