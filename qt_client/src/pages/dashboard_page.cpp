#include "dashboard_page.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QFrame>
#include <QTableWidget>
#include <QTreeWidget>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QColor>
#include <QComboBox>
#include <QFileDialog>
#include <QFile>

DashboardPage::DashboardPage(ApiClient* api, QWidget* parent) : QWidget(parent), api_(api) {
    auto* layout = new QVBoxLayout(this);
    auto* btnRow = new QHBoxLayout();
    healthBtn_ = new QPushButton(this);
    runtimeBtn_ = new QPushButton(this);
    configBtn_ = new QPushButton(this);
    exportBtn_ = new QPushButton(this);
    groupFilter_ = new QComboBox(this);
    groupFilter_->addItems({"all","engine","risk","execution","logging","runtime","strategy"});
    statusDot_ = new QFrame(this);
    statusDot_->setFixedSize(12, 12);
    statusDot_->setStyleSheet("background:#c0392b;border-radius:6px;");
    statusText_ = new QLabel(this);
    latencyText_ = new QLabel(this);

    btnRow->addWidget(healthBtn_);
    btnRow->addWidget(runtimeBtn_);
    btnRow->addWidget(configBtn_);
    btnRow->addWidget(groupFilter_);
    btnRow->addWidget(exportBtn_);
    btnRow->addWidget(statusDot_);
    btnRow->addWidget(statusText_);
    btnRow->addWidget(latencyText_);
    btnRow->addStretch();

    healthTitle_ = new QLabel(this);
    runtimeTitle_ = new QLabel(this);
    configTitle_ = new QLabel(this);
    healthLabel_ = new QLabel(this);
    runtimeLabel_ = new QLabel(this);
    configLabel_ = new QLabel(this);
    configTable_ = new QTableWidget(0, 3, this);
    configTable_->horizontalHeader()->setStretchLastSection(true);
    configTable_->setVisible(false);

    configTree_ = new QTreeWidget(this);
    configTree_->setColumnCount(2);
    configTree_->setRootIsDecorated(true);
    configTree_->setAlternatingRowColors(true);
    banner_ = new QLabel(this);
    banner_->setVisible(false);
    banner_->setStyleSheet("background:#f1c40f;color:#2c3e50;padding:6px;border-radius:4px;");
    healthLabel_->setWordWrap(true);
    runtimeLabel_->setWordWrap(true);
    configLabel_->setWordWrap(true);
    configLabel_->setVisible(false);

    layout->addLayout(btnRow);
    layout->addWidget(banner_);
    layout->addWidget(healthTitle_);
    layout->addWidget(healthLabel_);
    layout->addWidget(runtimeTitle_);
    layout->addWidget(runtimeLabel_);
    layout->addWidget(configTitle_);
    layout->addWidget(configLabel_);
    layout->addWidget(configTable_);
    layout->addWidget(configTree_);
    layout->addStretch();

    connect(healthBtn_, &QPushButton::clicked, this, [this]() { api_->getHealth(); });
    connect(runtimeBtn_, &QPushButton::clicked, this, [this]() { api_->getRuntime(); });
    connect(configBtn_, &QPushButton::clicked, this, [this]() { api_->getEffectiveConfig(); });
    connect(exportBtn_, &QPushButton::clicked, this, [this]() {
        if (lastConfigRaw_.isEmpty()) return;
        const auto path = QFileDialog::getSaveFileName(this, tr2(lang_, "导出生效配置", "Export Effective Config"), "effective_config.json", "JSON (*.json)");
        if (path.isEmpty()) return;
        QFile f(path);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(lastConfigRaw_);
            f.close();
        }
    });

    connect(api_, &ApiClient::responseReady, this, [this](const QString& route, int code, const QByteArray& body) {
        if (route == "/health") {
            healthLabel_->setText(QString("[%1] %2").arg(code).arg(QString::fromUtf8(body)));
        } else if (route == "/strategy/grid/runtime") {
            runtimeLabel_->setText(QString("[%1] %2").arg(code).arg(QString::fromUtf8(body)));
        } else if (route == "/config/effective") {
            configLabel_->setText(QString("[%1] %2").arg(code).arg(QString::fromUtf8(body)));
            lastConfigRaw_ = body;

            configTable_->setRowCount(0);
            configTree_->clear();
            const QString groupSel = groupFilter_->currentText();
            const auto doc = QJsonDocument::fromJson(body);
            if (!doc.isNull() && doc.isObject()) {
                const auto root = doc.object();
                const QStringList groups{"engine", "risk", "execution", "logging", "runtime", "strategy"};
                for (const auto& g : groups) {
                    if (groupSel != "all" && groupSel != g) continue;
                    if (!root.contains(g) || !root.value(g).isObject()) continue;
                    const auto obj = root.value(g).toObject();

                    auto* groupNode = new QTreeWidgetItem(configTree_);
                    groupNode->setText(0, g);
                    groupNode->setText(1, "");

                    const bool highlightGroup = (g == "risk" || g == "execution");
                    if (highlightGroup) {
                        groupNode->setBackground(0, QColor("#fff3cd"));
                        groupNode->setBackground(1, QColor("#fff3cd"));
                    }

                    for (auto it = obj.begin(); it != obj.end(); ++it) {
                        auto* leaf = new QTreeWidgetItem(groupNode);
                        leaf->setText(0, it.key());
                        leaf->setText(1, it.value().toVariant().toString());

                        const bool isKeyRisk = (g == "risk") || (g == "execution" && it.key() == "dry_run");
                        if (isKeyRisk) {
                            leaf->setBackground(0, QColor("#fff3cd"));
                            leaf->setBackground(1, QColor("#fff3cd"));
                        }

                        // 兼容保留平铺表格逻辑（可隐藏调试）
                        const int r = configTable_->rowCount();
                        configTable_->insertRow(r);
                        configTable_->setItem(r, 0, new QTableWidgetItem(g));
                        configTable_->setItem(r, 1, new QTableWidgetItem(it.key()));
                        configTable_->setItem(r, 2, new QTableWidgetItem(it.value().toVariant().toString()));
                    }
                    groupNode->setExpanded(true);
                }
            }
        }
    });

    connect(api_, &ApiClient::requestMetric, this, [this](const QString& route, int, qint64 latencyMs, bool ok) {
        if (route == "/health" || route == "/strategy/grid/runtime" || route == "/config/effective") {
            statusDot_->setStyleSheet(ok ? "background:#27ae60;border-radius:6px;" : "background:#c0392b;border-radius:6px;");
            latencyText_->setText(tr2(lang_, "延迟: ", "Latency: ") + QString::number(latencyMs) + " ms");
            statusText_->setText(ok ? tr2(lang_, "已连接", "Connected") : tr2(lang_, "异常", "Error"));

            if (!ok) {
                banner_->setText(tr2(lang_, "连接异常：请检查核心服务是否运行在 127.0.0.1:8080", "Connection issue: ensure core service is running on 127.0.0.1:8080"));
                banner_->setVisible(true);
            } else {
                banner_->setVisible(false);
            }
        }
    });

    // 自动刷新（1秒）
    timer_ = new QTimer(this);
    timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout, this, [this]() {
        api_->getHealth();
        api_->getRuntime();
        static int n = 0;
        if ((++n % 5) == 0) api_->getEffectiveConfig();
    });
    timer_->start();

    connect(groupFilter_, &QComboBox::currentTextChanged, this, [this]() {
        api_->getEffectiveConfig();
    });

    applyTexts();
}

void DashboardPage::setLanguage(AppLang lang) {
    lang_ = lang;
    applyTexts();
}

void DashboardPage::applyTexts() {
    healthBtn_->setText(tr2(lang_, "刷新健康状态", "Refresh Health"));
    runtimeBtn_->setText(tr2(lang_, "刷新策略运行态", "Refresh Runtime"));
    configBtn_->setText(tr2(lang_, "刷新生效配置", "Refresh Effective Config"));
    exportBtn_->setText(tr2(lang_, "导出配置JSON", "Export Config JSON"));
    healthTitle_->setText(tr2(lang_, "健康接口返回：", "Health Response:"));
    runtimeTitle_->setText(tr2(lang_, "策略运行态返回：", "Runtime Response:"));
    configTitle_->setText(tr2(lang_, "生效配置（分组视图）：", "Effective Config (grouped view):"));
    configTable_->setHorizontalHeaderLabels(QStringList{tr2(lang_, "分组", "Group"), tr2(lang_, "键", "Key"), tr2(lang_, "值", "Value")});
    configTree_->setHeaderLabels(QStringList{tr2(lang_, "键/分组", "Key/Group"), tr2(lang_, "值", "Value")});
    if (statusText_->text().isEmpty()) statusText_->setText(tr2(lang_, "未连接", "Disconnected"));
    if (latencyText_->text().isEmpty()) latencyText_->setText(tr2(lang_, "延迟: -", "Latency: -"));
    if (healthLabel_->text().isEmpty()) healthLabel_->setText(tr2(lang_, "未加载", "not loaded"));
    if (runtimeLabel_->text().isEmpty()) runtimeLabel_->setText(tr2(lang_, "未加载", "not loaded"));
    if (configLabel_->text().isEmpty()) configLabel_->setText(tr2(lang_, "未加载", "not loaded"));
    groupFilter_->setToolTip(tr2(lang_, "按分组筛选（树形可展开/收起）", "Filter by group (tree expandable/collapsible)"));
}
