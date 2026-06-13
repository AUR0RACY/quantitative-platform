#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cmath>

namespace qp::engine {

// ---- Single trade record ----
struct TradeRecord {
    int64_t bar_index;
    double  price;
    int     quantity;    // + buy, - sell
    double  pnl;         // realized P&L for closing trades
};

// ---- Equity point ----
struct EquityPoint {
    int64_t bar_index;
    double  equity;
};

// ---- Backtest report ----
struct BacktestReport {
    // Inputs
    std::string strategy_name;
    std::string instrument;
    int         total_bars;

    // Returns
    double total_return_pct;     // e.g. 12.5 = +12.5%
    double annualized_return_pct;
    double max_drawdown_pct;     // always positive, e.g. 8.3 = -8.3% peak-to-trough
    double sharpe_ratio;
    double calmar_ratio;         // annualized_return / max_drawdown

    // Trades
    int    total_trades   = 0;
    int    winning_trades  = 0;
    int    losing_trades   = 0;
    double win_rate_pct    = 0.0;
    double avg_win         = 0.0;
    double avg_loss        = 0.0;
    double profit_factor   = 0.0;  // gross_win / |gross_loss|

    // Time series
    std::vector<EquityPoint> equity_curve;
    std::vector<TradeRecord>  trades;
};

// ---- Report calculator ----
inline BacktestReport compute_report(
    const std::string& strategy_name,
    const std::string& instrument,
    const std::vector<EquityPoint>& equity,
    const std::vector<TradeRecord>& trades,
    int total_bars)
{
    BacktestReport r;
    r.strategy_name = strategy_name;
    r.instrument    = instrument;
    r.total_bars    = total_bars;

    // Equity metrics
    if (!equity.empty()) {
        double start_eq = equity.front().equity;
        double end_eq   = equity.back().equity;
        r.total_return_pct = (end_eq - start_eq) / start_eq * 100.0;

        // Max drawdown
        double peak = start_eq;
        double mdd  = 0.0;
        for (const auto& p : equity) {
            if (p.equity > peak) peak = p.equity;
            double dd = (peak - p.equity) / peak;
            if (dd > mdd) mdd = dd;
        }
        r.max_drawdown_pct = mdd * 100.0;

        // Sharpe (simplified — assumes risk-free = 0%)
        if (equity.size() >= 2) {
            double sum_ret = 0.0, sum_ret_sq = 0.0;
            for (size_t i = 1; i < equity.size(); ++i) {
                double ret = (equity[i].equity - equity[i-1].equity) / equity[i-1].equity;
                sum_ret += ret;
                sum_ret_sq += ret * ret;
            }
            double mean = sum_ret / (equity.size() - 1);
            double variance = sum_ret_sq / (equity.size() - 1) - mean * mean;
            double stddev = std::sqrt(variance);
            if (stddev > 0.0) {
                r.sharpe_ratio = mean / stddev * std::sqrt(252); // annualized (daily bars)
            }
        }

        // Annualized return (simplified — assumes 252 trading days)
        double years = static_cast<double>(total_bars) / 252.0;
        if (years > 0.0) {
            double total_ret = (end_eq - start_eq) / start_eq;
            r.annualized_return_pct = (std::pow(1.0 + total_ret, 1.0 / years) - 1.0) * 100.0;
        }

        // Calmar
        if (r.max_drawdown_pct > 0.0) {
            r.calmar_ratio = r.annualized_return_pct / r.max_drawdown_pct;
        }
    }

    // Trade metrics
    r.total_trades = static_cast<int>(trades.size());
    double gross_win  = 0.0;
    double gross_loss = 0.0;
    for (const auto& t : trades) {
        if (t.pnl > 0) {
            ++r.winning_trades;
            gross_win += t.pnl;
        } else if (t.pnl < 0) {
            ++r.losing_trades;
            gross_loss += std::abs(t.pnl);
        }
    }
    if (r.total_trades > 0) {
        r.win_rate_pct = static_cast<double>(r.winning_trades) / r.total_trades * 100.0;
    }
    if (r.winning_trades > 0) r.avg_win = gross_win / r.winning_trades;
    if (r.losing_trades > 0) r.avg_loss = gross_loss / r.losing_trades;
    if (gross_loss > 0.0) r.profit_factor = gross_win / gross_loss;

    r.trades       = trades;
    r.equity_curve = equity;
    return r;
}

}  // namespace qp::engine
