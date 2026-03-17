# Async Logger Module

## 目标
将日志写盘从行情热路径中解耦，降低阻塞风险。

## 组件
- `logger/log_event.hpp`：固定大小日志事件
- `logger/async_jsonl_logger.hpp/.cpp`：异步 JSONL 日志器（MPSC queue + 单消费者写线程）

## 线程语义
- 生产者：任意业务线程（market/ws/rest/app）
- 消费者：logger 后台线程（唯一）

## 输出格式
每行一条 JSON（JSON Lines），字段：
- ts_ms, level, module, event, message, value1, value2

## 集成点
`app/main.cpp` 已将关键事件（start/tick/summary）写入 `market_logs.jsonl`。
