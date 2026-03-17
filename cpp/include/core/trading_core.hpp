#pragma once

#include "core/oms.hpp"
#include "core/risk_engine.hpp"
#include "logger/async_jsonl_logger.hpp"
#include "market/market_types.hpp"

#include <atomic>
#include <optional>

namespace fq::core {

// TradingCore:
// 在“执行模块”上线前，先实现 Risk + OMS 的核心流程。
// 输入：行情 tick + 手动下单请求
// 输出：订单受理/拒绝/撤销结果（并写日志）
class TradingCore {
public:
    TradingCore(RiskConfig cfg, logger::AsyncJsonlLogger& logger);

    void on_tick(const market::MarketTick& tick);

    // submit_manual_order:
    // 返回创建的 order_id（无论最终 Accepted 还是 Rejected，都会落在 OMS）。
    uint64_t submit_manual_order(const OrderRequest& req);

    bool cancel_order(uint64_t order_id);

    std::optional<OrderRecord> get_order(uint64_t order_id) const;

    void update_risk_config(const RiskConfig& cfg) { risk_.set_config(cfg); }
    const RiskConfig& risk_config() const { return risk_.config(); }

private:
    static uint64_t now_ms();

    logger::AsyncJsonlLogger& logger_;
    OMS oms_;
    RiskEngine risk_;

    std::atomic<uint64_t> seq_{0};
    double last_price_{0.0};

    // 账户快照（演示版，后续由真实账户模块替代）
    AccountSnapshot account_;
};

} // namespace fq::core
