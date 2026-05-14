#include "http/http_server.h"
#include "include/log.h"
#include "include/util.h"



flz::Logger::ptr g_logger = FLZ_LOG_ROOT();
flz::IOManager::ptr worker;
void run() {
    g_logger->setLevel(flz::LogLevel::INFO);
    flz::Address::ptr addr = flz::Address::LookupAnyIPAddress("0.0.0.0:8020");
    if(!addr) {
        FLZ_LOG_ERROR(g_logger) << "get address error";
        return;
    }

    flz::http::HttpServer::ptr http_server(new flz::http::HttpServer(true, worker.get()));
    //flz::http::HttpServer::ptr http_server(new flz::http::HttpServer(true));
    bool ssl = false;
    while(!http_server->bind(addr, ssl)) {
        FLZ_LOG_ERROR(g_logger) << "bind " << *addr << " fail";
        sleep(1);
    }

	auto sd = http_server->getServletDispatch();
	sd->addServlet("/flz/test",[](flz::http::HttpRequest::ptr req,flz::http::HttpResponse::ptr rsp,flz::http::HttpSession::ptr session){
			 //rsp->setBody(rsp->toString());
			Json::Value json; 
			json["msg"] = "hello world";
			json["id"] = "1";
			rsp->setBody(flz::JsonUtil::ToString(json));
			 return 0;
			});


	/*
    if(ssl) {
        //http_server->loadCertificates("/home/apps/soft/sylar/keys/server.crt", "/home/apps/soft/sylar/keys/server.key");
    }
	*/

    http_server->start();
}

int main(int argc, char** argv) {
    flz::IOManager iom(4);
    worker.reset(new flz::IOManager(4, false));
    iom.schedule(run);
    return 0;
}
