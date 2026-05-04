#include <iostream>
#include "../http/http_connection.h"
#include "../include/log.h"
#include "../include/iomanager.h"
#include "../http/http_parser.h"
#include <fstream>

static flz::Logger::ptr g_logger = FLZ_LOG_ROOT();

void test_https() {
    auto r = flz::http::HttpConnection::DoGet("http://10.0.0.1:8080/HelloWorld", 300, {
                        {"Accept-Encoding", "gzip, deflate, br"},
                        {"Connection", "keep-alive"},
                        {"User-Agent", "curl/7.29.0"}
            });
    FLZ_LOG_INFO(g_logger) << "result=" << r->result
        << " error=" << r->error
        << " rsp=" << (r->response ? r->response->toString() : "");

    //sylar::http::HttpConnectionPool::ptr pool(new flz::http::HttpConnectionPool(
    //            "www.baidu.com", "", 80, false, 10, 1000 * 30, 5));
    auto pool = flz::http::HttpConnectionPool::Create(
                    "https://www.baidu.com", "", 10, 1000 * 30, 5);
    flz::IOManager::GetThis()->addTimer(1000, [pool](){
            auto r = pool->doGet("/", 3000, {
                        {"Accept-Encoding", "gzip, deflate, br"},
                        {"User-Agent", "curl/7.29.0"}
                    });
            FLZ_LOG_INFO(g_logger) << r->toString();
    }, true);
}

int main(int argc, char** argv) {
    flz::IOManager iom(2);
    //iom.schedule(run);
    iom.schedule(test_https);
    return 0;
}
