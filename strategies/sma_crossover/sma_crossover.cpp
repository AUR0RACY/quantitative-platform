#include "sma_crossover.hpp"

namespace qp::strategies {

SmaCrossover::SmaCrossover() = default;

void SmaCrossover::init() {
    m_fast_sum  = 0.0;
    m_slow_sum  = 0.0;
    m_bar_count = 0;
    m_prev_fast_ma = 0.0;
    m_prev_slow_ma = 0.0;
    for (auto& v : m_close_buf) v = 0.0;
}

engine::Signal SmaCrossover::on_bar(const qp::Bar& bar) {
    // Rolling circular buffer
    m_close_buf[m_bar_count % MAX_PERIOD] = bar.close;

    m_fast_sum += bar.close;
    m_slow_sum += bar.close;

    if (m_bar_count >= m_fast_period) {
        int idx = (m_bar_count - m_fast_period) % MAX_PERIOD;
        m_fast_sum -= m_close_buf[idx];
    }
    if (m_bar_count >= m_slow_period) {
        int idx = (m_bar_count - m_slow_period) % MAX_PERIOD;
        m_slow_sum -= m_close_buf[idx];
    }

    ++m_bar_count;

    // Need enough data for both MAs
    if (m_bar_count < m_slow_period + 1) {
        return engine::Signal::None;
    }

    double fast_ma = m_fast_sum / m_fast_period;
    double slow_ma = m_slow_sum / m_slow_period;

    double prev_fast = m_prev_fast_ma;
    double prev_slow = m_prev_slow_ma;

    m_prev_fast_ma = fast_ma;
    m_prev_slow_ma = slow_ma;

    // Golden cross: fast MA crosses above slow MA
    if (prev_fast <= prev_slow && fast_ma > slow_ma) {
        return engine::Signal::Buy;
    }
    // Death cross: fast MA crosses below slow MA
    if (prev_fast >= prev_slow && fast_ma < slow_ma) {
        return engine::Signal::Sell;
    }

    return engine::Signal::None;
}

std::vector<engine::StrategyParam> SmaCrossover::params() const {
    return {
        {"fast_period", static_cast<double>(m_fast_period), 2.0, 100.0, 1.0},
        {"slow_period", static_cast<double>(m_slow_period), 5.0, 200.0, 1.0},
    };
}

void SmaCrossover::set_param(const std::string& name, double value) {
    if (name == "fast_period") {
        m_fast_period = static_cast<int>(value);
    } else if (name == "slow_period") {
        m_slow_period = static_cast<int>(value);
    }
}

}  // namespace qp::strategies

// ---- DLL exports ----
extern "C" __declspec(dllexport) qp::engine::IStrategy* create_strategy() {
    return new qp::strategies::SmaCrossover();
}

extern "C" __declspec(dllexport) void destroy_strategy(qp::engine::IStrategy* s) {
    delete s;
}
