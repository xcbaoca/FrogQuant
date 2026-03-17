#include "core/trading_core.hpp"

#include <chrono>
#include <cstring>

namespace fq::core {
namespace {

const char* rr_to_str(RejectReason rr) {
    switch (rr) {
        case RejectReason::None: return "None";
        case RejectReason::MaxSingleOrderNotional: return "MaxSingleOrderNotional";
        case RejectReason::MaxPositionRatio: return "MaxPositionRatio";
        case RejectReason::DailyLossStop: return "DailyLossStop";
        case RejectReason::PriceUnavailable: return "PriceUnavailable";
        default: return "Unknown";
    }
}

logger::LogEvent make_log(logger::LogLevel lv,
                          const char* module,
                          const char* event,
                          const char* msg,
                          double v1 = 0.0,
                          double v2 = 0.0) {
    logger::LogEvent ev{};
    ev.ts_ms = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
    ev.level = lv;
    std::strncpy(ev.module, module, sizeof(ev.module) - 1);
    std::strncpy(ev.event, event, sizeof(ev.event) - 1);
    std::strncpy(ev.message, msg, sizeof(ev.message) - 1);
    ev.value1 = v1;
    ev.value2 = v2;
    return ev;
}

} // namespace

TradingCore::TradingCore(RiskConfig cfg, logger::AsyncJsonlLogger& logger)
    : logger_(logger), risk_(cfg) {}

void TradingCore::on_tick(const market::MarketTick& tick) {
    last_price_ = tick.price;
    logger_.try_log(make_log(logger::LogLevel::Debug,
                             "core",
                             "tick",
                             "last price updated",
                             tick.price,
                             static_cast<double>(tick.source == market::TickSource::WebSocket ? 1 : 2)));
}

uint64_t TradingCore::submit_manual_order(const OrderRequest& req) {
    const uint64_t id = ++seq_;

    OrderRecord order;
    order.order_id = id;
    order.symbol = req.symbol;
    order.side = req.side;
    order.type = req.type;
    order.qty = req.qty;
    order.ref_price = (req.type == OrderType::Limit && req.limit_price > 0.0) ? req.limit_price : last_price_;
    order.status = OrderStatus::New;
    order.create_ts_ms = now_ms();

    // 没有可用价格时直接拒绝。
    if (order.ref_price <= 0.0) {
        oms_.submit(order);
        oms_.reject(id, RejectReason::PriceUnavailable);
        logger_.try_log(make_log(logger::LogLevel::Warn,
                                 "core",
                                 "order_reject",
                                 "price unavailable",
                                 static_cast<double>(id),
                                 0.0));
        return id;
    }

    // 先提交 OMS（保证可追踪）
    if (!oms_.submit(order)) {
        logger_.try_log(make_log(logger::LogLevel::Error,
                                 "core",
                                 "order_submit",
                                 "duplicate order id",
                                 static_cast<double>(id),
                                 0.0));
        return id;
    }

    // 风控检查
    const auto rr = risk_.check_pre_trade(order, account_);
    if (rr == RejectReason::None) {
        oms_.accept(id);

        // 演示版：受理后更新持仓名义金额（仅买单累加；卖单可扩展）
        const double notional = order.qty * order.ref_price;
        if (order.side == OrderSide::Buy) {
            account_.position_notional_usd += notional;
        }

        logger_.try_log(make_log(logger::LogLevel::Info,
                                 "core",
                                 "order_accept",
                                 "order accepted",
                                 static_cast<double>(id),
                                 notional));
    } else {
        oms_.reject(id, rr);
        logger_.try_log(make_log(logger::LogLevel::Warn,
                                 "core",
                                 "order_reject",
                                 rr_to_str(rr),
                                 static_cast<double>(id),
                                 order.qty * order.ref_price));
    }

    return id;
}

bool TradingCore::cancel_order(uint64_t order_id) {
    const bool ok = oms_.cancel(order_id);
    logger_.try_log(make_log(ok ? logger::LogLevel::Info : logger::LogLevel::Warn,
                             "core",
                             "order_cancel",
                             ok ? "cancelled" : "cancel_failed",
                             static_cast<double>(order_id),
                             0.0));
    return ok;
}

std::optional<OrderRecord> TradingCore::get_order(uint64_t order_id) const {
    return oms_.get(order_id);
}

uint64_t TradingCore::now_ms() {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

} // namespace fq::core
