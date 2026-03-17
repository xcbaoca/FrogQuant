# Config vs Runtime Update Policy

## 原则
- **配置文件（启动级）**：低频、系统级、风险级参数。修改后重启生效。
- **客户端运行时修改（热更新）**：策略参数与操作指令。

## 配置文件（`config/engine.yaml`）
建议放入以下项：
- 交易环境：`symbol`, `exchange.base_url`, `execution.dry_run`
- 控制服务：`control.bind_ip`, `control.port`
- 风控硬阈值：
  - `risk.max_single_order_notional_usd`
  - `risk.max_position_ratio`
  - `risk.daily_loss_stop_ratio`
- 策略默认值：`grid.*`
- 运维参数：
  - `health.unhealthy_after_no_tick_sec`
  - `service.max_events_cache`
  - `service.max_orders_cache`
  - `service.order_trim_batch`
  - `runtime.run_seconds`

## 客户端可修改（运行时）
通过本地 API 白名单允许：
- `POST /strategy/grid/config`
  - `levels`
  - `order_qty`
  - `take_profit_ratio`
  - `dynamic_enabled`
  - `recenter_threshold_ratio`
  - `band_ratio`
- `POST /strategy/grid/enable`
- `POST /trade/manual/order`
- `POST /trade/manual/cancel`

## 不应由客户端直接修改
- API 凭证
- 风控硬阈值（默认）
- bind_ip/port
- 日志路径与运行时系统参数

## 加载方式
- 默认读取：`config/engine.yaml`
- 可用环境变量覆盖路径：`FQ_CONFIG=/path/to/engine.yaml`
- 部分运维参数可被环境变量覆盖：
  - `FQ_EXEC_DRY_RUN`
  - `FQ_RUN_SECONDS`
  - `FQ_HEALTH_UNHEALTHY_AFTER_NO_TICK_SEC`
  - `FQ_LOG_PATH`
