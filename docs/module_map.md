# Module Map (Phase 3: Market + Async Logger + Risk/OMS)

## market/
- `market_types.hpp`：统一 tick 数据结构
- `binance_ws_client.hpp/.cpp`：Binance Testnet WS 客户端（主通道）
- `binance_rest_client.hpp/.cpp`：Binance Testnet REST 客户端（fallback）
- `market_data_service.hpp/.cpp`：市场数据服务编排层

## logger/
- `log_event.hpp`：固定大小日志事件结构
- `async_jsonl_logger.hpp/.cpp`：异步 JSONL 日志模块

## core/
- `order_types.hpp`：订单域模型、风险配置、拒单原因
- `risk_engine.hpp/.cpp`：pre-trade 风控校验
- `oms.hpp/.cpp`：订单状态机（New/Accepted/Rejected/Cancelled）
- `trading_core.hpp/.cpp`：Risk+OMS 核心编排

## execution/
- `binance_testnet_execution.hpp/.cpp`：Binance Testnet 执行模块（MARKET/LIMIT/CANCEL）

## strategy/
- `long_grid_strategy.hpp/.cpp`：单向做多动态合约网格策略（v2）

## control/
- `local_http_server.hpp/.cpp`：本地 HTTP 控制服务（127.0.0.1）

## app/
- `main.cpp`：服务启动/装配/主循环（精简入口）
- `app_support.hpp/.cpp`：配置加载、JSON辅助、通用运行工具
- `bootstrap.hpp/.cpp`：启动阶段配置合并与自检
- `control_api_router.hpp/.cpp`：本地控制 API 路由处理
- `runtime_loop.hpp/.cpp`：行情回调与策略意图推进逻辑
- `run_guard.hpp/.cpp`：运行等待策略（常驻/限时）

## config/
- `engine.yaml`：默认服务端配置
- `engine.yaml.example`：配置模板

## core/
- `app_config.hpp/.cpp`：启动配置加载（yaml-like key/value）

## qt_client/
- `frogquant_qt_client.pro`：Qt Creator(qmake) 打开入口
- `CMakeLists.txt`：Qt Creator(CMake) 打开入口
- `src/main_window.*`：主窗口与 4 页面容器
- `src/api_client.*`：HTTP API 客户端（127.0.0.1:8080）
- `src/pages/*`：Dashboard / Strategy / Trading / Orders-Events

## deploy/
- `frogquant.service`：systemd 服务模板
- `frogquant.logrotate`：日志轮转配置
- `install_service.sh`：一键安装脚本
- `healthcheck.sh`：本地健康检查脚本
- `frogquant-healthcheck.service/.timer`：定时健康检查任务
- `frogquant.env.example`：部署环境变量模板

## config/
- `engine.example.yaml`：配置模板
- `engine.yaml`：本地运行配置（可由 `FQ_CONFIG_PATH` 覆盖）

## _trash/
- `2026-03-16-cleanup/`：历史 demo/旧架构文件临时归档（可回滚）

## tests/
- `test_market_parser.cpp`：REST 与 WS JSON 解析单测
- `test_async_logger.cpp`：异步日志模块单测
- `test_risk_oms_core.cpp`：Risk+OMS+TradingCore 单测
- `test_execution_module.cpp`：执行模块单测
- `test_long_grid_strategy.cpp`：网格策略单测
