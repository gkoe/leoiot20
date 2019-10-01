#pragma once

#include "esp_http_client.h"

class HttpClientClass
{
public:
    void get(const char *url,  char* response, int responseLength, bool https=false, const char* user = nullptr, const char* password = nullptr);
    void post(const char *url, const char *payload, bool https=false, const char* user = nullptr, const char* password = nullptr);
private:
    esp_http_client_handle_t getHttpClient(const char* url);
    void getFullUrl(char *fullUrl, const char *url, bool https, const char *user, const char *password);

};
extern HttpClientClass HttpClient;
