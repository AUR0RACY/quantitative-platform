#pragma once

#include "engine/strategy_interface.hpp"

namespace qp::strategies {

class SmaCrossover : public engine::IStrategy {
public:
    SmaCrossover();
    ~SmaCrossover() override = default;

    void init() override;
    engine::Signal on_bar(const qp::Bar& bar) override;

    std::vector<engine::StrategyParam> params() const override;
    void set_param(const std::string& name, double value) override;

private:
    int    m_fast_period = 5;
    int    m_slow_period = 20;
    double m_fast_sum    = 0.0;
    double m_slow_sum    = 0.0;
    int    m_bar_count   = 0;

    double m_prev_fast_ma = 0.0;
    double m_prev_slow_ma = 0.0;

    // Circular buffer for SMA calculation
    static constexpr int MAX_PERIOD = 200;
    double m_close_buf[MAX_PERIOD] = {};
};

}  // namespace qp::strategies
