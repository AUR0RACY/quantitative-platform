#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include "data/database.hpp"

class AccountPanel : public QWidget {
    Q_OBJECT
public:
    explicit AccountPanel(QWidget *parent = nullptr);
    void loadAccounts(qp::data::Database &db);

signals:
    void batchTradeRequested(const std::vector<int64_t> &accountIds);

private:
    QTableWidget *m_table;
    QPushButton  *m_addBtn;
    QPushButton  *m_batchTradeBtn;
    QLabel       *m_summaryLabel;
};
