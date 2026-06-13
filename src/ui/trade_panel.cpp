#include "trade_panel.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QDateTime>

TradePanel::TradePanel(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);

    // ---- Header: P&L + controls ----
    auto *header = new QHBoxLayout;
    auto *title = new QLabel("Live Trading");
    title->setStyleSheet("font-size: 14px; font-weight: bold; padding: 4px;");
    m_pnlLabel = new QLabel("P&L: ¥0.00");
    m_pnlLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #26a65b; padding: 4px;");
    header->addWidget(title);
    header->addStretch();
    header->addWidget(m_pnlLabel);
    layout->addLayout(header);

    // ---- Order entry bar ----
    auto *orderBar = new QHBoxLayout;
    m_instrumentCombo = new QComboBox;
    m_instrumentCombo->addItems({"ag2506", "ag2512", "rb2510", "au2508"});
    m_instrumentCombo->setStyleSheet(
        "QComboBox { background: #2a2a2a; color: #ccc; border: 1px solid #444; padding: 4px; border-radius: 3px; }");

    m_actionCombo = new QComboBox;
    m_actionCombo->addItems({"Buy (Open Long)", "Sell (Close Long)",
                              "Sell Short", "Buy to Cover"});
    m_actionCombo->setStyleSheet(m_instrumentCombo->styleSheet());

    m_priceSpin = new QDoubleSpinBox;
    m_priceSpin->setRange(0, 999999);
    m_priceSpin->setValue(5200);
    m_priceSpin->setDecimals(0);
    m_priceSpin->setStyleSheet("QDoubleSpinBox { background: #2a2a2a; color: #ccc; border: 1px solid #444; padding: 4px; }");

    m_volumeSpin = new QSpinBox;
    m_volumeSpin->setRange(1, 100);
    m_volumeSpin->setValue(1);
    m_volumeSpin->setStyleSheet(m_priceSpin->styleSheet());

    m_sendBtn = new QPushButton("Send Order");
    m_sendBtn->setStyleSheet(
        "QPushButton { background-color: #c0392b; color: white; border: none; "
        "border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #e74c3c; }");

    orderBar->addWidget(new QLabel("Instrument:"));
    orderBar->addWidget(m_instrumentCombo);
    orderBar->addWidget(new QLabel("Action:"));
    orderBar->addWidget(m_actionCombo);
    orderBar->addWidget(new QLabel("Price:"));
    orderBar->addWidget(m_priceSpin);
    orderBar->addWidget(new QLabel("Vol:"));
    orderBar->addWidget(m_volumeSpin);
    orderBar->addWidget(m_sendBtn);
    layout->addLayout(orderBar);

    // ---- Position table ----
    m_positionTable = new QTableWidget(0, 7);
    m_positionTable->setHorizontalHeaderLabels(
        {"Instrument", "Direction", "Qty", "Avg Cost", "Last Price", "P&L", "Margin"});
    m_positionTable->horizontalHeader()->setStretchLastSection(true);
    m_positionTable->setStyleSheet(
        "QTableWidget { background-color: #252525; color: #ccc; gridline-color: #333; }"
        "QHeaderView::section { background-color: #333; color: #ccc; padding: 4px; }");
    auto *posGroup = new QGroupBox("Positions");
    auto *pgLayout = new QVBoxLayout(posGroup);
    pgLayout->addWidget(m_positionTable);
    layout->addWidget(posGroup);

    // ---- Order log ----
    m_orderTable = new QTableWidget(0, 6);
    m_orderTable->setHorizontalHeaderLabels({"Time", "Instrument", "Direction", "Price", "Vol", "Status"});
    m_orderTable->horizontalHeader()->setStretchLastSection(true);
    m_orderTable->setStyleSheet(m_positionTable->styleSheet());
    auto *ordGroup = new QGroupBox("Orders");
    auto *ogLayout = new QVBoxLayout(ordGroup);
    ogLayout->addWidget(m_orderTable);
    layout->addWidget(ordGroup);

    // ---- Log ----
    m_logOutput = new QTextEdit;
    m_logOutput->setReadOnly(true);
    m_logOutput->setMaximumHeight(80);
    m_logOutput->setStyleSheet(
        "QTextEdit { background-color: #1e1e1e; color: #888; border: 1px solid #333; "
        "border-radius: 4px; font-family: Consolas; font-size: 10px; }");
    layout->addWidget(m_logOutput);

    // ---- Wire send button ----
    connect(m_sendBtn, &QPushButton::clicked, [this]() {
        qp::data::OrderAction action;
        switch (m_actionCombo->currentIndex()) {
        case 0: action = qp::data::OrderAction::OpenLong;    break;
        case 1: action = qp::data::OrderAction::CloseLong;   break;
        case 2: action = qp::data::OrderAction::OpenShort;   break;
        case 3: action = qp::data::OrderAction::CloseShort;  break;
        default: return;
        }
        emit sendOrderRequested(m_instrumentCombo->currentText(), action,
                                m_priceSpin->value(), m_volumeSpin->value());
    });
}

void TradePanel::updatePosition(const qp::ctp::InvestorPosition &pos) {
    m_positionTable->setRowCount(0);
    int r = m_positionTable->rowCount();
    m_positionTable->insertRow(r);
    QString dir = (pos.PosiDirection == '2') ? "Long" :
                  (pos.PosiDirection == '3') ? "Short" : "Net";
    m_positionTable->setItem(r, 0, new QTableWidgetItem(pos.InstrumentID));
    m_positionTable->setItem(r, 1, new QTableWidgetItem(dir));
    m_positionTable->setItem(r, 2, new QTableWidgetItem(QString::number(pos.Position)));
    m_positionTable->setItem(r, 3, new QTableWidgetItem(QString::number(pos.OpenCost, 'f', 1)));
    m_positionTable->setItem(r, 4, new QTableWidgetItem(QString::number(pos.LastPrice, 'f', 1)));
    m_positionTable->setItem(r, 5, new QTableWidgetItem(
        QString::number(pos.PositionProfit, 'f', 2)));

    double margin = pos.UseMargin;
    m_positionTable->setItem(r, 6, new QTableWidgetItem(QString::number(margin, 'f', 2)));

    // Color P&L
    QColor pnlColor = pos.PositionProfit >= 0 ? QColor("#26a65b") : QColor("#e74c3c");
    m_positionTable->item(r, 5)->setForeground(pnlColor);

    m_pnlLabel->setText(QString("P&L: ¥%1").arg(pos.PositionProfit, 0, 'f', 2));
    m_pnlLabel->setStyleSheet(
        QString("font-size: 20px; font-weight: bold; color: %1; padding: 4px;")
        .arg(pos.PositionProfit >= 0 ? "#26a65b" : "#e74c3c"));
}

void TradePanel::updateOrder(const qp::ctp::Order &order) {
    int r = m_orderTable->rowCount();
    m_orderTable->insertRow(r);
    m_orderTable->setItem(r, 0, new QTableWidgetItem(
        QDateTime::currentDateTime().toString("HH:mm:ss")));
    m_orderTable->setItem(r, 1, new QTableWidgetItem(order.InstrumentID));
    QString dir = (order.Direction == '0') ? "Buy" : "Sell";
    if (order.CombOffsetFlag[0] == '0') dir += " (Open)"; else dir += " (Close)";
    m_orderTable->setItem(r, 2, new QTableWidgetItem(dir));
    m_orderTable->setItem(r, 3, new QTableWidgetItem(QString::number(order.LimitPrice, 'f', 1)));
    m_orderTable->setItem(r, 4, new QTableWidgetItem(QString::number(order.VolumeTotalOriginal)));
    QString status = (order.OrderStatus == '2') ? "Pending" :
                     (order.OrderStatus == '0') ? "Filled" : "Unknown";
    m_orderTable->setItem(r, 5, new QTableWidgetItem(status));
}

void TradePanel::logMessage(const QString &msg) {
    m_logOutput->append(QString("[%1] %2")
        .arg(QDateTime::currentDateTime().toString("HH:mm:ss"), msg));
}
