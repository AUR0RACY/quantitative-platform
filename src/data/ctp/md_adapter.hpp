#pragma once
// CTP Market Data adapter — wraps CThostFtdcMdApi callbacks into Qt signals/slots

#include "ctp_types.hpp"
#include <string>
#include <functional>
#include <QObject>
#include <QTimer>
#include <memory>
#include <atomic>

namespace qp::data {

using TickCallback = std::function<void(const ctp::DepthMarketData&)>;

class MdAdapter : public QObject {
    Q_OBJECT
public:
    explicit MdAdapter(QObject *parent = nullptr);
    ~MdAdapter() override;

    // ---- Connection ----
    void connectCtp(const std::string& address,       // tcp://180.168.146.187:10211
                 const std::string& broker_id,
                 const std::string& user_id,
                 const std::string& password);
    void disconnect();

    // ---- Subscription ----
    void subscribe(const std::string& instrument);
    void unsubscribe(const std::string& instrument);

    // ---- Status ----
    bool is_connected() const { return m_connected; }
    bool is_logged_in() const { return m_loggedIn; }

    // ---- Callback ----
    void set_on_tick(TickCallback cb) { m_onTick = cb; }

signals:
    void connected();
    void disconnected();
    void tickReceived(const ctp::DepthMarketData &tick);
    void errorOccurred(const QString &msg);

private:
    void simulateTick();         // generate test ticks until real CTP dll is available

    // Real CTP state (placeholder pointers — cast to CThostFtdcMdApi when dll loaded)
    void* m_mdApi = nullptr;     // CThostFtdcMdApi*
    std::atomic<bool> m_connected{false};
    std::atomic<bool> m_loggedIn{false};

    std::string m_brokerId;
    std::string m_userId;
    TickCallback m_onTick;

    QTimer *m_simTimer = nullptr;  // temporary: simulated tick timer
    bool m_simulationMode = true;
    double m_simPrice = 5200.0;
};

} // namespace qp::data
