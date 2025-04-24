#pragma once

#include <string>
// 简化后的头文件包含
#include "ThostFtdcTraderApi.h"

class TraderSpi : public CThostFtdcTraderSpi
{
public:
    TraderSpi(CThostFtdcTraderApi* api);
    ~TraderSpi();

    // User API callback implementation
    void OnFrontConnected() override;
    void OnFrontDisconnected(int nReason) override;
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRtnOrder(CThostFtdcOrderField *pOrder) override;
    void OnRtnTrade(CThostFtdcTradeField *pTrade) override;

    // Business functions
    void Login(const std::string& brokerID, const std::string& userID, const std::string& password);
    void ConfirmSettlement();
    void QryInstrument(const std::string& instrumentID);
    void QryTradingAccount();
    void QryInvestorPosition(const std::string& instrumentID);
    void OrderInsert(const std::string& instrumentID, char direction, char offsetFlag, 
                    double price, int volume, char priceType);

private:
    bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
    CThostFtdcTraderApi* m_pTraderApi;
    int m_nRequestID;
    
    std::string m_brokerID;
    std::string m_userID;
    std::string m_investorID;
    std::string m_tradingDay;
    int m_frontID;
    int m_sessionID;
}; 