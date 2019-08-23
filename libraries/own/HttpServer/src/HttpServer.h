#pragma once

#include <map>
#include <esp_http_server.h>

class HttpServerClass
{
    public:
        void init();
        void addRoute(const httpd_uri* httpdUri);
        std::map<char *, char *> getQueryParams(char *queryString);
        
    private:
    httpd_handle_t startWebserver();
    httpd_handle_t _server;
};
extern HttpServerClass HttpServer;
