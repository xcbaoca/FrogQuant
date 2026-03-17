#include "app/control_api_router.hpp"

#include <algorithm>
#include <optional>
#include <sstream>

namespace fq::app {

control::LocalHttpServer::RequestHandler make_control_api_handler(ControlApiContext ctx) {
    return [ctx](const std::string& method,
                 const std::string& target,
                 const std::string& body,
                 unsigned& status,
                 std::string& resp) mutable {
        if (method == "GET" && target == "/health") {
            const uint64_t now = now_ms();
            const uint64_t last_tick = ctx.last_tick_ms.load();
            const bool has_tick = (last_tick > 0);
            const bool stale = has_tick && ((now - last_tick) > static_cast<uint64_t>(ctx.cfg.health_unhealthy_after_no_tick_sec) * 1000ULL);
            const bool healthy = !stale;

            status = healthy ? 200 : 503;
            resp = std::string{"{\"ok\":"} + (healthy ? "true" : "false") +
                   ",\"api_version\":\"v1.2\",\"core\":\"running\",\"dry_run\":" + (ctx.exec_cfg.dry_run ? "true" : "false") +
                   ",\"uptime_sec\":" + std::to_string((now - ctx.start_ms) / 1000ULL) +
                   ",\"tick_count\":" + std::to_string(ctx.tick_count.load()) +
                   ",\"last_tick_ms\":" + std::to_string(last_tick) +
                   ",\"strategy_enabled\":" + (ctx.strategy_enabled.load() ? "true" : "false") +
                   ",\"stale_tick\":" + (stale ? "true" : "false") + "}";
            return true;
        }

        if (method == "GET" && target == "/strategy/grid/runtime") {
            std::scoped_lock lk(ctx.grid_mu);
            status = 200;
            resp = runtime_to_json(ctx.grid.runtime_view(), ctx.strategy_enabled.load());
            return true;
        }

        if (method == "GET" && target == "/config/effective") {
            status = 200;
            resp = effective_config_json(ctx.cfg);
            return true;
        }

        if (method == "GET" && target == "/credentials/status") {
            std::string k, s;
            read_credentials_file(ctx.cfg.credentials_path, k, s);
            const bool kset = !ctx.exec_cfg.api_key.empty() || !k.empty();
            const bool sset = !ctx.exec_cfg.secret_key.empty() || !s.empty();
            status = 200;
            resp = std::string{"{\"ok\":true,\"api_key_set\":"} + (kset ? "true" : "false") +
                   ",\"secret_set\":" + (sset ? "true" : "false") +
                   ",\"path\":\"" + ctx.cfg.credentials_path + "\"}";
            return true;
        }

        if (method == "POST" && target == "/credentials/update") {
            const auto k = json_get_string(body, "api_key").value_or("");
            const auto s = json_get_string(body, "secret_key").value_or("");
            if (k.empty() || s.empty()) {
                status = 400;
                resp = R"({"ok":false,"code":"E_BAD_REQUEST","error":"api_key and secret_key required"})";
                return true;
            }
            if (!write_credentials_file(ctx.cfg.credentials_path, k, s)) {
                status = 500;
                resp = R"({"ok":false,"code":"E_WRITE_FAILED","error":"cannot write credentials file"})";
                return true;
            }
            ctx.push_event("credentials_updated(restart_required)");
            status = 200;
            resp = R"({"ok":true,"code":"OK","restart_required":true})";
            return true;
        }

        if (method == "POST" && target == "/strategy/grid/enable") {
            auto en = json_get_bool(body, "enabled");
            if (!en.has_value()) {
                status = 400;
                resp = R"({"ok":false,"code":"E_BAD_REQUEST","error":"enabled(boolean) required"})";
                return true;
            }
            ctx.strategy_enabled.store(*en);
            save_runtime_state(ctx.cfg.runtime_state_path, ctx.strategy_enabled.load(), ctx.risk_cfg);
            ctx.push_event(std::string("strategy_enable=") + (*en ? "true" : "false"));
            status = 200;
            resp = std::string{"{\"ok\":true,\"code\":\"OK\",\"enabled\":"} + (*en ? "true" : "false") + "}";
            return true;
        }

        if (method == "POST" && target == "/strategy/grid/config") {
            std::scoped_lock lk(ctx.grid_mu);
            auto g = ctx.grid.runtime_view().config;
            if (auto v = json_get_int(body, "levels")) g.levels = *v;
            if (auto v = json_get_number(body, "order_qty")) g.order_qty = *v;
            if (auto v = json_get_number(body, "take_profit_ratio")) g.take_profit_ratio = *v;
            if (auto v = json_get_bool(body, "dynamic_enabled")) g.dynamic_enabled = *v;
            if (auto v = json_get_number(body, "recenter_threshold_ratio")) g.recenter_threshold_ratio = *v;
            if (auto v = json_get_number(body, "band_ratio")) g.band_ratio = *v;

            if (g.levels < 2 || g.levels > 200) { status = 400; resp = R"({"ok":false,"code":"E_CFG_LEVELS_RANGE","error":"levels must be in [2,200]"})"; return true; }
            if (g.order_qty <= 0.0 || g.order_qty > 1000.0) { status = 400; resp = R"({"ok":false,"code":"E_CFG_QTY_RANGE","error":"order_qty must be in (0,1000]"})"; return true; }

            ctx.grid.update_config(g);
            ctx.push_event("grid_config_updated");
            status = 200;
            resp = R"({"ok":true,"code":"OK"})";
            return true;
        }

        if (method == "GET" && target == "/risk/config") {
            status = 200;
            resp = std::string{"{\"ok\":true,\"max_single_order_notional_usd\":"} + std::to_string(ctx.risk_cfg.max_single_order_notional_usd) +
                   ",\"max_position_ratio\":" + std::to_string(ctx.risk_cfg.max_position_ratio) +
                   ",\"daily_loss_stop_ratio\":" + std::to_string(ctx.risk_cfg.daily_loss_stop_ratio) + "}";
            return true;
        }

        if (method == "POST" && target == "/risk/config") {
            auto n = json_get_number(body, "max_single_order_notional_usd");
            auto p = json_get_number(body, "max_position_ratio");
            auto d = json_get_number(body, "daily_loss_stop_ratio");

            core::RiskConfig next = ctx.risk_cfg;
            if (n.has_value()) next.max_single_order_notional_usd = *n;
            if (p.has_value()) next.max_position_ratio = *p;
            if (d.has_value()) next.daily_loss_stop_ratio = *d;

            if (next.max_single_order_notional_usd <= 0.0 || next.max_position_ratio <= 0.0 || next.max_position_ratio > 1.0 ||
                next.daily_loss_stop_ratio <= 0.0 || next.daily_loss_stop_ratio > 1.0) {
                status = 400;
                resp = R"({"ok":false,"code":"E_RISK_RANGE","error":"invalid risk ranges"})";
                return true;
            }

            ctx.risk_cfg = next;
            ctx.core.update_risk_config(next);
            save_runtime_state(ctx.cfg.runtime_state_path, ctx.strategy_enabled.load(), ctx.risk_cfg);
            ctx.push_event("risk_config_updated");
            status = 200;
            resp = R"({"ok":true,"code":"OK"})";
            return true;
        }

        if (method == "POST" && target == "/trade/manual/order") {
            core::OrderRequest req;
            req.symbol = json_get_string(body, "symbol").value_or(ctx.cfg.symbol);
            req.side = (json_get_string(body, "side").value_or("BUY") == "SELL") ? core::OrderSide::Sell : core::OrderSide::Buy;
            req.type = (json_get_string(body, "type").value_or("MARKET") == "LIMIT") ? core::OrderType::Limit : core::OrderType::Market;
            req.qty = json_get_number(body, "qty").value_or(0.0);
            req.limit_price = json_get_number(body, "price").value_or(0.0);
            if (req.qty <= 0.0) { status = 400; resp = R"({"ok":false,"code":"E_BAD_QTY","error":"qty must be > 0"})"; return true; }

            const auto id = ctx.core.submit_manual_order(req);
            auto ord = ctx.core.get_order(id);
            if (!ord.has_value()) { status = 500; resp = R"({"ok":false,"code":"E_INTERNAL","error":"order missing after submit"})"; return true; }

            {
                std::scoped_lock lk(ctx.state_mu);
                ctx.orders.push_back(ApiOrderItem{id, status_to_str(ord->status), reject_to_str(ord->reject_reason), now_ms()});
                if (ctx.orders.size() > ctx.max_orders) {
                    ctx.orders.erase(ctx.orders.begin(), ctx.orders.begin() + std::min(ctx.order_trim_batch, ctx.orders.size()));
                }
            }
            ctx.push_event(std::string("manual_order id=") + std::to_string(id));

            if (ord->status == core::OrderStatus::Accepted) {
                ctx.exec.place_order(req.symbol, req.side, req.type, req.qty,
                                     req.type == core::OrderType::Limit ? std::optional<double>(req.limit_price) : std::nullopt,
                                     std::string("qt_") + std::to_string(id));
            }

            status = 200;
            resp = std::string{"{\"ok\":true,\"code\":\"OK\",\"internal_order_id\":"} + std::to_string(id) +
                   ",\"status\":\"" + status_to_str(ord->status) + "\"" +
                   ",\"reject_reason\":\"" + reject_to_str(ord->reject_reason) + "\"}";
            return true;
        }

        if (method == "POST" && target == "/trade/manual/cancel") {
            auto internal_id = json_get_number(body, "internal_order_id");
            if (!internal_id.has_value()) { status = 400; resp = R"({"ok":false,"code":"E_BAD_REQUEST","error":"internal_order_id required"})"; return true; }
            const bool ok = ctx.core.cancel_order(static_cast<uint64_t>(*internal_id));
            ctx.push_event(std::string("manual_cancel id=") + std::to_string(static_cast<uint64_t>(*internal_id)));
            status = ok ? 200 : 400;
            resp = ok ? R"({"ok":true,"code":"OK","status":"Cancelled"})" : R"({"ok":false,"code":"E_CANCEL_FAILED","error":"cancel failed"})";
            return true;
        }

        if (method == "GET" && target.rfind("/orders", 0) == 0) {
            std::scoped_lock lk(ctx.state_mu);
            std::ostringstream os;
            os << "{\"ok\":true,\"items\":[";
            for (size_t i = 0; i < ctx.orders.size(); ++i) {
                if (i) os << ",";
                const auto& o = ctx.orders[i];
                os << "{\"internal_id\":" << o.internal_id << ",\"status\":\"" << o.status
                   << "\",\"reject_reason\":\"" << o.reject_reason << "\",\"ts_ms\":" << o.ts_ms << "}";
            }
            os << "]}";
            status = 200;
            resp = os.str();
            return true;
        }

        if (method == "GET" && target.rfind("/events", 0) == 0) {
            std::scoped_lock lk(ctx.state_mu);
            std::ostringstream os;
            os << "{\"ok\":true,\"items\":[";
            for (size_t i = 0; i < ctx.events.size(); ++i) {
                if (i) os << ",";
                os << "\"" << ctx.events[i] << "\"";
            }
            os << "]}";
            status = 200;
            resp = os.str();
            return true;
        }

        return false;
    };
}

} // namespace fq::app
