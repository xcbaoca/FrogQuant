#pragma once

#include "core/order_types.hpp"

#include <optional>
#include <string>

namespace fq::execution {

// ExecConfig: 执行模块配置。
// - api_key/secret_key: Binance Testnet API 凭据
// - base_url: 默认 USD-M Futures Testnet
// - dry_run: true 时仅构造请求不真正下单
struct ExecConfig {
    std::string api_key;
    std::string secret_key;
    std::string base_url{"https://testnet.binancefuture.com"};
    bool dry_run{true};
};

// ExecResult: 执行结果统一结构。
struct ExecResult {
    bool success{false};
    long http_status{0};
    std::string body;
    std::string error;
};

// BinanceTestnetExecution:
// 提供 MARKET/LIMIT 下单与撤单能力。
class BinanceTestnetExecution {
public:
    explicit BinanceTestnetExecution(ExecConfig cfg);

    ExecResult place_order(const std::string& symbol,
                           fq::core::OrderSide side,
                           fq::core::OrderType type,
                           double qty,
                           std::optional<double> price = std::nullopt,
                           std::optional<std::string> client_order_id = std::nullopt) const;

    ExecResult cancel_order(const std::string& symbol, const std::string& order_id) const;

    // 测试辅助：从 Binance 下单响应中提取 orderId。
    static std::optional<std::string> parse_order_id_from_json(const std::string& body);

private:
    static std::string side_to_str(fq::core::OrderSide side);
    static std::string type_to_str(fq::core::OrderType type);

    static std::string extract_json_value(const std::string& json, const std::string& key);

    std::string sign_query_hmac_sha256(const std::string& query) const;
    ExecResult send_signed_request(const std::string& method,
                                   const std::string& path,
                                   const std::string& query) const;

    ExecConfig cfg_;
};

} // namespace fq::execution
