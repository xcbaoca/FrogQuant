#include "strategy_page.hpp"

#include <QCheckBox>
#include <QFormLayout>
#include <QJsonObject>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QMessageBox>

StrategyPage::StrategyPage(ApiClient* api, QWidget* parent) : QWidget(parent), api_(api) {
    auto* root = new QVBoxLayout(this);
    form_ = new QFormLayout();

    title_ = new QLabel(this);

    levels_ = new QLineEdit("8", this);
    qty_ = new QLineEdit("0.001", this);
    tp_ = new QLineEdit("0.002", this);
    recenter_ = new QLineEdit("0.004", this);
    band_ = new QLineEdit("0.06", this);

    levels_->setValidator(new QIntValidator(2, 200, levels_));
    auto* qtyVal = new QDoubleValidator(0.0, 1000.0, 8, qty_);
    qtyVal->setNotation(QDoubleValidator::StandardNotation);
    qty_->setValidator(qtyVal);
    auto* tpVal = new QDoubleValidator(0.0, 0.2, 8, tp_);
    tpVal->setNotation(QDoubleValidator::StandardNotation);
    tp_->setValidator(tpVal);
    auto* recVal = new QDoubleValidator(0.0, 0.2, 8, recenter_);
    recVal->setNotation(QDoubleValidator::StandardNotation);
    recenter_->setValidator(recVal);
    auto* bandVal = new QDoubleValidator(0.002, 1.0, 8, band_);
    bandVal->setNotation(QDoubleValidator::StandardNotation);
    band_->setValidator(bandVal);

    auto* riskNVal = new QDoubleValidator(0.000001, 1000000000.0, 8, riskNotional_);
    riskNVal->setNotation(QDoubleValidator::StandardNotation);
    riskNotional_->setValidator(riskNVal);
    auto* ratioVal = new QDoubleValidator(0.000001, 1.0, 8, riskPosRatio_);
    ratioVal->setNotation(QDoubleValidator::StandardNotation);
    riskPosRatio_->setValidator(ratioVal);
    auto* lossVal = new QDoubleValidator(0.000001, 1.0, 8, riskLossRatio_);
    lossVal->setNotation(QDoubleValidator::StandardNotation);
    riskLossRatio_->setValidator(lossVal);
    enabled_ = new QCheckBox(this);
    enabled_->setChecked(true);

    riskNotional_ = new QLineEdit("10.0", this);
    riskPosRatio_ = new QLineEdit("0.8", this);
    riskLossRatio_ = new QLineEdit("0.05", this);

    form_->addRow("levels", levels_);
    form_->addRow("order_qty", qty_);
    form_->addRow("take_profit_ratio", tp_);
    form_->addRow("recenter_threshold_ratio", recenter_);
    form_->addRow("band_ratio", band_);
    form_->addRow(enabled_);
    form_->addRow("risk.max_single_order_notional_usd", riskNotional_);
    form_->addRow("risk.max_position_ratio", riskPosRatio_);
    form_->addRow("risk.daily_loss_stop_ratio", riskLossRatio_);

    applyBtn_ = new QPushButton(this);
    enableBtn_ = new QPushButton(this);
    refreshBtn_ = new QPushButton(this);
    resetBtn_ = new QPushButton(this);
    riskApplyBtn_ = new QPushButton(this);
    riskRefreshBtn_ = new QPushButton(this);
    result_ = new QLabel("result: -", this);
    result_->setWordWrap(true);

    root->addWidget(title_);
    root->addLayout(form_);
    root->addWidget(applyBtn_);
    root->addWidget(enableBtn_);
    root->addWidget(refreshBtn_);
    root->addWidget(resetBtn_);
    root->addWidget(riskApplyBtn_);
    root->addWidget(riskRefreshBtn_);
    root->addWidget(result_);
    root->addStretch();

    connect(applyBtn_, &QPushButton::clicked, this, [this]() {
        const int lv = levels_->text().toInt();
        const double q = qty_->text().toDouble();
        const double tp = tp_->text().toDouble();
        const double rc = recenter_->text().toDouble();
        const double bd = band_->text().toDouble();

        const bool ok = (lv >= 2 && lv <= 200) && (q > 0.0 && q <= 1000.0) && (tp > 0.0 && tp <= 0.2) &&
                        (rc > 0.0 && rc <= 0.2) && (bd > 0.002 && bd <= 1.0);
        if (!ok) {
            QMessageBox::warning(this,
                                 tr2(lang_, "参数校验失败", "Validation Failed"),
                                 tr2(lang_, "请检查参数范围（levels/qty/tp/recenter/band）。", "Please check parameter ranges (levels/qty/tp/recenter/band)."));
            return;
        }

        const auto ret = QMessageBox::question(this,
                                               tr2(lang_, "确认应用", "Confirm Apply"),
                                               tr2(lang_, "确定要应用新的网格参数吗？", "Apply new grid parameters?"));
        if (ret != QMessageBox::Yes) return;

        QJsonObject obj;
        obj["levels"] = lv;
        obj["order_qty"] = q;
        obj["take_profit_ratio"] = tp;
        obj["recenter_threshold_ratio"] = rc;
        obj["band_ratio"] = bd;
        api_->postGridConfig(obj);
    });

    connect(enableBtn_, &QPushButton::clicked, this, [this]() {
        const auto ret = QMessageBox::question(this,
                                               tr2(lang_, "确认启停", "Confirm Toggle"),
                                               tr2(lang_, "确定要更新策略启停状态吗？", "Update strategy enable state?"));
        if (ret != QMessageBox::Yes) return;
        api_->postGridEnable(enabled_->isChecked());
    });
    connect(refreshBtn_, &QPushButton::clicked, this, [this]() { api_->getRuntime(); });
    connect(resetBtn_, &QPushButton::clicked, this, [this]() {
        levels_->setText("8");
        qty_->setText("0.001");
        tp_->setText("0.002");
        recenter_->setText("0.004");
        band_->setText("0.06");
        enabled_->setChecked(true);
        riskNotional_->setText("10.0");
        riskPosRatio_->setText("0.8");
        riskLossRatio_->setText("0.05");
    });

    connect(riskApplyBtn_, &QPushButton::clicked, this, [this]() {
        QJsonObject obj;
        obj["max_single_order_notional_usd"] = riskNotional_->text().toDouble();
        obj["max_position_ratio"] = riskPosRatio_->text().toDouble();
        obj["daily_loss_stop_ratio"] = riskLossRatio_->text().toDouble();
        api_->postRiskConfig(obj);
    });
    connect(riskRefreshBtn_, &QPushButton::clicked, this, [this]() { api_->getRiskConfig(); });

    connect(api_, &ApiClient::responseReady, this, [this](const QString& route, int code, const QByteArray& body) {
        if (route == "/strategy/grid/config" || route == "/strategy/grid/enable" || route == "/strategy/grid/runtime" ||
            route == "/risk/config") {
            result_->setText(QString("%1 [%2]: %3").arg(route).arg(code).arg(QString::fromUtf8(body)));
            if (route == "/risk/config" && code >= 200 && code < 300) {
                const auto doc = QJsonDocument::fromJson(body);
                if (!doc.isNull() && doc.isObject()) {
                    const auto o = doc.object();
                    if (o.contains("max_single_order_notional_usd")) riskNotional_->setText(QString::number(o.value("max_single_order_notional_usd").toDouble()));
                    if (o.contains("max_position_ratio")) riskPosRatio_->setText(QString::number(o.value("max_position_ratio").toDouble()));
                    if (o.contains("daily_loss_stop_ratio")) riskLossRatio_->setText(QString::number(o.value("daily_loss_stop_ratio").toDouble()));
                }
            }
        }
    });

    timer_ = new QTimer(this);
    timer_->setInterval(1200);
    connect(timer_, &QTimer::timeout, this, [this]() {
        api_->getRuntime();
        static int n = 0;
        if ((++n % 5) == 0) api_->getRiskConfig();
    });
    timer_->start();

    applyTexts();
}

void StrategyPage::setLanguage(AppLang lang) {
    lang_ = lang;
    applyTexts();
}

void StrategyPage::applyTexts() {
    title_->setText(tr2(lang_, "动态网格参数（支持运行时更新）", "Dynamic Grid Parameters (runtime updatable)"));
    enabled_->setText(tr2(lang_, "启用策略", "Strategy Enabled"));
    applyBtn_->setText(tr2(lang_, "应用参数", "Apply Config"));
    enableBtn_->setText(tr2(lang_, "应用启停", "Apply Enable"));
    refreshBtn_->setText(tr2(lang_, "刷新运行态", "Refresh Runtime"));
    resetBtn_->setText(tr2(lang_, "恢复默认参数", "Reset Defaults"));
    riskApplyBtn_->setText(tr2(lang_, "应用风控参数", "Apply Risk Config"));
    riskRefreshBtn_->setText(tr2(lang_, "刷新风控参数", "Refresh Risk Config"));

    int r = 0;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "层数", "Levels"));
    ++r;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "每单数量", "Order Qty"));
    ++r;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "止盈比例", "Take Profit Ratio"));
    ++r;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "重心重建阈值", "Recenter Threshold"));
    ++r;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "带宽比例", "Band Ratio"));
    ++r;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "单笔最大名义金额", "Max Single Order Notional"));
    ++r;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "最大持仓比例", "Max Position Ratio"));
    ++r;
    if (auto* lab = form_->itemAt(r, QFormLayout::LabelRole); lab && lab->widget()) static_cast<QLabel*>(lab->widget())->setText(tr2(lang_, "日亏损熔断比例", "Daily Loss Stop Ratio"));
}
