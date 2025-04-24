# CTP 交易API实现

2025 年 4 月

本项目为学习用，使用SimNow的模拟环境，利用上期所的CTP API实现一个完整交易流程的主要功能。

## 项目功能

1. 实现数据的实时订阅
2. 数据订阅后通过zeromq Push给strategyengine
3. strategyengine从zmq中拉取数据，并将报单输送给orderapi
4. orderapi实现下单，callback等。

## 项目结构

```
CTP_API/
├── CMakeLists.txt                 # CMake构建文件
├── README.md                      # 项目说明文档
├── config.ini                     # 配置文件
├── run_mdapi_test.sh              # 行情模块测试脚本
├── test_market_data.sh            # 行情模块和ZeroMQ测试脚本
├── TraderapiMduserapi_6.7.7_MacOS # CTP API框架
└── src/                           # 源代码目录
    ├── main.cpp                   # 主程序入口
    ├── test_mdapi.cpp             # 行情模块测试程序
    ├── test_zmq_receiver.cpp      # ZeroMQ接收测试程序
    ├── mdapi/                     # 市场数据模块
    │   ├── MdSpi.h                # 市场数据回调接口类
    │   └── MdSpi.cpp              # 市场数据回调接口实现
    ├── traderapi/                 # 交易API模块
    │   ├── TraderSpi.h            # 交易回调接口类
    │   └── TraderSpi.cpp          # 交易回调接口实现
    └── strategy/                  # 策略引擎模块
        ├── StrategyEngine.h       # 策略引擎类
        └── StrategyEngine.cpp     # 策略引擎实现
```

## 依赖项

1. ZeroMQ (用于进程间通信)
2. CTP API 6.7.7 (用于市场数据和交易接口)
3. CMake (构建系统)
4. C++14 兼容的编译器

## 构建指南

### 安装依赖项

在MacOS上安装依赖项:

```bash
# 安装ZeroMQ
brew install zeromq

# 安装CMake (如果未安装)
brew install cmake
```

### 构建项目

```bash
# 创建构建目录
mkdir build && cd build

# 生成Makefile
cmake ..

# 编译项目
make

# 安装 (可选)
make install
```

## 使用指南

### 配置文件

在运行程序前，需要编辑配置文件 `config.ini`，填入您的SimNow账号信息：

```ini
[CTP]
BrokerID=9999
UserID=您的SimNow账号
Password=您的SimNow密码
MdFrontAddr=tcp://180.168.146.187:10212
TradeFrontAddr=tcp://180.168.146.187:10202

[Instruments]
List=rb2410,IF2406
```

### 测试行情模块

项目提供了单独测试行情模块的两种方式：

1. 只测试行情接收:

```bash
./run_mdapi_test.sh
```

2. 同时测试行情接收和ZeroMQ发布/订阅:

```bash
./test_market_data.sh
```

第二种方式会同时启动行情接收程序和ZeroMQ接收程序，可以验证整个数据流是否正常工作。

### 运行完整系统

```bash
./bin/ctp_trading
```

## 修改策略

要修改交易策略，编辑 `src/strategy/StrategyEngine.cpp` 文件中的 `OnMarketData` 方法，实现您自己的交易逻辑。

## 注意事项

1. 本项目仅用于学习和研究，不应在实际交易环境中使用。
2. 使用SimNow环境进行测试时，需要注意交易时间限制。
3. 请遵守相关法律法规和交易所规则。

## 扩展方向

1. 实现完整的行情和交易数据持久化
2. 添加风控模块
3. 实现更复杂的策略引擎和回测系统
4. 添加Web界面进行监控和管理
