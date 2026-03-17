#pragma once

#include "core/order_types.hpp"

#include <optional>
#include <unordered_map>

namespace fq::core {

// OMS: 订单状态管理 + 幂等保障。
class OMS {
public:
    bool submit(const OrderRecord& order);
    bool accept(uint64_t order_id);
    bool reject(uint64_t order_id, RejectReason reason);
    bool cancel(uint64_t order_id);

    std::optional<OrderRecord> get(uint64_t order_id) const;

private:
    std::unordered_map<uint64_t, OrderRecord> orders_;
};

} // namespace fq::core
