# Change Log

## 2026-03-16 - Market module v1 (testnet live data)
- 初始化项目骨架（CMake + tests + scripts + docs）
- 新增 `market_types.hpp` 统一 tick 结构
- 新增 `BinanceWsClient`（Testnet WS 主通道）
- 新增 `BinanceRestClient`（Testnet REST fallback）
- 新增 `MarketDataService`（WS+REST 编排）
- 新增 app 入口：`cpp/src/app/main.cpp`
- 新增解析测试：`test_market_parser.cpp`
- 全部行情模块代码补充详细注释（线程语义、解析逻辑、重连策略）
- 文档更新：architecture/module_map/acceptance_checklist/README

## 2026-03-16 - Async logger module v1
- 新增 `logger/log_event.hpp`（定长日志事件）
- 新增 `logger/async_jsonl_logger.hpp/.cpp`（异步 JSONL 写盘）
- `main.cpp` 接入异步日志（start/tick/summary）
- 新增测试：`test_async_logger.cpp`
- 构建系统更新：新增 logger 源文件与测试目标
- 文档更新：module_map + async_logger_module

## 2026-03-16 - Risk + OMS module v1
- 新增 `core/order_types.hpp`（订单域模型 + 风控参数）
- 新增 `core/risk_engine.hpp/.cpp`（pre-trade 风控）
- 新增 `core/oms.hpp/.cpp`（状态机 + 幂等）
- 新增 `core/trading_core.hpp/.cpp`（行情驱动 + 手动下单编排）
- `main.cpp` 接入手动下单演示（accept/reject/cancel）
- 新增测试：`test_risk_oms_core.cpp`
- 构建系统更新：新增 core 源文件与测试目标
- 文档更新：module_map + risk_oms_module + acceptance_checklist + README

## 2026-03-16 - Execution module v1
- 新增 `execution/binance_testnet_execution.hpp/.cpp`
- 支持 Testnet MARKET/LIMIT/CANCEL（签名 + HTTP 请求）
- 支持 `dry_run` 模式（默认建议开启）
- `main.cpp` 接入执行模块演示（内部Accepted后触发执行）
- 新增测试：`test_execution_module.cpp`
- 构建系统更新：新增 execution 源文件与测试目标
- 文档更新：execution_module + module_map + README + acceptance_checklist
- 新增执行配置启动自检（仅显示凭证是否已设置，不泄露密钥）
- 新增 `clientOrderId` 透传支持，默认演示为 `fq_<internal_id>`
- 新增执行响应体截断打印，便于快速定位“下单成功但界面未见单”问题

## 2026-03-16 - Strategy long-grid module v1
- 新增 `strategy/long_grid_strategy.hpp/.cpp`
- 实现单向做多网格：下探触发 BUY，成交后挂 SELL 止盈，卖出后回到 Idle
- `main.cpp` 接入策略意图生成演示（随 tick 输出 intent）
- 新增测试：`test_long_grid_strategy.cpp`
- 构建系统更新：新增 strategy 源文件与测试目标
- 文档更新：strategy_long_grid_module + module_map + acceptance_checklist + README

## 2026-03-16 - Strategy dynamic-grid upgrade v2
- 将固定价格区间网格升级为动态网格（锚点 + 漂移阈值重建）
- 新增策略参数：`dynamic_enabled/recenter_threshold_ratio/band_ratio`
- 新增 `update_config()`：运行时参数更新接口（后续 Qt 控制面板可调用）
- 新增 `runtime_view()`：策略运行态快照（锚点、重建次数、网格层状态）
- 更新 `main.cpp` 使用动态网格默认参数
- 更新测试：覆盖动态重建与运行时参数更新
- 文档更新：strategy_long_grid_module + acceptance_checklist

## 2026-03-16 - Local control API v1 (for Qt)
- 新增 `control/local_http_server.hpp/.cpp`，仅监听 `127.0.0.1:8080`
- 新增 GET `/strategy/grid/runtime`：读取动态网格运行态
- 新增 POST `/strategy/grid/config`：运行时更新动态网格参数
- `main.cpp` 接入控制服务，并增加策略并发访问互斥保护
- 修复执行凭证读取为环境变量（移除硬编码敏感信息）
- 文档新增：`qt_control_api_v1.md`
- 补充接口契约：`api_version`、参数边界校验、结构化错误码（如 `E_CFG_LEVELS_RANGE`）

## 2026-03-16 - Qt control API v1.1
- 控制服务升级为通用路由处理（便于继续扩展）
- 新增 `GET /health`
- 新增 `POST /strategy/grid/enable`
- 新增 `POST /trade/manual/order`
- 新增 `POST /trade/manual/cancel`
- 新增 `GET /orders` 与 `GET /events`
- 运行态 `api_version` 升级为 `v1.1`

## 2026-03-16 - Qt5.15 client skeleton
- 新增 `qt_client/` 客户端工程（支持 qmake + CMake）
- 新增 `frogquant_qt_client.pro`（Qt Creator 直接打开）
- 新增 `src/api_client.*`（封装本地 HTTP 接口）
- 新增 4 个页面：Dashboard / Strategy / Trading / Orders-Events
- 文档新增：`qt_client_setup_qt5_15.md`
- 新增中英双语切换（窗口/Tab/按钮文本实时切换）
- 增强 UI：Dashboard/Strategy/Orders-Events 自动刷新
- 增强 UI：Orders/Events 由文本框升级为双表格展示（最近50条）
- 增强双语：页面标题与表头文案可切换
- 增强交互：Strategy/Trading 输入校验 + 危险操作二次确认弹窗
- 增强可观测性：Dashboard 增加连接状态灯与请求延迟显示
- 增强展示：Orders/Events 表格加入时间与摘要字段，支持 JSON 摘要解析
- 增强双语：Strategy/Trading 字段标签支持中英切换
- 增强容错：Dashboard 增加连接失败提示条（banner）
- 增强交互：Trading 的 side/type 改为下拉框，降低输入错误
- 增强策略页：新增“恢复默认参数”按钮

## 2026-03-16 - Server runtime reliability upgrade
- 修复服务端默认 15 秒后自动退出的问题，改为 daemon 模式（直到 SIGINT/SIGTERM）
- 新增 `FQ_RUN_SECONDS`（可选）用于限时运行调试
- 新增 `FQ_LOG_PATH` 支持自定义日志路径（便于服务器部署）
- 优化本地控制服务停止逻辑：acceptor 改为 non-blocking + 轮询睡眠，避免 stop/join 阻塞
- 增加内存保护：订单历史缓存上限（超限分段清理）
- 强化 `/health`：新增 `uptime_sec/tick_count/last_tick_ms/strategy_enabled`
- 新增部署资产：`deploy/frogquant.service`、`deploy/frogquant.logrotate`、`deploy/install_service.sh`
- 新增部署环境模板：`deploy/frogquant.env.example`
- 新增启动前自检：日志路径可写、非 dry-run 时必须提供 API 凭证
- 新增健康判定：无 tick 超阈值返回 503（`stale_tick=true`）
- 新增定时健康检查：`healthcheck.sh` + `frogquant-healthcheck.timer`
- install 脚本支持一键安装并启用 healthcheck timer
- 新增文档：`server_deployment_long_running.md`
- 代码清理：主程序增加结构化注释、统一缓存上限常量、reject_reason 字符串化
- 脚本清理：healthcheck 使用临时文件自动清理，避免 /tmp 残留
- 代码结构清理：将历史 demo/旧架构源码迁移至 `_trash/2026-03-16-cleanup/`（保留可回滚）
- 新增清理清单：`_trash/2026-03-16-cleanup/CLEANUP_MANIFEST.md`

## 2026-03-16 - Config landing (server + client mutability split)
- 新增 `config/engine.example.yaml` 与 `config/engine.yaml`
- 服务端主程序接入配置加载（支持 `FQ_CONFIG_PATH`）
- 环境变量优先覆盖关键项（log_path/dry_run/run_seconds/health阈值）
- 明确“客户端可热更新”仅限策略参数与启停，其余通过配置/环境变量管理
- 新增 `GET /config/effective`（返回非敏感生效配置）
- Qt Dashboard 新增生效配置面板与刷新按钮（并周期拉取）
- Qt Dashboard 生效配置从原始 JSON 升级为分组表格视图（engine/risk/execution/logging/runtime/strategy）
- Qt Dashboard 增加分组筛选、关键配置高亮（risk + execution.dry_run）与配置导出
- Qt Dashboard 生效配置视图升级为 QTreeWidget 树形分组（可展开/收起）
- 服务端代码整理：将 main.cpp 通用工具/配置解析/JSON辅助抽离到 `app/app_support.*`
- 服务端代码整理：将控制路由处理从 main.cpp 抽离到 `app/control_api_router.*`
- 服务端代码整理：新增 `app/bootstrap.*` 统一处理配置加载、环境覆盖、启动前自检
- 服务端代码整理：新增 `app/runtime_loop.*` 抽离行情回调与策略推进逻辑，进一步瘦身 main.cpp
- 服务端代码整理：新增 `app/run_guard.*` 抽离主循环等待逻辑（常驻/限时）
- 新增凭证管理 API：`/credentials/status` + `/credentials/update`（本机、重启生效）
- Bootstrap 支持从 `execution.credentials_path` 读取凭证（环境变量优先）
- Qt Trading 页新增凭证状态查看与凭证写入入口（密码输入框）
- 新增风控配置 API：`/risk/config`（GET/POST），支持客户端配置并持久化运行态
- Qt Strategy 页新增风控参数面板（读取/更新）
- 新增基准程序：`frogquant_benchmark_trading_core`（吞吐 + p50/p95/p99）
- 新增基准说明文档：`docs/benchmark_trading_core.md`
- 修复手动下单回执：新增 `reject_reason` 字段，便于客户端直接展示拒单原因
- 文档新增：`config_vs_client_mutability.md`
- 配置落地：新增 `core/app_config.hpp/.cpp`，主程序从 `config/engine.yaml` 加载关键参数
- 新增配置模板：`config/engine.yaml.example`
- 新增策略/配置边界文档：`docs/config_and_runtime_policy.md`
