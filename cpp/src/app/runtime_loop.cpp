#include "app/runtime_loop.hpp"

#include <iostream>

namespace fq::app {

std::function<void(const market::MarketTick&)> make_tick_handler(RuntimeLoopContext ctx) {
    return [ctx](const market::MarketTick& t) mutable {
        ++ctx.tick_count;
        ctx.last_tick_ms.store(now_ms());
        ctx.core.on_tick(t);
        if (!ctx.strategy_enabled.load()) return;

        std::scoped_lock lk(ctx.grid_mu);
        const auto intents = ctx.grid.on_price_tick(t.price);
        for (const auto& it : intents) {
            ctx.push_event(std::string("strategy_intent grid_id=") + std::to_string(it.grid_id));
            std::cout << "[strategy] intent side=" << (it.side == core::OrderSide::Buy ? "BUY" : "SELL")
                      << " type=" << (it.type == core::OrderType::Limit ? "LIMIT" : "MARKET")
                      << " qty=" << it.qty << " px=" << it.price << " grid_id=" << it.grid_id << "\n";
        }
    };
}

} // namespace fq::app
