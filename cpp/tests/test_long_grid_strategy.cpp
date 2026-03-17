#include "strategy/long_grid_strategy.hpp"

#include <cstdlib>
#include <iostream>

static void expect(bool cond, const char* msg) {
    if (!cond) {
        std::cerr << "[FAIL] " << msg << "\n";
        std::exit(1);
    }
}

int main() {
    fq::strategy::LongGridConfig cfg;
    cfg.levels = 6;
    cfg.order_qty = 1.0;
    cfg.take_profit_ratio = 0.01;
    cfg.dynamic_enabled = true;
    cfg.recenter_threshold_ratio = 0.01;
    cfg.band_ratio = 0.10;

    fq::strategy::LongGridStrategy st(cfg);

    // 首个 tick 初始化网格
    auto intents0 = st.on_price_tick(100.0);
    (void)intents0;
    auto view0 = st.runtime_view();
    expect(view0.anchor_price > 0.0, "anchor should be initialized");
    expect(view0.levels.size() == 6, "levels size should match config");

    // 价格下探，触发一层买单
    auto intents = st.on_price_tick(96.0);
    expect(!intents.empty(), "should emit buy intent");
    expect(intents[0].side == fq::core::OrderSide::Buy, "first intent should be buy");

    const auto gid = intents[0].grid_id;

    // 买单成交，应该生成对应卖单意图
    auto sell = st.on_buy_filled(gid);
    expect(sell.has_value(), "buy filled should produce sell intent");
    expect(sell->side == fq::core::OrderSide::Sell, "second intent should be sell");

    // 卖单成交后，网格层回到 Idle
    st.on_sell_filled(gid);

    // 价格大幅偏离，触发动态重建
    auto before = st.runtime_view().rebuild_count;
    st.on_price_tick(110.0);
    auto after = st.runtime_view().rebuild_count;
    expect(after > before, "dynamic grid should rebuild after drift threshold");

    // 模拟 Qt 参数更新
    auto new_cfg = cfg;
    new_cfg.levels = 10;
    st.update_config(new_cfg);
    expect(st.runtime_view().levels.size() == 10, "levels should be updated by config update");

    std::cout << "[PASS] frogquant_strategy_tests\n";
    return 0;
}
