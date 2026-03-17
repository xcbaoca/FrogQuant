# Market Architecture v1

## 目标
先把“可拿到 Binance Testnet 实盘行情”做稳定，再进入风险/OMS/执行模块。

## 组件
1. `BinanceWsClient`（主通道）
   - 订阅 `wss://stream.testnet.binance.vision:9443/ws/btcusdt@trade`
   - 解析 trade JSON -> `MarketTick`
   - 断线自动重连

2. `BinanceRestClient`（兜底）
   - 调用 `https://testnet.binance.vision/api/v3/ticker/price?symbol=BTCUSDT`
   - 获取最新价格

3. `MarketDataService`（编排层）
   - 启动 WS + REST fallback
   - 统一回调输出 `MarketTick`

4. `AsyncJsonlLogger`（异步日志）
   - MPSC 入队
   - 后台线程写 JSONL

5. `TradingCore`（Risk + OMS）
   - `on_tick` 更新价格
   - `submit_manual_order` -> 风控 -> OMS状态流转
   - `cancel_order` -> OMS撤单流转

## 数据流
WS/REST -> parse -> MarketTick(normalized) -> TradingCore(Risk+OMS) -> AsyncLogger(JSONL)

## 延迟与稳定性权衡
- WS 低延迟、实时性更好（主）
- REST 稳定性更高（辅）
- Service 层同时开启两路，保障“有数据优先”
