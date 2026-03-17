# TradingCore Benchmark

## 目标
评估 `TradingCore::submit_manual_order` 的吞吐和延迟分位数。

## 构建
```bash
cd FrogQuant
cmake -S . -B build
cmake --build build -j
```

## 运行
默认 200000 次：
```bash
./build/frogquant_benchmark_trading_core
```

自定义迭代次数（例如 500000）：
```bash
./build/frogquant_benchmark_trading_core 500000
```

## 输出指标
- `ops_per_sec`
- `latency_ns p50/p95/p99/max`
- `dropped_logs`

> 说明：基准期间将 Risk 放宽，避免 reject 干扰吞吐测量。
