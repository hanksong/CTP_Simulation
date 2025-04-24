#!/bin/bash

# 检查ZeroMQ是否已安装
if ! brew list --formula | grep -q "^zeromq$"; then
    echo "错误: 未安装ZeroMQ。请运行以下命令安装："
    echo "brew install zeromq"
    exit 1
fi

# 设置ZeroMQ路径
export ZEROMQ_PATH=$(brew --prefix zeromq)

# 创建构建目录
mkdir -p build
cd build

# 配置CMake并只构建ZeroMQ接收器
echo "运行CMake配置..."
cmake .. -DCMAKE_PREFIX_PATH="$ZEROMQ_PATH"

echo "编译ZeroMQ接收器..."
make test_zmq_receiver

# 运行ZeroMQ接收器
if [ -f "./bin/test_zmq_receiver" ]; then
    echo "启动ZeroMQ接收器..."
    echo "接收器将持续运行，按Ctrl+C停止"
    ./bin/test_zmq_receiver
else
    echo "错误: 找不到ZeroMQ接收器可执行文件。编译可能未成功。"
    exit 1
fi

cd .. 