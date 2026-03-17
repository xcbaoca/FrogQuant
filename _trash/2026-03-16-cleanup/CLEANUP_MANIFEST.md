# Cleanup Manifest (2026-03-16)

这些文件已从主代码路径移到 `_trash/2026-03-16-cleanup/`，用于代码结构清理与降噪。

## 处理原则
- 不直接删除，先移动到 `_trash`（可回滚）
- 优先移动：未在当前 CMake 主目标/测试目标中使用的历史 demo 与旧架构文件

## 已移动
- `cpp/src/binance_adapter_demo.cpp`
- `cpp/src/binance_engine_integration_demo.cpp`
- `cpp/src/binance_testnet_order_exec_demo.cpp`
- `cpp/src/main.cpp`
- `cpp/src/perf_queue_benchmark.cpp`
- `cpp/src/zmq_sharded_runner.cpp`
- `cpp/src/zmq_signal_runner.cpp`
- `cpp/src/core/engine.cpp`
- `cpp/src/core/engine.hpp`
- `cpp/src/core/engine_shard.cpp`
- `cpp/src/core/engine_shard.hpp`
- `cpp/src/core/execution_event.hpp`
- `cpp/src/core/execution_sim.cpp`
- `cpp/src/core/execution_sim.hpp`
- `cpp/src/core/id_map.hpp`
- `cpp/src/core/latency_stats.hpp`
- `cpp/src/core/lockfree_spsc_queue.hpp`
- `cpp/src/core/logger.hpp`
- `cpp/src/core/order_journal.cpp`
- `cpp/src/core/order_journal.hpp`
- `cpp/src/core/pipeline.cpp`
- `cpp/src/core/pipeline_runner.cpp`
- `cpp/src/core/pipeline_runner.hpp`
- `cpp/src/core/portfolio.cpp`
- `cpp/src/core/portfolio.hpp`
- `cpp/src/core/router.cpp`
- `cpp/src/core/sharded_pipeline_demo.cpp`
- `cpp/src/core/sharded_pipeline_runner.cpp`
- `cpp/src/core/sharded_pipeline_runner.hpp`
- `cpp/src/core/signal_guard.cpp`
- `cpp/src/core/signal_guard.hpp`
- `cpp/src/core/symbol_shard_router.hpp`
- `cpp/src/core/thread_affinity.cpp`
- `cpp/src/core/thread_affinity.hpp`
- `cpp/src/core/time_utils.hpp`
- `cpp/src/core/types.hpp`
- `cpp/src/core/wait_strategy.hpp`
- `cpp/src/core/async_logger.cpp`
- `cpp/src/core/async_logger.hpp`
- `cpp/src/core/config.cpp`
- `cpp/src/core/config.hpp`
- `cpp/src/core/oms.hpp`
- `cpp/src/core/risk.hpp`

如需回滚，直接从 `_trash/2026-03-16-cleanup/` 移回原路径。
