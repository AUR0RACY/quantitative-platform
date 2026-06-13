#include "md_adapter.hpp"
#include <QDebug>
#include <cmath>
#include <cstdlib>
#include <ctime>

namespace qp::data {

MdAdapter::MdAdapter(QObject *parent) : QObject(parent) {
    // Simulation mode: generate ticks every 500ms
    m_simTimer = new QTimer(this);
    m_simTimer->setInterval(500);
    connect(m_simTimer, &QTimer::timeout, this, &MdAdapter::simulateTick);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

MdAdapter::~MdAdapter() {
    disconnect();
}

void MdAdapter::connectCtp(const std::string& address, const std::string& broker_id,
                         const std::string& user_id, const std::string& password) {
    Q_UNUSED(address)
    m_brokerId = broker_id;
    m_userId   = user_id;
    Q_UNUSED(password)

    if (m_simulationMode) {
        m_connected = true;
        m_loggedIn  = true;
        m_simTimer->start();
        emit connected();
        qDebug() << "[MdAdapter] Connected (simulation mode) — broker:" << broker_id.c_str()
                 << " user:" << user_id.c_str();
    }
}

void MdAdapter::disconnect() {
    if (m_simTimer) m_simTimer->stop();
    m_connected = false;
    m_loggedIn  = false;
    emit disconnected();
}

void MdAdapter::subscribe(const std::string& instrument) {
    Q_UNUSED(instrument)
    qDebug() << "[MdAdapter] Subscribe:" << instrument.c_str();
}

void MdAdapter::unsubscribe(const std::string& instrument) {
    Q_UNUSED(instrument)
}

void MdAdapter::simulateTick() {
    ctp::DepthMarketData tick;
    std::strncpy(tick.InstrumentID, "ag2506", sizeof(tick.InstrumentID));
    std::strncpy(tick.ExchangeID,   "SHFE",  sizeof(tick.ExchangeID));

    double change = (std::rand() % 100 - 50) * 0.5;
    m_simPrice += change;
    if (m_simPrice < 5100) m_simPrice = 5100;
    if (m_simPrice > 5400) m_simPrice = 5400;

    tick.LastPrice = m_simPrice;
    tick.BidPrice1 = m_simPrice - 1;
    tick.AskPrice1 = m_simPrice + 1;
    tick.BidVolume1 = 10;
    tick.AskVolume1 = 10;
    tick.Volume     = 5000 + std::rand() % 5000;
    tick.OpenInterest = 100000 + std::rand() % 10000;
    tick.UpdateMillisec = static_cast<int>(std::time(nullptr)) % 1000;

    if (m_onTick) m_onTick(tick);
    emit tickReceived(tick);
}

} // namespace qp::data
