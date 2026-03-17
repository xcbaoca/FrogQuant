#pragma once

#include <cstdint>
#include <string>

namespace fq::core {

// OrderSide: 下单方向。
enum class OrderSide : uint8_t {
    Buy = 1,
    Sell = 2,
};

// OrderType: 首版先支持市价与限价。
enum class OrderType : uint8_t {
    Market = 1,
    Limit = 2,
};

// OrderStatus: OMS 状态机。
// New -> Accepted -> Cancelled（本阶段无成交回报）
// New -> Rejected
// Accepted -> Cancelled
// 其他流转均非法。
enum class OrderStatus : uint8_t {
    New = 0,
    Accepted = 1,
    Rejected = 2,
    Cancelled = 3,
};

// RejectReason: 风控拒绝原因。
enum class RejectReason : uint8_t {
    None = 0,
    MaxSingleOrderNotional = 1,
    MaxPositionRatio = 2,
    DailyLossStop = 3,
    PriceUnavailable = 4,
};

// OrderRequest: 外部下单请求（后续可由 HTTP/Qt 调用生成）。
struct OrderRequest {
    std::string symbol;
    OrderSide side{OrderSide::Buy};
    OrderType type{OrderType::Market};
    double qty{0.0};
    double limit_price{0.0}; // type=Limit 时使用
};

// OrderRecord: OMS 内部订单记录。
struct OrderRecord {
    uint64_t order_id{0};
    std::string symbol;
    OrderSide side{OrderSide::Buy};
    OrderType type{OrderType::Market};
    double qty{0.0};
    double ref_price{0.0};
    OrderStatus status{OrderStatus::New};
    RejectReason reject_reason{RejectReason::None};
    uint64_t create_ts_ms{0};
};

// RiskConfig: 风控参数（按你要求默认值）。
struct RiskConfig {
    double max_single_order_notional_usd{10.0};
    double max_position_ratio{0.80};
    double daily_loss_stop_ratio{0.05};
};

// AccountSnapshot: 风控检查所需账户快照。
struct AccountSnapshot {
    double equity_usd{1000.0};
    double position_notional_usd{0.0};
    double day_loss_usd{0.0};
};

} // namespace fq::core
