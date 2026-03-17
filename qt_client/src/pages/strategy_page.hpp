#pragma once

#include "../api_client.hpp"
#include "../i18n.hpp"

#include <QWidget>

class QLineEdit;
class QCheckBox;
class QLabel;
class QPushButton;
class QTimer;

class QFormLayout;

class StrategyPage : public QWidget {
    Q_OBJECT
public:
    explicit StrategyPage(ApiClient* api, QWidget* parent = nullptr);
    void setLanguage(AppLang lang);

private:
    void applyTexts();

    ApiClient* api_;
    AppLang lang_ {AppLang::ZhCN};

    QFormLayout* form_ {nullptr};
    QLineEdit *levels_, *qty_, *tp_, *recenter_, *band_;
    QLineEdit *riskNotional_, *riskPosRatio_, *riskLossRatio_;
    QCheckBox* enabled_;
    QLabel* result_;
    QLabel* title_;
    QPushButton *applyBtn_, *enableBtn_, *refreshBtn_, *resetBtn_, *riskApplyBtn_, *riskRefreshBtn_;
    QTimer* timer_ {nullptr};
};
