#include "app/bootstrap.hpp"

namespace fq::app {

bool bootstrap(BootstrapResult& out, std::string& err) {
    auto& cfg = out.cfg;

    const std::string config_path = env_or_empty("FQ_CONFIG_PATH")[0] ? env_or_empty("FQ_CONFIG_PATH") : "config/engine.yaml";
    load_config(cfg, config_path);

    // 环境变量覆盖（部署友好）
    cfg.log_path = env_or_empty("FQ_LOG_PATH")[0] ? env_or_empty("FQ_LOG_PATH") : cfg.log_path;
    cfg.exec_dry_run = env_is_true("FQ_EXEC_DRY_RUN", cfg.exec_dry_run);
    cfg.run_seconds = env_or_int("FQ_RUN_SECONDS", cfg.run_seconds);
    cfg.health_unhealthy_after_no_tick_sec = env_or_int("FQ_HEALTH_UNHEALTHY_AFTER_NO_TICK_SEC", cfg.health_unhealthy_after_no_tick_sec);

    if (!path_writable_probe(cfg.log_path)) {
        err = std::string("log path not writable: ") + cfg.log_path;
        return false;
    }

    out.risk_cfg.max_single_order_notional_usd = cfg.risk_max_single_order_notional_usd;
    out.risk_cfg.max_position_ratio = cfg.risk_max_position_ratio;
    out.risk_cfg.daily_loss_stop_ratio = cfg.risk_daily_loss_stop_ratio;

    out.exec_cfg.api_key = env_or_empty("BINANCE_TESTNET_API_KEY");
    out.exec_cfg.secret_key = env_or_empty("BINANCE_TESTNET_SECRET_KEY");
    out.exec_cfg.base_url = cfg.exec_base_url;
    out.exec_cfg.dry_run = cfg.exec_dry_run;

    // 凭证来源优先级：环境变量 > credentials 文件
    if (out.exec_cfg.api_key.empty() || out.exec_cfg.secret_key.empty()) {
        std::string k, s;
        if (read_credentials_file(cfg.credentials_path, k, s)) {
            if (out.exec_cfg.api_key.empty()) out.exec_cfg.api_key = k;
            if (out.exec_cfg.secret_key.empty()) out.exec_cfg.secret_key = s;
        }
    }

    if (!out.exec_cfg.dry_run && (out.exec_cfg.api_key.empty() || out.exec_cfg.secret_key.empty())) {
        err = "FQ_EXEC_DRY_RUN=false but API credentials are missing";
        return false;
    }

    return true;
}

} // namespace fq::app
