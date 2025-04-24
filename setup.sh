#!/bin/bash

echo "=== CTP API 项目环境设置脚本 ==="
echo ""

# 检查是否已安装Homebrew
if ! command -v brew &> /dev/null; then
    echo "错误: 未找到Homebrew，请先安装Homebrew。"
    echo "安装命令: /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
    exit 1
fi

# 安装必要的依赖
echo "正在检查并安装必要的依赖..."
brew install cmake
brew install zeromq

echo ""
echo "=== 依赖检查完成 ==="
echo ""

# 创建必要的目录结构
echo "正在创建目录结构..."
mkdir -p src/mdapi
mkdir -p src/traderapi
mkdir -p src/strategy
mkdir -p build

# 检查CTP API文件夹
if [ ! -d "TraderapiMduserapi_6.7.7_MacOS" ]; then
    echo ""
    echo "警告: 未找到CTP API文件夹 (TraderapiMduserapi_6.7.7_MacOS)"
    echo "请从上期所官网下载CTP API (6.7.7版本，MacOS版):"
    echo "http://www.sfit.com.cn/5_2_DocumentDown.htm"
    echo "下载后，请将文件解压到项目根目录，确保目录名称为 TraderapiMduserapi_6.7.7_MacOS"
    echo ""
else
    echo "已找到CTP API文件夹。"
fi

# 检查config.ini
if grep -q "your_simnow_userid" config.ini 2>/dev/null; then
    echo ""
    echo "警告: 请更新config.ini中的SimNow凭据。"
    echo "编辑config.ini文件并填入您的实际账号和密码。"
    echo ""
fi

# 设置权限
echo "设置脚本执行权限..."
chmod +x run_mdapi_test.sh
chmod +x test_market_data.sh
chmod +x run_zmq_receiver.sh

echo ""
echo "=== 环境设置完成 ==="
echo ""
echo "使用方法:"
echo "1. 编辑config.ini填入您的SimNow账号和密码"
echo "2. 运行测试命令:"
echo "   - 测试行情接收: ./run_mdapi_test.sh"
echo "   - 测试ZeroMQ接收器: ./run_zmq_receiver.sh"
echo "   - 测试完整数据流: ./test_market_data.sh"
echo ""
echo "祝您测试愉快!" 