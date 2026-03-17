# Config vs Client Mutability

## 配置文件管理（启动生效）
`config/engine.yaml` 负责服务端启动参数：

- `engine.symbol`
- `engine.market_poll_ms`
- `engine.control_port`
- `risk.*`（三个硬风控阈值）
- `execution.dry_run`
- `execution.base_url`
- `logging.path`
- `runtime.*`
- `strategy.*`（默认值）

> 启动时通过 `FQ_CONFIG_PATH` 指定配置路径（默认 `config/engine.yaml`）。

## 客户端可热更新（运行时）
通过本地 API（Qt 客户端）可修改：

- `POST /strategy/grid/config`
  - levels
  - order_qty
  - take_profit_ratio
  - dynamic_enabled
  - recenter_threshold_ratio
  - band_ratio
- `POST /strategy/grid/enable`
  - enabled
- `POST /risk/config`
  - max_single_order_notional_usd
  - max_position_ratio
  - daily_loss_stop_ratio

说明：风控参数更新后会立即作用于 pre-trade 检查，并持久化到 `runtime_state_path`。

## 客户端可管理但非热生效（需重启）
- `GET /credentials/status`
- `POST /credentials/update`

说明：凭证写入 `execution.credentials_path`，为安全起见不在 `/config/effective` 明文回显。

## 客户端不允许直接修改（安全/运维项）
- API key / secret
- 风控硬阈值 `risk.*`
- 监听端口与基础运行参数
- 日志路径/缓存上限/健康阈值

这些参数应由配置文件或环境变量管理。
