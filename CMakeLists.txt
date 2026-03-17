cmake_minimum_required(VERSION 3.20)
project(FrogQuant VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# 依赖说明：
# - Boost: 保留给后续 lockfree / 并发模块
# - OpenSSL: WS over TLS + HMAC 签名
# - CURL: REST fallback + 执行 HTTP 请求
# - Threads: 后台线程
find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

add_executable(frogquant_market_app
  cpp/src/app/main.cpp
  cpp/src/app/app_support.cpp
  cpp/src/app/bootstrap.cpp
  cpp/src/app/control_api_router.cpp
  cpp/src/app/runtime_loop.cpp
  cpp/src/app/run_guard.cpp
  cpp/src/market/binance_ws_client.cpp
  cpp/src/market/binance_rest_client.cpp
  cpp/src/market/market_data_service.cpp
  cpp/src/logger/async_jsonl_logger.cpp
  cpp/src/core/oms.cpp
  cpp/src/core/risk_engine.cpp
  cpp/src/core/trading_core.cpp
  cpp/src/core/app_config.cpp
  cpp/src/execution/binance_testnet_execution.cpp
  cpp/src/strategy/long_grid_strategy.cpp
  cpp/src/control/local_http_server.cpp
)

target_include_directories(frogquant_market_app PRIVATE cpp/include)

target_link_libraries(frogquant_market_app PRIVATE
  OpenSSL::SSL
  OpenSSL::Crypto
  CURL::libcurl
  Threads::Threads
)

target_compile_options(frogquant_market_app PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

enable_testing()

add_executable(frogquant_market_tests
  cpp/tests/test_market_parser.cpp
  cpp/src/market/binance_ws_client.cpp
  cpp/src/market/binance_rest_client.cpp
)

target_include_directories(frogquant_market_tests PRIVATE cpp/include)

target_link_libraries(frogquant_market_tests PRIVATE
  OpenSSL::SSL
  OpenSSL::Crypto
  CURL::libcurl
  Threads::Threads
)
add_test(NAME frogquant_market_tests COMMAND frogquant_market_tests)

add_executable(frogquant_async_logger_tests
  cpp/tests/test_async_logger.cpp
  cpp/src/logger/async_jsonl_logger.cpp
)

target_include_directories(frogquant_async_logger_tests PRIVATE cpp/include)

target_link_libraries(frogquant_async_logger_tests PRIVATE Threads::Threads)
add_test(NAME frogquant_async_logger_tests COMMAND frogquant_async_logger_tests)

add_executable(frogquant_risk_oms_core_tests
  cpp/tests/test_risk_oms_core.cpp
  cpp/src/logger/async_jsonl_logger.cpp
  cpp/src/core/oms.cpp
  cpp/src/core/risk_engine.cpp
  cpp/src/core/trading_core.cpp
)

target_include_directories(frogquant_risk_oms_core_tests PRIVATE cpp/include)

target_link_libraries(frogquant_risk_oms_core_tests PRIVATE Threads::Threads)
add_test(NAME frogquant_risk_oms_core_tests COMMAND frogquant_risk_oms_core_tests)

add_executable(frogquant_execution_tests
  cpp/tests/test_execution_module.cpp
  cpp/src/execution/binance_testnet_execution.cpp
)

target_include_directories(frogquant_execution_tests PRIVATE cpp/include)

target_link_libraries(frogquant_execution_tests PRIVATE
  OpenSSL::SSL
  OpenSSL::Crypto
  CURL::libcurl
)
add_test(NAME frogquant_execution_tests COMMAND frogquant_execution_tests)

add_executable(frogquant_strategy_tests
  cpp/tests/test_long_grid_strategy.cpp
  cpp/src/strategy/long_grid_strategy.cpp
)

target_include_directories(frogquant_strategy_tests PRIVATE cpp/include)
add_test(NAME frogquant_strategy_tests COMMAND frogquant_strategy_tests)

add_executable(frogquant_benchmark_trading_core
  cpp/src/bench/benchmark_trading_core.cpp
  cpp/src/logger/async_jsonl_logger.cpp
  cpp/src/core/oms.cpp
  cpp/src/core/risk_engine.cpp
  cpp/src/core/trading_core.cpp
)

target_include_directories(frogquant_benchmark_trading_core PRIVATE cpp/include)
target_link_libraries(frogquant_benchmark_trading_core PRIVATE Threads::Threads)
