#!/bin/bash

echo "===== CTP API 文件迁移工具 ====="
echo "该脚本将帮助您将CTP API文件从原始位置复制到CTP_API_Clean目录中"
echo ""

# 确保目标目录存在
mkdir -p ./CTP_API_Clean/API

# 寻找API目录的正确路径
SOURCE_PATH=""

if [ -d "TraderapiMduserapi_6.7.7_MacOS/API" ]; then
    SOURCE_PATH="TraderapiMduserapi_6.7.7_MacOS/API"
    echo "找到API源目录: $SOURCE_PATH"
elif [ -d "TraderapiMduserapi_6.7.7_MacOS/Traderapi&Mduserapi_6.7.2_MacOS生产版/API" ]; then
    SOURCE_PATH="TraderapiMduserapi_6.7.7_MacOS/Traderapi&Mduserapi_6.7.2_MacOS生产版/API"
    echo "找到API源目录: $SOURCE_PATH"
else
    echo "错误: 找不到源API目录。"
    echo "请确保您已下载CTP API并解压到TraderapiMduserapi_6.7.7_MacOS目录。"
    exit 1
fi

echo ""
echo "即将复制API文件从: $SOURCE_PATH"
echo "到目标目录: CTP_API_Clean/API"
echo ""
echo "按回车键继续，或Ctrl+C取消..."
read

# 复制文件
echo "正在复制文件..."
cp -R "./$SOURCE_PATH/"* "./CTP_API_Clean/API/"

# 检查复制是否成功
if [ $? -eq 0 ]; then
    echo ""
    echo "文件复制成功！"
    # 检查框架文件是否存在
    if [ -d "./CTP_API_Clean/API/thostmduserapi_se.framework" ] && [ -d "./CTP_API_Clean/API/thosttraderapi_se.framework" ]; then
        echo "已确认框架文件存在。"
        echo ""
        echo "现在您可以使用以下命令来运行测试:"
        echo "  ./run_mdapi_test.sh    - 测试市场数据API"
        echo "  ./run_zmq_receiver.sh  - 运行ZeroMQ接收器"
        echo "  ./test_market_data.sh  - 完整测试市场数据链路"
    else
        echo "警告: 未找到框架文件。请检查源目录是否包含完整的API文件。"
    fi
else
    echo "错误: 文件复制失败。"
fi

echo ""
echo "===== 操作完成 =====" 