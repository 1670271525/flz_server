#pragma once
#ifndef __FLZ_HTTP_WS_SERVER_H__
#define __FLZ_HTTP_WS_SERVER_H__

#include "../include/tcp_server.h"
#include "ws_session.h"
#include "ws_servlet.h"

namespace flz {


namespace http {

class WSServer : public TcpServer {
public:
    typedef std::shared_ptr<WSServer> ptr;

    WSServer(flz::IOManager* worker = flz::IOManager::GetThis()
             , flz::IOManager* io_worker = flz::IOManager::GetThis()
             , flz::IOManager* accept_worker = flz::IOManager::GetThis());

    WSServletDispatch::ptr getWSServletDispatch() const { return m_dispatch;}
    void setWSServletDispatch(WSServletDispatch::ptr v) { m_dispatch = v;}
protected:
    virtual void handleClient(Socket::ptr client) override;
protected:
    WSServletDispatch::ptr m_dispatch;
};

}

}

#endif
