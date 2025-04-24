#include "TraderSpi.h"
#include <iostream>
#include <cstring>

TraderSpi::TraderSpi(CThostFtdcTraderApi* api)
    : m_pTraderApi(api), m_nRequestID(0), m_frontID(0), m_sessionID(0)
{
}

TraderSpi::~TraderSpi()
{
}

void TraderSpi::OnFrontConnected()
{
    std::cout << "Trader front connected" << std::endl;
}

void TraderSpi::OnFrontDisconnected(int nReason)
{
    std::cout << "Trader front disconnected, reason: " << nReason << std::endl;
}

void TraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
        std::cout << "Trader logged in successfully" << std::endl;
        
        if (pRspUserLogin)
        {
            m_frontID = pRspUserLogin->FrontID;
            m_sessionID = pRspUserLogin->SessionID;
            m_tradingDay = pRspUserLogin->TradingDay;
            
            std::cout << "Trading day: " << m_tradingDay << std::endl;
            std::cout << "Front ID: " << m_frontID << std::endl;
            std::cout << "Session ID: " << m_sessionID << std::endl;
            
            // After successful login, confirm settlement info
            ConfirmSettlement();
        }
    }
}

void TraderSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
        std::cout << "Trader logged out successfully" << std::endl;
    }
}

void TraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
        std::cout << "Settlement info confirmed" << std::endl;
        if (pSettlementInfoConfirm)
        {
            std::cout << "Confirmation date: " << pSettlementInfoConfirm->ConfirmDate 
                      << ", time: " << pSettlementInfoConfirm->ConfirmTime << std::endl;
        }
    }
}

void TraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo) && pInstrument)
    {
        std::cout << "Instrument ID: " << pInstrument->InstrumentID 
                  << ", Exchange ID: " << pInstrument->ExchangeID
                  << ", Product ID: " << pInstrument->ProductID
                  << ", Product Class: " << pInstrument->ProductClass << std::endl;
    }
    
    if (bIsLast)
    {
        std::cout << "Instrument query completed" << std::endl;
    }
}

void TraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo) && pTradingAccount)
    {
        std::cout << "Account ID: " << pTradingAccount->AccountID
                  << ", Available: " << pTradingAccount->Available
                  << ", Balance: " << pTradingAccount->Balance
                  << ", Frozen Margin: " << pTradingAccount->FrozenMargin << std::endl;
    }
    
    if (bIsLast)
    {
        std::cout << "Trading account query completed" << std::endl;
    }
}

void TraderSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo) && pInvestorPosition)
    {
        std::cout << "Instrument ID: " << pInvestorPosition->InstrumentID
                  << ", Position: " << pInvestorPosition->Position
                  << ", Position Cost: " << pInvestorPosition->PositionCost
                  << ", Position Date: " << pInvestorPosition->PositionDate << std::endl;
    }
    
    if (bIsLast)
    {
        std::cout << "Position query completed" << std::endl;
    }
}

void TraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (IsErrorRspInfo(pRspInfo))
    {
        std::cout << "Order insert failed" << std::endl;
        if (pInputOrder)
        {
            std::cout << "Instrument ID: " << pInputOrder->InstrumentID
                      << ", Price: " << pInputOrder->LimitPrice
                      << ", Volume: " << pInputOrder->VolumeTotalOriginal << std::endl;
        }
    }
}

void TraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    if (pOrder)
    {
        std::cout << "Order update - Instrument ID: " << pOrder->InstrumentID
                  << ", Order Status: " << pOrder->OrderStatus
                  << ", Volume Traded: " << pOrder->VolumeTraded
                  << ", Volume Total: " << pOrder->VolumeTotal << std::endl;
    }
}

void TraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    if (pTrade)
    {
        std::cout << "Trade - Instrument ID: " << pTrade->InstrumentID
                  << ", Price: " << pTrade->Price
                  << ", Volume: " << pTrade->Volume
                  << ", Direction: " << pTrade->Direction
                  << ", Offset Flag: " << pTrade->OffsetFlag << std::endl;
    }
}

void TraderSpi::Login(const std::string& brokerID, const std::string& userID, const std::string& password)
{
    m_brokerID = brokerID;
    m_userID = userID;
    m_investorID = userID;  // In CTP, usually investorID is the same as userID
    
    CThostFtdcReqUserLoginField req = {0};
    strncpy(req.BrokerID, brokerID.c_str(), sizeof(req.BrokerID) - 1);
    strncpy(req.UserID, userID.c_str(), sizeof(req.UserID) - 1);
    strncpy(req.Password, password.c_str(), sizeof(req.Password) - 1);
    // 添加系统信息参数
    TThostFtdcSystemInfoLenType length = 0;
    TThostFtdcClientSystemInfoType systemInfo = {0};

    int result = m_pTraderApi->ReqUserLogin(&req, ++m_nRequestID, length, systemInfo);
    std::cout << "Sending trader login request: " << ((result == 0) ? "Success" : "Failure") << std::endl;
}

void TraderSpi::ConfirmSettlement()
{
    CThostFtdcSettlementInfoConfirmField req = {0};
    strncpy(req.BrokerID, m_brokerID.c_str(), sizeof(req.BrokerID) - 1);
    strncpy(req.InvestorID, m_investorID.c_str(), sizeof(req.InvestorID) - 1);
    
    int result = m_pTraderApi->ReqSettlementInfoConfirm(&req, ++m_nRequestID);
    std::cout << "Sending settlement confirmation request: " << ((result == 0) ? "Success" : "Failure") << std::endl;
}

void TraderSpi::QryInstrument(const std::string& instrumentID)
{
    CThostFtdcQryInstrumentField req = {0};
    if (!instrumentID.empty())
    {
        strncpy(req.InstrumentID, instrumentID.c_str(), sizeof(req.InstrumentID) - 1);
    }
    
    int result = m_pTraderApi->ReqQryInstrument(&req, ++m_nRequestID);
    std::cout << "Sending instrument query request: " << ((result == 0) ? "Success" : "Failure") << std::endl;
}

void TraderSpi::QryTradingAccount()
{
    CThostFtdcQryTradingAccountField req = {0};
    strncpy(req.BrokerID, m_brokerID.c_str(), sizeof(req.BrokerID) - 1);
    strncpy(req.InvestorID, m_investorID.c_str(), sizeof(req.InvestorID) - 1);
    
    int result = m_pTraderApi->ReqQryTradingAccount(&req, ++m_nRequestID);
    std::cout << "Sending trading account query request: " << ((result == 0) ? "Success" : "Failure") << std::endl;
}

void TraderSpi::QryInvestorPosition(const std::string& instrumentID)
{
    CThostFtdcQryInvestorPositionField req = {0};
    strncpy(req.BrokerID, m_brokerID.c_str(), sizeof(req.BrokerID) - 1);
    strncpy(req.InvestorID, m_investorID.c_str(), sizeof(req.InvestorID) - 1);
    
    if (!instrumentID.empty())
    {
        strncpy(req.InstrumentID, instrumentID.c_str(), sizeof(req.InstrumentID) - 1);
    }
    
    int result = m_pTraderApi->ReqQryInvestorPosition(&req, ++m_nRequestID);
    std::cout << "Sending position query request: " << ((result == 0) ? "Success" : "Failure") << std::endl;
}

void TraderSpi::OrderInsert(const std::string& instrumentID, char direction, char offsetFlag, 
                          double price, int volume, char priceType)
{
    CThostFtdcInputOrderField req = {0};
    
    // Basic order information
    strncpy(req.BrokerID, m_brokerID.c_str(), sizeof(req.BrokerID) - 1);
    strncpy(req.InvestorID, m_investorID.c_str(), sizeof(req.InvestorID) - 1);
    strncpy(req.UserID, m_userID.c_str(), sizeof(req.UserID) - 1);
    strncpy(req.InstrumentID, instrumentID.c_str(), sizeof(req.InstrumentID) - 1);
    
    // Order reference - format: frontID + sessionID + requestID
    std::string orderRef = std::to_string(m_frontID) + std::to_string(m_sessionID) + std::to_string(m_nRequestID);
    strncpy(req.OrderRef, orderRef.c_str(), sizeof(req.OrderRef) - 1);
    
    // Order price and volume
    req.Direction = direction;  // '0' for Buy, '1' for Sell
    req.CombOffsetFlag[0] = offsetFlag;  // '0' for Open, '1' for Close
    req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;  // '1' for Speculation
    req.LimitPrice = price;
    req.VolumeTotalOriginal = volume;
    
    // Other settings
    req.OrderPriceType = priceType;  // '2' for Limit Price
    req.TimeCondition = THOST_FTDC_TC_GFD;  // Good For Day
    req.VolumeCondition = THOST_FTDC_VC_AV;  // Any Volume
    req.MinVolume = 1;
    req.ContingentCondition = THOST_FTDC_CC_Immediately;  // Immediately
    req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;  // Not Force Close
    req.IsAutoSuspend = 0;
    req.UserForceClose = 0;
    
    int result = m_pTraderApi->ReqOrderInsert(&req, ++m_nRequestID);
    std::cout << "Sending order insert request: " << ((result == 0) ? "Success" : "Failure") << std::endl;
}

bool TraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
    bool bResult = (pRspInfo != nullptr) && (pRspInfo->ErrorID != 0);
    if (bResult)
    {
        std::cout << "Error ID: " << pRspInfo->ErrorID 
                  << ", Error Msg: " << pRspInfo->ErrorMsg << std::endl;
    }
    return bResult;
} 