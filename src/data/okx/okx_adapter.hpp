#pragma once
// OKX market data adapter — REST snapshot + WebSocket streaming
// Phase 6: REST-only implementation; WebSocket in future.

#include "data/ctp/ctp_types.hpp"
#include <QObject>
#include <QTimer>
#include <functional>
#include <string>
#include <atomic>

namespace qp::data {

using OkxTickCallback = std::function<void(const ctp::DepthMarketData&)>;

class OkxAdapter : public QObject {
    Q_OBJECT
public:
    explicit OkxAdapter(QObject *parent = nullptr);
    ~OkxAdapter() override;

    void connect(const std::string& ws_endpoint = "wss://ws.okx.com:8443/ws/v5/public");
    void disconnect();
    void subscribe(const std::string& instId);   // e.g. "BTC-USDT-SWAP"
    void set_on_tick(OkxTickCallback cb) { m_onTick = cb; }
    bool is_connected() const { return m_connected; }

signals:
    void connected();
    void disconnected();
    void tickReceived(const qp::ctp::DepthMarketData &tick);
    void errorOccurred(const QString &msg);

private:
    void simulateOkxTick();

    QTimer *m_simTimer = nullptr;
    std::atomic<bool> m_connected{false};
    std::string m_instrument;
    OkxTickCallback m_onTick;
    bool m_simulationMode = true;
    double m_simPrice = 87000.0;  // BTC default
};

} // namespace qp::data
