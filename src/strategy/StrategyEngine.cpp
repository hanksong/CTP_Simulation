#include "StrategyEngine.h"
#include <iostream>
#include <sstream>
#include <cstring>
// 使用C风格的ZeroMQ API - 这里使用绝对路径，需要确保此路径存在
#include "/opt/homebrew/include/zmq.h"

StrategyEngine::StrategyEngine()
    : m_running(false), m_orderCallback(nullptr)
{
}

StrategyEngine::~StrategyEngine()
{
    Stop();
}

void StrategyEngine::Start()
{
    if (!m_running.exchange(true))
    {
        // Start the data processing thread
        m_thread = std::thread([this] { ProcessMarketData(); });
        std::cout << "Strategy engine started" << std::endl;
    }
}

void StrategyEngine::Stop()
{
    if (m_running.exchange(false))
    {
        if (m_thread.joinable())
        {
            m_thread.join();
            std::cout << "Strategy engine stopped" << std::endl;
        }
    }
}

void StrategyEngine::Subscribe(const std::vector<std::string>& instruments)
{
    std::cout << "Strategy engine subscribing to:" << std::endl;
    for (const auto& instrument : instruments)
    {
        std::cout << "  - " << instrument << std::endl;
    }
}

void StrategyEngine::SetOrderCallback(OrderCallbackFunc callback)
{
    m_orderCallback = callback;
}

void StrategyEngine::ProcessMarketData()
{
    // Setup ZeroMQ subscriber using C API
    void* context = zmq_ctx_new();
    void* subscriber = zmq_socket(context, ZMQ_PULL);
    
    int rc = zmq_connect(subscriber, "tcp://localhost:5555");
    if (rc != 0) {
        std::cerr << "ZeroMQ连接失败: " << zmq_strerror(zmq_errno()) << std::endl;
        zmq_close(subscriber);
        zmq_ctx_destroy(context);
        return;
    }
    
    std::cout << "Market data processor connected to ZeroMQ" << std::endl;
    
    // 用于ZeroMQ轮询的结构
    zmq_pollitem_t items[1];
    items[0].socket = subscriber;
    items[0].events = ZMQ_POLLIN;
    
    // 缓冲区，用于接收消息
    char buffer[1024];
    
    while (m_running)
    {
        // 使用轮询来实现非阻塞接收
        int poll_rc = zmq_poll(items, 1, 1); // 超时1毫秒
        
        if (poll_rc > 0 && (items[0].revents & ZMQ_POLLIN)) {
            // 接收消息
            int recv_size = zmq_recv(subscriber, buffer, sizeof(buffer) - 1, 0);
            if (recv_size > 0) {
                // 确保字符串以null结尾
                buffer[recv_size] = '\0';
                
                // 处理接收到的数据
                std::string data(buffer, recv_size);
                
                // 解析消息 - 格式: instrumentID,price
                std::istringstream ss(data);
                std::string instrument;
                double price;
                
                std::getline(ss, instrument, ',');
                ss >> price;
                
                // 处理市场数据
                OnMarketData(instrument, price);
            }
        }
        
        // 短暂休眠以避免CPU占用过高
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // 清理ZeroMQ资源
    zmq_close(subscriber);
    zmq_ctx_destroy(context);
}

void StrategyEngine::OnMarketData(const std::string& instrument, double price)
{
    std::cout << "Strategy received data: " << instrument << " @ " << price << std::endl;
    
    // Implement your strategy logic here
    // For demonstration, we'll just place a simple order when price is above a threshold
    static double lastPrice = 0.0;
    
    if (m_orderCallback && lastPrice > 0.0)
    {
        // Example strategy: Buy when price increases by 0.5%
        if (price > lastPrice * 1.005)
        {
            std::cout << "Strategy triggered: Buy order for " << instrument << std::endl;
            
            // Direction: '0' for Buy
            // OffsetFlag: '0' for Open
            // PriceType: '2' for LimitPrice
            m_orderCallback(instrument, '0', '0', price, 1, '2');
        }
        // Sell when price decreases by 0.5%
        else if (price < lastPrice * 0.995)
        {
            std::cout << "Strategy triggered: Sell order for " << instrument << std::endl;
            
            // Direction: '1' for Sell
            // OffsetFlag: '1' for Close
            // PriceType: '2' for LimitPrice
            m_orderCallback(instrument, '1', '1', price, 1, '2');
        }
    }
    
    lastPrice = price;
} 