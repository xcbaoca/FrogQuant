#include "main_window.hpp"

#include "pages/dashboard_page.hpp"
#include "pages/logs_page.hpp"
#include "pages/strategy_page.hpp"
#include "pages/trading_page.hpp"

#include <QComboBox>
#include <QTabWidget>
#include <QToolBar>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    resize(1180, 760);

    auto* tb = addToolBar("lang");
    langBox_ = new QComboBox(this);
    langBox_->addItem("中文", static_cast<int>(AppLang::ZhCN));
    langBox_->addItem("English", static_cast<int>(AppLang::EnUS));
    tb->addWidget(langBox_);

    tabs_ = new QTabWidget(this);
    setCentralWidget(tabs_);

    dashboard_ = new DashboardPage(&api_, this);
    strategy_ = new StrategyPage(&api_, this);
    trading_ = new TradingPage(&api_, this);
    logs_ = new LogsPage(&api_, this);

    tabs_->addTab(dashboard_, "");
    tabs_->addTab(strategy_, "");
    tabs_->addTab(trading_, "");
    tabs_->addTab(logs_, "");

    connect(langBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        lang_ = static_cast<AppLang>(langBox_->itemData(idx).toInt());
        dashboard_->setLanguage(lang_);
        strategy_->setLanguage(lang_);
        trading_->setLanguage(lang_);
        logs_->setLanguage(lang_);
        applyTexts();
    });

    dashboard_->setLanguage(lang_);
    strategy_->setLanguage(lang_);
    trading_->setLanguage(lang_);
    logs_->setLanguage(lang_);
    applyTexts();
}

void MainWindow::applyTexts() {
    setWindowTitle(tr2(lang_, "FrogQuant 客户端 (Qt 5.15)", "FrogQuant Client (Qt 5.15)"));
    tabs_->setTabText(0, tr2(lang_, "总览", "Dashboard"));
    tabs_->setTabText(1, tr2(lang_, "策略", "Strategy"));
    tabs_->setTabText(2, tr2(lang_, "交易", "Trading"));
    tabs_->setTabText(3, tr2(lang_, "订单/事件", "Orders/Events"));
}
