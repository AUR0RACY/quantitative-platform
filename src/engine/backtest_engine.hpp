#pragma once

#include "core/types.hpp"
#include "engine/strategy_interface.hpp"
#include "engine/report.hpp"
#include "engine/dll_loader.hpp"
#include <vector>
#include <string>

namespace qp::engine {

// ---- Position state ----
struct Position {
    enum class Side { Flat, Long, Short };
    Side   side  = Side::Flat;
    int    qty   = 0;
    double avg_price = 0.0;
    double cost_basis = 0.0;  // total entry cost (price * qty * multiplier)
};

// ---- Backtest engine ----
class BacktestEngine {
public:
    BacktestEngine() = default;

    // Load strategy from DLL
    void load_strategy(const std::string& dll_path);

    // Access loaded strategy (for parameter tuning, etc.)
    IStrategy* strategy() { return m_dll ? m_dll->strategy() : nullptr; }

    // Run backtest on given bars
    BacktestReport run(
        const std::vector<qp::Bar>& bars,
        const std::string& strategy_name,
        const std::string& instrument,
        int multiplier = 1,
        double initial_equity = 1000000.0,
        double commission_per_lot = 0.0,
        double slippage_ticks = 0.0,
        double tick_size = 1.0
    );

private:
    void simulate_fill(Signal signal, double price, int multiplier,
                        double commission_per_lot, double slippage_ticks,
                        double tick_size, int64_t bar_index);

    // State
    std::unique_ptr<DllLoader> m_dll;
    Position     m_pos;
    double       m_equity      = 0.0;
    double       m_cash        = 0.0;
    int          m_total_bars  = 0;

    std::vector<EquityPoint> m_equity_curve;
    std::vector<TradeRecord> m_trades;
};

}  // namespace qp::engine
