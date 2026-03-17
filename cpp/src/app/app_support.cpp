#include "app/app_support.hpp"

#include <cctype>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <filesystem>

namespace fq::app {

uint64_t now_ms() {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

const char* env_or_empty(const char* key) {
    const char* v = std::getenv(key);
    return v ? v : "";
}

bool env_is_true(const char* key, bool default_value) {
    const char* v = std::getenv(key);
    if (!v) return default_value;
    std::string s(v);
    return (s == "1" || s == "true" || s == "TRUE" || s == "yes" || s == "YES");
}

int env_or_int(const char* key, int default_value) {
    const char* v = std::getenv(key);
    if (!v) return default_value;
    try { return std::stoi(v); } catch (...) { return default_value; }
}

namespace {
std::string trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

std::unordered_map<std::string, std::string> parse_simple_yaml(const std::string& path) {
    std::unordered_map<std::string, std::string> kv;
    std::ifstream in(path);
    if (!in.good()) return kv;

    std::string line;
    std::string section;
    while (std::getline(in, line)) {
        auto t = trim(line);
        if (t.empty() || t[0] == '#') continue;
        const auto pos = t.find(':');
        if (pos == std::string::npos) continue;

        auto key = trim(t.substr(0, pos));
        auto val = trim(t.substr(pos + 1));
        if (val.empty()) {
            section = key;
            continue;
        }
        if (!section.empty() && line.rfind("  ", 0) == 0) key = section + "." + key;
        if (!val.empty() && (val.front() == '"' || val.front() == '\'')) val = val.substr(1, val.size() - 2);
        kv[key] = val;
    }
    return kv;
}
} // namespace

void load_config(AppConfig& cfg, const std::string& path) {
    const auto kv = parse_simple_yaml(path);
    if (kv.empty()) return;

    auto get_s = [&](const std::string& k, std::string& out) { if (kv.contains(k)) out = kv.at(k); };
    auto get_i = [&](const std::string& k, int& out) { if (kv.contains(k)) try { out = std::stoi(kv.at(k)); } catch (...) {} };
    auto get_sz = [&](const std::string& k, size_t& out) { if (kv.contains(k)) try { out = static_cast<size_t>(std::stoull(kv.at(k))); } catch (...) {} };
    auto get_d = [&](const std::string& k, double& out) { if (kv.contains(k)) try { out = std::stod(kv.at(k)); } catch (...) {} };
    auto get_b = [&](const std::string& k, bool& out) {
        if (!kv.contains(k)) return;
        const auto& v = kv.at(k);
        out = (v == "1" || v == "true" || v == "TRUE" || v == "yes" || v == "YES");
    };

    get_s("engine.symbol", cfg.symbol);
    get_i("engine.market_poll_ms", cfg.market_poll_ms);
    get_i("engine.control_port", cfg.control_port);

    get_d("risk.max_single_order_notional_usd", cfg.risk_max_single_order_notional_usd);
    get_d("risk.max_position_ratio", cfg.risk_max_position_ratio);
    get_d("risk.daily_loss_stop_ratio", cfg.risk_daily_loss_stop_ratio);

    get_b("execution.dry_run", cfg.exec_dry_run);
    get_s("execution.base_url", cfg.exec_base_url);

    get_s("logging.path", cfg.log_path);
    get_s("execution.credentials_path", cfg.credentials_path);

    get_i("runtime.health_unhealthy_after_no_tick_sec", cfg.health_unhealthy_after_no_tick_sec);
    get_i("runtime.run_seconds", cfg.run_seconds);
    get_b("runtime.strategy_start_enabled", cfg.strategy_start_enabled);
    get_s("runtime.state_path", cfg.runtime_state_path);
    get_sz("runtime.max_events_cache", cfg.max_events_cache);
    get_sz("runtime.max_orders_cache", cfg.max_orders_cache);
    get_sz("runtime.order_trim_batch", cfg.order_trim_batch);

    get_i("strategy.levels", cfg.grid_default.levels);
    get_d("strategy.order_qty", cfg.grid_default.order_qty);
    get_d("strategy.take_profit_ratio", cfg.grid_default.take_profit_ratio);
    get_b("strategy.dynamic_enabled", cfg.grid_default.dynamic_enabled);
    get_d("strategy.recenter_threshold_ratio", cfg.grid_default.recenter_threshold_ratio);
    get_d("strategy.band_ratio", cfg.grid_default.band_ratio);
}

bool path_writable_probe(const std::string& file_path) {
    std::ofstream out(file_path, std::ios::app);
    return out.good();
}

std::optional<double> json_get_number(const std::string& body, const std::string& key) {
    const std::string token = "\"" + key + "\"";
    auto p = body.find(token);
    if (p == std::string::npos) return std::nullopt;
    p = body.find(':', p);
    if (p == std::string::npos) return std::nullopt;
    ++p;
    while (p < body.size() && (body[p] == ' ' || body[p] == '\n')) ++p;
    size_t end = p;
    while (end < body.size() && (std::isdigit(static_cast<unsigned char>(body[end])) || body[end] == '.' || body[end] == '-')) ++end;
    if (end == p) return std::nullopt;
    try { return std::stod(body.substr(p, end - p)); } catch (...) { return std::nullopt; }
}

std::optional<int> json_get_int(const std::string& body, const std::string& key) {
    auto v = json_get_number(body, key);
    if (!v.has_value()) return std::nullopt;
    return static_cast<int>(*v);
}

std::optional<bool> json_get_bool(const std::string& body, const std::string& key) {
    const std::string token = "\"" + key + "\"";
    auto p = body.find(token);
    if (p == std::string::npos) return std::nullopt;
    p = body.find(':', p);
    if (p == std::string::npos) return std::nullopt;
    ++p;
    while (p < body.size() && (body[p] == ' ' || body[p] == '\n')) ++p;
    if (body.compare(p, 4, "true") == 0) return true;
    if (body.compare(p, 5, "false") == 0) return false;
    return std::nullopt;
}

std::optional<std::string> json_get_string(const std::string& body, const std::string& key) {
    const std::string token = "\"" + key + "\"";
    auto p = body.find(token);
    if (p == std::string::npos) return std::nullopt;
    p = body.find(':', p);
    if (p == std::string::npos) return std::nullopt;
    p = body.find('"', p);
    if (p == std::string::npos) return std::nullopt;
    ++p;
    auto e = body.find('"', p);
    if (e == std::string::npos) return std::nullopt;
    return body.substr(p, e - p);
}

const char* status_to_str(fq::core::OrderStatus s) {
    switch (s) {
        case fq::core::OrderStatus::New: return "New";
        case fq::core::OrderStatus::Accepted: return "Accepted";
        case fq::core::OrderStatus::Rejected: return "Rejected";
        case fq::core::OrderStatus::Cancelled: return "Cancelled";
        default: return "Unknown";
    }
}

const char* reject_to_str(fq::core::RejectReason r) {
    switch (r) {
        case fq::core::RejectReason::None: return "None";
        case fq::core::RejectReason::MaxSingleOrderNotional: return "MaxSingleOrderNotional";
        case fq::core::RejectReason::MaxPositionRatio: return "MaxPositionRatio";
        case fq::core::RejectReason::DailyLossStop: return "DailyLossStop";
        case fq::core::RejectReason::PriceUnavailable: return "PriceUnavailable";
        default: return "Unknown";
    }
}

std::string effective_config_json(const AppConfig& cfg) {
    std::ostringstream os;
    os << "{";
    os << "\"engine\":{";
    os << "\"symbol\":\"" << cfg.symbol << "\",";
    os << "\"market_poll_ms\":" << cfg.market_poll_ms << ",";
    os << "\"control_port\":" << cfg.control_port << "},";

    os << "\"risk\":{";
    os << "\"max_single_order_notional_usd\":" << cfg.risk_max_single_order_notional_usd << ",";
    os << "\"max_position_ratio\":" << cfg.risk_max_position_ratio << ",";
    os << "\"daily_loss_stop_ratio\":" << cfg.risk_daily_loss_stop_ratio << "},";

    os << "\"execution\":{";
    os << "\"dry_run\":" << (cfg.exec_dry_run ? "true" : "false") << ",";
    os << "\"base_url\":\"" << cfg.exec_base_url << "\",";
    os << "\"credentials_path\":\"" << cfg.credentials_path << "\"},";

    os << "\"logging\":{";
    os << "\"path\":\"" << cfg.log_path << "\"},";

    os << "\"runtime\":{";
    os << "\"health_unhealthy_after_no_tick_sec\":" << cfg.health_unhealthy_after_no_tick_sec << ",";
    os << "\"run_seconds\":" << cfg.run_seconds << ",";
    os << "\"strategy_start_enabled\":" << (cfg.strategy_start_enabled ? "true" : "false") << ",";
    os << "\"state_path\":\"" << cfg.runtime_state_path << "\",";
    os << "\"max_events_cache\":" << cfg.max_events_cache << ",";
    os << "\"max_orders_cache\":" << cfg.max_orders_cache << ",";
    os << "\"order_trim_batch\":" << cfg.order_trim_batch << "},";

    os << "\"strategy\":{";
    os << "\"levels\":" << cfg.grid_default.levels << ",";
    os << "\"order_qty\":" << cfg.grid_default.order_qty << ",";
    os << "\"take_profit_ratio\":" << cfg.grid_default.take_profit_ratio << ",";
    os << "\"dynamic_enabled\":" << (cfg.grid_default.dynamic_enabled ? "true" : "false") << ",";
    os << "\"recenter_threshold_ratio\":" << cfg.grid_default.recenter_threshold_ratio << ",";
    os << "\"band_ratio\":" << cfg.grid_default.band_ratio << "}";
    os << "}";
    return os.str();
}

std::string runtime_to_json(const fq::strategy::GridRuntimeView& view, bool enabled) {
    std::ostringstream os;
    os << "{";
    os << "\"api_version\":\"v1.2\",";
    os << "\"strategy_enabled\":" << (enabled ? "true" : "false") << ",";
    os << "\"anchor_price\":" << view.anchor_price << ",";
    os << "\"rebuild_count\":" << view.rebuild_count << ",";
    os << "\"config\":{";
    os << "\"levels\":" << view.config.levels << ",";
    os << "\"order_qty\":" << view.config.order_qty << ",";
    os << "\"take_profit_ratio\":" << view.config.take_profit_ratio << ",";
    os << "\"dynamic_enabled\":" << (view.config.dynamic_enabled ? "true" : "false") << ",";
    os << "\"recenter_threshold_ratio\":" << view.config.recenter_threshold_ratio << ",";
    os << "\"band_ratio\":" << view.config.band_ratio;
    os << "},\"levels\":[";
    for (size_t i = 0; i < view.levels.size(); ++i) {
        if (i) os << ",";
        const auto& lv = view.levels[i];
        os << "{\"id\":" << lv.id << ",\"buy_price\":" << lv.buy_price
           << ",\"sell_price\":" << lv.sell_price << ",\"state\":" << static_cast<int>(lv.state) << "}";
    }
    os << "]}";
    return os.str();
}

bool read_credentials_file(const std::string& path, std::string& api_key, std::string& secret_key) {
    std::ifstream in(path);
    if (!in.good()) return false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("BINANCE_TESTNET_API_KEY=", 0) == 0) api_key = line.substr(24);
        if (line.rfind("BINANCE_TESTNET_SECRET_KEY=", 0) == 0) secret_key = line.substr(27);
    }
    return true;
}

bool write_credentials_file(const std::string& path, const std::string& api_key, const std::string& secret_key) {
    try {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    } catch (...) {}
    std::ofstream out(path, std::ios::trunc);
    if (!out.good()) return false;
    out << "BINANCE_TESTNET_API_KEY=" << api_key << "\n";
    out << "BINANCE_TESTNET_SECRET_KEY=" << secret_key << "\n";
    out.close();
    return true;
}

bool load_runtime_state(const std::string& path, bool& strategy_enabled, core::RiskConfig& risk_cfg) {
    std::ifstream in(path);
    if (!in.good()) return false;
    std::stringstream buf;
    buf << in.rdbuf();
    const auto s = buf.str();

    auto find_bool = [&](const std::string& key, bool& out) {
        const std::string t = "\"" + key + "\":";
        auto p = s.find(t);
        if (p == std::string::npos) return;
        p += t.size();
        out = (s.compare(p, 4, "true") == 0);
    };
    auto find_num = [&](const std::string& key, double& out) {
        const std::string t = "\"" + key + "\":";
        auto p = s.find(t);
        if (p == std::string::npos) return;
        p += t.size();
        auto e = s.find_first_of(",}", p);
        if (e == std::string::npos) return;
        try { out = std::stod(s.substr(p, e - p)); } catch (...) {}
    };

    find_bool("strategy_enabled", strategy_enabled);
    find_num("max_single_order_notional_usd", risk_cfg.max_single_order_notional_usd);
    find_num("max_position_ratio", risk_cfg.max_position_ratio);
    find_num("daily_loss_stop_ratio", risk_cfg.daily_loss_stop_ratio);
    return true;
}

bool save_runtime_state(const std::string& path, bool strategy_enabled, const core::RiskConfig& risk_cfg) {
    try {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    } catch (...) {}
    std::ofstream out(path, std::ios::trunc);
    if (!out.good()) return false;
    out << "{"
        << "\"strategy_enabled\":" << (strategy_enabled ? "true" : "false") << ","
        << "\"risk\":{"
        << "\"max_single_order_notional_usd\":" << risk_cfg.max_single_order_notional_usd << ","
        << "\"max_position_ratio\":" << risk_cfg.max_position_ratio << ","
        << "\"daily_loss_stop_ratio\":" << risk_cfg.daily_loss_stop_ratio
        << "}}";
    return true;
}

} // namespace fq::app
