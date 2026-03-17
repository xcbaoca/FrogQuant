#pragma once

#include "core/order_types.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace fq::strategy {

// LongGridConfig: 动态单向做多网格参数。
// - dynamic_enabled=true: 网格会以当前价格为锚点动态重建
// - recenter_threshold_ratio: 偏离锚点达到该比例时触发重建
// - band_ratio: 重建时网格总宽度（相对锚点）
struct LongGridConfig {
    int levels{8};
    double order_qty{0.001};
    double take_profit_ratio{0.002};

    bool dynamic_enabled{true};
    double recenter_threshold_ratio{0.004}; // 0.4%
    double band_ratio{0.06};                // ±3%
};

struct StrategyOrderIntent {
    fq::core::OrderSide side{fq::core::OrderSide::Buy};
    fq::core::OrderType type{fq::core::OrderType::Limit};
    double qty{0.0};
    double price{0.0};
    uint64_t grid_id{0};
};

enum class GridLevelState : uint8_t {
    Idle = 0,
    BuyPlaced = 1,
    BuyFilled = 2,
    SellPlaced = 3,
};

struct GridLevel {
    uint64_t id{0};
    double buy_price{0.0};
    double sell_price{0.0};
    GridLevelState state{GridLevelState::Idle};
};

// GridRuntimeView: 给未来 Qt 客户端读取的只读快照。
struct GridRuntimeView {
    LongGridConfig config;
    double anchor_price{0.0};
    uint64_t rebuild_count{0};
    std::vector<GridLevel> levels;
};

class LongGridStrategy {
public:
    explicit LongGridStrategy(LongGridConfig cfg);

    std::vector<StrategyOrderIntent> on_price_tick(double price);
    std::optional<StrategyOrderIntent> on_buy_filled(uint64_t grid_id);
    void on_sell_filled(uint64_t grid_id);

    // update_config:
    // 未来可由 Qt 控制面板调用。当前实现：更新参数并立即按最近锚点重建网格。
    void update_config(const LongGridConfig& cfg);

    GridRuntimeView runtime_view() const;
    const std::vector<GridLevel>& levels() const { return levels_; }

private:
    static double round2(double v);
    void sanitize_config();
    void rebuild_grid(double anchor_price);

    LongGridConfig cfg_;
    std::vector<GridLevel> levels_;
    double anchor_price_{0.0};
    uint64_t rebuild_count_{0};
};

} // namespace fq::strategy
