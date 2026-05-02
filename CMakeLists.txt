#
#
#set(CMAKE_CXX_STANDARD 11)
#
#project(TcpServer)
#
#include_directories(${CMAKE_SOURCE_DIR}/include)
#file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cc")
#list(REMOVE_ITEM SOURCES ${CMAKE_SOURCE_DIR}/src/test_threadpool.cc ${CMAKE_SOURCE_DIR}/src/test_tcp_server.cc)
#
#file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/test/test_http.cc")
#
#set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/out)
#
#set(CMAKE_BUILD_TYPE Debug)
#
#
#add_executable(TcpServer ${SOURCES})
#

# 1. （推荐添加）指定CMake最低版本要求，增强兼容性
cmake_minimum_required(VERSION 3.10)
# 2. 规范顺序：先声明项目（靠前放置）
project(TcpServer)

# 3. 设置C++11标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 强制使用C++11，避免编译器降级


find_package(yaml-cpp REQUIRED)



# 4. 包含头文件目录（你的原始配置，保留）
include_directories(${CMAKE_SOURCE_DIR}/include)

# 5. 正确收集src目录下所有.cc文件（无语法错误）
file(GLOB_RECURSE SRC_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cc")

file(GLOB_RECURSE HTTP_SOURCES "${CMAKE_SOURCE_DIR}/http/*.cc")


file(GLOB_RECURSE STREAM_SOURCES "${CMAKE_SOURCE_DIR}/streams/*.cc")

# 6. 正确收集test目录下所有.cc文件（单独定义变量，避免覆盖）
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_SOURCE_DIR}/test/test_http_server.cc")

# 7. 合并src和test的源文件到总变量（核心：避免覆盖，整合所有需要的.cc文件）
set(ALL_SOURCES ${SRC_SOURCES} ${HTTP_SOURCES}  ${TEST_SOURCES} ${STREAM_SOURCES})

# 8. 指定可执行文件输出目录（你的原始配置，保留）
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/out)

# 9. 设置构建模式为Debug（支持调试，你的原始配置，保留）
set(CMAKE_BUILD_TYPE Debug)

# 10. 构建可执行文件（使用合并后的所有源文件）
add_executable(test_http_server ${ALL_SOURCES})

target_link_libraries(test_http_server PRIVATE yaml-cpp)


