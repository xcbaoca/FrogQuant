#include "core/risk_engine.hpp"

#include <cmath>

namespace fq::core {

RiskEngine::RiskEngine(RiskConfig cfg) : cfg_(cfg) {}

RejectReason RiskEngine::check_pre_trade(const OrderRecord& order,
                                         const AccountSnapshot& account) const {
    const double notional = std::fabs(order.qty * order.ref_price);
    if (notional > cfg_.max_single_order_notional_usd) {
        return RejectReason::MaxSingleOrderNotional;
    }

    const double max_position_allowed = account.equity_usd * cfg_.max_position_ratio;
    if (account.position_notional_usd + notional > max_position_allowed) {
        return RejectReason::MaxPositionRatio;
    }

    const double day_loss_ratio = (account.equity_usd > 0.0)
                                      ? (account.day_loss_usd / account.equity_usd)
                                      : 1.0;
    if (day_loss_ratio >= cfg_.daily_loss_stop_ratio) {
        return RejectReason::DailyLossStop;
    }

    return RejectReason::None;
}

} // namespace fq::core
