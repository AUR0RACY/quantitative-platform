#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace qp::engine {

// ---- Strategy parameter (hot-tunable) ----
struct StrategyParam {
    std::string name;
    double      value;
    double      min_val;
    double      max_val;
    double      step;
};

// ---- Signal types ----
enum class Signal : int {
    None       = 0,
    Buy        = 1,   // open long
    Sell       = 2,   // close long
    SellShort  = 3,   // open short
    BuyToCover = 4,   // close short
};

// ---- Abstract strategy interface ----
// Each strategy DLL must implement this and export:
//   extern "C" __declspec(dllexport) qp::engine::IStrategy* create_strategy();
//   extern "C" __declspec(dllexport) void destroy_strategy(IStrategy*);

class IStrategy {
public:
    virtual ~IStrategy() = default;

    // Called once before any bars are processed
    virtual void init() = 0;

    // Called on each bar — return trade signal
    virtual Signal on_bar(const qp::Bar& bar) = 0;

    // Parameter metadata (for hot-tuning UI panel)
    virtual std::vector<StrategyParam> params() const = 0;

    // Set parameter by name at runtime (no recompile)
    virtual void set_param(const std::string& name, double value) = 0;

    // Optional: override for tick-level strategies (Phase 5+)
    virtual Signal on_tick(const qp::Tick& tick) {
        (void)tick;
        return Signal::None;
    }
};

// DLL export type
using CreateStrategyFn = IStrategy* (*)();
using DestroyStrategyFn = void (*)(IStrategy*);

}  // namespace qp::engine
