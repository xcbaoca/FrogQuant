#pragma once

#include <string>

namespace fq::core {

// AppConfig:
// 服务端统一配置（启动加载）。
// - 配置文件用于低频/高风险参数
// - 客户端只允许改策略运行参数（见控制接口白名单）
struct AppConfig {
    std::string symbol{"BTCUSDT"};

    // execution
    std::string exchange_base_url{"https://testnet.binancefuture.com"};
    bool dry_run{true};

    // control api
    std::string control_bind_ip{"127.0.0.1"};
    int control_port{8080};

    // risk hard limits
    double risk_max_single_order_notional_usd{10.0};
    double risk_max_position_ratio{0.80};
    double risk_daily_loss_stop_ratio{0.05};

    // strategy defaults
    int grid_levels{8};
    double grid_order_qty{0.001};
    double grid_take_profit_ratio{0.002};
    bool grid_dynamic_enabled{true};
    double grid_recenter_threshold_ratio{0.004};
    double grid_band_ratio{0.06};

    // runtime / service
    int health_unhealthy_after_no_tick_sec{15};
    int max_events_cache{200};
    int max_orders_cache{5000};
    int order_trim_batch{1000};
    int run_seconds{0};
};

// load_app_config:
// 从 yaml-like 文件读取配置（key: value）。
// 目前支持扁平 key（如 risk.max_position_ratio: 0.8）。
// 文件不存在时返回默认配置。
AppConfig load_app_config(const std::string& path);

} // namespace fq::core
