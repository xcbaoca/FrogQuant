#include "core/oms.hpp"

namespace fq::core {

bool OMS::submit(const OrderRecord& order) {
    if (orders_.contains(order.order_id)) return false;
    orders_[order.order_id] = order;
    return true;
}

bool OMS::accept(uint64_t order_id) {
    auto it = orders_.find(order_id);
    if (it == orders_.end()) return false;
    if (it->second.status != OrderStatus::New) return false;
    it->second.status = OrderStatus::Accepted;
    return true;
}

bool OMS::reject(uint64_t order_id, RejectReason reason) {
    auto it = orders_.find(order_id);
    if (it == orders_.end()) return false;
    if (it->second.status != OrderStatus::New) return false;
    it->second.status = OrderStatus::Rejected;
    it->second.reject_reason = reason;
    return true;
}

bool OMS::cancel(uint64_t order_id) {
    auto it = orders_.find(order_id);
    if (it == orders_.end()) return false;
    if (it->second.status != OrderStatus::Accepted) return false;
    it->second.status = OrderStatus::Cancelled;
    return true;
}

std::optional<OrderRecord> OMS::get(uint64_t order_id) const {
    auto it = orders_.find(order_id);
    if (it == orders_.end()) return std::nullopt;
    return it->second;
}

} // namespace fq::core
