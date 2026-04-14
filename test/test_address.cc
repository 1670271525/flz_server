#include "../include/address.h"
#include "../include/log.h"

flz::Logger::ptr g_logger = FLZ_LOG_ROOT();

void test() {
    std::vector<flz::Address::ptr> addrs;

    FLZ_LOG_INFO(g_logger) << "begin";
    bool v = flz::Address::Lookup(addrs, "localhost:3080");
    //bool v = flz::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    FLZ_LOG_INFO(g_logger) << "end";
    if(!v) {
        FLZ_LOG_ERROR(g_logger) << "lookup fail";
        return;
    }

    for(size_t i = 0; i < addrs.size(); ++i) {
        FLZ_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }

    auto addr = flz::Address::LookupAny("localhost:4080");
    if(addr) {
        FLZ_LOG_INFO(g_logger) << *addr;
    } else {
        FLZ_LOG_ERROR(g_logger) << "error";
    }
}

void test_iface() {
    std::multimap<std::string, std::pair<flz::Address::ptr, uint32_t> > results;

    bool v = flz::Address::GetInterfaceAddress(results);
    if(!v) {
        FLZ_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto& i: results) {
        FLZ_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - "
            << i.second.second;
    }
}

void test_ipv4() {
    //auto addr = sylar::IPAddress::Create("www.sylar.top");
    auto addr = flz::IPAddress::Create("127.0.0.8");
    if(addr) {
        FLZ_LOG_INFO(g_logger) << addr->toString();
    }
}

int main(int argc, char** argv) {
    //test_ipv4();
    //test_iface();
    test();
    return 0;
}
