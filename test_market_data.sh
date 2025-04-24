#!/bin/bash

# 首先检查CTP API文件夹是否存在
if [ ! -d "CTP_API_Clean" ]; then
    echo "错误: 找不到CTP API文件夹。请确保您已下载CTP API并放置在CTP_API_Clean目录中。"
    echo "您可以按照以下步骤操作:"
    echo "1. mkdir -p ./CTP_API_Clean/API"
    echo "2. 复制原始API文件: cp -R ./TraderapiMduserapi_6.7.7_MacOS/API/* ./CTP_API_Clean/API/"
    exit 1
fi

# 检查CMake是否已安装
if ! command -v cmake &> /dev/null; then
    echo "错误: 未安装CMake。请运行以下命令安装："
    echo "brew install cmake"
    exit 1
fi

# 检查ZeroMQ是否已安装
if ! brew list --formula | grep -q "^zeromq$"; then
    echo "错误: 未安装ZeroMQ。请运行以下命令安装："
    echo "brew install zeromq"
    exit 1
fi

# 设置ZeroMQ路径
export ZEROMQ_PATH=$(brew --prefix zeromq)

# 设置API目录路径
CTP_API_PATH="$(pwd)/CTP_API_Clean/API"

if [ ! -d "$CTP_API_PATH" ]; then
    echo "错误: 找不到API目录。请检查CTP_API_Clean文件夹结构。"
    exit 1
else
    echo "找到API目录: $CTP_API_PATH"
    export CTP_API_PATH
fi

# 检查config.ini中的凭据
if grep -q "your_simnow_userid" config.ini; then
    echo "警告: 您需要在config.ini中更新您的SimNow凭据。"
    echo "请编辑config.ini文件并填入您的实际账号和密码。"
fi

# 创建构建目录
mkdir -p build
cd build

# 配置CMake
echo "运行CMake配置..."
cmake .. -DCMAKE_PREFIX_PATH="$ZEROMQ_PATH"

# 构建
echo "编译项目..."
make

# 启动ZeroMQ接收器
echo "启动ZeroMQ接收器..."
if [ -f "./bin/test_zmq_receiver" ]; then
    ./bin/test_zmq_receiver &
    ZMQ_PID=$!
    
    # 等待接收器启动
    sleep 1
    
    # 运行市场数据测试
    echo "运行市场数据测试..."
    if [ -f "./bin/test_mdapi" ]; then
        ./bin/test_mdapi
    else
        echo "错误: 找不到测试可执行文件。编译可能未成功。"
        # 如果测试程序不存在，杀掉ZeroMQ接收器
        kill $ZMQ_PID 2>/dev/null
        exit 1
    fi
    
    # 当市场数据测试退出时，杀掉ZeroMQ接收器
    echo "测试完成，正在清理..."
    kill $ZMQ_PID 2>/dev/null
else
    echo "错误: 找不到ZeroMQ接收器可执行文件。编译可能未成功。"
    exit 1
fi

cd .. 