#include "api_client.hpp"

#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

ApiClient::ApiClient(QObject* parent) : QObject(parent) {}

void ApiClient::getHealth() { get("/health"); }
void ApiClient::getRuntime() { get("/strategy/grid/runtime"); }
void ApiClient::getEffectiveConfig() { get("/config/effective"); }
void ApiClient::postGridConfig(const QJsonObject& obj) { post("/strategy/grid/config", obj); }
void ApiClient::postGridEnable(bool enabled) { post("/strategy/grid/enable", QJsonObject{{"enabled", enabled}}); }
void ApiClient::postManualOrder(const QJsonObject& obj) { post("/trade/manual/order", obj); }
void ApiClient::postManualCancel(qulonglong internalOrderId) { post("/trade/manual/cancel", QJsonObject{{"internal_order_id", static_cast<double>(internalOrderId)}}); }
void ApiClient::getOrders() { get("/orders"); }
void ApiClient::getEvents() { get("/events"); }
void ApiClient::getCredentialsStatus() { get("/credentials/status"); }
void ApiClient::postCredentialsUpdate(const QString& apiKey, const QString& secretKey) {
    post("/credentials/update", QJsonObject{{"api_key", apiKey}, {"secret_key", secretKey}});
}
void ApiClient::getRiskConfig() { get("/risk/config"); }
void ApiClient::postRiskConfig(const QJsonObject& obj) { post("/risk/config", obj); }

void ApiClient::get(const QString& path) {
    QNetworkRequest req(QUrl(baseUrl_ + path));
    auto* reply = nam_.get(req);
    QElapsedTimer tm; tm.start();
    timers_.insert(reply, tm);

    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, path]() {
        const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray body = reply->readAll();
        const qint64 cost = timers_.contains(reply) ? timers_.value(reply).elapsed() : -1;
        timers_.remove(reply);
        emit responseReady(path, code, body);
        emit requestMetric(path, code, cost, (code >= 200 && code < 300));
        reply->deleteLater();
    });
}

void ApiClient::post(const QString& path, const QJsonObject& obj) {
    QNetworkRequest req(QUrl(baseUrl_ + path));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    auto* reply = nam_.post(req, QJsonDocument(obj).toJson(QJsonDocument::Compact));
    QElapsedTimer tm; tm.start();
    timers_.insert(reply, tm);

    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, path]() {
        const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray body = reply->readAll();
        const qint64 cost = timers_.contains(reply) ? timers_.value(reply).elapsed() : -1;
        timers_.remove(reply);
        emit responseReady(path, code, body);
        emit requestMetric(path, code, cost, (code >= 200 && code < 300));
        reply->deleteLater();
    });
}
