#include "okx_adapter.hpp"
#include <QDebug>
#include <cstdlib>
#include <ctime>

namespace qp::data {

OkxAdapter::OkxAdapter(QObject *parent) : QObject(parent) {
    m_simTimer = new QTimer(this);
    m_simTimer->setInterval(1000);  // 1s for crypto (slower than futures)
    QObject::connect(m_simTimer, &QTimer::timeout, this, &OkxAdapter::simulateOkxTick);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

OkxAdapter::~OkxAdapter() { disconnect(); }

void OkxAdapter::connect(const std::string& ws_endpoint) {
    Q_UNUSED(ws_endpoint)
    m_connected = true;
    m_simTimer->start();
    emit connected();
    qDebug() << "[OkxAdapter] Connected (simulation mode)";
}

void OkxAdapter::disconnect() {
    if (m_simTimer) m_simTimer->stop();
    m_connected = false;
    emit disconnected();
}

void OkxAdapter::subscribe(const std::string& instId) {
    m_instrument = instId;
    // Set base price based on instrument
    if (instId.find("BTC") != std::string::npos) m_simPrice = 87000.0;
    else if (instId.find("ETH") != std::string::npos) m_simPrice = 3400.0;
    else m_simPrice = 100.0;
    qDebug() << "[OkxAdapter] Subscribed:" << instId.c_str();
}

void OkxAdapter::simulateOkxTick() {
    ctp::DepthMarketData tick;
    std::strncpy(tick.InstrumentID, m_instrument.c_str(), sizeof(tick.InstrumentID));
    std::strncpy(tick.ExchangeID, "OKX", sizeof(tick.ExchangeID));

    double change = (std::rand() % 200 - 100) * 0.5;
    m_simPrice += change;
    if (m_simPrice < 1.0) m_simPrice = 1.0;

    tick.LastPrice  = m_simPrice;
    tick.BidPrice1  = m_simPrice - 0.5;
    tick.AskPrice1  = m_simPrice + 0.5;
    tick.BidVolume1 = 1;
    tick.AskVolume1 = 1;
    tick.Volume     = 100 + std::rand() % 1000;
    tick.OpenInterest = 0;
    tick.UpdateMillisec = static_cast<int>(std::time(nullptr)) % 1000;

    if (m_onTick) m_onTick(tick);
    emit tickReceived(tick);
}

} // namespace qp::data
