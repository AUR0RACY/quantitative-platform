#include <QApplication>
#include <QMessageBox>
#include "ui/mainwindow.hpp"
#include "ui/account_panel.hpp"
#include "engine/backtest_engine.hpp"
#include "data/database.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Quantitative Platform");
    app.setApplicationVersion("0.4.0");

    // ---- Initialize database ----
    try {
        qp::data::Database db("quant.db");
        db.migrate();
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Database Error", e.what());
        return 1;
    }

    MainWindow window;

    // ---- Wire: Strategy compile → backtest ----
    window.strategyPanel()->setCompileCallback([&window](const QString &code) {
        Q_UNUSED(code)
        std::cout << "[Compile] Strategy code saved (DLL build via CMake required for now)."
                  << std::endl;
    });

    // ---- Wire: Backtest run ----
    QObject::connect(window.backtestPanel(), &BacktestPanel::runRequested, [&window]() {
        try {
            qp::engine::BacktestEngine engine;
            std::string dllPath = "strategies/Debug/sma_crossover.dll";
            engine.load_strategy(dllPath);

            // Generate demo bars
            std::vector<qp::Bar> bars;
            double price = 5200.0;
            auto now = std::chrono::system_clock::now();
            for (int i = 0; i < 200; ++i) {
                price += std::sin(i * 0.3) * 20.0;
                if (price < 5000) price = 5000;
                qp::Bar b;
                b.time = now + std::chrono::hours(i);
                b.open  = price;
                b.high  = price + 30;
                b.low   = price - 30;
                b.close = price + 15;
                b.volume = 10000;
                bars.push_back(b);
            }

            auto report = engine.run(bars, "SMA Crossover", "AG2506",
                                      15, 1000000.0, 15.0, 1.0, 1.0);

            window.backtestPanel()->setReport(
                QString::fromStdString(report.strategy_name),
                QString::fromStdString(report.instrument),
                report.total_return_pct,
                report.max_drawdown_pct,
                report.sharpe_ratio,
                report.calmar_ratio,
                report.total_trades,
                report.win_rate_pct,
                report.avg_win,
                report.avg_loss,
                report.profit_factor);
        } catch (const std::exception& e) {
            std::cerr << "[Backtest] Error: " << e.what() << std::endl;
        }
    });

    // ---- Wire: Account panel ----
    QObject::connect(window.accountPanel(), &AccountPanel::batchTradeRequested,
        [](const std::vector<int64_t> &ids) {
            std::cout << "[BatchTrade] Requested for " << ids.size()
                      << " accounts (Phase 5)" << std::endl;
        });

    window.show();
    return app.exec();
}
