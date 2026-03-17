#pragma once

#include <cstdint>

namespace fq::logger {

// LogLevel: 日志级别，用于后续过滤与告警分流。
enum class LogLevel : uint8_t {
    Debug = 0,
    Info = 1,
    Warn = 2,
    Error = 3,
};

// LogEvent: 固定大小日志事件。
// 设计目的：
// 1) 避免日志热路径动态分配
// 2) 可安全放入 lockfree 队列
// 3) 后台线程统一序列化为 JSON Lines
struct LogEvent {
    uint64_t ts_ms{0};
    LogLevel level{LogLevel::Info};

    // 模块名（例如 market.ws / market.rest / runtime）
    char module[24]{};

    // 事件类型（例如 tick / reconnect / parse_error）
    char event[24]{};

    // 主消息（简要说明）
    char message[160]{};

    // 可选数值字段（根据事件语义填写）
    double value1{0.0};
    double value2{0.0};
};

} // namespace fq::logger
