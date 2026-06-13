#pragma once
// CTP Trade adapter — wraps CThostFtdcTraderApi callbacks

#include "ctp_types.hpp"
#include <string>
#include <vector>
#include <functional>
#include <QObject>
#include <QTimer>
#include <atomic>
#include <memory>

namespace qp::data {

using OrderCallback   = std::function<void(const ctp::Order&)>;
using TradeCallback   = std::function<void(const ctp::Trade&)>;
using PositionCallback = std::function<void(const ctp::InvestorPosition&)>;
using AccountCallback  = std::function<void(const ctp::TradingAccount&)>;
using ErrorCallback    = std::function<void(const ctp::RspInfo&)>;

enum class OrderAction { OpenLong, CloseLong, OpenShort, CloseShort };

class TraderAdapter : public QObject {
    Q_OBJECT
public:
    explicit TraderAdapter(QObject *parent = nullptr);
    ~TraderAdapter() override;

    // ---- Connection ----
    void connectCtp(const std::string& address,
                 const std::string& broker_id,
                 const std::string& user_id,
                 const std::string& password,
                 const std::string& auth_code = "",
                 const std::string& app_id    = "");
    void disconnect();
    bool is_connected() const { return m_connected; }

    // ---- Order ----
    int  sendOrder(const std::string& instrument, OrderAction action,
                   double price, int volume);
    void cancelOrder(const std::string& order_ref);

    // ---- Query ----
    void queryPosition();
    void queryAccount();
    void queryOrder();
    void queryTrade();

    // ---- Callbacks ----
    void set_on_order(OrderCallback cb)       { m_onOrder = cb; }
    void set_on_trade(TradeCallback cb)       { m_onTrade = cb; }
    void set_on_position(PositionCallback cb) { m_onPosition = cb; }
    void set_on_account(AccountCallback cb)   { m_onAccount = cb; }
    void set_on_error(ErrorCallback cb)       { m_onError = cb; }

    // ---- State ----
    const std::vector<ctp::InvestorPosition>& positions() const { return m_positions; }
    double available() const { return m_account.Available; }
    double balance() const  { return m_account.Balance; }

signals:
    void connected();
    void disconnected();
    void orderUpdated(const ctp::Order &order);
    void tradeUpdated(const ctp::Trade &trade);
    void positionUpdated();
    void errorOccurred(const QString &msg);

private:
    void* m_traderApi = nullptr;  // CThostFtdcTraderApi*
    std::atomic<bool> m_connected{false};
    int m_orderRef = 0;
    int m_frontID  = 0;
    int m_sessionID = 0;

    std::string m_brokerId;
    std::string m_userId;

    std::vector<ctp::Order>            m_orders;
    std::vector<ctp::Trade>            m_trades;
    std::vector<ctp::InvestorPosition> m_positions;
    ctp::TradingAccount                m_account;
    ctp::RspUserLogin                  m_loginInfo;

    // Callbacks
    OrderCallback    m_onOrder;
    TradeCallback    m_onTrade;
    PositionCallback m_onPosition;
    AccountCallback  m_onAccount;
    ErrorCallback    m_onError;
};

} // namespace qp::data
