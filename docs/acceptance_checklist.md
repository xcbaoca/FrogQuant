# Acceptance Checklist - Market + Async Logger + Risk/OMS

## 行情模块验收
- [ ] 能连接 Binance Testnet WS 并持续输出 tick
- [ ] WS 断线后能自动重连
- [ ] REST fallback 能定期拉取价格

## 异步日志模块验收
- [ ] `market_logs.jsonl` 生成成功
- [ ] JSONL 每行字段完整

## Risk + OMS 验收
- [ ] 小单（<=10 USD）可 Accepted
- [ ] 大单（>10 USD）会 Rejected
- [ ] Accepted 订单可 Cancelled
- [ ] 非法状态流转被拒绝

## Execution 验收（Testnet）
- [ ] 支持 MARKET/LIMIT 下单请求构造与签名
- [ ] 支持 CANCEL 请求
- [ ] dry-run 模式可安全演示

## Strategy 验收（Long-only Dynamic Grid v2）
- [ ] 首个 tick 正确初始化锚点与网格
- [ ] 价格下探时触发 BUY LIMIT 意图
- [ ] BUY 成交后生成 SELL LIMIT 止盈意图
- [ ] SELL 成交后网格层回到 Idle
- [ ] 偏离锚点超过阈值时触发动态重建
- [ ] 运行时参数更新可生效（为 Qt 调参预留）

## Qt 控制接口验收（127.0.0.1）
- [ ] `GET /health` 返回服务健康状态
- [ ] `GET /config/effective` 返回非敏感生效配置
- [ ] `GET /strategy/grid/runtime` 返回运行态 JSON（包含 `api_version`）
- [ ] `POST /strategy/grid/config` 可更新参数并返回 `{\"ok\":true,\"code\":\"OK\"}`
- [ ] `POST /strategy/grid/enable` 可启停策略
- [ ] `POST /trade/manual/order` 可创建内部订单并返回状态
- [ ] `POST /trade/manual/cancel` 可撤销内部订单
- [ ] `GET /orders` 与 `GET /events` 可返回历史信息
- [ ] 非法参数返回结构化错误码（如 `E_CFG_LEVELS_RANGE`）
- [ ] 服务仅监听 `127.0.0.1`，不对外网暴露

## 测试验收
- [ ] `frogquant_market_tests` 通过
- [ ] `frogquant_async_logger_tests` 通过
- [ ] `frogquant_risk_oms_core_tests` 通过
- [ ] `frogquant_execution_tests` 通过
- [ ] `frogquant_strategy_tests` 通过
