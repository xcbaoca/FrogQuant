#pragma once

#include "app/app_support.hpp"
#include "core/order_types.hpp"
#include "execution/binance_testnet_execution.hpp"

#include <string>

namespace fq::app {

struct BootstrapResult {
    AppConfig cfg;
    core::RiskConfig risk_cfg;
    execution::ExecConfig exec_cfg;
};

// 负责：
// - 加载配置 + 环境变量覆盖
// - 启动前自检（日志路径、凭证）
// - 构建 risk/exec 配置对象
bool bootstrap(BootstrapResult& out, std::string& err);

} // namespace fq::app
