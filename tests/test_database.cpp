#include "data/database.hpp"
#include <iostream>
#include <string>
#include <cstdio>

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

int main() {
    std::cout << "Phase 2 — Database Tests" << std::endl;

    const char* db_path = "test_quant.db";
    std::remove(db_path);

    qp::data::Database db(db_path);
    db.migrate();
    std::cout << "  migrate... OK" << std::endl;

    // --- Instrument ---
    {
        qp::Instrument ins{"ag2506", "SHFE", "Silver 2506", 1.0, 15};
        db.insertInstrument(ins);
        auto got = db.getInstrument("ag2506");
        bool ok = got.has_value() && got->symbol == "ag2506" && got->multiplier == 15;
        check("insert_instrument", ok);
    }
    {
        qp::Instrument ins{"ag2506", "SHFE", "Silver 2506 v2", 2.0, 30};
        db.insertInstrument(ins); // INSERT OR IGNORE
        auto got = db.getInstrument("ag2506");
        bool ok = got.has_value() && got->name == "Silver 2506";
        check("insert_duplicate_ignored", ok);
    }

    // --- Bars ---
    {
        auto now = std::chrono::system_clock::now();
        std::vector<qp::Bar> bars;
        for (int i = 0; i < 5; ++i) {
            qp::Bar b;
            b.time = now + std::chrono::hours(i);
            b.open = 100.0 + i;
            b.high = 105.0 + i;
            b.low  = 95.0  + i;
            b.close = 102.0 + i;
            b.volume = 1000 + i * 100;
            b.open_interest = 5000;
            bars.push_back(b);
        }
        db.insertBars(1, qp::BarPeriod::Hour1, bars);
        auto got = db.getBars(1, qp::BarPeriod::Hour1,
                               now, now + std::chrono::hours(10));
        bool ok = got.size() == 5 &&
                  std::abs(got[0].open - 100.0) < 0.001 &&
                  got[4].volume == 1400;
        check("bars_crud", ok);
    }

    // --- Ticks ---
    {
        auto now = std::chrono::system_clock::now();
        std::vector<qp::Tick> ticks;
        for (int i = 0; i < 3; ++i) {
            qp::Tick t;
            t.time = now + std::chrono::seconds(i);
            t.last_price = 5200.0 + i * 10;
            t.volume = 10;
            t.bid_price1 = 5199.0; t.bid_volume1 = 5;
            t.ask_price1 = 5201.0; t.ask_volume1 = 5;
            ticks.push_back(t);
        }
        db.insertTicks(1, ticks);
        auto got = db.getTicks(1, now, now + std::chrono::seconds(10));
        bool ok = got.size() == 3 &&
                  std::abs(got[2].last_price - 5220.0) < 0.001;
        check("ticks_crud", ok);
    }

    // --- Account CRUD ---
    {
        qp::data::Account acc;
        acc.name = "Test Fund";
        acc.broker = "Guangfa Futures";
        acc.ctp_address = "tcp://180.168.146.187:10201";
        acc.ctp_broker_id = "0150";
        acc.ctp_user_id = "test123";
        acc.ctp_password = "pass123";
        acc.ctp_auth_code = "0000000000000000";
        acc.enabled = true;

        int64_t id = db.insertAccount(acc);
        check("account_insert", id > 0);

        auto got = db.getAccount(id);
        bool ok = got.has_value() && got->name == "Test Fund" && got->enabled;
        check("account_read", ok);

        got->name = "Updated Fund";
        got->enabled = false;
        db.updateAccount(*got);
        auto got2 = db.getAccount(id);
        bool ok2 = got2.has_value() && got2->name == "Updated Fund" && !got2->enabled;
        check("account_update", ok2);

        db.deleteAccount(id);
        auto got3 = db.getAccount(id);
        check("account_delete", !got3.has_value());
    }
    {
        qp::data::Account a1{0, "FundA", "B", "", "", "", "", "", "", true};
        qp::data::Account a2{0, "FundB", "B", "", "", "", "", "", "", true};
        db.insertAccount(a1);
        db.insertAccount(a2);
        auto list = db.listAccounts();
        check("account_list", list.size() >= 2);
    }

    // Cleanup
    std::remove(db_path);

    std::cout << std::endl;
    if (failures == 0) {
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << failures << " test(s) FAILED." << std::endl;
        return 1;
    }
}
