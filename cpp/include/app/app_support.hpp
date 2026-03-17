#pragma once

#include "core/order_types.hpp"
#include "strategy/long_grid_strategy.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace fq::app {

struct ApiOrderItem {
    uint64_t internal_id{0};
    std::string status;
    std::string reject_reason;
    uint64_t ts_ms{0};
};

struct AppConfig {
    std::string symbol{"BTCUSDT"};
    int market_poll_ms{1000};
    int control_port{8080};

    double risk_max_single_order_notional_usd{10.0};
    double risk_max_position_ratio{0.80};
    double risk_daily_loss_stop_ratio{0.05};

    bool exec_dry_run{true};
    std::string exec_base_url{"https://testnet.binancefuture.com"};

    std::string log_path{"market_logs.jsonl"};
    std::string credentials_path{"config/credentials.env"};

    int health_unhealthy_after_no_tick_sec{15};
    int run_seconds{0};
    bool strategy_start_enabled{true};

    std::string runtime_state_path{"config/runtime_state.json"};

    size_t max_events_cache{200};
    size_t max_orders_cache{5000};
    size_t order_trim_batch{1000};

    fq::strategy::LongGridConfig grid_default{};
};

uint64_t now_ms();
const char* env_or_empty(const char* key);
bool env_is_true(const char* key, bool default_value);
int env_or_int(const char* key, int default_value);

void load_config(AppConfig& cfg, const std::string& path);
bool path_writable_probe(const std::string& file_path);

std::optional<double> json_get_number(const std::string& body, const std::string& key);
std::optional<int> json_get_int(const std::string& body, const std::string& key);
std::optional<bool> json_get_bool(const std::string& body, const std::string& key);
std::optional<std::string> json_get_string(const std::string& body, const std::string& key);

const char* status_to_str(fq::core::OrderStatus s);
const char* reject_to_str(fq::core::RejectReason r);

std::string effective_config_json(const AppConfig& cfg);
std::string runtime_to_json(const fq::strategy::GridRuntimeView& view, bool enabled);

bool read_credentials_file(const std::string& path, std::string& api_key, std::string& secret_key);
bool write_credentials_file(const std::string& path, const std::string& api_key, const std::string& secret_key);

bool load_runtime_state(const std::string& path, bool& strategy_enabled, core::RiskConfig& risk_cfg);
bool save_runtime_state(const std::string& path, bool strategy_enabled, const core::RiskConfig& risk_cfg);

} // namespace fq::app
