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
    app.setApplicationVersion("0.6.0");

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

    // ---- MD → Chart (candlestick mode) ----
    QObject::connect(&mdAdapter, &qp::data::MdAdapter::tickReceived,
        [&window](const qp::ctp::DepthMarketData &tick) {
            auto now = std::chrono::system_clock::now();
            double t = std::chrono::duration_cast<std::chrono::seconds>(
                now.time_since_epoch()).count();
            CandlestickData d;
            d.time.append(t);
            d.open.append(tick.LastPrice - 5);
            d.high.append(tick.LastPrice + 10);
            d.low.append(tick.LastPrice - 10);
            d.close.append(tick.LastPrice);
            d.volume.append(tick.Volume);
            window.chartPanel()->loadData(d);
        });

    // ---- Account ----
    QObject::connect(window.accountPanel(), &AccountPanel::batchTradeRequested,
        [](const std::vector<int64_t> &ids) {
            std::cout << "[BatchTrade] " << ids.size() << " accounts." << std::endl;
        });

    // ---- Trade → Trader ----
    QObject::connect(window.tradePanel(), &TradePanel::sendOrderRequested,
        [&traderAdapter, &window](const QString &inst, qp::data::OrderAction action,
                                   double price, int volume) {
            int ref = traderAdapter.sendOrder(inst.toStdString(), action, price, volume);
            window.tradePanel()->logMessage(
                QString("Order #%1: %2 %3@%4 x%5")
                .arg(ref).arg(inst).arg((int)action).arg(price).arg(volume));
        });

    traderAdapter.set_on_order([&window](const qp::ctp::Order &o) {
        window.tradePanel()->updateOrder(o);
    });
    traderAdapter.set_on_position([&window](const qp::ctp::InvestorPosition &p) {
        window.tradePanel()->updatePosition(p);
    });

    // ---- Backtest ----
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
            auto r = engine.run(bars, "SMA Crossover", "AG2506", 15, 1e6, 15, 1, 1);
            window.backtestPanel()->setReport(
                QString::fromStdString(r.strategy_name),
                QString::fromStdString(r.instrument),
                r.total_return_pct, r.max_drawdown_pct,
                r.sharpe_ratio, r.calmar_ratio,
                r.total_trades, r.win_rate_pct,
                r.avg_win, r.avg_loss, r.profit_factor);
        } catch (const std::exception& e) {
            std::cerr << "[Backtest] " << e.what() << std::endl;
        }
    });

    window.strategyPanel()->setCompileCallback([](const QString &) {
        std::cout << "[Compile] Strategy saved." << std::endl;
    });

    // ---- Start CTP (simulated) ----
    mdAdapter.connectCtp("tcp://180.168.146.187:10211", "9999", "test", "test");
    mdAdapter.subscribe("ag2506");
    traderAdapter.connectCtp("tcp://180.168.146.187:10202", "9999", "test", "test");
    traderAdapter.queryPosition();
    traderAdapter.queryAccount();

    window.show();
    return app.exec();
}
