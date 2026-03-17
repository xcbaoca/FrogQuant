#include "market/binance_rest_client.hpp"
#include "market/binance_ws_client.hpp"

#include <cstdlib>
#include <iostream>

// 极简断言函数：失败即退出。
static void expect(bool cond, const char* msg) {
    if (!cond) {
        std::cerr << "[FAIL] " << msg << "\n";
        std::exit(1);
    }
}

int main() {
    // ---------- REST 解析测试 ----------
    const std::string rest_json = R"({"symbol":"BTCUSDT","price":"68123.45000000"})";
    const auto p = fq::market::BinanceRestClient::parse_price_json(rest_json);
    expect(p.has_value(), "REST parser should extract price");
    expect(*p > 68123.44 && *p < 68123.46, "REST parser price should be correct");

    // ---------- WS trade 解析测试 ----------
    const std::string ws_json =
        R"({"e":"trade","E":1710000000123,"s":"BTCUSDT","t":12345,"p":"68120.12","q":"0.001"})";

    fq::market::MarketTick tick;
    const bool ok = fq::market::BinanceWsClient::parse_trade_json(ws_json, tick);

    expect(ok, "WS parser should parse trade payload");
    expect(tick.symbol == "BTCUSDT", "WS parser symbol should match");
    expect(tick.price > 68120.11 && tick.price < 68120.13, "WS parser price should match");
    expect(tick.exchange_ts == 1710000000123ULL, "WS parser exchange ts should match");

    std::cout << "[PASS] frogquant_market_tests\n";
    return 0;
}
