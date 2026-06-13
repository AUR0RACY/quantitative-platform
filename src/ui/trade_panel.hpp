#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTextEdit>
#include "data/ctp/ctp_types.hpp"
#include "data/ctp/trader_adapter.hpp"

class TradePanel : public QWidget {
    Q_OBJECT
public:
    explicit TradePanel(QWidget *parent = nullptr);

    void updatePosition(const ::qp::ctp::InvestorPosition &pos);
    void updateOrder(const ::qp::ctp::Order &order);
    void logMessage(const QString &msg);

    void setTraderAdapter(qp::data::TraderAdapter *adapter) { m_trader = adapter; }

signals:
    void sendOrderRequested(const QString &instrument, qp::data::OrderAction action,
                            double price, int volume);

private:
    QTableWidget   *m_positionTable;
    QTableWidget   *m_orderTable;
    QComboBox      *m_instrumentCombo;
    QComboBox      *m_actionCombo;
    QDoubleSpinBox *m_priceSpin;
    QSpinBox       *m_volumeSpin;
    QPushButton    *m_sendBtn;
    QLabel         *m_pnlLabel;
    QTextEdit      *m_logOutput;
    qp::data::TraderAdapter *m_trader = nullptr;
};
