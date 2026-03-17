# Strategy Module - Long-only Dynamic Contract Grid (v2)

## 目标
将固定网格升级为动态网格，并为后续 Qt 客户端参数调节预留接口：
- 单向做多（Long-only）
- 动态合约网格（Dynamic Contract Grid）
- 支持运行时更新策略参数

## 组件
- `strategy/long_grid_strategy.hpp/.cpp`

## 策略逻辑（v2）
1. 首个 tick 作为锚点 `anchor_price` 初始化网格。
2. 网格以锚点为中心，按 `band_ratio` 计算上下边界并均匀分层。
3. 价格下探到某层（`price <= buy_price`）时，发 BUY LIMIT 意图。
4. BUY 成交后，按 `take_profit_ratio` 生成 SELL LIMIT 止盈意图。
5. SELL 成交后，该层回到 Idle。
6. 若 `dynamic_enabled=true` 且价格偏离锚点超过 `recenter_threshold_ratio`，整套网格自动重建。

## Qt 参数联动准备
- 提供 `update_config(const LongGridConfig&)`，可由后续 Qt 控制面板调用。
- 提供 `runtime_view()`，返回：
  - 当前 config
  - 当前锚点价格
  - 重建次数
  - 全部网格层状态

## 低延迟注意
- 每个 tick 最多发 1 笔意图，避免瞬时批量发单。
- 策略只产出意图，执行与回报处理在下游模块完成。
