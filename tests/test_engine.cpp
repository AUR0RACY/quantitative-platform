#include "engine/backtest_engine.hpp"
#include "core/types.hpp"
#include <iostream>
#include <string>
#include <cmath>

static int failures = 0;

static void check(const std::string& name, bool condition, const std::string& msg = "") {
    if (condition) {
        std::cout << "  " << name << "... OK" << std::endl;
    } else {
        std::cout << "  " << name << "... FAIL";
        if (!msg.empty()) std::cout << ": " << msg;
        std::cout << std::endl;
        ++failures;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "Phase 3 — Engine Tests" << std::endl;

    // Locate strategy DLL
    std::string dll_path;
    if (argc >= 2) {
        dll_path = argv[1];
    } else {
        // Default: look in build/strategies/Debug/
        dll_path = "strategies/Debug/sma_crossover.dll";
    }

    // ---- Generate synthetic data (trend then reversal) ----
    std::vector<qp::Bar> bars;
    double price = 100.0;
    auto now = std::chrono::system_clock::now();
    for (int i = 0; i < 200; ++i) {
        // Up-trend phase: 0-99; then down-trend: 100-199
        if (i < 100) {
            price += 0.5 + (std::sin(i * 0.3) * 0.3);
        } else {
            price -= 0.5 + (std::cos(i * 0.2) * 0.2);
        }
        if (price < 10.0) price = 10.0;

        qp::Bar b;
        b.time = now + std::chrono::hours(i);
        b.open  = price;
        b.high  = price + 2.0;
        b.low   = price - 2.0;
        b.close = price + 1.0;    // slight bullish bias per bar
        b.volume = 10000;
        b.open_interest = 5000;
        bars.push_back(b);
    }

    check("generate_bars", bars.size() == 200);

    // ---- Load strategy ----
    qp::engine::BacktestEngine engine;

    try {
        engine.load_strategy(dll_path);
        check("load_dll", true);
    } catch (const std::exception& e) {
        check("load_dll", false, std::string("Failed — ") + e.what() +
             " (run 'cmake --build . --config Debug' first, or pass DLL path as arg)");
        std::cout << std::endl << "1 test(s) FAILED." << std::endl;
        return 1;
    }

    // ---- Hot-tune parameter ----
    auto* strategy = engine.strategy();
    if (strategy) {
        auto params = strategy->params();
        check("get_params", params.size() == 2);

        strategy->set_param("fast_period", 10);
        auto params2 = strategy->params();
        bool found = false;
        for (auto& p : params2) {
            if (p.name == "fast_period") found = (std::abs(p.value - 10.0) < 0.01);
        }
        check("set_param", found);
    } else {
        check("get_params", false, "strategy is null");
        check("set_param", false, "strategy is null");
    }

    // ---- Run backtest ----
    auto report = engine.run(
        bars, "SMA Crossover", "AG2506",
        15,          // multiplier (silver contract ~15kg)
        1000000.0,   // initial equity 1M CNY
        15.0,        // commission 15 CNY/lot
        1.0,         // slippage 1 tick
        1.0          // tick size 1 CNY
    );

    check("equity_curve", report.equity_curve.size() == 200);
    check("total_return", !std::isnan(report.total_return_pct));
    check("max_drawdown", report.max_drawdown_pct >= 0.0);
    check("sharpe", !std::isnan(report.sharpe_ratio));

    // Report summary
    std::cout << std::endl;
    std::cout << " Backtest Report — " << report.strategy_name << std::endl;
    std::cout << " Instrument:         " << report.instrument << std::endl;
    std::cout << " Total Bars:         " << report.total_bars << std::endl;
    std::cout << " Total Return:       " << report.total_return_pct << "%" << std::endl;
    std::cout << " Annualized Return:  " << report.annualized_return_pct << "%" << std::endl;
    std::cout << " Max Drawdown:       " << report.max_drawdown_pct << "%" << std::endl;
    std::cout << " Sharpe Ratio:       " << report.sharpe_ratio << std::endl;
    std::cout << " Calmar Ratio:       " << report.calmar_ratio << std::endl;
    std::cout << " Total Trades:       " << report.total_trades << std::endl;
    std::cout << " Win Rate:           " << report.win_rate_pct << "%" << std::endl;
    std::cout << " Avg Win:            " << report.avg_win << std::endl;
    std::cout << " Avg Loss:           " << report.avg_loss << std::endl;
    std::cout << " Profit Factor:      " << report.profit_factor << std::endl;
    std::cout << std::endl;

    // Results
    if (failures == 0) {
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << failures << " test(s) FAILED." << std::endl;
        return 1;
    }
}
