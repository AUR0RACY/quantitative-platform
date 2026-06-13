#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>

class BacktestPanel : public QWidget {
    Q_OBJECT
public:
    explicit BacktestPanel(QWidget *parent = nullptr);

    void setReport(const QString &strategy, const QString &instrument,
                   double totalReturn, double maxDD, double sharpe,
                   double calmar, int totalTrades, double winRate,
                   double avgWin, double avgLoss, double profitFactor);

signals:
    void runRequested();

private:
    QTableWidget *m_metricsTable;
    QLabel       *m_statusLabel;
    QPushButton  *m_runBtn;
    QTextEdit    *m_logOutput;
};
