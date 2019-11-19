// https://github.com/espressif/esp-idf/blob/a7e8d87d3e5ccc9e5ffcd701a1bac587ba4f43ea/examples/protocols/esp_http_client/main/esp_http_client_example.c

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
// #include "protocol_examples_common.h"
#include "esp_log.h"
#include "tcpip_adapter.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "esp_http_client.h"

#include "esp_tls.h"

#include <HttpClient.h>
#include <Constants.h>
#include <Logger.h>

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD("HttpClient", "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD("HttpClient", "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD("HttpClient", "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD("HttpClient", "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD("HttpClient", "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // Write out data
            // printf("%.*s", evt->data_len, (char*)evt->data);
        }

        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD("HttpClient", "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI("HttpClient", "HTTP_EVENT_DISCONNECTED");
        // int mbedtls_err = 0;
        // esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
        // if (err != 0) {
        //     ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
        //     ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
        // }
        break;
    }
    return ESP_OK;
}

void HttpClientClass::getFullUrl(char *fullUrl, const char *url, bool https, const char *user, const char *password)
{
    if (user != nullptr) // with basic authentication
    {
        if (https)
        {
            sprintf(fullUrl, "https://%s:%s@%s", user, password, url);
        }
        else
        {
            sprintf(fullUrl, "http://%s:%s@%s", user, password, url);
        }
    }
    else // without authentication
    {
        if (https)
        {
            sprintf(fullUrl, "https://%s", url);
        }
        else
        {
            sprintf(fullUrl, "http://%s", url);
        }
    }
}

void HttpClientClass::post(const char *url, const char *payload, bool https, const char *user, const char *password)
{
    char fullUrl[LENGTH_PAYLOAD];
    getFullUrl(fullUrl, url, https, user, password);
    esp_http_client_config_t config = {};
    config.url = fullUrl;
    config.event_handler = _http_event_handler;
    config.auth_type = HTTP_AUTH_TYPE_BASIC;
    config.method=HTTP_METHOD_POST;
    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    sprintf(loggerMessage, "url: %s, payload: %s", fullUrl, payload);
    Logger.debug("HttpClient, post()", loggerMessage);
    esp_http_client_handle_t client = esp_http_client_init(&config);
    // esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, payload, strlen(payload));
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        sprintf(loggerMessage, "HTTP POST Status = %d, content_length = %d",
                esp_http_client_get_status_code(client), esp_http_client_get_content_length(client));
        Logger.debug("HttpClient, post()", loggerMessage);
    }
    else
    {
        sprintf(loggerMessage, "HTTP POST request failed: %s", esp_err_to_name(err));
        Logger.error("HttpClient, post()", loggerMessage);
        // ESP_LOGE("HttpClient", "HTTP POST request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

void HttpClientClass::get(const char *url, char *response, int responseLength, bool https, const char *user, const char *password)
{
    char fullUrl[LENGTH_PAYLOAD];
    getFullUrl(fullUrl, url, https, user, password);
    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    response[0] = 0;
    sprintf(loggerMessage, "url: %s", fullUrl);
    Logger.debug("HttpClient, get()", loggerMessage);
    esp_http_client_handle_t client = getHttpClient(fullUrl);
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        int contentLength = esp_http_client_get_content_length(client);
        sprintf(loggerMessage, "HTTP GET Status = %d, content_length = %d",
                esp_http_client_get_status_code(client), contentLength);
        Logger.debug("HttpClient, get()", loggerMessage);
        // ESP_LOGI("HttpClient", "HTTP GET Status = %d, content_length = %d",
        //          esp_http_client_get_status_code(client),
        //          esp_http_client_get_content_length(client));
        if (contentLength == -1 || contentLength + 1 > responseLength)
        {
            sprintf(loggerMessage, "esp_http_client_read failed, contentLength: %d GT responseLength: %d", contentLength, responseLength);
            Logger.error("HttpClient, get()", loggerMessage);
        }
        else
        {
            int readLength = esp_http_client_read(client, response, contentLength);
            if (readLength <= 0)
            {
                sprintf(loggerMessage, "esp_http_client_read failed, readLength: %d", readLength);
                Logger.error("HttpClient, get()", loggerMessage);
            }
            response[readLength] = 0;
            snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, response);
            Logger.debug("HttpClient, get(), body: ", loggerMessage);
        }
    }
    else
    {
        sprintf(loggerMessage, "HTTP GET request failed: %s", esp_err_to_name(err));
        Logger.error("HttpClient, get()", loggerMessage);
        // ESP_LOGE("HttpClient", "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

esp_http_client_handle_t HttpClientClass::getHttpClient(const char *url)
{
    esp_http_client_config_t config = {};
    config.url = url; // "https://gerald:piKla87Sie57@leonding.synology.me/esplogs/logentries";
    config.auth_type = HTTP_AUTH_TYPE_BASIC;
    config.cert_pem = nullptr;
    config.event_handler = _http_event_handler;
    esp_http_client_handle_t client = esp_http_client_init(&config);
    return client;
}

HttpClientClass HttpClient;