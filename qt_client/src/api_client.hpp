#pragma once

#include <QObject>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QElapsedTimer>
#include <QHash>

class ApiClient : public QObject {
    Q_OBJECT
public:
    explicit ApiClient(QObject* parent = nullptr);

    void getHealth();
    void getRuntime();
    void getEffectiveConfig();
    void postGridConfig(const QJsonObject& obj);
    void postGridEnable(bool enabled);
    void postManualOrder(const QJsonObject& obj);
    void postManualCancel(qulonglong internalOrderId);
    void getOrders();
    void getEvents();
    void getCredentialsStatus();
    void postCredentialsUpdate(const QString& apiKey, const QString& secretKey);
    void getRiskConfig();
    void postRiskConfig(const QJsonObject& obj);

signals:
    void responseReady(const QString& route, int httpCode, const QByteArray& body);
    void requestMetric(const QString& route, int httpCode, qint64 latencyMs, bool ok);

private:
    void get(const QString& path);
    void post(const QString& path, const QJsonObject& obj);

    QString baseUrl_ {"http://127.0.0.1:8080"};
    QNetworkAccessManager nam_;
    QHash<QObject*, QElapsedTimer> timers_;
};
