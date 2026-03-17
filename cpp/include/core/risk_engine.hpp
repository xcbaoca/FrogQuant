#pragma once

#include "core/order_types.hpp"

namespace fq::core {

// RiskEngine:
// 下单前风控检查器，返回 RejectReason。
class RiskEngine {
public:
    explicit RiskEngine(RiskConfig cfg);

    RejectReason check_pre_trade(const OrderRecord& order,
                                 const AccountSnapshot& account) const;

    void set_config(const RiskConfig& cfg) { cfg_ = cfg; }
    const RiskConfig& config() const { return cfg_; }

private:
    RiskConfig cfg_;
};

} // namespace fq::core
