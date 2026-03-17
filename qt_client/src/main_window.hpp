#pragma once

#include "api_client.hpp"
#include "i18n.hpp"

#include <QMainWindow>

class QTabWidget;
class QComboBox;
class DashboardPage;
class StrategyPage;
class TradingPage;
class LogsPage;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void applyTexts();

    AppLang lang_ {AppLang::ZhCN};
    ApiClient api_;
    QTabWidget* tabs_ {nullptr};
    QComboBox* langBox_ {nullptr};
    DashboardPage* dashboard_ {nullptr};
    StrategyPage* strategy_ {nullptr};
    TradingPage* trading_ {nullptr};
    LogsPage* logs_ {nullptr};
};
