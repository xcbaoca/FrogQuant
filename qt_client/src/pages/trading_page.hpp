#pragma once

#include "../api_client.hpp"
#include "../i18n.hpp"

#include <QWidget>

class QLineEdit;
class QLabel;
class QPushButton;
class QComboBox;

class QFormLayout;

class TradingPage : public QWidget {
    Q_OBJECT
public:
    explicit TradingPage(ApiClient* api, QWidget* parent = nullptr);
    void setLanguage(AppLang lang);

private:
    void applyTexts();

    ApiClient* api_;
    AppLang lang_ {AppLang::ZhCN};

    QFormLayout* form_ {nullptr};
    QLineEdit *symbol_, *qty_, *price_, *cancelId_;
    QComboBox *sideBox_, *typeBox_;
    QLineEdit *apiKey_, *secretKey_;
    QLabel *credStatus_;
    QLabel* result_;
    QLabel* title_;
    QPushButton *orderBtn_, *cancelBtn_, *credSaveBtn_, *credCheckBtn_;
};
