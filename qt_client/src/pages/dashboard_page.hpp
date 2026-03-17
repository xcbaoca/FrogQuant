#pragma once

#include "../api_client.hpp"
#include "../i18n.hpp"

#include <QWidget>

class QLabel;
class QPushButton;
class QTimer;
class QFrame;
class QTableWidget;
class QTreeWidget;
class QComboBox;

class DashboardPage : public QWidget {
    Q_OBJECT
public:
    explicit DashboardPage(ApiClient* api, QWidget* parent = nullptr);
    void setLanguage(AppLang lang);

private:
    void applyTexts();

    ApiClient* api_;
    AppLang lang_ {AppLang::ZhCN};

    QPushButton* healthBtn_ {nullptr};
    QPushButton* runtimeBtn_ {nullptr};
    QPushButton* configBtn_ {nullptr};
    QPushButton* exportBtn_ {nullptr};
    QLabel* healthTitle_ {nullptr};
    QLabel* runtimeTitle_ {nullptr};
    QLabel* configTitle_ {nullptr};
    QLabel* healthLabel_ {nullptr};
    QLabel* runtimeLabel_ {nullptr};
    QLabel* configLabel_ {nullptr};
    QTableWidget* configTable_ {nullptr};
    QTreeWidget* configTree_ {nullptr};
    QComboBox* groupFilter_ {nullptr};
    QLabel* statusText_ {nullptr};
    QLabel* latencyText_ {nullptr};
    QLabel* banner_ {nullptr};
    QFrame* statusDot_ {nullptr};

    QTimer* timer_ {nullptr};

    QByteArray lastConfigRaw_;
};
