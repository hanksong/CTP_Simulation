#pragma once

#include <string>
#include <vector>
// 简化后的头文件包含
#include "ThostFtdcMdApi.h"

class MdSpi : public CThostFtdcMdSpi
{
public:
    MdSpi(CThostFtdcMdApi* api);
    ~MdSpi();

    // User API callback implementation
    void OnFrontConnected() override;
    void OnFrontDisconnected(int nReason) override;
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) override;

    // Business functions
    void Login(const std::string& brokerID, const std::string& userID, const std::string& password);
    void SubscribeMarketData(const std::vector<std::string>& instruments);
    
private:
    bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
    CThostFtdcMdApi* m_pMdApi;
    int m_nRequestID;
}; 