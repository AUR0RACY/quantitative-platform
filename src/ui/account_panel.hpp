#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
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
    QPushButton  *m_connectBtn;
    QLabel       *m_summaryLabel;
    QLineEdit    *m_ctpAddress;
    QLineEdit    *m_ctpBrokerId;
    QLineEdit    *m_ctpUserId;
    QLineEdit    *m_ctpPassword;
    QLineEdit    *m_ctpAuthCode;
    QLineEdit    *m_ctpAppId;
    bool m_connected = false;
};
