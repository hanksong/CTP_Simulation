#include "MdSpi.h"
#include <iostream>
#include <cstring>
// 使用C风格的ZeroMQ API - 这里使用绝对路径，需要确保此路径存在
#include "/opt/homebrew/include/zmq.h"

// Global ZeroMQ context and socket
static void* context = zmq_ctx_new();
static void* publisher = zmq_socket(context, ZMQ_PUSH);

MdSpi::MdSpi(CThostFtdcMdApi* api)
    : m_pMdApi(api), m_nRequestID(0)
{
    // Setup ZeroMQ publisher
    int rc = zmq_bind(publisher, "tcp://*:5555");
    if (rc != 0) {
        std::cerr << "ZeroMQ绑定失败: " << zmq_strerror(zmq_errno()) << std::endl;
    } else {
        std::cout << "Market data publisher started on port 5555" << std::endl;
    }
}

MdSpi::~MdSpi()
{
    // 清理ZeroMQ资源
    zmq_close(publisher);
    zmq_ctx_destroy(context);
}

void MdSpi::OnFrontConnected()
{
    std::cout << "Market data front connected" << std::endl;
}

void MdSpi::OnFrontDisconnected(int nReason)
{
    std::cout << "Market data front disconnected, reason: " << nReason << std::endl;
}

void MdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
        std::cout << "Market data logged in successfully" << std::endl;
        if (pRspUserLogin)
        {
            std::cout << "Trading day: " << pRspUserLogin->TradingDay << std::endl;
        }
    }
}

void MdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
        std::cout << "Market data logged out successfully" << std::endl;
    }
}

void MdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo))
    {
        if (pSpecificInstrument)
        {
            std::cout << "Market data subscription successful for instrument: " 
                      << pSpecificInstrument->InstrumentID << std::endl;
        }
    }
}

void MdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    if (pDepthMarketData)
    {
        std::cout << "Received market data for: " << pDepthMarketData->InstrumentID 
                  << ", Trading Day: " << pDepthMarketData->TradingDay
                  << ", Last Price: " << pDepthMarketData->LastPrice 
                  << ", Open Price: " << pDepthMarketData->OpenPrice
                  << ", Close Price: " << pDepthMarketData->ClosePrice
                  << std::endl;
                
        // Serialize and send market data through ZeroMQ
        // For simplicity, we're just sending the instrument ID and last price
        std::string message = std::string(pDepthMarketData->InstrumentID) + "," 
                            + std::string(pDepthMarketData->TradingDay) + ","
                            + std::to_string(pDepthMarketData->LastPrice) + ","
                            + std::to_string(pDepthMarketData->OpenPrice) + ","
                            + std::to_string(pDepthMarketData->ClosePrice);
        
        // 使用C风格API发送消息
        int rc = zmq_send(publisher, message.c_str(), message.size(), 0);
        if (rc == -1) {
            std::cerr << "ZeroMQ发送失败: " << zmq_strerror(zmq_errno()) << std::endl;
        }
    }
}

void MdSpi::Login(const std::string& brokerID, const std::string& userID, const std::string& password)
{
    CThostFtdcReqUserLoginField req = {0};
    strncpy(req.BrokerID, brokerID.c_str(), sizeof(req.BrokerID) - 1);
    strncpy(req.UserID, userID.c_str(), sizeof(req.UserID) - 1);
    strncpy(req.Password, password.c_str(), sizeof(req.Password) - 1);
    
    int result = m_pMdApi->ReqUserLogin(&req, ++m_nRequestID);
    std::cout << "Sending login request: " << ((result == 0) ? "Success" : "Failure") << std::endl;
    // 错误信息
    if (result != 0) {
        std::cerr << "Login failed, error code: " << result << std::endl;
    }
}

void MdSpi::SubscribeMarketData(const std::vector<std::string>& instruments)
{
    if (instruments.empty())
    {
        std::cout << "No instruments to subscribe" << std::endl;
        return;
    }
    
    // Convert instrument strings to char*[]
    char** ppInstrumentID = new char*[instruments.size()];
    for (size_t i = 0; i < instruments.size(); ++i)
    {
        ppInstrumentID[i] = new char[31]; // CTP instrument ID max length is 31
        strncpy(ppInstrumentID[i], instruments[i].c_str(), 30);
        ppInstrumentID[i][30] = '\0';
    }
    
    int result = m_pMdApi->SubscribeMarketData(ppInstrumentID, instruments.size());
    std::cout << "Subscribing to market data: " << ((result == 0) ? "Success" : "Failure") << std::endl;
    
    // Clean up
    for (size_t i = 0; i < instruments.size(); ++i)
    {
        delete[] ppInstrumentID[i];
    }
    delete[] ppInstrumentID;
}

bool MdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
    bool bResult = (pRspInfo != nullptr) && (pRspInfo->ErrorID != 0);
    if (bResult)
    {
        std::cout << "Error ID: " << pRspInfo->ErrorID 
                  << ", Error Msg: " << pRspInfo->ErrorMsg << std::endl;
    }
    return bResult;
} 