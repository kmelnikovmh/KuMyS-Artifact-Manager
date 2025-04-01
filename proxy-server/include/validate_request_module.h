#ifndef PROXY_VALIDATE_MODULE_
#define PROXY_VALIDATE_MODULE_

#include <LightJson.hpp>                        // todo
#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>
#include <iostream>

namespace kymus_proxy_server {
class Proxy : public Fastcgipp::Request<wchar_t> {
private:
    bool validate_request();

    void set_pass_to_main_server();
    void set_error_to_client();

    bool response();

public:
    Proxy();
};
}
#endif
