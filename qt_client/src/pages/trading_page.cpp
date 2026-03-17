#include "trading_page.hpp"

#include <QFormLayout>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDoubleValidator>
#include <QMessageBox>
#include <QComboBox>

TradingPage::TradingPage(ApiClient* api, QWidget* parent) : QWidget(parent), api_(api) {
    auto* root = new QVBoxLayout(this);
    form_ = new QFormLayout();

    title_ = new QLabel(this);

    symbol_ = new QLineEdit("BTCUSDT", this);
    sideBox_ = new QComboBox(this);
    sideBox_->addItems({"BUY", "SELL"});
    typeBox_ = new QComboBox(this);
    typeBox_->addItems({"MARKET", "LIMIT"});
    qty_ = new QLineEdit("0.001", this);
    price_ = new QLineEdit("0", this);
    cancelId_ = new QLineEdit("1", this);
    apiKey_ = new QLineEdit(this);
    secretKey_ = new QLineEdit(this);
    apiKey_->setEchoMode(QLineEdit::Password);
    secretKey_->setEchoMode(QLineEdit::Password);

    auto* qtyVal = new QDoubleValidator(0.0, 1000.0, 8, qty_);
    qtyVal->setNotation(QDoubleValidator::StandardNotation);
    qty_->setValidator(qtyVal);
    auto* pxVal = new QDoubleValidator(0.0, 100000000.0, 8, price_);
    pxVal->setNotation(QDoubleValidator::StandardNotation);
    price_->setValidator(pxVal);

    form_->addRow("symbol", symbol_);
    form_->addRow("side(BUY/SELL)", sideBox_);
    form_->addRow("type(MARKET/LIMIT)", typeBox_);
    form_->addRow("qty", qty_);
    form_->addRow("price(limit)", price_);
    form_->addRow("internal_order_id", cancelId_);
    form_->addRow("api_key", apiKey_);
    form_->addRow("secret_key", secretKey_);

    orderBtn_ = new QPushButton(this);
    cancelBtn_ = new QPushButton(this);
    credSaveBtn_ = new QPushButton(this);
    credCheckBtn_ = new QPushButton(this);
    credStatus_ = new QLabel("cred: -", this);
    result_ = new QLabel("result: -", this);
    result_->setWordWrap(true);

    root->addWidget(title_);
    root->addLayout(form_);
    root->addWidget(orderBtn_);
    root->addWidget(cancelBtn_);
    root->addWidget(credSaveBtn_);
    root->addWidget(credCheckBtn_);
    root->addWidget(credStatus_);
    root->addWidget(result_);
    root->addStretch();

    connect(orderBtn_, &QPushButton::clicked, this, [this]() {
        const QString symbol = symbol_->text().trimmed().toUpper();
        const QString side = sideBox_->currentText();
        const QString type = typeBox_->currentText();
        const double qty = qty_->text().toDouble();
        const double price = price_->text().toDouble();

        const bool sideOk = (side == "BUY" || side == "SELL");
        const bool typeOk = (type == "MARKET" || type == "LIMIT");
        const bool qtyOk = qty > 0.0;
        const bool limitOk = (type != "LIMIT") || (price > 0.0);
        if (symbol.isEmpty() || !sideOk || !typeOk || !qtyOk || !limitOk) {
            QMessageBox::warning(this,
                                 tr2(lang_, "下单参数错误", "Order Validation Failed"),
                                 tr2(lang_, "请检查 symbol/side/type/qty/price。", "Please check symbol/side/type/qty/price."));
            return;
        }

        const auto ret = QMessageBox::question(this,
                                               tr2(lang_, "确认下单", "Confirm Order"),
                                               tr2(lang_, "确定提交该手动订单吗？", "Submit this manual order?"));
        if (ret != QMessageBox::Yes) return;

        QJsonObject obj;
        obj["symbol"] = symbol;
        obj["side"] = side;
        obj["type"] = type;
        obj["qty"] = qty;
        obj["price"] = price;
        api_->postManualOrder(obj);
    });

    connect(cancelBtn_, &QPushButton::clicked, this, [this]() {
        const auto id = cancelId_->text().toULongLong();
        if (id == 0) {
            QMessageBox::warning(this,
                                 tr2(lang_, "撤单参数错误", "Cancel Validation Failed"),
                                 tr2(lang_, "internal_order_id 必须大于 0。", "internal_order_id must be > 0."));
            return;
        }
        const auto ret = QMessageBox::question(this,
                                               tr2(lang_, "确认撤单", "Confirm Cancel"),
                                               tr2(lang_, "确定撤销该内部订单吗？", "Cancel this internal order?"));
        if (ret != QMessageBox::Yes) return;
        api_->postManualCancel(id);
    });

    connect(credCheckBtn_, &QPushButton::clicked, this, [this]() { api_->getCredentialsStatus(); });
    connect(credSaveBtn_, &QPushButton::clicked, this, [this]() {
        if (apiKey_->text().isEmpty() || secretKey_->text().isEmpty()) {
            QMessageBox::warning(this,
                                 tr2(lang_, "凭证参数错误", "Credential Validation Failed"),
                                 tr2(lang_, "api_key 与 secret_key 不能为空。", "api_key and secret_key must not be empty."));
            return;
        }
        const auto ret = QMessageBox::question(this,
                                               tr2(lang_, "确认更新凭证", "Confirm Credential Update"),
                                               tr2(lang_, "将更新服务端凭证文件（需重启生效），继续吗？", "This updates server credential file (restart required). Continue?"));
        if (ret != QMessageBox::Yes) return;
        api_->postCredentialsUpdate(apiKey_->text(), secretKey_->text());
    });

    connect(api_, &ApiClient::responseReady, this, [this](const QString& route, int code, const QByteArray& body) {
        if (route == "/trade/manual/order" || route == "/trade/manual/cancel") {
            result_->setText(QString("%1 [%2]: %3").arg(route).arg(code).arg(QString::fromUtf8(body)));
        } else if (route == "/credentials/status" || route == "/credentials/update") {
            credStatus_->setText(QString("%1 [%2]: %3").arg(route).arg(code).arg(QString::fromUtf8(body)));
        }
    });

    applyTexts();
}

void TradingPage::setLanguage(AppLang lang) {
    lang_ = lang;
    applyTexts();
}

void TradingPage::applyTexts() {
    title_->setText(tr2(lang_, "手动交易（受风控约束）", "Manual Trading (risk-gated)"));
    orderBtn_->setText(tr2(lang_, "提交下单", "Submit Order"));
    cancelBtn_->setText(tr2(lang_, "撤销内部订单", "Cancel Internal Order"));
    credSaveBtn_->setText(tr2(lang_, "保存凭证", "Save Credentials"));
    credCheckBtn_->setText(tr2(lang_, "检查凭证状态", "Check Credential Status"));

    int r = 0;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "交易对", "Symbol"));
    ++r;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "方向(BUY/SELL)", "Side(BUY/SELL)"));
    ++r;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "类型(MARKET/LIMIT)", "Type(MARKET/LIMIT)"));
    ++r;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "数量", "Qty"));
    ++r;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "价格(限价)", "Price(limit)"));
    ++r;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "内部订单ID", "Internal Order ID"));
    ++r;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "API Key", "API Key"));
    ++r;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "Secret Key", "Secret Key"));
}