#include "../include/socket.h"
#include "../include/iomanager.h"

static flz::Logger::ptr g_looger = FLZ_LOG_ROOT();

void test_socket() {
	flz::IPAddress::ptr addr = flz::Address::LookupAnyIPAddress("www.baidu.com");
    if(addr) {
        FLZ_LOG_INFO(g_looger) << "get address: " << addr->toString();
    } else {
        FLZ_LOG_ERROR(g_looger) << "get address fail";
        return;
    }

    flz::Socket::ptr sock = flz::Socket::CreateTCP(addr);
    addr->setPort(80);
    FLZ_LOG_INFO(g_looger) << "addr=" << addr->toString();
    if(!sock->connect(addr)) {
        FLZ_LOG_ERROR(g_looger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        FLZ_LOG_INFO(g_looger) << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <= 0) {
        FLZ_LOG_INFO(g_looger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if(rt <= 0) {
        FLZ_LOG_INFO(g_looger) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    FLZ_LOG_INFO(g_looger) << buffs;
}



int main(){
	
	flz::IOManager iom;
	iom.schedule(&test_socket);

	return 0;


}
