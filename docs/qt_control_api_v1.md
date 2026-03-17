# Qt Control API v1.1 (Localhost Only)

## 目标
为后续 Qt 客户端提供本地控制入口，当前仅开放动态网格参数读写。

- 监听地址：`127.0.0.1:8080`
- 安全边界：仅本机可访问（不绑定 0.0.0.0）

## 接口

### 0) 健康检查
`GET /health`

返回示例：
```json
{"ok":true,"api_version":"v1.2","core":"running","dry_run":true}
```

### 0.2) 生效配置（非敏感）
`GET /config/effective`

返回当前服务端生效配置（不包含 API key/secret）。

### 0.5) 策略启停
`POST /strategy/grid/enable`

请求：
```json
{"enabled":true}
```

响应：
```json
{"ok":true,"code":"OK","enabled":true}
```

### 1) 获取运行态
`GET /strategy/grid/runtime`

返回示例：
```json
{
  "api_version": "v1",
  "anchor_price": 60234.5,
  "rebuild_count": 3,
  "config": {
    "levels": 8,
    "order_qty": 0.001,
    "take_profit_ratio": 0.002,
    "dynamic_enabled": true,
    "recenter_threshold_ratio": 0.004,
    "band_ratio": 0.06
  },
  "levels": [
    {"id":1,"buy_price":58427.46,"sell_price":58544.31,"state":0}
  ]
}
```

### 2) 更新参数
`POST /strategy/grid/config`

请求体（可部分字段更新）：
```json
{
  "levels": 10,
  "order_qty": 0.001,
  "take_profit_ratio": 0.002,
  "dynamic_enabled": true,
  "recenter_threshold_ratio": 0.003,
  "band_ratio": 0.08
}
```

参数边界（v1 契约）：
- `levels`: [2, 200]
- `order_qty`: (0, 1000]
- `take_profit_ratio`: (0, 0.2]
- `recenter_threshold_ratio`: (0, 0.2]
- `band_ratio`: (0.002, 1.0]

成功返回：
```json
{"ok":true,"code":"OK"}
```

失败返回：
```json
{"ok":false,"code":"E_CFG_LEVELS_RANGE","error":" levels must be in [2,200]"}
```

### 3) 手动下单
`POST /trade/manual/order`

请求示例：
```json
{"symbol":"BTCUSDT","side":"BUY","type":"MARKET","qty":0.001}
```

### 4) 手动撤单（内部单）
`POST /trade/manual/cancel`

请求示例：
```json
{"internal_order_id":1}
```

### 5) 订单与事件
- `GET /orders`
- `GET /events`

### 6) 凭证管理（本机）
- `GET /credentials/status`
- `POST /credentials/update`

### 7) 风控参数（客户端可配）
- `GET /risk/config`
- `POST /risk/config`

请求示例：
```json
{"max_single_order_notional_usd":20,"max_position_ratio":0.9,"daily_loss_stop_ratio":0.08}
```

请求示例：
```json
{"api_key":"xxx","secret_key":"yyy"}
```

响应示例：
```json
{"ok":true,"code":"OK","restart_required":true}
```

## Curl 调试
```bash
curl http://127.0.0.1:8080/health
curl http://127.0.0.1:8080/config/effective
curl http://127.0.0.1:8080/strategy/grid/runtime
curl -X POST http://127.0.0.1:8080/strategy/grid/enable -H 'Content-Type: application/json' -d '{"enabled":true}'

curl -X POST http://127.0.0.1:8080/strategy/grid/config \
  -H 'Content-Type: application/json' \
  -d '{"levels":10,"band_ratio":0.08,"recenter_threshold_ratio":0.003}'

curl -X POST http://127.0.0.1:8080/trade/manual/order -H 'Content-Type: application/json' -d '{"symbol":"BTCUSDT","side":"BUY","type":"MARKET","qty":0.001}'
curl -X POST http://127.0.0.1:8080/trade/manual/cancel -H 'Content-Type: application/json' -d '{"internal_order_id":1}'

curl http://127.0.0.1:8080/orders
curl http://127.0.0.1:8080/events
```
