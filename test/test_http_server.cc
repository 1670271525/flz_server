#include "../http/http_server.h"
#include "../include/log.h"

static flz::Logger::ptr g_logger = FLZ_LOG_ROOT();

#define XX(...) #__VA_ARGS__


flz::IOManager::ptr worker;
void run() {
    g_logger->setLevel(flz::LogLevel::INFO);
    //sylar::http::HttpServer::ptr server(new sylar::http::HttpServer(true, worker.get(), sylar::IOManager::GetThis()));
    flz::http::HttpServer::ptr server(new flz::http::HttpServer(true));
    flz::Address::ptr addr = flz::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/flz/xx", [](flz::http::HttpRequest::ptr req
                ,flz::http::HttpResponse::ptr rsp
                ,flz::http::HttpSession::ptr session) {
            rsp->setBody(req->toString());
            return 0;
    });

    sd->addGlobServlet("/flz/*", [](flz::http::HttpRequest::ptr req
                ,flz::http::HttpResponse::ptr rsp
                ,flz::http::HttpSession::ptr session) {
            rsp->setBody("Glob:\r\n" + req->toString());
            return 0;
    });

    sd->addGlobServlet("/flz/*", [](flz::http::HttpRequest::ptr req
                ,flz::http::HttpResponse::ptr rsp
                ,flz::http::HttpSession::ptr session) {
            rsp->setBody(XX(<html>
<head><title>404 Not Found</title></head>
<body>
<center><h1>404 Not Found</h1></center>
<hr><center>nginx/1.16.0</center>
</body>
</html>
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
<!-- a padding to disable MSIE and Chrome friendly error page -->
));
            return 0;
    });

    server->start();
}

int main(int argc, char** argv) {
    flz::IOManager iom(1, true, "main");
    worker.reset(new flz::IOManager(3, false, "worker"));
    iom.schedule(run);
    return 0;
}
