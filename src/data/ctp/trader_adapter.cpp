#include "trader_adapter.hpp"
#include <QDebug>
#include <sstream>
#include <iomanip>

namespace qp::data {

TraderAdapter::TraderAdapter(QObject *parent) : QObject(parent) {}

TraderAdapter::~TraderAdapter() {
    disconnect();
}

void TraderAdapter::connectCtp(const std::string& address, const std::string& broker_id,
                             const std::string& user_id, const std::string& password,
                             const std::string& auth_code, const std::string& app_id) {
    Q_UNUSED(address)
    Q_UNUSED(password)
    Q_UNUSED(auth_code)
    Q_UNUSED(app_id)
    m_brokerId = broker_id;
    m_userId   = user_id;
    m_connected = true;
    m_frontID   = 1;
    m_sessionID = 100;

    m_loginInfo.FrontID   = m_frontID;
    m_loginInfo.SessionID = m_sessionID;
    std::strncpy(m_loginInfo.BrokerID, broker_id.c_str(), sizeof(m_loginInfo.BrokerID));
    std::strncpy(m_loginInfo.UserID,   user_id.c_str(),   sizeof(m_loginInfo.UserID));

    // Simulated account
    m_account = {};
    std::strncpy(m_account.BrokerID, broker_id.c_str(), sizeof(m_account.BrokerID));
    m_account.Balance   = 1000000.0;
    m_account.Available = 1000000.0;

    m_positions.clear();
    m_orders.clear();
    m_trades.clear();

    emit connected();
    qDebug() << "[TraderAdapter] Connected (stub mode) — broker:" << broker_id.c_str();
}

void TraderAdapter::disconnect() {
    m_connected = false;
    emit disconnected();
}

int TraderAdapter::sendOrder(const std::string& instrument, OrderAction action,
                              double price, int volume) {
    if (!m_connected) return -1;
    ++m_orderRef;

    ctp::Order order;
    std::strncpy(order.InstrumentID, instrument.c_str(), sizeof(order.InstrumentID));
    std::strncpy(order.BrokerID, m_brokerId.c_str(), sizeof(order.BrokerID));
    std::strncpy(order.InvestorID, m_userId.c_str(), sizeof(order.InvestorID));
    std::ostringstream ref;
    ref << std::setw(8) << std::setfill('0') << m_orderRef;
    std::strncpy(order.OrderRef, ref.str().c_str(), sizeof(order.OrderRef));
    order.LimitPrice = price;
    order.VolumeTotalOriginal = volume;
    order.OrderStatus = '2'; // no traded yet
    order.FrontID   = m_frontID;
    order.SessionID = m_sessionID;

    switch (action) {
    case OrderAction::OpenLong:    order.Direction = '0'; order.CombOffsetFlag[0] = '0'; break;
    case OrderAction::CloseLong:   order.Direction = '1'; order.CombOffsetFlag[0] = '1'; break;
    case OrderAction::OpenShort:   order.Direction = '1'; order.CombOffsetFlag[0] = '0'; break;
    case OrderAction::CloseShort:  order.Direction = '0'; order.CombOffsetFlag[0] = '1'; break;
    }

    m_orders.push_back(order);
    if (m_onOrder) m_onOrder(order);
    emit orderUpdated(order);

    qDebug() << "[TraderAdapter] Order sent:" << instrument.c_str()
             << "price:" << price << "vol:" << volume;
    return m_orderRef;
}

void TraderAdapter::cancelOrder(const std::string& order_ref) {
    Q_UNUSED(order_ref)
}

void TraderAdapter::queryPosition() {
    // Stub: simulate one position
    m_positions.clear();
    ctp::InvestorPosition pos;
    std::strncpy(pos.InstrumentID, "ag2506", sizeof(pos.InstrumentID));
    std::strncpy(pos.BrokerID, m_brokerId.c_str(), sizeof(pos.BrokerID));
    pos.PosiDirection  = '2'; // long
    pos.Position       = 2;
    pos.OpenCost       = 5200.0;
    pos.PositionProfit = 1000.0;
    pos.UseMargin      = 15600.0;
    m_positions.push_back(pos);
    emit positionUpdated();
}

void TraderAdapter::queryAccount() {
    emit positionUpdated();
}

void TraderAdapter::queryOrder() {}
void TraderAdapter::queryTrade() {}

} // namespace qp::data
