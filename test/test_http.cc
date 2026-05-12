#include "http/http.h"




void test_request() {
    flz::http::HttpRequest::ptr req(new flz::http::HttpRequest);
    req->setHeader("host" , "www.baidu.com");
    req->setBody("hello flz");
    req->dump(std::cout) << std::endl;
}

void test_response() {
    flz::http::HttpResponse::ptr rsp(new flz::http::HttpResponse);
    rsp->setHeader("X-X", "flz");
    rsp->setBody("hello flz");
    rsp->setStatus((flz::http::HttpStatus)400);
    rsp->setClose(false);

    rsp->dump(std::cout) << std::endl;
}

int main(int argc, char** argv) {
    test_request();
    test_response();
    return 0;
}



