#pragma once
#ifndef __FLZ_HTTP_CONFIG_SERVLET_H__
#define __FLZ_HTTP_CONFIG_SERVLET_H__


#include "../servlet.h"


namespace flz {


namespace http {

class ConfigServlet : public Servlet {
public:
    ConfigServlet();
    virtual int32_t handle(flz::http::HttpRequest::ptr request
                   , flz::http::HttpResponse::ptr response
                   , flz::http::HttpSession::ptr session) override;
};

}



}

#endif

