# FrogQuant

当前阶段已打通：
1. Market（Binance Testnet：WS 主 + REST fallback）
2. Async Logger（JSONL）
3. Risk + OMS
4. Execution（MARKET/LIMIT/CANCEL）
5. Strategy（Long-only Dynamic Grid）
6. Local Control API（127.0.0.1）
7. Qt5.15 Client（Qt Creator 可直接打开）

## 构建运行
```bash
cd FrogQuant
cmake -S . -B build
cmake --build build -j
./build/frogquant_market_app
ctest --test-dir build --output-on-failure
```

## 长期运行（服务器）
默认常驻运行，直到 `SIGINT/SIGTERM`。

可选环境变量：
- `FQ_RUN_SECONDS=60`（仅调试）
- `FQ_LOG_PATH=/var/log/frogquant/market_logs.jsonl`
- `FQ_HEALTH_UNHEALTHY_AFTER_NO_TICK_SEC=15`
- `FQ_CONFIG_PATH=/path/to/engine.yaml`

配置文件：
- `config/engine.yaml`（默认读取）
- `config/engine.example.yaml`（模板）

## 文档
- `docs/server_deployment_long_running.md`
- `docs/config_vs_client_mutability.md`
- `docs/qt_control_api_v1.md`
- `docs/qt_client_setup_qt5_15.md`
- `docs/module_map.md`
- `docs/change_log.md`
