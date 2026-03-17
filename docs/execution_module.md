# Execution Module (Binance Testnet)

## 目标
在 Risk + OMS 之后接入执行能力，支持手动下单路径：
- MARKET
- LIMIT
- CANCEL

## 组件
- `execution/binance_testnet_execution.hpp/.cpp`

## 能力说明
1. 使用 Binance Testnet Futures REST 下单/撤单
2. 使用 HMAC-SHA256 对 query 签名
3. 支持 dry-run（默认建议开启）
4. 提供 `ExecResult` 统一执行结果
5. 支持 `newClientOrderId` 透传，便于网页检索
6. 控制台打印响应体截断预览，便于排障

## 关键环境变量
- `BINANCE_TESTNET_API_KEY`
- `BINANCE_TESTNET_SECRET_KEY`
- `FQ_EXEC_DRY_RUN=true/false`

## 启动自检（已实现）
程序启动会打印：
- `api_key_set=true/false`
- `secret_set=true/false`
- `dry_run=true/false`

该自检仅显示“是否配置”，不会输出真实密钥内容。

## 集成说明
`app/main.cpp` 中：
1. 内部订单先经过 Risk+OMS
2. 若 Accepted，再调用 execution.place_order
3. 下单时附带 `clientOrderId=fq_<internal_id>`
4. 控制台输出 place/cancel 的 body 预览（截断）
5. 拿到外部 `orderId` 后可调用 cancel_order

## 安全建议
- 默认 `dry_run=true`
- 先在 testnet 验证所有路径再切换真实环境
