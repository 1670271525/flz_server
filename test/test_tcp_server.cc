#include "../include/tcp_server.h"
#include "../include/threadpool.h"
#include <unordered_set>
#include <memory.h>


int main(int argc,char* argv[]){
	if(argc != 2){
		std::cout<< "arg error\n";
		return 1;
	}
	uint16_t listen_port = std::stoi(argv[1]);
	
	if(listen_port<1024 || listen_port>65535){
		std::cerr<<"port error,must in(1024~65535)";
		return 1;
	}

	auto echo = [](const std::string& data) -> std::string {
		return std::string("echo->") + data;
	};
	auto server(new flz::TcpServer(listen_port,echo));
	if(!server->init()){
		std::cerr<<"server init fail!\n";
		return 1;
	}	
	server->start();
	
	return 0;

}
