#include "logger/async_jsonl_logger.hpp"

#include <chrono>
#include <thread>

namespace fq::logger {

AsyncJsonlLogger::AsyncJsonlLogger(std::string file_path, std::size_t queue_capacity)
    : file_path_(std::move(file_path)), q_(queue_capacity) {}

AsyncJsonlLogger::~AsyncJsonlLogger() {
    stop();
}

void AsyncJsonlLogger::start() {
    if (running_.exchange(true)) return;
    worker_ = std::thread(&AsyncJsonlLogger::run, this);
}

void AsyncJsonlLogger::stop() {
    if (!running_.exchange(false)) return;
    if (worker_.joinable()) worker_.join();
}

bool AsyncJsonlLogger::try_log(const LogEvent& ev) {
    if (!q_.push(ev)) {
        dropped_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    return true;
}

uint64_t AsyncJsonlLogger::dropped_count() const {
    return dropped_.load(std::memory_order_relaxed);
}

void AsyncJsonlLogger::run() {
    std::ofstream ofs(file_path_, std::ios::app);
    if (!ofs.is_open()) {
        // 文件打开失败时无法可靠恢复，这里直接退出线程。
        return;
    }

    while (running_.load(std::memory_order_relaxed)) {
        LogEvent ev;
        int processed = 0;

        // 批量出队，减少 flush 频率与系统调用次数。
        while (processed < 256 && q_.pop(ev)) {
            ofs << "{"
                << "\"ts_ms\":" << ev.ts_ms << ','
                << "\"level\":\"" << level_to_str(ev.level) << "\","
                << "\"module\":\"" << escape_json(ev.module) << "\","
                << "\"event\":\"" << escape_json(ev.event) << "\","
                << "\"message\":\"" << escape_json(ev.message) << "\","
                << "\"value1\":" << ev.value1 << ','
                << "\"value2\":" << ev.value2
                << "}\n";
            ++processed;
        }

        if (processed == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        } else {
            ofs.flush();
        }
    }

    // 收尾：停止后把残留队列刷完。
    LogEvent ev;
    while (q_.pop(ev)) {
        ofs << "{"
            << "\"ts_ms\":" << ev.ts_ms << ','
            << "\"level\":\"" << level_to_str(ev.level) << "\","
            << "\"module\":\"" << escape_json(ev.module) << "\","
            << "\"event\":\"" << escape_json(ev.event) << "\","
            << "\"message\":\"" << escape_json(ev.message) << "\","
            << "\"value1\":" << ev.value1 << ','
            << "\"value2\":" << ev.value2
            << "}\n";
    }

    ofs.flush();
}

const char* AsyncJsonlLogger::level_to_str(LogLevel lv) {
    switch (lv) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warn: return "WARN";
        case LogLevel::Error: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string AsyncJsonlLogger::escape_json(const char* s) {
    std::string out;
    while (*s) {
        const char c = *s++;
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    return out;
}

} // namespace fq::logger
