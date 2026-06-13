#include "account_panel.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QCheckBox>

AccountPanel::AccountPanel(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);

    // Header
    auto *header = new QHBoxLayout;
    auto *title = new QLabel("Account Dashboard");
    title->setStyleSheet("font-size: 14px; font-weight: bold; padding: 4px;");
    m_summaryLabel = new QLabel("");
    m_summaryLabel->setStyleSheet("color: #888; padding: 4px;");
    m_batchTradeBtn = new QPushButton("Batch Trade (Selected)");
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
    header->addWidget(m_batchTradeBtn);
    header->addWidget(m_addBtn);
    layout->addLayout(header);

    // Account table
    m_table = new QTableWidget(0, 6);
    m_table->setHorizontalHeaderLabels({"☑", "Name", "Broker", "CTP Address", "User ID", "Enabled"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setStyleSheet(
        "QTableWidget { background-color: #252525; color: #ccc; gridline-color: #333; }"
        "QHeaderView::section { background-color: #333; color: #ccc; padding: 4px; }");
    layout->addWidget(m_table);

    connect(m_batchTradeBtn, &QPushButton::clicked, [this]() {
        std::vector<int64_t> ids;
        for (int r = 0; r < m_table->rowCount(); ++r) {
            auto *cb = qobject_cast<QCheckBox*>(m_table->cellWidget(r, 0));
            if (cb && cb->isChecked()) {
                ids.push_back(m_table->item(r, 1)->data(Qt::UserRole).toLongLong());
            }
        }
        emit batchTradeRequested(ids);
    });
}

void AccountPanel::loadAccounts(qp::data::Database &db) {
    (void)db;
    m_table->setRowCount(0);
    m_summaryLabel->setText("0 accounts loaded. Add accounts in Phase 5 (CTP integration).");
}
