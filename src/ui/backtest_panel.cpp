#include "backtest_panel.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>

BacktestPanel::BacktestPanel(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);

    // --- Header ---
    auto *header = new QHBoxLayout;
    auto *title = new QLabel("Backtest");
    title->setStyleSheet("font-size: 14px; font-weight: bold; padding: 4px;");
    m_runBtn = new QPushButton("Run Backtest");
    m_runBtn->setStyleSheet(
        "QPushButton { background-color: #1a7f3f; color: white; border: none; "
        "border-radius: 4px; padding: 6px 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #22994d; }");
    m_statusLabel = new QLabel("");
    header->addWidget(title);
    header->addStretch();
    header->addWidget(m_statusLabel);
    header->addWidget(m_runBtn);
    layout->addLayout(header);

    // --- Metrics table ---
    m_metricsTable = new QTableWidget(0, 2);
    m_metricsTable->setHorizontalHeaderLabels({"Metric", "Value"});
    m_metricsTable->horizontalHeader()->setStretchLastSection(true);
    m_metricsTable->setStyleSheet(
        "QTableWidget { background-color: #252525; color: #ccc; gridline-color: #333; }"
        "QHeaderView::section { background-color: #333; color: #ccc; padding: 4px; }");

    auto *metricsGroup = new QGroupBox("Performance Metrics");
    auto *mgLayout = new QVBoxLayout(metricsGroup);
    mgLayout->addWidget(m_metricsTable);
    layout->addWidget(metricsGroup);

    // --- Log output ---
    m_logOutput = new QTextEdit;
    m_logOutput->setReadOnly(true);
    m_logOutput->setMaximumHeight(120);
    m_logOutput->setStyleSheet(
        "QTextEdit { background-color: #1e1e1e; color: #aaa; border: 1px solid #333; "
        "border-radius: 4px; font-family: Consolas; font-size: 11px; }");
    auto *logGroup = new QGroupBox("Log");
    auto *lgLayout = new QVBoxLayout(logGroup);
    lgLayout->addWidget(m_logOutput);
    layout->addWidget(logGroup);

    connect(m_runBtn, &QPushButton::clicked, this, &BacktestPanel::runRequested);
}

void BacktestPanel::setReport(const QString &strategy, const QString &instrument,
                               double totalReturn, double maxDD, double sharpe,
                               double calmar, int totalTrades, double winRate,
                               double avgWin, double avgLoss, double profitFactor) {
    m_metricsTable->setRowCount(0);
    auto addRow = [&](const QString &k, const QString &v) {
        int r = m_metricsTable->rowCount();
        m_metricsTable->insertRow(r);
        m_metricsTable->setItem(r, 0, new QTableWidgetItem(k));
        m_metricsTable->setItem(r, 1, new QTableWidgetItem(v));
    };
    addRow("Strategy", strategy);
    addRow("Instrument", instrument);
    addRow("Total Return", QString::number(totalReturn, 'f', 2) + "%");
    addRow("Max Drawdown", QString::number(maxDD, 'f', 2) + "%");
    addRow("Sharpe Ratio", QString::number(sharpe, 'f', 3));
    addRow("Calmar Ratio", QString::number(calmar, 'f', 3));
    addRow("Total Trades", QString::number(totalTrades));
    addRow("Win Rate", QString::number(winRate, 'f', 1) + "%");
    addRow("Avg Win", QString::number(avgWin, 'f', 2));
    addRow("Avg Loss", QString::number(avgLoss, 'f', 2));
    addRow("Profit Factor", QString::number(profitFactor, 'f', 2));

    m_statusLabel->setText("Complete");
    m_statusLabel->setStyleSheet("color: #26a65b; font-weight: bold; padding: 4px;");
    m_logOutput->append(QString("[Backtest] %1 on %2 — Return: %3%, Sharpe: %4")
        .arg(strategy, instrument)
        .arg(totalReturn, 0, 'f', 2)
        .arg(sharpe, 0, 'f', 3));
}
