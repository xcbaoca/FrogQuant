#pragma once

#include <atomic>

namespace fq::app {

// run_until_stop:
// - run_seconds > 0: 运行固定时长
// - run_seconds <= 0: 常驻直到 stop_flag=true
void run_until_stop(std::atomic<bool>& stop_flag, int run_seconds);

} // namespace fq::app
