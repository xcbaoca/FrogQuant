# Risk + OMS Module

## 目标
在执行模块上线前，先完成交易核心的决策与状态管理能力：
- pre-trade 风控
- 订单生命周期状态机
- 手动下单与撤单核心接口

## 组件
- `core/order_types.hpp`：订单域模型与风控配置
- `core/risk_engine.hpp/.cpp`：风控检查器
- `core/oms.hpp/.cpp`：订单状态机与幂等
- `core/trading_core.hpp/.cpp`：行情驱动 + 风控 + OMS 编排

## 当前行为
1. `TradingCore::on_tick` 更新最新价格
2. `submit_manual_order` 创建订单并执行风控
3. 风控通过 => OMS Accepted；不通过 => OMS Rejected
4. `cancel_order` 仅对 Accepted 订单生效

## 风控阈值（按需求默认）
- 单笔最大名义金额：10 USD
- 最大持仓比例：80%
- 日亏损熔断比例：5%

## 说明
当前阶段不发真实单，只完成“可验证的决策与状态管理”。
执行模块将在下一阶段接入 Binance Testnet 下单。
