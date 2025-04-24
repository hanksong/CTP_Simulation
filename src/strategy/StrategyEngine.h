#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>

// 定义回调函数类型
typedef void (*OrderCallbackFunc)(const std::string& instrument, 
                                  char direction, char offsetFlag,
                                  double price, int volume, char priceType);

class StrategyEngine 
{
public:
    StrategyEngine();
    ~StrategyEngine();

    // Start the strategy engine
    void Start();

    // Stop the strategy engine
    void Stop();

    // Subscribe to market data updates
    void Subscribe(const std::vector<std::string>& instruments);

    // Set order callback
    void SetOrderCallback(OrderCallbackFunc callback);

private:
    // Market data processing thread function
    void ProcessMarketData();

    // Strategy logic - replace with actual strategy
    void OnMarketData(const std::string& instrument, double price);

    std::atomic<bool> m_running;
    std::thread m_thread;
    OrderCallbackFunc m_orderCallback;
}; 