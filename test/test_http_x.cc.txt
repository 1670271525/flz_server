#include "../include/tcp_server.h"
#include "../include/httpService.h"


std::string handle(const std::string data){
	flz::HttpRequest::ptr http = std::make_shared<flz::HttpRequest>();
	std::string str = data;
	http->deserialize(str);
	http->hprint();
	
	flz::HttpResponse::ptr resp = std::make_shared<flz::HttpResponse>();
	resp->addCode(200,"success");


	return std::string("success");
}


flz::HttpResponse login(flz::HttpRequest& req){
	flz::HttpResponse resp;
	std::cout<<"获得参数: "<<req.getRequestBody()<<"\n";
	std::cout<<"##################\n";
	resp.addCode(200,"OK");
	resp.addBodyText("<html><h1>result done!</h1></html>");
	return resp;
}

std::string http_handle(const std::string data){
	flz::HttpServer::ptr server(new flz::HttpServer());
	std::string seq = data;
	server->insertService("/login",login);
	return server->handlerHttpRequest(seq);
}

int main(){
	flz::TcpServer::ptr ts = std::make_shared<flz::TcpServer>(8080,http_handle,flz::ServerType::HTTP);
	ts->init();
	ts->start();



	return 0;
}
