#include "logs_page.hpp"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

LogsPage::LogsPage(ApiClient* api, QWidget* parent) : QWidget(parent), api_(api) {
    auto* root = new QVBoxLayout(this);
    auto* row = new QHBoxLayout();
    ordersBtn_ = new QPushButton(this);
    eventsBtn_ = new QPushButton(this);
    ordersTitle_ = new QLabel(this);
    eventsTitle_ = new QLabel(this);

    ordersTable_ = new QTableWidget(0, 3, this);
    eventsTable_ = new QTableWidget(0, 3, this);
    ordersTable_->horizontalHeader()->setStretchLastSection(true);
    eventsTable_->horizontalHeader()->setStretchLastSection(true);

    row->addWidget(ordersBtn_);
    row->addWidget(eventsBtn_);

    root->addLayout(row);
    root->addWidget(ordersTitle_);
    root->addWidget(ordersTable_);
    root->addWidget(eventsTitle_);
    root->addWidget(eventsTable_);

    connect(ordersBtn_, &QPushButton::clicked, this, [this]() { api_->getOrders(); });
    connect(eventsBtn_, &QPushButton::clicked, this, [this]() { api_->getEvents(); });

    connect(api_, &ApiClient::responseReady, this, [this](const QString& route, int code, const QByteArray& body) {
        if (route == "/orders") {
            appendTableRow(ordersTable_, route, code, body);
        } else if (route == "/events") {
            appendTableRow(eventsTable_, route, code, body);
        }
    });

    timer_ = new QTimer(this);
    timer_->setInterval(1500);
    connect(timer_, &QTimer::timeout, this, [this]() {
        api_->getOrders();
        api_->getEvents();
    });
    timer_->start();

    applyTexts();
}

void LogsPage::setLanguage(AppLang lang) {
    lang_ = lang;
    applyTexts();
}

void LogsPage::applyTexts() {
    ordersBtn_->setText(tr2(lang_, "刷新订单", "Refresh Orders"));
    eventsBtn_->setText(tr2(lang_, "刷新事件", "Refresh Events"));
    ordersTitle_->setText(tr2(lang_, "订单返回（最近）", "Orders Responses (recent)"));
    eventsTitle_->setText(tr2(lang_, "事件返回（最近）", "Events Responses (recent)"));

    QStringList headers{tr2(lang_, "时间", "Time"), tr2(lang_, "状态", "Status"), tr2(lang_, "摘要", "Summary")};
    ordersTable_->setHorizontalHeaderLabels(headers);
    eventsTable_->setHorizontalHeaderLabels(headers);
}

void LogsPage::appendTableRow(QTableWidget* table, const QString& route, int code, const QByteArray& body) {
    QString summary = QString::fromUtf8(body);
    const auto doc = QJsonDocument::fromJson(body);
    if (!doc.isNull() && doc.isObject()) {
        const auto obj = doc.object();
        if (obj.contains("items") && obj.value("items").isArray()) {
            const auto arr = obj.value("items").toArray();
            summary = QString("items=%1").arg(arr.size());
            if (!arr.isEmpty() && arr.first().isObject()) {
                const auto first = arr.first().toObject();
                if (first.contains("internal_id")) {
                    summary += QString(", first_internal_id=%1").arg(first.value("internal_id").toVariant().toString());
                }
                if (first.contains("status")) {
                    summary += QString(", status=%1").arg(first.value("status").toString());
                }
            }
        }
    }

    const int row = table->rowCount();
    table->insertRow(row);
    table->setItem(row, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("HH:mm:ss")));
    table->setItem(row, 1, new QTableWidgetItem(QString("%1 [%2]").arg(route).arg(code)));
    table->setItem(row, 2, new QTableWidgetItem(summary));
    while (table->rowCount() > 50) table->removeRow(0);
}
