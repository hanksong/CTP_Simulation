#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <map>
#include "mdapi/MdSpi.h"

// Function to read config file
std::map<std::string, std::string> readConfig(const std::string& filename) {
    std::map<std::string, std::string> config;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << filename << std::endl;
        return config;
    }
    
    std::string line;
    std::string section;
    
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || line[0] == '#')
            continue;
            
        // Section header
        if (line[0] == '[' && line[line.length() - 1] == ']') {
            section = line.substr(1, line.length() - 2);
            continue;
        }
        
        // Key-value pair
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = section + "." + line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            config[key] = value;
        }
    }
    
    return config;
}

// Function to split a string by delimiter
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int main(int argc, char* argv[])
{
    std::cout << "CTP Market Data Test Starting..." << std::endl;
    
    // Read configuration
    std::string configFile = "config.ini";
    if (argc > 1) {
        configFile = argv[1];
    }
    
    std::cout << "Reading configuration from " << configFile << std::endl;
    auto config = readConfig(configFile);
    
    // Check if the necessary config values are available
    if (config["CTP.BrokerID"].empty() || config["CTP.UserID"].empty() || 
        config["CTP.Password"].empty() || config["CTP.MdFrontAddr"].empty()) {
        std::cerr << "Missing required configuration. Please check " << configFile << std::endl;
        return 1;
    }
    
    const std::string BROKER_ID = config["CTP.BrokerID"];
    const std::string USER_ID = config["CTP.UserID"];
    const std::string PASSWORD = config["CTP.Password"];
    const std::string MD_FRONT_ADDR = config["CTP.MdFrontAddr"];
    
    // Parse instruments
    std::vector<std::string> INSTRUMENTS;
    if (!config["Instruments.List"].empty()) {
        INSTRUMENTS = split(config["Instruments.List"], ',');
    } else {
        INSTRUMENTS = {"rb2410", "IF2406"};  // Default if not specified
    }
    
    std::cout << "Configured to use:" << std::endl
              << "  Broker ID: " << BROKER_ID << std::endl
              << "  User ID: " << USER_ID << std::endl
              << "  MD Front: " << MD_FRONT_ADDR << std::endl
              << "  Instruments: ";
              
    for (const auto& instrument : INSTRUMENTS) {
        std::cout << instrument << " ";
    }
    std::cout << std::endl;

    // Initialize Market Data API
    CThostFtdcMdApi* pMdApi = CThostFtdcMdApi::CreateFtdcMdApi();
    MdSpi mdSpi(pMdApi);
    pMdApi->RegisterSpi(&mdSpi);
    pMdApi->RegisterFront(const_cast<char*>(MD_FRONT_ADDR.c_str()));
    pMdApi->Init();
    std::cout << "Market Data API initialized, version: " << pMdApi->GetApiVersion() << std::endl;

    // Wait for connection
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Login to market data API
    mdSpi.Login(BROKER_ID, USER_ID, PASSWORD);

    // Wait for login to complete
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Subscribe to market data
    mdSpi.SubscribeMarketData(INSTRUMENTS);

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    // Clean up
    pMdApi->Release();

    return 0;
} 