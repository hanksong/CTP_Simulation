cmake_minimum_required(VERSION 3.10)
project(CTP_API VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 使用环境变量中的CTP_API_PATH
if(DEFINED ENV{CTP_API_PATH})
    set(CTP_API_PATH $ENV{CTP_API_PATH})
    message(STATUS "Using CTP API path from environment: ${CTP_API_PATH}")
else()
    # 如果没有定义环境变量，尝试几个可能的路径
    if(EXISTS "${CMAKE_SOURCE_DIR}/CTP_API_Clean/API")
        set(CTP_API_PATH "${CMAKE_SOURCE_DIR}/CTP_API_Clean/API")
    else()
        message(FATAL_ERROR "Could not find CTP API path. Please create CTP_API_Clean/API directory and copy API files there.")
    endif()
    message(STATUS "Using CTP API path: ${CTP_API_PATH}")
endif()

# 设置API库的具体路径
set(TRADER_API_FRAMEWORK "${CTP_API_PATH}/thosttraderapi_se.framework")
set(MD_API_FRAMEWORK "${CTP_API_PATH}/thostmduserapi_se.framework")
set(TRADER_API_LIB "${TRADER_API_FRAMEWORK}/Versions/A/thosttraderapi_se")
set(MD_API_LIB "${MD_API_FRAMEWORK}/Versions/A/thostmduserapi_se")

# 添加MD API头文件目录
set(MD_API_HEADERS "${MD_API_FRAMEWORK}/Versions/A/Headers")
if(NOT EXISTS "${MD_API_HEADERS}")
    message(WARNING "MD API Headers directory not found at ${MD_API_HEADERS}")
endif()

# 添加Trader API头文件目录
set(TRADER_API_HEADERS "${TRADER_API_FRAMEWORK}/Versions/A/Headers")
if(NOT EXISTS "${TRADER_API_HEADERS}")
    message(WARNING "Trader API Headers directory not found at ${TRADER_API_HEADERS}")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CTP_API_PATH}
    ${MD_API_HEADERS}
    ${TRADER_API_HEADERS}
)

# Find ZeroMQ package
find_package(ZeroMQ QUIET)
if(NOT ZeroMQ_FOUND)
    # 如果标准方式没找到，尝试直接使用路径
    if(DEFINED ENV{ZEROMQ_PATH})
        set(ZEROMQ_PATH $ENV{ZEROMQ_PATH})
        message(STATUS "Using ZeroMQ path from environment: ${ZEROMQ_PATH}")
        set(ZeroMQ_INCLUDE_DIRS "${ZEROMQ_PATH}/include")
        set(ZeroMQ_LIBRARIES "${ZEROMQ_PATH}/lib/libzmq.dylib")
    else()
        # 尝试使用brew查询路径
        execute_process(
            COMMAND brew --prefix zeromq
            OUTPUT_VARIABLE BREW_ZEROMQ_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(BREW_ZEROMQ_PATH)
            message(STATUS "Using ZeroMQ path from brew: ${BREW_ZEROMQ_PATH}")
            set(ZeroMQ_INCLUDE_DIRS "${BREW_ZEROMQ_PATH}/include")
            set(ZeroMQ_LIBRARIES "${BREW_ZEROMQ_PATH}/lib/libzmq.dylib")
        else()
            message(FATAL_ERROR "ZeroMQ not found. Please install ZeroMQ or set ZEROMQ_PATH environment variable.")
        endif()
    endif()
endif()

include_directories(${ZeroMQ_INCLUDE_DIRS})
message(STATUS "ZeroMQ include dirs: ${ZeroMQ_INCLUDE_DIRS}")
message(STATUS "ZeroMQ libraries: ${ZeroMQ_LIBRARIES}")

# Source files for main application
set(SRC_FILES
    src/main.cpp
    src/mdapi/MdSpi.cpp
    src/traderapi/TraderSpi.cpp
    src/strategy/StrategyEngine.cpp
)

# Create main executable
add_executable(ctp_trading ${SRC_FILES})

# Source files for market data test
set(MDAPI_TEST_FILES
    src/test_mdapi.cpp
    src/mdapi/MdSpi.cpp
)

# Create market data test executable
add_executable(test_mdapi ${MDAPI_TEST_FILES})

# ZeroMQ receiver test
add_executable(test_zmq_receiver src/test_zmq_receiver.cpp)

# Check if library files exist
if(NOT EXISTS "${TRADER_API_LIB}")
    message(WARNING "Trader API library not found at ${TRADER_API_LIB}")
endif()

if(NOT EXISTS "${MD_API_LIB}")
    message(WARNING "MD API library not found at ${MD_API_LIB}")
endif()

# Print found paths
message(STATUS "Using MD API library: ${MD_API_LIB}")
message(STATUS "Using Trader API library: ${TRADER_API_LIB}")

# CTP libraries linking - directly link with copied libraries
if(APPLE)
    # Link directly with the library files
    target_include_directories(ctp_trading PRIVATE 
        ${CTP_API_PATH}
    )
    
    target_link_libraries(ctp_trading
        ${TRADER_API_LIB}
        ${MD_API_LIB}
        ${ZeroMQ_LIBRARIES}
    )
    
    target_include_directories(test_mdapi PRIVATE 
        ${CTP_API_PATH}
    )
    
    target_link_libraries(test_mdapi
        ${MD_API_LIB}
        ${ZeroMQ_LIBRARIES}
    )
else()
    # Non-Apple platform linking
    target_link_libraries(ctp_trading 
        ${TRADER_API_LIB}
        ${MD_API_LIB}
        ${ZeroMQ_LIBRARIES}
    )
    
    target_link_libraries(test_mdapi 
        ${MD_API_LIB}
        ${ZeroMQ_LIBRARIES}
    )
endif()

# Link libraries for ZeroMQ receiver test
target_link_libraries(test_zmq_receiver
    ${ZeroMQ_LIBRARIES}
)

# For MacOS, set rpath for the libs directory
if(APPLE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_SOURCE_DIR}/libs")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

# Installation
install(TARGETS ctp_trading test_mdapi test_zmq_receiver DESTINATION bin) 