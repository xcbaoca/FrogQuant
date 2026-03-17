# Qt Client Setup (Qt 5.15 + Qt Creator)

## 目标
确保你在本机已安装的 Qt 5.15 下，可直接用 Qt Creator 打开、编译、调试。

## 打开方式（推荐）
### 方式 A：qmake 工程（最稳）
1. 在 Qt Creator 选择 `Open File or Project`
2. 打开：`qt_client/frogquant_qt_client.pro`
3. 选择 Qt 5.15 Kit
4. Build + Run

### 方式 B：CMake 工程
1. 打开：`qt_client/CMakeLists.txt`
2. 确保 Kit 使用 Qt 5.15（Widgets + Network）
3. Build + Run

## 运行前提
先启动核心服务（本地 API 在 127.0.0.1:8080）：
- `GET /health`
- `GET /strategy/grid/runtime`
- `POST /strategy/grid/config`
- `POST /strategy/grid/enable`
- `POST /trade/manual/order`
- `POST /trade/manual/cancel`
- `GET /orders`
- `GET /events`

## Qt 客户端页面
- Dashboard（含自动刷新 health/runtime + 生效配置分组表格）
- Strategy（含自动刷新 runtime）
- Trading
- Orders/Events（双表格展示 + 自动刷新）

## 中英双语切换
- 主窗口工具栏提供语言下拉框：`中文 / English`
- 切换后会实时更新：
  - 窗口标题
  - Tab 标题
  - 页面按钮文案
  - 页面标题与表头文案

## 交互与校验增强（已实现）
- Strategy 页参数输入有范围校验（与后端契约一致）
- Trading 页下单/撤单参数有前置校验
- 应用参数、启停策略、下单、撤单均有二次确认弹窗

## 展示增强（已实现）
- Dashboard 顶部显示连接状态灯（绿/红）与最近请求延迟
- Dashboard 新增连接异常提示条（黄色 banner）
- Dashboard 生效配置支持：树形分组折叠/展开、分组筛选、风险/干跑高亮、JSON 导出
- Orders/Events 表格新增时间列与摘要列（自动解析 JSON items 数量）
- Strategy/Trading 字段标签支持中英双语切换
- Trading 的 side/type 改为下拉框（避免误输入）
- Strategy 新增“恢复默认参数”按钮

## 调试建议
- Qt Creator 左侧 Application Output 观察请求结果
- 若页面返回 0/404，先检查 core 进程是否启动、端口是否监听 127.0.0.1:8080
