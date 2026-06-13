#include "database.hpp"
#include <cassert>
#include <ctime>

namespace qp::data {

using qp::Timestamp;
using qp::BarPeriod;

// ---- Helpers ----

static std::int64_t timestamp_to_unix(Timestamp ts) {
    return std::chrono::duration_cast<std::chrono::seconds>(
        ts.time_since_epoch()).count();
}

static Timestamp unix_to_timestamp(std::int64_t unix) {
    return Timestamp(std::chrono::seconds(unix));
}

static int bar_period_seconds(BarPeriod p) {
    return static_cast<int>(p);
}

// ---- Database ----

Database::Database(const std::string& path)
    : m_db(std::make_unique<::SQLite::Database>(path,
          ::SQLite::OPEN_READWRITE | ::SQLite::OPEN_CREATE))
{
    m_db->exec("PRAGMA journal_mode = WAL");
    m_db->exec("PRAGMA foreign_keys = ON");
}

void Database::migrate() {
    ::SQLite::Transaction txn(*m_db);

    m_db->exec(R"(
        CREATE TABLE IF NOT EXISTS instruments (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            symbol     TEXT    NOT NULL UNIQUE,
            exchange   TEXT    NOT NULL,
            name       TEXT    NOT NULL DEFAULT '',
            tick_size  REAL    NOT NULL DEFAULT 0.0,
            multiplier INTEGER NOT NULL DEFAULT 1
        );
    )");

    m_db->exec(R"(
        CREATE TABLE IF NOT EXISTS bars (
            id            INTEGER PRIMARY KEY AUTOINCREMENT,
            instrument_id INTEGER NOT NULL,
            period        INTEGER NOT NULL,
            time          INTEGER NOT NULL,   -- unix seconds
            open          REAL    NOT NULL,
            high          REAL    NOT NULL,
            low           REAL    NOT NULL,
            close         REAL    NOT NULL,
            volume        INTEGER NOT NULL DEFAULT 0,
            open_interest INTEGER NOT NULL DEFAULT 0,
            FOREIGN KEY (instrument_id) REFERENCES instruments(id),
            UNIQUE(instrument_id, period, time)
        );
        CREATE INDEX IF NOT EXISTS idx_bars_time
            ON bars(instrument_id, period, time);
    )");

    m_db->exec(R"(
        CREATE TABLE IF NOT EXISTS ticks (
            id            INTEGER PRIMARY KEY AUTOINCREMENT,
            instrument_id INTEGER NOT NULL,
            time          INTEGER NOT NULL,   -- unix millis
            last_price    REAL    NOT NULL,
            volume        INTEGER NOT NULL DEFAULT 0,
            bid_price1    REAL    NOT NULL DEFAULT 0.0,
            bid_volume1   INTEGER NOT NULL DEFAULT 0,
            ask_price1    REAL    NOT NULL DEFAULT 0.0,
            ask_volume1   INTEGER NOT NULL DEFAULT 0,
            FOREIGN KEY (instrument_id) REFERENCES instruments(id)
        );
        CREATE INDEX IF NOT EXISTS idx_ticks_time
            ON ticks(instrument_id, time);
    )");

    m_db->exec(R"(
        CREATE TABLE IF NOT EXISTS accounts (
            id             INTEGER PRIMARY KEY AUTOINCREMENT,
            name           TEXT    NOT NULL,
            broker         TEXT    NOT NULL DEFAULT '',
            ctp_address    TEXT    NOT NULL DEFAULT '',
            ctp_broker_id  TEXT    NOT NULL DEFAULT '',
            ctp_user_id    TEXT    NOT NULL DEFAULT '',
            ctp_password   TEXT    NOT NULL DEFAULT '',
            ctp_auth_code  TEXT    NOT NULL DEFAULT '',
            ctp_app_id     TEXT    NOT NULL DEFAULT '',
            enabled        INTEGER NOT NULL DEFAULT 1
        );
    )");

    txn.commit();
}

// ---- Instruments ----

void Database::insertInstrument(const qp::Instrument& ins) {
    ::SQLite::Statement stmt(*m_db, R"(
        INSERT OR IGNORE INTO instruments (symbol, exchange, name, tick_size, multiplier)
        VALUES (?, ?, ?, ?, ?)
    )");
    stmt.bind(1, ins.symbol);
    stmt.bind(2, ins.exchange);
    stmt.bind(3, ins.name);
    stmt.bind(4, ins.tick_size);
    stmt.bind(5, ins.multiplier);
    stmt.exec();
}

std::vector<qp::Instrument> Database::listInstruments(const std::string& exchange_filter) {
    std::vector<qp::Instrument> result;
    std::string sql = "SELECT symbol, exchange, name, tick_size, multiplier FROM instruments";
    if (!exchange_filter.empty()) {
        sql += " WHERE exchange = ?";
    }

    ::SQLite::Statement stmt(*m_db, sql);
    if (!exchange_filter.empty()) {
        stmt.bind(1, exchange_filter);
    }

    while (stmt.executeStep()) {
        qp::Instrument ins;
        ins.symbol     = stmt.getColumn(0).getString();
        ins.exchange   = stmt.getColumn(1).getString();
        ins.name       = stmt.getColumn(2).getString();
        ins.tick_size  = stmt.getColumn(3).getDouble();
        ins.multiplier = stmt.getColumn(4).getInt();
        result.push_back(ins);
    }
    return result;
}

std::optional<qp::Instrument> Database::getInstrument(const std::string& symbol) {
    ::SQLite::Statement stmt(*m_db, R"(
        SELECT symbol, exchange, name, tick_size, multiplier
        FROM instruments WHERE symbol = ?
    )");
    stmt.bind(1, symbol);

    if (stmt.executeStep()) {
        qp::Instrument ins;
        ins.symbol     = stmt.getColumn(0).getString();
        ins.exchange   = stmt.getColumn(1).getString();
        ins.name       = stmt.getColumn(2).getString();
        ins.tick_size  = stmt.getColumn(3).getDouble();
        ins.multiplier = stmt.getColumn(4).getInt();
        return ins;
    }
    return std::nullopt;
}

// ---- Bars ----

void Database::insertBar(int64_t instrument_id, BarPeriod period, const qp::Bar& bar) {
    ::SQLite::Statement stmt(*m_db, R"(
        INSERT OR REPLACE INTO bars (instrument_id, period, time, open, high, low, close, volume, open_interest)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    stmt.bind(1, instrument_id);
    stmt.bind(2, bar_period_seconds(period));
    stmt.bind(3, timestamp_to_unix(bar.time));
    stmt.bind(4, bar.open);
    stmt.bind(5, bar.high);
    stmt.bind(6, bar.low);
    stmt.bind(7, bar.close);
    stmt.bind(8, static_cast<std::int64_t>(bar.volume));
    stmt.bind(9, static_cast<std::int64_t>(bar.open_interest));
    stmt.exec();
}

void Database::insertBars(int64_t instrument_id, BarPeriod period,
                           const std::vector<qp::Bar>& bars) {
    ::SQLite::Transaction txn(*m_db);
    for (const auto& bar : bars) {
        insertBar(instrument_id, period, bar);
    }
    txn.commit();
}

std::vector<qp::Bar> Database::getBars(int64_t instrument_id, BarPeriod period,
                                        Timestamp from, Timestamp to) {
    std::vector<qp::Bar> result;
    ::SQLite::Statement stmt(*m_db, R"(
        SELECT time, open, high, low, close, volume, open_interest
        FROM bars
        WHERE instrument_id = ? AND period = ? AND time >= ? AND time <= ?
        ORDER BY time ASC
    )");
    stmt.bind(1, instrument_id);
    stmt.bind(2, bar_period_seconds(period));
    stmt.bind(3, timestamp_to_unix(from));
    stmt.bind(4, timestamp_to_unix(to));

    while (stmt.executeStep()) {
        qp::Bar bar;
        bar.time          = unix_to_timestamp(stmt.getColumn(0).getInt64());
        bar.open          = stmt.getColumn(1).getDouble();
        bar.high          = stmt.getColumn(2).getDouble();
        bar.low           = stmt.getColumn(3).getDouble();
        bar.close         = stmt.getColumn(4).getDouble();
        bar.volume        = static_cast<uint64_t>(stmt.getColumn(5).getInt64());
        bar.open_interest = static_cast<uint64_t>(stmt.getColumn(6).getInt64());
        result.push_back(bar);
    }
    return result;
}

// ---- Ticks ----

void Database::insertTick(int64_t instrument_id, const qp::Tick& tick) {
    ::SQLite::Statement stmt(*m_db, R"(
        INSERT OR REPLACE INTO ticks (instrument_id, time, last_price, volume,
                                       bid_price1, bid_volume1, ask_price1, ask_volume1)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )");
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
        tick.time.time_since_epoch()).count();
    stmt.bind(1, instrument_id);
    stmt.bind(2, static_cast<std::int64_t>(millis));
    stmt.bind(3, tick.last_price);
    stmt.bind(4, static_cast<std::int64_t>(tick.volume));
    stmt.bind(5, tick.bid_price1);
    stmt.bind(6, static_cast<std::int64_t>(tick.bid_volume1));
    stmt.bind(7, tick.ask_price1);
    stmt.bind(8, static_cast<std::int64_t>(tick.ask_volume1));
    stmt.exec();
}

void Database::insertTicks(int64_t instrument_id, const std::vector<qp::Tick>& ticks) {
    ::SQLite::Transaction txn(*m_db);
    for (const auto& tick : ticks) {
        insertTick(instrument_id, tick);
    }
    txn.commit();
}

std::vector<qp::Tick> Database::getTicks(int64_t instrument_id, Timestamp from, Timestamp to) {
    std::vector<qp::Tick> result;
    auto from_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        from.time_since_epoch()).count();
    auto to_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        to.time_since_epoch()).count();

    ::SQLite::Statement stmt(*m_db, R"(
        SELECT time, last_price, volume, bid_price1, bid_volume1, ask_price1, ask_volume1
        FROM ticks
        WHERE instrument_id = ? AND time >= ? AND time <= ?
        ORDER BY time ASC
    )");
    stmt.bind(1, instrument_id);
    stmt.bind(2, static_cast<std::int64_t>(from_ms));
    stmt.bind(3, static_cast<std::int64_t>(to_ms));

    while (stmt.executeStep()) {
        qp::Tick tick;
        tick.time       = Timestamp(std::chrono::milliseconds(stmt.getColumn(0).getInt64()));
        tick.last_price = stmt.getColumn(1).getDouble();
        tick.volume     = static_cast<uint64_t>(stmt.getColumn(2).getInt64());
        tick.bid_price1 = stmt.getColumn(3).getDouble();
        tick.bid_volume1 = static_cast<uint64_t>(stmt.getColumn(4).getInt64());
        tick.ask_price1 = stmt.getColumn(5).getDouble();
        tick.ask_volume1 = static_cast<uint64_t>(stmt.getColumn(6).getInt64());
        result.push_back(tick);
    }
    return result;
}

// ---- Accounts ----

int64_t Database::insertAccount(const Account& acc) {
    ::SQLite::Statement stmt(*m_db, R"(
        INSERT INTO accounts (name, broker, ctp_address, ctp_broker_id,
                               ctp_user_id, ctp_password, ctp_auth_code,
                               ctp_app_id, enabled)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    stmt.bind(1, acc.name);
    stmt.bind(2, acc.broker);
    stmt.bind(3, acc.ctp_address);
    stmt.bind(4, acc.ctp_broker_id);
    stmt.bind(5, acc.ctp_user_id);
    stmt.bind(6, acc.ctp_password);
    stmt.bind(7, acc.ctp_auth_code);
    stmt.bind(8, acc.ctp_app_id);
    stmt.bind(9, static_cast<int>(acc.enabled));
    stmt.exec();

    return static_cast<int64_t>(m_db->getLastInsertRowid());
}

void Database::updateAccount(const Account& acc) {
    ::SQLite::Statement stmt(*m_db, R"(
        UPDATE accounts SET
            name = ?, broker = ?, ctp_address = ?, ctp_broker_id = ?,
            ctp_user_id = ?, ctp_password = ?, ctp_auth_code = ?,
            ctp_app_id = ?, enabled = ?
        WHERE id = ?
    )");
    stmt.bind(1, acc.name);
    stmt.bind(2, acc.broker);
    stmt.bind(3, acc.ctp_address);
    stmt.bind(4, acc.ctp_broker_id);
    stmt.bind(5, acc.ctp_user_id);
    stmt.bind(6, acc.ctp_password);
    stmt.bind(7, acc.ctp_auth_code);
    stmt.bind(8, acc.ctp_app_id);
    stmt.bind(9, static_cast<int>(acc.enabled));
    stmt.bind(10, acc.id);
    stmt.exec();
}

void Database::deleteAccount(int64_t id) {
    ::SQLite::Statement stmt(*m_db, "DELETE FROM accounts WHERE id = ?");
    stmt.bind(1, id);
    stmt.exec();
}

std::vector<Account> Database::listAccounts() {
    std::vector<Account> result;
    ::SQLite::Statement stmt(*m_db, R"(
        SELECT id, name, broker, ctp_address, ctp_broker_id,
               ctp_user_id, ctp_password, ctp_auth_code, ctp_app_id, enabled
        FROM accounts ORDER BY id
    )");

    while (stmt.executeStep()) {
        Account acc;
        acc.id            = stmt.getColumn(0).getInt64();
        acc.name          = stmt.getColumn(1).getString();
        acc.broker        = stmt.getColumn(2).getString();
        acc.ctp_address   = stmt.getColumn(3).getString();
        acc.ctp_broker_id = stmt.getColumn(4).getString();
        acc.ctp_user_id   = stmt.getColumn(5).getString();
        acc.ctp_password  = stmt.getColumn(6).getString();
        acc.ctp_auth_code = stmt.getColumn(7).getString();
        acc.ctp_app_id    = stmt.getColumn(8).getString();
        acc.enabled       = stmt.getColumn(9).getInt() != 0;
        result.push_back(acc);
    }
    return result;
}

std::optional<Account> Database::getAccount(int64_t id) {
    ::SQLite::Statement stmt(*m_db, R"(
        SELECT id, name, broker, ctp_address, ctp_broker_id,
               ctp_user_id, ctp_password, ctp_auth_code, ctp_app_id, enabled
        FROM accounts WHERE id = ?
    )");
    stmt.bind(1, id);

    if (stmt.executeStep()) {
        Account acc;
        acc.id            = stmt.getColumn(0).getInt64();
        acc.name          = stmt.getColumn(1).getString();
        acc.broker        = stmt.getColumn(2).getString();
        acc.ctp_address   = stmt.getColumn(3).getString();
        acc.ctp_broker_id = stmt.getColumn(4).getString();
        acc.ctp_user_id   = stmt.getColumn(5).getString();
        acc.ctp_password  = stmt.getColumn(6).getString();
        acc.ctp_auth_code = stmt.getColumn(7).getString();
        acc.ctp_app_id    = stmt.getColumn(8).getString();
        acc.enabled       = stmt.getColumn(9).getInt() != 0;
        return acc;
    }
    return std::nullopt;
}

} // namespace qp::data
