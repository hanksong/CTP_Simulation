#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <cstring>
// 使用C风格的ZeroMQ API - 这里使用绝对路径，需要确保此路径存在
#include "/opt/homebrew/include/zmq.h"

int main() {
    std::cout << "Starting ZeroMQ Market Data Receiver..." << std::endl;
    
    // 初始化ZeroMQ上下文和套接字
    void* context = zmq_ctx_new();
    void* subscriber = zmq_socket(context, ZMQ_PULL);
    
    try {
        // 连接到发布者
        int rc = zmq_connect(subscriber, "tcp://localhost:5555");
        if (rc != 0) {
            std::cerr << "ZeroMQ连接失败: " << zmq_strerror(zmq_errno()) << std::endl;
            zmq_close(subscriber);
            zmq_ctx_destroy(context);
            return 1;
        }
        
        std::cout << "Connected to the market data publisher on port 5555" << std::endl;
        
        // 用于ZeroMQ轮询的结构
        zmq_pollitem_t items[1];
        items[0].socket = subscriber;
        items[0].events = ZMQ_POLLIN;
        
        // 缓冲区，用于接收消息
        char buffer[1024];
        
        // 接收和显示消息
        while (true) {
            // 使用轮询来实现非阻塞接收
            int poll_rc = zmq_poll(items, 1, 1); // 超时1毫秒
            
            if (poll_rc > 0 && (items[0].revents & ZMQ_POLLIN)) {
                // 接收消息
                int recv_size = zmq_recv(subscriber, buffer, sizeof(buffer) - 1, 0);
                if (recv_size > 0) {
                    // 确保字符串以null结尾
                    buffer[recv_size] = '\0';
                    
                    // 显示接收到的数据
                    std::string data(buffer, recv_size);
                    std::cout << "Received market data: " << data << std::endl;
                    
                    // 解析消息（格式：instrumentID,price）
                    size_t commaPos = data.find(',');
                    if (commaPos != std::string::npos) {
                        std::string instrument = data.substr(0, commaPos);
                        double price = std::stod(data.substr(commaPos + 1));
                        
                        std::cout << "Instrument: " << instrument << ", Price: " << price << std::endl;
                    }
                }
            }
            
            // 短暂休眠以避免CPU占用过高
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        zmq_close(subscriber);
        zmq_ctx_destroy(context);
        return 1;
    }
    
    // 清理资源（理论上不会执行到这里，因为上面的循环是无限的）
    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    
    return 0;
} 