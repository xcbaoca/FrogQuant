#include "core/app_config.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>

namespace fq::core {
namespace {

std::string trim(std::string s) {
    auto not_space = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    return s;
}

bool to_bool(const std::string& v, bool def) {
    std::string s = v;
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (s == "true" || s == "1" || s == "yes") return true;
    if (s == "false" || s == "0" || s == "no") return false;
    return def;
}

int to_int(const std::string& v, int def) {
    try { return std::stoi(v); } catch (...) { return def; }
}

double to_double(const std::string& v, double def) {
    try { return std::stod(v); } catch (...) { return def; }
}

} // namespace

AppConfig load_app_config(const std::string& path) {
    AppConfig cfg;

    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "[config] file not found, using defaults: " << path << "\n";
        return cfg;
    }

    std::string line;
    int ln = 0;
    while (std::getline(in, line)) {
        ++ln;
        auto x = trim(line);
        if (x.empty() || x[0] == '#') continue;

        const auto pos = x.find(':');
        if (pos == std::string::npos) continue;

        std::string key = trim(x.substr(0, pos));
        std::string val = trim(x.substr(pos + 1));
        if (!val.empty() && (val.front() == '"' || val.front() == '\'')) val.erase(val.begin());
        if (!val.empty() && (val.back() == '"' || val.back() == '\'')) val.pop_back();

        if (key == "symbol") cfg.symbol = val;
        else if (key == "exchange.base_url") cfg.exchange_base_url = val;
        else if (key == "execution.dry_run") cfg.dry_run = to_bool(val, cfg.dry_run);
        else if (key == "control.bind_ip") cfg.control_bind_ip = val;
        else if (key == "control.port") cfg.control_port = to_int(val, cfg.control_port);
        else if (key == "risk.max_single_order_notional_usd") cfg.risk_max_single_order_notional_usd = to_double(val, cfg.risk_max_single_order_notional_usd);
        else if (key == "risk.max_position_ratio") cfg.risk_max_position_ratio = to_double(val, cfg.risk_max_position_ratio);
        else if (key == "risk.daily_loss_stop_ratio") cfg.risk_daily_loss_stop_ratio = to_double(val, cfg.risk_daily_loss_stop_ratio);
        else if (key == "grid.levels") cfg.grid_levels = to_int(val, cfg.grid_levels);
        else if (key == "grid.order_qty") cfg.grid_order_qty = to_double(val, cfg.grid_order_qty);
        else if (key == "grid.take_profit_ratio") cfg.grid_take_profit_ratio = to_double(val, cfg.grid_take_profit_ratio);
        else if (key == "grid.dynamic_enabled") cfg.grid_dynamic_enabled = to_bool(val, cfg.grid_dynamic_enabled);
        else if (key == "grid.recenter_threshold_ratio") cfg.grid_recenter_threshold_ratio = to_double(val, cfg.grid_recenter_threshold_ratio);
        else if (key == "grid.band_ratio") cfg.grid_band_ratio = to_double(val, cfg.grid_band_ratio);
        else if (key == "health.unhealthy_after_no_tick_sec") cfg.health_unhealthy_after_no_tick_sec = to_int(val, cfg.health_unhealthy_after_no_tick_sec);
        else if (key == "service.max_events_cache") cfg.max_events_cache = to_int(val, cfg.max_events_cache);
        else if (key == "service.max_orders_cache") cfg.max_orders_cache = to_int(val, cfg.max_orders_cache);
        else if (key == "service.order_trim_batch") cfg.order_trim_batch = to_int(val, cfg.order_trim_batch);
        else if (key == "runtime.run_seconds") cfg.run_seconds = to_int(val, cfg.run_seconds);
        else {
            std::cerr << "[config] ignore unknown key at line " << ln << ": " << key << "\n";
        }
    }

    return cfg;
}

} // namespace fq::core
