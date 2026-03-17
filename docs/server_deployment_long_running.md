# Server Deployment (Long-running)

## 目标
让 FrogQuant 在服务器长期稳定运行，并支持自动重启、日志轮转、健康检查。

## 配置加载
- 默认配置文件：`config/engine.yaml`
- 模板：`config/engine.yaml.example`
- 环境变量覆盖路径：`FQ_CONFIG=/path/to/engine.yaml`

## 关键改动（已完成）
- 主程序默认常驻运行（直到 SIGINT/SIGTERM）
- 可选 `FQ_RUN_SECONDS` 用于限时调试
- `/health` 增强字段：`uptime_sec/tick_count/last_tick_ms/strategy_enabled`
- 支持 `FQ_LOG_PATH` 指定日志路径

## systemd 部署
仓库内已提供：
- `deploy/frogquant.service`
- `deploy/frogquant.logrotate`
- `deploy/install_service.sh`
- `deploy/healthcheck.sh`
- `deploy/frogquant-healthcheck.service`
- `deploy/frogquant-healthcheck.timer`

### 快速安装
```bash
cd FrogQuant
cmake -S . -B build
cmake --build build -j
bash deploy/install_service.sh
```

### 查看状态
```bash
systemctl status frogquant --no-pager
systemctl status frogquant-healthcheck.timer --no-pager
journalctl -u frogquant -f
journalctl -u frogquant-healthcheck.service -f
```

## 环境变量（推荐在 `/etc/default/frogquant`）
- `BINANCE_TESTNET_API_KEY=...`
- `BINANCE_TESTNET_SECRET_KEY=...`
- `FQ_EXEC_DRY_RUN=true|false`
- `FQ_LOG_PATH=/var/log/frogquant/market_logs.jsonl`
- `FQ_RUN_SECONDS=0`（0或不设=常驻）
- `FQ_HEALTH_UNHEALTHY_AFTER_NO_TICK_SEC=15`

已提供模板：`deploy/frogquant.env.example`（安装脚本会自动拷贝到 `/etc/default/frogquant`）

## 日志轮转
`deploy/frogquant.logrotate` 默认：
- 每日轮转
- 保留14份
- gzip 压缩

## 健康检查
```bash
curl http://127.0.0.1:8080/health
```
返回关注字段：
- `uptime_sec`
- `tick_count`
- `last_tick_ms`
- `strategy_enabled`
- `stale_tick`

说明：当超过 `FQ_HEALTH_UNHEALTHY_AFTER_NO_TICK_SEC` 未收到新 tick 时，`/health` 会返回 HTTP 503 + `ok=false`。

### 定时健康检查（systemd timer）
- 每 60 秒执行一次 `deploy/healthcheck.sh`
- 健康日志默认写入：`/var/log/frogquant/healthcheck.log`
- 可选告警命令：`FQ_HEALTHCHECK_ALERT_CMD`
