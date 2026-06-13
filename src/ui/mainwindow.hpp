#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QVector>
#include "chart_panel.hpp"
#include "strategy_panel.hpp"
#include "backtest_panel.hpp"
#include "trade_panel.hpp"
#include "account_panel.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

    ChartPanel    *chartPanel()    { return m_chartPanel; }
    StrategyPanel *strategyPanel() { return m_strategyPanel; }
    BacktestPanel *backtestPanel() { return m_backtestPanel; }
    TradePanel    *tradePanel()    { return m_tradePanel; }
    AccountPanel  *accountPanel()  { return m_accountPanel; }

private:
    void setupTheme();
    void switchPage(int index);

    QStackedWidget *m_stack;
    ChartPanel     *m_chartPanel;
    StrategyPanel  *m_strategyPanel;
    BacktestPanel  *m_backtestPanel;
    TradePanel     *m_tradePanel;
    AccountPanel   *m_accountPanel;
    QVector<QPushButton*> m_navBtns;
};
