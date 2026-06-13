#include <QApplication>
#include <QMessageBox>
#include "ui/mainwindow.hpp"
#include "engine/backtest_engine.hpp"
#include "data/database.hpp"
#include "data/ctp/md_adapter.hpp"
#include "data/ctp/trader_adapter.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Quantitative Platform");
    app.setApplicationVersion("0.5.0");

    // ---- Database ----
    try { qp::data::Database("quant.db").migrate(); }
    catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "DB Error", e.what());
        return 1;
    }

    MainWindow window;

    // ---- CTP Adapters ----
    qp::data::MdAdapter mdAdapter;
    qp::data::TraderAdapter traderAdapter;
    window.tradePanel()->setTraderAdapter(&traderAdapter);

    // ---- MD → Chart ----
    QObject::connect(&mdAdapter, &qp::data::MdAdapter::tickReceived,
        [&window](const qp::ctp::DepthMarketData &tick) {
            // Append to rolling chart data
            QVector<double> time, open, high, low, close, vol;
            auto now = std::chrono::system_clock::now();
            double t = std::chrono::duration_cast<std::chrono::seconds>(
                now.time_since_epoch()).count();
            time.append(t);
            double p = tick.LastPrice;
            open.append(p - 5);
            high.append(p + 10);
            low.append(p - 10);
            close.append(p);
            vol.append(tick.Volume);
            window.chartPanel()->loadCandlestickData(time, open, high, low, close, vol);
        });

    // ---- Account → CTP connect ----
    QObject::connect(window.accountPanel(), &AccountPanel::batchTradeRequested,
        [&](const std::vector<int64_t> &ids) {
            std::cout << "[BatchTrade] " << ids.size() << " accounts selected." << std::endl;
        });

    // ---- Trade Panel → Trader ----
    QObject::connect(window.tradePanel(), &TradePanel::sendOrderRequested,
        [&traderAdapter, &window](const QString &inst, qp::data::OrderAction action,
                                   double price, int volume) {
            int ref = traderAdapter.sendOrder(inst.toStdString(), action, price, volume);
            window.tradePanel()->logMessage(
                QString("Order #%1: %2 %3@%4 x%5")
                .arg(ref).arg(inst).arg(static_cast<int>(action)).arg(price).arg(volume));
        });

    // ---- Trader callbacks → Trade Panel ----
    traderAdapter.set_on_order([&window](const qp::ctp::Order &o) {
        window.tradePanel()->updateOrder(o);
    });
    traderAdapter.set_on_position([&window](const qp::ctp::InvestorPosition &p) {
        window.tradePanel()->updatePosition(p);
    });
    traderAdapter.set_on_account([&window](const qp::ctp::TradingAccount &a) {
        Q_UNUSED(a)
    });

    // ---- Backtest wiring ----
    QObject::connect(window.backtestPanel(), &BacktestPanel::runRequested, [&window]() {
        try {
            qp::engine::BacktestEngine engine;
            engine.load_strategy("strategies/Debug/sma_crossover.dll");
            std::vector<qp::Bar> bars;
            double price = 5200.0;
            auto now = std::chrono::system_clock::now();
            for (int i = 0; i < 200; ++i) {
                price += std::sin(i * 0.3) * 20.0;
                qp::Bar b; b.time = now + std::chrono::hours(i);
                b.open = price; b.high = price+30; b.low = price-30;
                b.close = price+15; b.volume = 10000; bars.push_back(b);
            }
            auto report = engine.run(bars, "SMA Crossover", "AG2506", 15, 1000000.0, 15.0, 1.0, 1.0);
            window.backtestPanel()->setReport(
                QString::fromStdString(report.strategy_name),
                QString::fromStdString(report.instrument),
                report.total_return_pct, report.max_drawdown_pct,
                report.sharpe_ratio, report.calmar_ratio,
                report.total_trades, report.win_rate_pct,
                report.avg_win, report.avg_loss, report.profit_factor);
        } catch (const std::exception& e) {
            std::cerr << "[Backtest] " << e.what() << std::endl;
        }
    });

    // ---- Strategy compile stub ----
    window.strategyPanel()->setCompileCallback([](const QString &) {
        std::cout << "[Compile] Save strategy and build via CMake." << std::endl;
    });

    // ---- Start MD (simulated) ----
    mdAdapter.connectCtp("tcp://180.168.146.187:10211", "9999", "test", "test");
    mdAdapter.subscribe("ag2506");

    // ---- Start Trader (simulated) ----
    traderAdapter.connectCtp("tcp://180.168.146.187:10202", "9999", "test", "test");
    traderAdapter.queryPosition();
    traderAdapter.queryAccount();

    window.show();
    return app.exec();
}
