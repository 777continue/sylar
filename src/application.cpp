#include "application.h"
#include "config.h"
#include "env.h"
#include "log.h"
#include "daemon.h"
#include <unistd.h>
#include "module.h"
#include "worker.h"
#include "tcp_server.h"
#include "../http/ws_server.h"

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static sylar::ConfigVar<std::string>::ptr g_server_work_path =
    sylar::Config::Lookup("server.work_path"
            ,std::string("/home/d/work")
            , "server work path");

static sylar::ConfigVar<std::string>::ptr g_server_pid_file =
    sylar::Config::Lookup("server.pid_file"
            ,std::string("sylar.pid")
            , "server pid file");

//TCPserver
static sylar::ConfigVar<std::vector<TcpServerConf> >::ptr g_servers_conf
    = sylar::Config::Lookup("servers", std::vector<TcpServerConf>(), "http server config");
    
    

Application* Application::s_instance = nullptr;

Application::Application() {
    s_instance = this;
}

bool Application::init(int argc, char** argv) {
    m_argc = argc;
    m_argv = argv;
//singleton
    sylar::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
    sylar::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
    sylar::EnvMgr::GetInstance()->addHelp("c", "conf path default: ./conf");
    sylar::EnvMgr::GetInstance()->addHelp("p", "print help");
//help
    bool is_print_help = false;
    if(!sylar::EnvMgr::GetInstance()->init(argc, argv)) {
        is_print_help = true;
    }

    if(sylar::EnvMgr::GetInstance()->has("p")) {
        is_print_help = true;
    }
//load from config dir
    std::string conf_path = sylar::EnvMgr::GetInstance()->getConfigPath();
    SYLAR_LOG_INFO(g_logger) << "load conf path:" << conf_path;
    sylar::Config::LoadFromConfDir(conf_path);

//modules
    ModuleMgr::GetInstance()->init();
    std::vector<Module::ptr> modules;
    ModuleMgr::GetInstance()->listAll(modules);
//before
    for(auto i : modules) {
        i->onBeforeArgsParse(argc, argv);
    }

    if(is_print_help) {
        sylar::EnvMgr::GetInstance()->printHelp();
        return false;
    }
//after
    for(auto i : modules) {
        i->onAfterArgsParse(argc, argv);
    }
    modules.clear();
//run_type
    int run_type = 0;
    if(sylar::EnvMgr::GetInstance()->has("s")) {
        run_type = 1;
    }
    if(sylar::EnvMgr::GetInstance()->has("d")) {
        run_type = 2;
    }

    if(run_type == 0) {
        sylar::EnvMgr::GetInstance()->printHelp();
        return false;
    }
//touch pidfile
    std::string pidfile = g_server_work_path->getValue()
                                + "/" + g_server_pid_file->getValue();
    if(sylar::FSUtil::IsRunningPidfile(pidfile)) {
        SYLAR_LOG_ERROR(g_logger) << "server is running:" << pidfile;
        return false;
    }

    if(!sylar::FSUtil::Mkdir(g_server_work_path->getValue())) {
        SYLAR_LOG_FATAL(g_logger) << "create work path [" << g_server_work_path->getValue()
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}


bool Application::run() {
	g_logger->setLevel(sylar::LogLevel::INFO);
    bool is_daemon = sylar::EnvMgr::GetInstance()->has("d");
    return start_daemon(m_argc, m_argv,
            std::bind(&Application::main, this, std::placeholders::_1,
                std::placeholders::_2), is_daemon);
}


int Application::main(int argc, char** argv) {
    SYLAR_LOG_INFO(g_logger) << "main";
    //std::string conf_path = sylar::EnvMgr::GetInstance()->getConfigPath();
    //sylar::Config::LoadFromConfDir(conf_path, true);
    {
        std::string pidfile = g_server_work_path->getValue()
                                    + "/" + g_server_pid_file->getValue();
        std::ofstream ofs(pidfile);
        if(!ofs) {
            SYLAR_LOG_ERROR(g_logger) << "open pidfile " << pidfile << " failed";
            return false;
        }
        ofs << getpid();
    }

    m_mainIOManager.reset(new sylar::IOManager(1, true, "main"));
    m_mainIOManager->schedule(std::bind(&Application::run_fiber, this));
    m_mainIOManager->addTimer(2000, [](){}, true);
    m_mainIOManager->stop();
    return 0;
}


int Application::run_fiber() {
	//moudles
    std::vector<Module::ptr> modules;
    ModuleMgr::GetInstance()->listAll(modules);
    bool has_error = false;
    for(auto& i : modules) {
        if(!i->onLoad()) {
            SYLAR_LOG_ERROR(g_logger) << "module name="
                << i->getName() << " version=" << i->getVersion()
                << " filename=" << i->getFilename();
            has_error = true;
        }
    }
    if(has_error) {
        _exit(0);
    }
    //getval
    sylar::WorkerMgr::GetInstance()->init();
    auto http_confs = g_servers_conf->getValue();
//   
    for(auto& i : http_confs) {
    //output config in string
        SYLAR_LOG_INFO(g_logger) << std::endl << LexicalCast<TcpServerConf, std::string>()(i);
        //address
        std::vector<Address::ptr> address;
        for(auto& a : i.address) {
            size_t pos = a.find(":");
            if(pos == std::string::npos) {
                //SYLAR_LOG_ERROR(g_logger) << "invalid address: " << a;
                address.push_back(UnixAddress::ptr(new UnixAddress(a)));
                continue;
            }
            int32_t port = atoi(a.substr(pos + 1).c_str());
            //127.0.0.1
            auto addr = sylar::IPAddress::Create(a.substr(0, pos).c_str(), port);
            if(addr) {
                address.push_back(addr);
                continue;
            }
            std::vector<std::pair<Address::ptr, uint32_t> > result;
            if(sylar::Address::GetInterfaceAddresses(result,
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
            auto aaddr = sylar::Address::LookupAny(a);
            if(aaddr) {
                address.push_back(aaddr);
                continue;
            }
            SYLAR_LOG_ERROR(g_logger) << "invalid address: " << a;
            _exit(0);
        }
        //IOM
        IOManager* accept_worker = sylar::IOManager::GetThis();
        IOManager* process_worker = sylar::IOManager::GetThis();
        //accept_worker
        if(!i.accept_worker.empty()) {
            accept_worker = sylar::WorkerMgr::GetInstance()->getAsIOManager(i.accept_worker).get();
            if(!accept_worker) {
                SYLAR_LOG_ERROR(g_logger) << "accept_worker: " << i.accept_worker
                    << " not exists";
                _exit(0);
            }
        }
        //process_worker
        if(!i.process_worker.empty()) {
            process_worker = sylar::WorkerMgr::GetInstance()->getAsIOManager(i.process_worker).get();
            if(!process_worker) {
                SYLAR_LOG_ERROR(g_logger) << "process_worker: " << i.process_worker
                    << " not exists";
                _exit(0);
            }
        }
        
        //TcpServer
        TcpServer::ptr server;
        if(i.type == "http") {
            server.reset(new sylar::http::HttpServer(i.keepalive,
                            process_worker, accept_worker));
        } else if(i.type == "ws") {
            server.reset(new sylar::http::WSServer(
                            process_worker, accept_worker));
        } else {
            SYLAR_LOG_ERROR(g_logger) << "invalid server type=" << i.type
                << LexicalCast<TcpServerConf, std::string>()(i);
            _exit(0);
        }
        //bind address
        std::vector<Address::ptr> fails;
        if(!server->bind(address, fails, i.ssl)) {
            for(auto& x : fails) {
                SYLAR_LOG_ERROR(g_logger) << "bind address fail:"
                    << *x;
            }
            _exit(0);
        }
        // check the certificates 
        if(i.ssl) {
            if(!server->loadCertificates(i.cert_file, i.key_file)) {
                SYLAR_LOG_ERROR(g_logger) << "loadCertificates fail, cert_file="
                    << i.cert_file << " key_file=" << i.key_file;
            }
        }
        // name
        if(!i.name.empty()) {
            server->setName(i.name);
        }
        //start
		server->setConf(i);
		m_servers[i.type].push_back(server);
        server->start();
	}
    	for(auto& i : modules) {
        	i->onServerReady();
    	}
    return 0;
}


bool Application::getServer(const std::string& type, std::vector<TcpServer::ptr>& svrs) {
    auto it = m_servers.find(type);
    if(it == m_servers.end()) {
    	for(auto &i : m_servers){
    		SYLAR_LOG_INFO(g_logger) << i.first;
    	}
    	SYLAR_LOG_INFO(g_logger) << "traverse app::m_servers over";
        return false;
    }
    svrs = it->second;
    return true;
}

}


