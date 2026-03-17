#pragma once

#include "../api_client.hpp"
#include "../i18n.hpp"

#include <QWidget>

class QTableWidget;
class QPushButton;
class QLabel;
class QTimer;

class LogsPage : public QWidget {
    Q_OBJECT
public:
    explicit LogsPage(ApiClient* api, QWidget* parent = nullptr);
    void setLanguage(AppLang lang);

private:
    void applyTexts();
    void appendTableRow(QTableWidget* table, const QString& route, int code, const QByteArray& body);

    ApiClient* api_;
    AppLang lang_ {AppLang::ZhCN};

    QPushButton *ordersBtn_ {nullptr}, *eventsBtn_ {nullptr};
    QLabel *ordersTitle_ {nullptr}, *eventsTitle_ {nullptr};
    QTableWidget* ordersTable_ {nullptr};
    QTableWidget* eventsTable_ {nullptr};
    QTimer* timer_ {nullptr};
};
