#include "strategy/long_grid_strategy.hpp"

#include <algorithm>
#include <cmath>

namespace fq::strategy {

LongGridStrategy::LongGridStrategy(LongGridConfig cfg) : cfg_(cfg) {
    sanitize_config();
}

std::vector<StrategyOrderIntent> LongGridStrategy::on_price_tick(double price) {
    std::vector<StrategyOrderIntent> out;
    if (price <= 0.0) return out;

    // 首次 tick 用作锚点初始化。
    if (anchor_price_ <= 0.0) {
        rebuild_grid(price);
    }

    // 动态网格：价格偏离锚点超过阈值时，重建整套网格。
    if (cfg_.dynamic_enabled) {
        const double drift = std::fabs(price - anchor_price_) / anchor_price_;
        if (drift >= cfg_.recenter_threshold_ratio) {
            rebuild_grid(price);
        }
    }

    // Long-only: 只在价格 <= buy_price 时考虑挂买。
    // 为避免瞬时批量发单，每个 tick 最多发 1 笔最接近当前价的买单。
    GridLevel* candidate = nullptr;
    for (auto& lv : levels_) {
        if (lv.state != GridLevelState::Idle) continue;
        if (price <= lv.buy_price) {
            if (!candidate || std::fabs(lv.buy_price - price) < std::fabs(candidate->buy_price - price)) {
                candidate = &lv;
            }
        }
    }

    if (candidate) {
        candidate->state = GridLevelState::BuyPlaced;
        out.push_back(StrategyOrderIntent{fq::core::OrderSide::Buy,
                                          fq::core::OrderType::Limit,
                                          cfg_.order_qty,
                                          candidate->buy_price,
                                          candidate->id});
    }

    return out;
}

std::optional<StrategyOrderIntent> LongGridStrategy::on_buy_filled(uint64_t grid_id) {
    auto it = std::find_if(levels_.begin(), levels_.end(), [grid_id](const GridLevel& x) { return x.id == grid_id; });
    if (it == levels_.end()) return std::nullopt;
    if (it->state != GridLevelState::BuyPlaced && it->state != GridLevelState::BuyFilled) return std::nullopt;

    it->state = GridLevelState::SellPlaced;
    return StrategyOrderIntent{fq::core::OrderSide::Sell,
                               fq::core::OrderType::Limit,
                               cfg_.order_qty,
                               it->sell_price,
                               it->id};
}

void LongGridStrategy::on_sell_filled(uint64_t grid_id) {
    auto it = std::find_if(levels_.begin(), levels_.end(), [grid_id](const GridLevel& x) { return x.id == grid_id; });
    if (it == levels_.end()) return;
    it->state = GridLevelState::Idle;
}

void LongGridStrategy::update_config(const LongGridConfig& cfg) {
    cfg_ = cfg;
    sanitize_config();
    if (anchor_price_ > 0.0) {
        rebuild_grid(anchor_price_);
    }
}

GridRuntimeView LongGridStrategy::runtime_view() const {
    return GridRuntimeView{cfg_, anchor_price_, rebuild_count_, levels_};
}

double LongGridStrategy::round2(double v) {
    return std::round(v * 100.0) / 100.0;
}

void LongGridStrategy::sanitize_config() {
    if (cfg_.levels < 2) cfg_.levels = 2;
    if (cfg_.order_qty <= 0.0) cfg_.order_qty = 0.001;
    if (cfg_.take_profit_ratio <= 0.0) cfg_.take_profit_ratio = 0.002;
    if (cfg_.recenter_threshold_ratio <= 0.0) cfg_.recenter_threshold_ratio = 0.004;
    if (cfg_.band_ratio <= 0.002) cfg_.band_ratio = 0.06;
}

void LongGridStrategy::rebuild_grid(double anchor_price) {
    anchor_price_ = anchor_price;
    ++rebuild_count_;

    const double half = anchor_price_ * (cfg_.band_ratio * 0.5);
    const double lower = anchor_price_ - half;
    const double upper = anchor_price_ + half;
    const double step = (upper - lower) / static_cast<double>(cfg_.levels - 1);

    levels_.clear();
    levels_.reserve(static_cast<size_t>(cfg_.levels));
    for (int i = 0; i < cfg_.levels; ++i) {
        const double buy_px = lower + step * static_cast<double>(i);
        const double sell_px = buy_px * (1.0 + cfg_.take_profit_ratio);
        levels_.push_back(GridLevel{static_cast<uint64_t>(i + 1), round2(buy_px), round2(sell_px), GridLevelState::Idle});
    }
}

} // namespace fq::strategy
