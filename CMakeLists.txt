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

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include (cmake/utils.cmake)


# 3. 设置C++11标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 强制使用C++11，避免编译器降级


find_package(yaml-cpp REQUIRED)

find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

#find_package(MySQL REQUIRED)
#if(MYSQL_FOUND)
	#include_directories(${MYSQL_INCLUDE_DIR})
#endif()

find_package(Protobuf)
if(Protobuf_FOUND)
    include_directories(${Protobuf_INCLUDE_DIRS})
endif()

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIR})
endif()




# 定义所有源文件列表
set(LIB_SRC
    # src 目录下的源文件
    src/address.cc
    src/bytearray.cc
    src/channel.cc
    src/config.cc
    src/env.cc
    src/fd_manager.cc
    src/fiber.cc
    src/hook.cc
    src/httpService_x.cc
    src/iomanager.cc
    src/log.cc
    src/mutex.cc
    src/scheduler.cc
    src/socket.cc
    src/stream.cc
    src/tcp_server.cc
    src/Thread.cc
    src/threadpool.cc
    src/timer.cc
    src/timestamp.cc
    src/util.cc
	src/crypto_util.cc
	src/json_util.cc
	src/hash_util.cc
	src/application.cc
	src/worker.cc
    src/module.cc
	src/library.cc
	src/daemon.cc


    # http 目录下的源文件
    http/http.cc
    http/http_parser.cc
    http/http_connection.cc
    http/http_server.cc
    http/http_session.cc
    http/servlet.cc
    http/servlets/config_servlet.cc
	http/ws_session.cc
	http/ws_servlet.cc
	http/ws_server.cc
	http/ws_connection.cc
	


    
    # streams 目录下的源文件
    streams/socket_stream.cc
    
)


ragelmaker(http/http11_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/http)
ragelmaker(http/httpclient_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/http)
ragelmaker(src/uri.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 4. 包含头文件目录（你的原始配置，保留）
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# 支持从仓库根目录的绝对 include 路径（如 include/foo.h、http/foo.h）
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 5. 正确收集src目录下所有.cc文件（无语法错误）
#file(GLOB_RECURSE SRC_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cc")

#file(GLOB_RECURSE HTTP_SOURCES "${CMAKE_SOURCE_DIR}/http/*.cc")


#file(GLOB_RECURSE STREAM_SOURCES "${CMAKE_SOURCE_DIR}/streams/*.cc")

# 6. 正确收集test目录下所有.cc文件（单独定义变量，避免覆盖）
#file(GLOB_RECURSE TEST_SOURCES "${CMAKE_SOURCE_DIR}/test/test_http_server.cc")

# 7. 合并src和test的源文件到总变量（核心：避免覆盖，整合所有需要的.cc文件）
#set(ALL_SOURCES ${SRC_SOURCES} ${HTTP_SOURCES}  ${TEST_SOURCES} ${STREAM_SOURCES})


add_library(flz SHARED ${LIB_SRC})
#force_redefine_file_macro_for_sources(flz)


set(LIBS
        flz
		yaml-cpp
		jsoncpp
		${ZLIB_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${PROTOBUF_LIBRARIES}
	)


# 8. 指定可执行文件输出目录（你的原始配置，保留）
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 9. 设置构建模式为Debug（支持调试，你的原始配置，保留）
set(CMAKE_BUILD_TYPE Debug)

# 10. 构建可执行文件（使用合并后的所有源文件）
#add_executable(test_http_server "${LIB_SRC}")

flz_add_executable(test_http_server "test/test_http_server.cc" flz "${LIBS}")


flz_add_executable(test_uri "test/test_uri.cc" flz "${LIBS}")


flz_add_executable(test_http_connections "test/test_http_connections.cc" flz "${LIBS}")


flz_add_executable(my_http_server "samples/my_http_server.cc" flz "${LIBS}")


flz_add_executable(test_ws_client "test/test_ws_client.cc" flz "${LIBS}")


flz_add_executable(test_ws_server "test/test_ws_server.cc" flz "${LIBS}")

flz_add_executable(test_socket "test/test_socket.cc" flz "${LIBS}")

flz_add_executable(bin_flz "src/main.cc" flz "${LIBS}")
set_target_properties(bin_flz PROPERTIES OUTPUT_NAME "flz")

#target_link_libraries(my_http_server tcmalloc)


#target_link_libraries(test_http_server PRIVATE yaml-cpp)



