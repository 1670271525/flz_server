#ifndef __FLZ_TCP_SERVER_H__
#define __FLZ_TCP_SERVER_H__

#include <iostream>
#include <memory>
#include <string>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include "threadpool.h"
#include <unordered_map>
#include <memory>

typedef std::function<std::string(const std::string)> func_t;

namespace flz {

	struct Conn{
		int fd;
		std::string inBuf;
		std::string outBuf;
		explicit Conn(int f):fd(f){}
	};

	enum ServerType{
		NORMAL = 0,HTTP = 1
	};
	class TcpServer{
	private:
		ServerType m_type;
		int _listen_fd;
		uint16_t _listen_port;
		func_t _data_handler;
		bool _is_running;
		flz::ThreadPool::ptr m_threadpool;
		int m_epfd;
		std::unordered_map<int,std::shared_ptr<Conn>> m_connMap;
		std::string type = "tcp";

	public:
		typedef std::shared_ptr<TcpServer> ptr;
		typedef std::shared_ptr<Conn> connPtr;
		TcpServer(uint16_t port,func_t handler,ServerType type);
		~TcpServer();
		bool init();
		void start();
		void stop();
		void removeConn(int fd);
		void handleAccept();
		void handleClientEvent(int fd, uint32_t revents);
		void processPendingClose();
		void cleanupAllConns();
		void flushOutBuf(connPtr conn);

		void addClosePending(connPtr conn);
	public:
	private:
		void handlerRead(connPtr conn);
	};
}




#endif
