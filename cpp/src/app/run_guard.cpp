#include "app/run_guard.hpp"

#include <chrono>
#include <thread>

namespace fq::app {

void run_until_stop(std::atomic<bool>& stop_flag, int run_seconds) {
    if (run_seconds > 0) {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(run_seconds);
        while (!stop_flag.load() && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    } else {
        while (!stop_flag.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
}

} // namespace fq::app
