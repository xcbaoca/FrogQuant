#include "logger/async_jsonl_logger.hpp"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <thread>

static void expect(bool cond, const char* msg) {
    if (!cond) {
        std::cerr << "[FAIL] " << msg << "\n";
        std::exit(1);
    }
}

int main() {
    const std::string path = "test_market_logs.jsonl";
    std::filesystem::remove(path);

    fq::logger::AsyncJsonlLogger logger(path, 1024);
    logger.start();

    fq::logger::LogEvent ev{};
    ev.ts_ms = 1;
    ev.level = fq::logger::LogLevel::Info;
    std::strncpy(ev.module, "test", sizeof(ev.module) - 1);
    std::strncpy(ev.event, "log", sizeof(ev.event) - 1);
    std::strncpy(ev.message, "hello", sizeof(ev.message) - 1);
    ev.value1 = 42.0;

    expect(logger.try_log(ev), "logger push should succeed");

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    logger.stop();

    expect(std::filesystem::exists(path), "log file should exist");
    expect(std::filesystem::file_size(path) > 0, "log file should not be empty");

    std::filesystem::remove(path);
    std::cout << "[PASS] frogquant_async_logger_tests\n";
    return 0;
}
