#pragma once

#include <cstdint>
#include <string>
#include <chrono>

namespace qp {

using Timestamp = std::chrono::system_clock::time_point;

// ---- Market Data ----

struct Bar {
    Timestamp time;        // Bar start time
    double   open;
    double   high;
    double   low;
    double   close;
    uint64_t volume;
    uint64_t open_interest;
};

struct Tick {
    Timestamp time;
    double    last_price;
    uint64_t  volume;
    double    bid_price1;
    uint64_t  bid_volume1;
    double    ask_price1;
    uint64_t  ask_volume1;
};

enum class BarPeriod : int {
    Tick    = 0,
    Sec1    = 1,
    Min1    = 60,
    Min5    = 300,
    Min15   = 900,
    Min30   = 1800,
    Hour1   = 3600,
    Day1    = 86400,
};

// ---- Instrument ----

struct Instrument {
    std::string symbol;       // e.g. "ag2506"
    std::string exchange;     // "SHFE", "DCE", etc.
    std::string name;         // "白银2506"
    double      tick_size;    // minimum price movement
    int         multiplier;   // contract size
};

} // namespace qp
