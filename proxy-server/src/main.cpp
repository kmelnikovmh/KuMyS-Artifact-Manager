#include <fastcgi++/request.hpp>
#include <LightJson.hpp>

class Proxy: public Fastcgipp::Request<wchar_t> {
public:
    Proxy():
        Fastcgipp::Request<wchar_t>(5*1024)         // post max size 5kilobit
    {}

private:
    bool process_request(){}
    void response_json_to_main_server(){}
    void response_error_to_client(){}

    bool response() {
        if(process_request())
            response_error_to_client();
        else
            response_json_to_main_server();

        return true;
    }
};
 
#include <fastcgi++/manager.hpp>
 
int main() {
    Fastcgipp::Manager<Proxy> manager;
    manager.setupSignals();
    manager.listen();
    manager.start();
    manager.join();
 
    return 0;
}