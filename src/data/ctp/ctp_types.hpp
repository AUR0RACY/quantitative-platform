#pragma once
// CTP API type stubs — matches official CTP v6.7.x structures.
// Replace with official ThostFtdcUserApiStruct.h when available.

#include <cstdint>
#include <cstring>

// ---- CTP constants ----
static constexpr int TFTD_LEN_InstrumentID   = 32;
static constexpr int TFTD_LEN_ExchangeID     = 10;
static constexpr int TFTD_LEN_BrokerID       = 12;
static constexpr int TFTD_LEN_InvestorID     = 14;
static constexpr int TFTD_LEN_Password       = 42;
static constexpr int TFTD_LEN_AccountID      = 14;
static constexpr int TFTD_LEN_TradingDay     = 10;
static constexpr int TFTD_LEN_OrderSysID     = 22;
static constexpr int TFTD_LEN_ExchangeInstID = 32;
static constexpr int TFTD_LEN_OrderRef       = 14;
static constexpr int TFTD_LEN_TradeID        = 22;
static constexpr int TFTD_LEN_ErrorMsg       = 82;
static constexpr int TFTD_LEN_ClientID       = 12;
static constexpr int TFTD_LEN_AuthCode       = 18;
static constexpr int TFTD_LEN_AppID          = 34;

namespace qp::ctp {

// ---- Response info ----
struct RspInfo {
    int  ErrorID = 0;
    char ErrorMsg[TFTD_LEN_ErrorMsg] = {};
    bool is_ok() const { return ErrorID == 0; }
};

// ---- Login ----
struct ReqUserLogin {
    char TradingDay[TFTD_LEN_TradingDay] = {};
    char BrokerID[TFTD_LEN_BrokerID]     = {};
    char UserID[TFTD_LEN_InvestorID]     = {};
    char Password[TFTD_LEN_Password]     = {};
};

struct RspUserLogin {
    char TradingDay[TFTD_LEN_TradingDay] = {};
    char BrokerID[TFTD_LEN_BrokerID]     = {};
    char UserID[TFTD_LEN_InvestorID]     = {};
    char InvestorID[TFTD_LEN_InvestorID] = {};
    int  FrontID   = 0;
    int  SessionID = 0;
    int  MaxOrderRef = 0;
};

// ---- User Logout ----
struct UserLogout {
    char BrokerID[TFTD_LEN_BrokerID] = {};
    char UserID[TFTD_LEN_InvestorID] = {};
};

// ---- Market data (tick) ----
struct DepthMarketData {
    char   InstrumentID[TFTD_LEN_InstrumentID] = {};
    char   ExchangeID[TFTD_LEN_ExchangeID]     = {};
    char   TradingDay[TFTD_LEN_TradingDay]     = {};
    char   UpdateTime[10] = {};
    int    UpdateMillisec = 0;
    double LastPrice   = 0.0;
    double PreSettlementPrice = 0.0;
    double PreClosePrice = 0.0;
    double PreOpenInterest = 0.0;
    double OpenPrice   = 0.0;
    double HighestPrice = 0.0;
    double LowestPrice  = 0.0;
    double ClosePrice   = 0.0;
    double UpperLimitPrice = 0.0;
    double LowerLimitPrice = 0.0;
    double Volume       = 0;
    double Turnover     = 0.0;
    double OpenInterest = 0.0;
    double BidPrice1    = 0.0;
    double BidVolume1   = 0;
    double AskPrice1    = 0.0;
    double AskVolume1   = 0;
    double BidPrice2    = 0.0; double BidVolume2 = 0;
    double AskPrice2    = 0.0; double AskVolume2 = 0;
    double BidPrice3    = 0.0; double BidVolume3 = 0;
    double AskPrice3    = 0.0; double AskVolume3 = 0;
    double BidPrice4    = 0.0; double BidVolume4 = 0;
    double AskPrice4    = 0.0; double AskVolume4 = 0;
    double BidPrice5    = 0.0; double BidVolume5 = 0;
    double AskPrice5    = 0.0; double AskVolume5 = 0;
    double AveragePrice = 0.0;
    double SettlementPrice = 0.0;
    double CurrDelta     = 0.0;
};

// ---- Order ----
struct InputOrder {
    char   BrokerID[TFTD_LEN_BrokerID]        = {};
    char   InvestorID[TFTD_LEN_InvestorID]    = {};
    char   InstrumentID[TFTD_LEN_InstrumentID] = {};
    char   OrderRef[TFTD_LEN_OrderRef]         = {};
    char   ExchangeID[TFTD_LEN_ExchangeID]     = {};
    double LimitPrice  = 0.0;
    int    VolumeTotalOriginal = 0;
    char   OrderPriceType = '2';        // 1=any, 2=limit
    char   Direction       = '0';        // 0=buy, 1=sell
    char   CombOffsetFlag[6] = {'0'};    // 0=open, 1=close
    char   CombHedgeFlag[6]  = {'1'};    // 1=speculation
    int    MinVolume       = 1;
    char   TimeCondition   = '3';        // 3=GFD
    char   VolumeCondition = '1';        // 1=AV
    char   ContingentCondition = '1';    // 1=immediately
    char   ForceCloseReason  = '0';      // 0=not force close
    int    IsAutoSuspend     = 0;
    char   UserForceClose    = '\0';
    int    IsSwapOrder       = 0;
};

struct Order {
    char   BrokerID[TFTD_LEN_BrokerID]        = {};
    char   InvestorID[TFTD_LEN_InvestorID]    = {};
    char   InstrumentID[TFTD_LEN_InstrumentID] = {};
    char   OrderRef[TFTD_LEN_OrderRef]         = {};
    char   OrderSysID[TFTD_LEN_OrderSysID]     = {};
    char   ExchangeID[TFTD_LEN_ExchangeID]     = {};
    char   OrderLocalID[TFTD_LEN_OrderRef]     = {};
    char   InsertDate[10] = {};
    char   InsertTime[10] = {};
    double LimitPrice    = 0.0;
    int    VolumeTotalOriginal = 0;
    int    VolumeTraded  = 0;
    char   OrderStatus   = '0';        // 0=all traded, 1=part traded, 2=no traded, 3=cancelling, 5=cancelled
    char   OrderSubmitStatus = '0';
    char   Direction     = '0';
    char   CombOffsetFlag[6] = {};
    char   StatusMsg[TFTD_LEN_ErrorMsg] = {};
    int    FrontID       = 0;
    int    SessionID     = 0;
};

// ---- Trade ----
struct Trade {
    char   BrokerID[TFTD_LEN_BrokerID]        = {};
    char   InvestorID[TFTD_LEN_InvestorID]    = {};
    char   InstrumentID[TFTD_LEN_InstrumentID] = {};
    char   OrderRef[TFTD_LEN_OrderRef]         = {};
    char   OrderSysID[TFTD_LEN_OrderSysID]     = {};
    char   ExchangeID[TFTD_LEN_ExchangeID]     = {};
    char   TradeID[TFTD_LEN_TradeID]           = {};
    char   TradeTime[10] = {};
    double Price        = 0.0;
    int    Volume        = 0;
    char   Direction     = '0';
    char   OffsetFlag    = '0';
    char   HedgeFlag     = '1';
};

// ---- Position ----
struct InvestorPosition {
    char   InstrumentID[TFTD_LEN_InstrumentID] = {};
    char   BrokerID[TFTD_LEN_BrokerID]         = {};
    char   InvestorID[TFTD_LEN_InvestorID]     = {};
    char   ExchangeID[TFTD_LEN_ExchangeID]     = {};
    char   PosiDirection = '2';         // 1=net, 2=long, 3=short
    char   HedgeFlag     = '1';
    int    Position      = 0;
    int    TodayPosition = 0;
    int    YdPosition    = 0;
    double OpenCost      = 0.0;
    double PositionCost  = 0.0;
    double PositionProfit = 0.0;
    double PreSettlementPrice = 0.0;
    double LastPrice     = 0.0;
    double UseMargin     = 0.0;
    double Commission    = 0.0;
    double CloseProfit   = 0.0;
};

// ---- Trading Account ----
struct TradingAccount {
    char   BrokerID[TFTD_LEN_BrokerID]     = {};
    char   AccountID[TFTD_LEN_AccountID]   = {};
    char   InvestorID[TFTD_LEN_InvestorID] = {};
    double PreBalance       = 0.0;
    double Balance          = 0.0;
    double Available        = 0.0;
    double WithdrawQuota    = 0.0;
    double CurrMargin       = 0.0;
    double Commission       = 0.0;
    double CloseProfit      = 0.0;
    double PositionProfit   = 0.0;
    double FrozenMargin     = 0.0;
    double FrozenCommission = 0.0;
};

} // namespace qp::ctp
