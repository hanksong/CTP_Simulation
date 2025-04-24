#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "mdapi/MdSpi.h"
#include "traderapi/TraderSpi.h"
#include "strategy/StrategyEngine.h"

// Config parameters - would typically come from a config file
const std::string BROKER_ID = "9999";  // SimNow broker ID
const std::string USER_ID = "your_simnow_userid"; 
const std::string PASSWORD = "your_simnow_password";
const std::string MD_FRONT_ADDR = "tcp://180.168.146.187:10212";  // SimNow market data front
const std::string TRADE_FRONT_ADDR = "tcp://180.168.146.187:10202";  // SimNow trade front
const std::vector<std::string> INSTRUMENTS = {"rb2510", "IF2510"};  // Example instruments

// 全局指针，用于回调函数
TraderSpi* g_pTraderSpi = nullptr;

// 回调函数实现
void OrderCallback(const std::string& instrument, 
                  char direction, char offsetFlag,
                  double price, int volume, char priceType) {
    if (g_pTraderSpi != nullptr) {
        g_pTraderSpi->OrderInsert(instrument, direction, offsetFlag, price, volume, priceType);
    } else {
        std::cerr << "Error: TraderSpi not initialized in callback" << std::endl;
    }
}

int main(int argc, char* argv[])
{
    std::cout << "CTP Trading API Application Starting..." << std::endl;

    // Initialize Market Data API
    CThostFtdcMdApi* pMdApi = CThostFtdcMdApi::CreateFtdcMdApi();
    MdSpi mdSpi(pMdApi);
    pMdApi->RegisterSpi(&mdSpi);
    pMdApi->RegisterFront(const_cast<char*>(MD_FRONT_ADDR.c_str()));
    pMdApi->Init();
    std::cout << "Market Data API initialized, version: " << pMdApi->GetApiVersion() << std::endl;

    // Initialize Trader API
    CThostFtdcTraderApi* pTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
    TraderSpi traderSpi(pTraderApi);
    pTraderApi->RegisterSpi(&traderSpi);
    pTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);
    pTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
    pTraderApi->RegisterFront(const_cast<char*>(TRADE_FRONT_ADDR.c_str()));
    pTraderApi->Init();
    std::cout << "Trader API initialized, version: " << pTraderApi->GetApiVersion() << std::endl;

    // Wait for connection
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Login to market data and trader API
    mdSpi.Login(BROKER_ID, USER_ID, PASSWORD);
    traderSpi.Login(BROKER_ID, USER_ID, PASSWORD);

    // Wait for login to complete
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Subscribe to market data
    mdSpi.SubscribeMarketData(INSTRUMENTS);

    // Initialize and start strategy engine
    StrategyEngine strategy;
    
    // Set up callback from strategy to trader API
    // 设置全局指针，以便回调函数使用
    g_pTraderSpi = &traderSpi;
    strategy.SetOrderCallback(OrderCallback);

    strategy.Subscribe(INSTRUMENTS);
    strategy.Start();

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    // Clean up
    strategy.Stop();
    g_pTraderSpi = nullptr;  // 清除全局指针
    pTraderApi->Release();
    pMdApi->Release();

    return 0;
} 