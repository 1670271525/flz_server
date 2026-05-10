#include "../http/ws_server.h"
#include "../include/log.h"

static flz::Logger::ptr g_logger = FLZ_LOG_ROOT();

void run() {
    flz::http::WSServer::ptr server(new flz::http::WSServer);
    flz::Address::ptr addr = flz::Address::LookupAnyIPAddress("0.0.0.0:8020");
    if(!addr) {
        FLZ_LOG_ERROR(g_logger) << "get address error";
        return;
    }
    auto fun = [](flz::http::HttpRequest::ptr header
                  ,flz::http::WSFrameMessage::ptr msg
                  ,flz::http::WSSession::ptr session) {
        session->sendMessage(msg);
        return 0;


	};
	server->getWSServletDispatch()->addServlet("/flz", fun);
    while(!server->bind(addr)) {
        FLZ_LOG_ERROR(g_logger) << "bind " << *addr << " fail";
        sleep(1);
    }
    server->start();
}

int main(int argc, char** argv) {
    flz::IOManager iom(2);
    iom.schedule(run);
    return 0;
}
