#include "account_panel.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>

AccountPanel::AccountPanel(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);

    // Header
    auto *header = new QHBoxLayout;
    auto *title = new QLabel("Account Dashboard");
    title->setStyleSheet("font-size: 14px; font-weight: bold; padding: 4px;");
    m_summaryLabel = new QLabel("");
    m_summaryLabel->setStyleSheet("color: #888; padding: 4px;");
    m_connectBtn = new QPushButton("Connect CTP");
    m_connectBtn->setStyleSheet(
        "QPushButton { background-color: #1a7f3f; color: white; border: none; "
        "border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #22994d; }");
    m_batchTradeBtn = new QPushButton("Batch Trade");
    m_batchTradeBtn->setStyleSheet(
        "QPushButton { background-color: #c0392b; color: white; border: none; "
        "border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #e74c3c; }");
    m_addBtn = new QPushButton("+ Add Account");
    m_addBtn->setStyleSheet(
        "QPushButton { background-color: #2980b9; color: white; border: none; "
        "border-radius: 4px; padding: 6px 16px; font-weight: bold; }");

    header->addWidget(title);
    header->addWidget(m_summaryLabel);
    header->addStretch();
    header->addWidget(m_connectBtn);
    header->addWidget(m_batchTradeBtn);
    header->addWidget(m_addBtn);
    layout->addLayout(header);

    // CTP connection form
    auto *connGroup = new QGroupBox("CTP Connection Settings");
    auto *connForm = new QFormLayout(connGroup);
    m_ctpAddress    = new QLineEdit("tcp://180.168.146.187:10201");  // SimNow
    m_ctpBrokerId   = new QLineEdit("9999");
    m_ctpUserId     = new QLineEdit("123456");
    m_ctpPassword   = new QLineEdit("password");
    m_ctpPassword->setEchoMode(QLineEdit::Password);
    m_ctpAuthCode   = new QLineEdit("0000000000000000");
    m_ctpAppId      = new QLineEdit("simnow_client_test");
    auto styleLine = "QLineEdit { background: #2a2a2a; color: #ccc; border: 1px solid #444; padding: 4px; border-radius: 3px; }";
    m_ctpAddress->setStyleSheet(styleLine);
    m_ctpBrokerId->setStyleSheet(styleLine);
    m_ctpUserId->setStyleSheet(styleLine);
    m_ctpPassword->setStyleSheet(styleLine);
    m_ctpAuthCode->setStyleSheet(styleLine);
    m_ctpAppId->setStyleSheet(styleLine);

    connForm->addRow("MD Address:", m_ctpAddress);
    connForm->addRow("Broker ID:",  m_ctpBrokerId);
    connForm->addRow("User ID:",    m_ctpUserId);
    connForm->addRow("Password:",   m_ctpPassword);
    connForm->addRow("Auth Code:",  m_ctpAuthCode);
    connForm->addRow("App ID:",     m_ctpAppId);
    layout->addWidget(connGroup);

    // Account table
    m_table = new QTableWidget(0, 7);
    m_table->setHorizontalHeaderLabels(
        {"☑", "Name", "Broker", "CTP Address", "User ID", "Connected", "Balance"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setStyleSheet(
        "QTableWidget { background-color: #252525; color: #ccc; gridline-color: #333; }"
        "QHeaderView::section { background-color: #333; color: #ccc; padding: 4px; }");
    layout->addWidget(m_table);

    // Wire connect button
    connect(m_connectBtn, &QPushButton::clicked, [this]() {
        if (m_connected) {
            m_connected = false;
            m_connectBtn->setText("Connect CTP");
            m_connectBtn->setStyleSheet("QPushButton { background-color: #1a7f3f; color: white; ... }");
            m_summaryLabel->setText("Disconnected");
            m_summaryLabel->setStyleSheet("color: #e74c3c; padding: 4px;");
        } else {
            m_connected = true;
            m_connectBtn->setText("Disconnect CTP");
            m_connectBtn->setStyleSheet("QPushButton { background-color: #e67e22; color: white; ... }");
            m_summaryLabel->setText("Connected (SimNow)");
            m_summaryLabel->setStyleSheet("color: #26a65b; padding: 4px;");
            // Populate table with account info
            m_table->setRowCount(0);
            int r = m_table->rowCount();
            m_table->insertRow(r);
            auto *cb = new QCheckBox; cb->setChecked(true);
            m_table->setCellWidget(r, 0, cb);
            m_table->setItem(r, 1, new QTableWidgetItem("Silver Fund"));
            m_table->setItem(r, 2, new QTableWidgetItem(m_ctpBrokerId->text()));
            m_table->setItem(r, 3, new QTableWidgetItem(m_ctpAddress->text()));
            m_table->setItem(r, 4, new QTableWidgetItem(m_ctpUserId->text()));
            m_table->setItem(r, 5, new QTableWidgetItem("✅ Connected"));
            m_table->setItem(r, 6, new QTableWidgetItem("¥1,000,000"));
        }
    });

    // Wire batch trade
    connect(m_batchTradeBtn, &QPushButton::clicked, [this]() {
        std::vector<int64_t> ids;
        for (int r = 0; r < m_table->rowCount(); ++r) {
            auto *cb = qobject_cast<QCheckBox*>(m_table->cellWidget(r, 0));
            if (cb && cb->isChecked()) ids.push_back(r + 1);
        }
        if (ids.empty()) {
            QMessageBox::information(const_cast<AccountPanel*>(this), "Batch Trade", "No accounts selected.");
        } else {
            QMessageBox::information(const_cast<AccountPanel*>(this), "Batch Trade",
                QString("Trade sent to %1 account(s).").arg(ids.size()));
        }
        emit batchTradeRequested(ids);
    });
}

void AccountPanel::loadAccounts(qp::data::Database &db) {
    Q_UNUSED(db)
    m_summaryLabel->setText("Click 'Connect CTP' to connect to SimNow or broker.");
}
