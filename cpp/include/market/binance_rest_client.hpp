#pragma once

#include "market/market_types.hpp"

#include <optional>
#include <string>

namespace fq::market {

// BinanceRestClient:
// 通过 Binance Testnet 公共 REST 拉取最新价格。
// 用途：
// 1) 作为 WS 断流时的兜底数据源
// 2) 作为调试时的简单可控行情来源
class BinanceRestClient {
public:
    // 默认指向 Binance Testnet。
    explicit BinanceRestClient(std::string base_url = "https://testnet.binance.vision");

    // fetch_last_price:
    // 拉取 symbol 最新价格，成功返回标准化 MarketTick，失败返回 nullopt。
    std::optional<MarketTick> fetch_last_price(const std::string& symbol) const;

    // parse_price_json:
    // 从 ticker/price 的 JSON 文本中提取 price 字段。
    // 单独暴露为 static，便于纯单元测试（不依赖网络）。
    static std::optional<double> parse_price_json(const std::string& json);

private:
    // 简单 HTTP GET 工具。
    static std::optional<std::string> http_get(const std::string& url);

    // 毫秒时间戳工具。
    static uint64_t now_ms();

    std::string base_url_;
};

} // namespace fq::market
