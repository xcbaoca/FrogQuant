cmake_minimum_required(VERSION 3.16)
project(FrogQuantQtClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets Network)

add_executable(frogquant_qt_client
    src/main.cpp
    src/main_window.hpp
    src/main_window.cpp
    src/api_client.hpp
    src/api_client.cpp
    src/i18n.hpp
    src/pages/dashboard_page.hpp
    src/pages/dashboard_page.cpp
    src/pages/strategy_page.hpp
    src/pages/strategy_page.cpp
    src/pages/trading_page.hpp
    src/pages/trading_page.cpp
    src/pages/logs_page.hpp
    src/pages/logs_page.cpp
)

target_link_libraries(frogquant_qt_client PRIVATE Qt5::Widgets Qt5::Network)
