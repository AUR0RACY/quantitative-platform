#include "backtest_engine.hpp"
#include <cmath>
#include <stdexcept>

namespace qp::engine {

void BacktestEngine::load_strategy(const std::string& dll_path) {
    auto loader = std::make_unique<DllLoader>(dll_path);
    if (!loader->is_loaded()) {
        throw std::runtime_error("Failed to load strategy DLL: " + dll_path);
    }
    m_dll = std::move(loader);
}

BacktestReport BacktestEngine::run(
    const std::vector<qp::Bar>& bars,
    const std::string& strategy_name,
    const std::string& instrument,
    int multiplier,
    double initial_equity,
    double commission_per_lot,
    double slippage_ticks,
    double tick_size)
{
    if (!m_dll || !m_dll->strategy()) {
        throw std::runtime_error("No strategy loaded");
    }
    if (bars.empty()) {
        throw std::runtime_error("No bars to backtest");
    }

    // Reset state
    m_pos         = Position{};
    m_equity      = initial_equity;
    m_cash        = initial_equity;
    m_total_bars  = static_cast<int>(bars.size());
    m_equity_curve.clear();
    m_trades.clear();

    auto* strategy = m_dll->strategy();
    strategy->init();

    // Per-bar execution
    for (int64_t i = 0; i < static_cast<int64_t>(bars.size()); ++i) {
        const auto& bar = bars[i];

        // Run strategy on this bar
        Signal sig = strategy->on_bar(bar);

        // Execute signal
        if (sig != Signal::None) {
            double exec_price = bar.close; // execute at bar close
            simulate_fill(sig, exec_price, multiplier,
                          commission_per_lot, slippage_ticks,
                          tick_size, i);
        }

        // Mark-to-market: update equity with unrealized P&L
        double unrealized = 0.0;
        if (m_pos.side == Position::Side::Long) {
            unrealized = (bar.close - m_pos.avg_price) * m_pos.qty * multiplier;
        } else if (m_pos.side == Position::Side::Short) {
            unrealized = (m_pos.avg_price - bar.close) * m_pos.qty * multiplier;
        }
        m_equity = m_cash + unrealized;

        m_equity_curve.push_back({i, m_equity});
    }

    return compute_report(strategy_name, instrument, m_equity_curve, m_trades, m_total_bars);
}

void BacktestEngine::simulate_fill(
    Signal signal, double price, int multiplier,
    double commission_per_lot, double slippage_ticks,
    double tick_size, int64_t bar_index)
{
    // Slippage
    double exec_price = price;
    if (slippage_ticks > 0.0) {
        if (signal == Signal::Buy || signal == Signal::BuyToCover) {
            exec_price += slippage_ticks * tick_size;
        } else {
            exec_price -= slippage_ticks * tick_size;
        }
    }

    double pnl = 0.0;
    int lots = 1; // default trade size

    switch (signal) {
    case Signal::Buy:
        // Close any existing short first
        if (m_pos.side == Position::Side::Short) {
            pnl = (m_pos.avg_price - exec_price) * m_pos.qty * multiplier;
            m_cash += pnl + m_pos.cost_basis;
            m_cash -= commission_per_lot * m_pos.qty; // closing commission
            m_trades.push_back({bar_index, exec_price, -m_pos.qty, pnl});
            m_pos = Position{};
        }
        // Open long
        if (m_pos.side == Position::Side::Flat) {
            m_pos.side   = Position::Side::Long;
            m_pos.qty    = lots;
            m_pos.avg_price = exec_price;
            m_pos.cost_basis = exec_price * lots * multiplier;
            m_cash -= m_pos.cost_basis;
            m_cash -= commission_per_lot * lots;
        }
        break;

    case Signal::Sell:
        // Close long
        if (m_pos.side == Position::Side::Long) {
            pnl = (exec_price - m_pos.avg_price) * m_pos.qty * multiplier;
            m_cash += pnl + m_pos.cost_basis;
            m_cash -= commission_per_lot * m_pos.qty;
            m_trades.push_back({bar_index, exec_price, m_pos.qty, pnl});
            m_pos = Position{};
        }
        break;

    case Signal::SellShort:
        // Close any existing long first
        if (m_pos.side == Position::Side::Long) {
            pnl = (exec_price - m_pos.avg_price) * m_pos.qty * multiplier;
            m_cash += pnl + m_pos.cost_basis;
            m_cash -= commission_per_lot * m_pos.qty;
            m_trades.push_back({bar_index, exec_price, m_pos.qty, pnl});
            m_pos = Position{};
        }
        // Open short
        if (m_pos.side == Position::Side::Flat) {
            m_pos.side   = Position::Side::Short;
            m_pos.qty    = lots;
            m_pos.avg_price = exec_price;
            m_pos.cost_basis = exec_price * lots * multiplier;
            m_cash -= m_pos.cost_basis; // deposit margin (simplified)
            m_cash -= commission_per_lot * lots;
        }
        break;

    case Signal::BuyToCover:
        // Close short
        if (m_pos.side == Position::Side::Short) {
            pnl = (m_pos.avg_price - exec_price) * m_pos.qty * multiplier;
            m_cash += pnl + m_pos.cost_basis;
            m_cash -= commission_per_lot * m_pos.qty;
            m_trades.push_back({bar_index, exec_price, -m_pos.qty, pnl});
            m_pos = Position{};
        }
        break;

    default:
        break;
    }
}

}  // namespace qp::engine
