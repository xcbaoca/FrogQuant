QT += widgets network
CONFIG += c++17

TEMPLATE = app
TARGET = frogquant_qt_client

SOURCES += \
    src/main.cpp \
    src/main_window.cpp \
    src/api_client.cpp \
    src/pages/dashboard_page.cpp \
    src/pages/strategy_page.cpp \
    src/pages/trading_page.cpp \
    src/pages/logs_page.cpp

HEADERS += \
    src/main_window.hpp \
    src/api_client.hpp \
    src/i18n.hpp \
    src/pages/dashboard_page.hpp \
    src/pages/strategy_page.hpp \
    src/pages/trading_page.hpp \
    src/pages/logs_page.hpp
