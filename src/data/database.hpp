#pragma once

#include "core/types.hpp"
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Transaction.h>
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace qp::data {

// ---- Account (CTP connection info) ----
struct Account {
    int64_t  id;
    std::string name;          // display name, e.g. "Shanghai Silver Fund"
    std::string broker;        // broker name
    std::string ctp_address;   // tcp://broker-ip:port
    std::string ctp_broker_id;
    std::string ctp_user_id;
    std::string ctp_password;
    std::string ctp_auth_code;
    std::string ctp_app_id;
    bool       enabled;        // allow batch trading
};

// ---- Database manager ----
class Database {
public:
    explicit Database(const std::string& path = "quant.db");
    ~Database() = default;

    // ---- Init ----
    void migrate();

    // ---- Instruments ----
    void   insertInstrument(const qp::Instrument& ins);
    std::vector<qp::Instrument> listInstruments(const std::string& exchange_filter = "");
    std::optional<qp::Instrument> getInstrument(const std::string& symbol);

    // ---- Bars ----
    void   insertBar(int64_t instrument_id, qp::BarPeriod period, const qp::Bar& bar);
    void   insertBars(int64_t instrument_id, qp::BarPeriod period,
                      const std::vector<qp::Bar>& bars);
    std::vector<qp::Bar> getBars(int64_t instrument_id, qp::BarPeriod period,
                                  qp::Timestamp from, qp::Timestamp to);

    // ---- Ticks ----
    void   insertTick(int64_t instrument_id, const qp::Tick& tick);
    void   insertTicks(int64_t instrument_id, const std::vector<qp::Tick>& ticks);
    std::vector<qp::Tick> getTicks(int64_t instrument_id, qp::Timestamp from, qp::Timestamp to);

    // ---- Accounts ----
    int64_t     insertAccount(const Account& acc);
    void        updateAccount(const Account& acc);
    void        deleteAccount(int64_t id);
    std::vector<Account> listAccounts();
    std::optional<Account> getAccount(int64_t id);

private:
    std::unique_ptr<::SQLite::Database> m_db;
};

} // namespace qp::data
