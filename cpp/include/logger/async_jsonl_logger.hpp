#pragma once

#include "logger/log_event.hpp"

#include <atomic>
#include <boost/lockfree/queue.hpp>
#include <fstream>
#include <string>
#include <thread>

namespace fq::logger {

// AsyncJsonlLogger:
// - 多生产者 -> 单消费者（后台线程）
// - 生产者线程仅 enqueue，不做磁盘IO
// - 消费者线程把 LogEvent 写成 JSONL 文件
class AsyncJsonlLogger {
public:
    explicit AsyncJsonlLogger(std::string file_path, std::size_t queue_capacity = 8192);
    ~AsyncJsonlLogger();

    void start();
    void stop();

    // 非阻塞入队：
    // - true  : 入队成功
    // - false : 队列满，日志被丢弃
    bool try_log(const LogEvent& ev);

    uint64_t dropped_count() const;

private:
    void run();
    static const char* level_to_str(LogLevel lv);
    static std::string escape_json(const char* s);

    std::string file_path_;

    // 使用 runtime-size lockfree queue（MPSC 场景更合适）
    boost::lockfree::queue<LogEvent> q_;

    std::atomic<bool> running_{false};
    std::thread worker_;
    std::atomic<uint64_t> dropped_{0};
};

} // namespace fq::logger
