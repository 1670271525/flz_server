#include "include/application.h"
#include <unistd.h>
#include <signal.h>

#include "include/tcp_server.h"
#include "include/daemon.h"
#include "include/config.h"
#include "include/env.h"
#include "include/log.h"
#include "include/module.h"
#include "include/worker.h"
#include "http/ws_server.h"

     
namespace flz{



static	flz::Logger::ptr g_logger = FLZ_LOG_NAME("system");

static flz::ConfigVar<std::string>::ptr g_server_work_path =
    flz::Config::Lookup("server.work_path"
            ,std::string("/apps/work/flz")
            , "server work path");

static flz::ConfigVar<std::string>::ptr g_server_pid_file =
    flz::Config::Lookup("server.pid_file"
            ,std::string("flz.pid")
            , "server pid file");

static flz::ConfigVar<std::string>::ptr g_service_discovery_zk =
    flz::Config::Lookup("service_discovery.zk"
            ,std::string("")
            , "service discovery zookeeper");


static flz::ConfigVar<std::vector<TcpServerConf> >::ptr g_servers_conf
    = flz::Config::Lookup("servers", std::vector<TcpServerConf>(), "http server config");

Application* Application::s_instance = nullptr;

Application::Application() {
    s_instance = this;
}

bool Application::init(int argc, char** argv) {
    m_argc = argc;
    m_argv = argv;

    flz::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
    flz::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
    flz::EnvMgr::GetInstance()->addHelp("c", "conf path default: ./conf");
    flz::EnvMgr::GetInstance()->addHelp("p", "print help");

    bool is_print_help = false;
    if(!flz::EnvMgr::GetInstance()->init(argc, argv)) {
        is_print_help = true;
    }

    if(flz::EnvMgr::GetInstance()->has("p")) {
        is_print_help = true;
    }

    std::string conf_path = flz::EnvMgr::GetInstance()->getConfigPath();
    FLZ_LOG_INFO(g_logger) << "load conf path:" << conf_path;
    flz::Config::LoadFromConfDir(conf_path);

    ModuleMgr::GetInstance()->init();
    std::vector<Module::ptr> modules;
    ModuleMgr::GetInstance()->listAll(modules);

    for(auto i : modules) {
        i->onBeforeArgsParse(argc, argv);
    }

    if(is_print_help) {
        flz::EnvMgr::GetInstance()->printHelp();
        return false;
    }

    for(auto i : modules) {
        i->onAfterArgsParse(argc, argv);
    }
    modules.clear();

    int run_type = 0;
    if(flz::EnvMgr::GetInstance()->has("s")) {
        run_type = 1;
    }
    if(flz::EnvMgr::GetInstance()->has("d")) {
        run_type = 2;
    }

    if(run_type == 0) {
        flz::EnvMgr::GetInstance()->printHelp();
        return false;
    }

    std::string pidfile = g_server_work_path->getValue()
                                + "/" + g_server_pid_file->getValue();
    if(flz::FSUtil::IsRunningPidfile(pidfile)) {
        FLZ_LOG_ERROR(g_logger) << "server is running:" << pidfile;
        return false;
    }

    if(!flz::FSUtil::Mkdir(g_server_work_path->getValue())) {
        FLZ_LOG_FATAL(g_logger) << "create work path [" << g_server_work_path->getValue()
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Application::run() {
    bool is_daemon = flz::EnvMgr::GetInstance()->has("d");
    return start_daemon(m_argc, m_argv,
            std::bind(&Application::main, this, std::placeholders::_1,
                std::placeholders::_2), is_daemon);
}

int Application::main(int argc, char** argv) {
    signal(SIGPIPE, SIG_IGN);
    FLZ_LOG_INFO(g_logger) << "main";
    std::string conf_path = flz::EnvMgr::GetInstance()->getConfigPath();
    flz::Config::LoadFromConfDir(conf_path, true);
    {
        std::string pidfile = g_server_work_path->getValue()
                                    + "/" + g_server_pid_file->getValue();
        std::ofstream ofs(pidfile);
        if(!ofs) {
            FLZ_LOG_ERROR(g_logger) << "open pidfile " << pidfile << " failed";
            return false;
        }
        ofs << getpid();
    }

    m_mainIOManager.reset(new flz::IOManager(1, true, "main"));
    m_mainIOManager->schedule(std::bind(&Application::run_fiber, this));
    m_mainIOManager->addTimer(2000, [](){
            //FLZ_LOG_INFO(g_logger) << "hello";
    }, true);
    m_mainIOManager->stop();
    return 0;
}

int Application::run_fiber() {
    std::vector<Module::ptr> modules;
    ModuleMgr::GetInstance()->listAll(modules);
    bool has_error = false;
    for(auto& i : modules) {
        if(!i->onLoad()) {
            FLZ_LOG_ERROR(g_logger) << "module name="
                << i->getName() << " version=" << i->getVersion()
                << " filename=" << i->getFilename();
            has_error = true;
        }
    }
    if(has_error) {
        _exit(0);
    }

    flz::WorkerMgr::GetInstance()->init();
    auto http_confs = g_servers_conf->getValue();

    std::vector<TcpServer::ptr> svrs;
    for(auto& i : http_confs) {
        FLZ_LOG_DEBUG(g_logger) << std::endl << LexicalCast<TcpServerConf, std::string>()(i);

        std::vector<Address::ptr> address;
        for(auto& a : i.address) {
            size_t pos = a.find(":");
            if(pos == std::string::npos) {
                //FLZ_LOG_ERROR(g_logger) << "invalid address: " << a;
                address.push_back(UnixAddress::ptr(new UnixAddress(a)));
                continue;
            }
            int32_t port = atoi(a.substr(pos + 1).c_str());
            //127.0.0.1
            auto addr =flz::IPAddress::Create(a.substr(0, pos).c_str(), port);
            if(addr) {
                address.push_back(addr);
                continue;
            }
            std::vector<std::pair<Address::ptr, uint32_t> > result;
            if(flz::Address::GetInterfaceAddresses(result,
                                        a.substr(0, pos))) {
                for(auto& x : result) {
                    auto ipaddr = std::dynamic_pointer_cast<IPAddress>(x.first);
                    if(ipaddr) {
                        ipaddr->setPort(atoi(a.substr(pos + 1).c_str()));
                    }
                    address.push_back(ipaddr);
                }
                continue;
            }

            auto aaddr = flz::Address::LookupAny(a);
            if(aaddr) {
                address.push_back(aaddr);
                continue;
            }
            FLZ_LOG_ERROR(g_logger) << "invalid address: " << a;
            _exit(0);
        }
        IOManager* accept_worker = flz::IOManager::GetThis();
        IOManager* io_worker = flz::IOManager::GetThis();
        IOManager* process_worker = flz::IOManager::GetThis();
        if(!i.accept_worker.empty()) {
            accept_worker = flz::WorkerMgr::GetInstance()->getAsIOManager(i.accept_worker).get();
            if(!accept_worker) {
                FLZ_LOG_ERROR(g_logger) << "accept_worker: " << i.accept_worker
                    << " not exists";
                _exit(0);
            }
        }
        if(!i.io_worker.empty()) {
            io_worker = flz::WorkerMgr::GetInstance()->getAsIOManager(i.io_worker).get();
            if(!io_worker) {
                FLZ_LOG_ERROR(g_logger) << "io_worker: " << i.io_worker
                    << " not exists";
                _exit(0);
            }
        }
        if(!i.process_worker.empty()) {
            process_worker = flz::WorkerMgr::GetInstance()->getAsIOManager(i.process_worker).get();
            if(!process_worker) {
               FLZ_LOG_ERROR(g_logger) << "process_worker: " << i.process_worker
                    << " not exists";
                _exit(0);
            }
        }

        TcpServer::ptr server;
        if(i.type == "http") {
            server.reset(new flz::http::HttpServer(i.keepalive,
                            process_worker, io_worker, accept_worker));
        } else if(i.type == "ws") {
            server.reset(new flz::http::WSServer(
                            process_worker, io_worker, accept_worker));
        }else {
            FLZ_LOG_ERROR(g_logger) << "invalid server type=" << i.type
                << LexicalCast<TcpServerConf, std::string>()(i);
            _exit(0);
        }
        if(!i.name.empty()) {
            server->setName(i.name);
        }
        std::vector<Address::ptr> fails;
        if(!server->bind(address, fails, i.ssl)) {
            for(auto& x : fails) {
                FLZ_LOG_ERROR(g_logger) << "bind address fail:"
                    << *x;
            }
            _exit(0);
        }
        /*
		if(i.ssl) {
            if(!server->loadCertificates(i.cert_file, i.key_file)) {
                FLZ_LOG_ERROR(g_logger) << "loadCertificates fail, cert_file="
                    << i.cert_file << " key_file=" << i.key_file;
            }
        }
		*/
        server->setConf(i);
        //server->start();
        m_servers[i.type].push_back(server);
        svrs.push_back(server);
    }

    for(auto& i : modules) {
        i->onServerReady();
    }

    for(auto& i : svrs) {
        i->start();
    }

    

    for(auto& i : modules) {
        i->onServerUp();
    }
    //zip

	return 0;
}

bool Application::getServer(const std::string& type, std::vector<TcpServer::ptr>& svrs) {
    auto it = m_servers.find(type);
    if(it == m_servers.end()) {
        return false;
    }
    svrs = it->second;
    return true;
}

void Application::listAllServer(std::map<std::string, std::vector<TcpServer::ptr> >& servers) {
    servers = m_servers;
}





}
