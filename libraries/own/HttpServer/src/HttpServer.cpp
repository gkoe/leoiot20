#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_eth.h"

#include <iterator>
#include <map>

#include <Constants.h>
#include <HttpServer.h>
#include <Logger.h>
#include <EspConfig.h>
#include <EspMqttClient.h>

static const char *TAG = "httpserver";

/*****************************   Handlers for GET-Requests *****************************/

/**
 * TestHandler for route /echo
 * Gibt die Querystringparameter am seriellen Monitor aus und liefert sie als Echo zurück
 */
static esp_err_t echoHandler(httpd_req_t *req)
{
    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    char responseMessage[LENGTH_LOGGER_MESSAGE];

    int headerLength = httpd_req_get_hdr_value_len(req, "Host");
    sprintf(loggerMessage, "%d", headerLength);
    Logger.debug("HttpServer, echoHandler(), HeaderLength:", loggerMessage);
    if (headerLength > 0)
    {
        esp_err_t err = httpd_req_get_hdr_value_str(req, "Host", loggerMessage, headerLength + 1); // +1 erforderlich, sonst meldet er "truncated"
        if (err == ESP_OK)                                                                         // Lesen des Headers war OK
        {
            Logger.info("HttpServer, echoHandler(), Header Host:", loggerMessage);
        }
        else
        {
            sprintf(loggerMessage, "httpd_req_get_hdr_value_str() ERROR: %d", err);
            Logger.error("HttpServer, echoHandler()", loggerMessage);
        }
    }

    int queryLength = httpd_req_get_url_query_len(req);
    sprintf(loggerMessage, "%d", queryLength);
    Logger.debug("HttpServer, echoHandler(), QueryLength:", loggerMessage);

    if (queryLength > 0)
    {
        esp_err_t err = httpd_req_get_url_query_str(req, loggerMessage, queryLength + 1);
        if (err == ESP_OK) // Lesen der Querystringparameter war ok!
        {
            Logger.info("HttpServer, echoHandler(), QueryString:", loggerMessage);
            sprintf(responseMessage, "%s", loggerMessage);
        }
        else
        {
            sprintf(loggerMessage, "httpd_req_get_url_query_str() ERROR: %d", err);
            Logger.error("HttpServer, echoHandler()", loggerMessage);
            sprintf(responseMessage, "ERROR READ QUERYSTRING!");
        }
    }
    else
    {
        Logger.info("HttpServer, echoHandler(), QueryString:", "NO QUERYSTRING");
        sprintf(responseMessage, "NO QUERYSTRING");
    }
    httpd_resp_send(req, responseMessage, strlen(responseMessage));
    return ESP_OK;
}

/**
 * Liefert die Querystringparameter als Map von Key/Value-Pairs zurück
 */
std::map<char *, char *> HttpServerClass::getQueryParams(char *queryString)
{
    char *equalPtr;
    std::map<char *, char *> keyValuePairs = std::map<char *, char *>();

    char *keyValuePairText;
    char *rest = queryString;

    while ((keyValuePairText = strtok_r(rest, "&", &rest)))
    {
        printf("KeyValuePair gefunden: %s\n", keyValuePairText);
        equalPtr = strtok(keyValuePairText, "=");
        char *key = equalPtr;
        equalPtr = strtok(NULL, "=");
        char *value = equalPtr;
        keyValuePairs.insert(std::pair<char *, char *>(key, value));
    }
    return keyValuePairs;
}

static esp_err_t getRestartHandler(httpd_req_t *req)
{
    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    sprintf(loggerMessage, "Restart ESP by Http");
    Logger.info("HttpServer;getRestartHandler()", loggerMessage);
    vTaskDelay(1000 / portTICK_RATE_MS);
    esp_restart();
    httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
    return ESP_OK;
}

// /**
//  * Die Route /setconfig?key1=xxx&key2=yyy... setzt die
//  * entsprechenden Config-Strings im NVS
//  */
// static esp_err_t setconfigHandler(httpd_req_t *req)
// {
//     char loggerMessage[LENGTH_LOGGER_MESSAGE];
//     char queryString[LENGTH_LOGGER_MESSAGE];
//     char keyToDelete[LENGTH_MIDDLE_TEXT];
//     esp_err_t err;
//     int queryLength = httpd_req_get_url_query_len(req) + 1;
//     if (queryLength > 0)
//     {
//         err = httpd_req_get_url_query_str(req, queryString, queryLength);
//         if (err == ESP_OK)
//         {
//             printf("Querystring: '%s', Length: '%d'\n", queryString, queryLength);
//             if (queryLength > 1 && queryString[queryLength - 2] == '=') //  Zeilenumbruch am Ende,  leere Zuweisung auf Key ==> Key im NVS löschen
//             {
//                 strncpy(keyToDelete, queryString, queryLength - 2);
//                 EspConfig.deleteKey(keyToDelete);
//             }
//             else // Key/Value-Pairs im NVS anlegen
//             {
//                 std::map<char *, char *> configPairs = HttpServer.getQueryParams(queryString);
//                 std::map<char *, char *>::iterator itr;
//                 for (itr = configPairs.begin(); itr != configPairs.end(); itr++)
//                 {
//                     printf("Aus Map: Key: '%s', Value: '%s'\n", itr->first, itr->second);
//                     EspConfig.setNvsStringValue(itr->first, itr->second);
//                 }
//             }
//         }
//         else
//         {
//             sprintf(loggerMessage, "httpd_req_get_url_query_str() ERROR: %d", err);
//             Logger.error("HttpServer, setconfigHandler()", loggerMessage);
//         }
//     }
//     else
//     {
//         Logger.info("HttpServer, setconfigHandler(), QueryString:", "NO QUERYSTRING");
//     }
//     const char *resp_str = "OK";
//     err = httpd_resp_send(req, resp_str, strlen(resp_str));
//     if (err != ESP_OK)
//     {
//         sprintf(loggerMessage, "httpd_resp_send() ERROR: %d", err);
//         Logger.error("HttpServer, setconfigHandler()", loggerMessage);
//     }
//     return ESP_OK;
// }

static esp_err_t configHandler(httpd_req_t *req)
{
    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    //char readKey[LENGTH_MIDDLE_TEXT];
    char queryString[LENGTH_MIDDLE_TEXT];
    char response[LENGTH_LOGGER_MESSAGE];
    char keyToDelete[LENGTH_MIDDLE_TEXT];
    char readValue[LENGTH_MIDDLE_TEXT];
    esp_err_t err;
    int queryLength = httpd_req_get_url_query_len(req) + 1;
    if (queryLength > 1) // gibt es überhaupt Parameter ==> spezielle Config lesen oder setzen
    {
        err = httpd_req_get_url_query_str(req, queryString, queryLength);
        if (err == ESP_OK)
        {
            if (queryString != NULL)
            {
                printf("Querystring: '%s', Length: '%d'\n", queryString, queryLength);
                if (queryLength > 1 && queryString[queryLength - 2] == '=') //  Zeilenumbruch am Ende,  leere Zuweisung auf Key ==> Key im NVS löschen
                {
                    strncpy(keyToDelete, queryString, queryLength - 2);
                    keyToDelete[queryLength - 2]=0;
                    EspConfig.deleteKey(keyToDelete);
                    sprintf(response, "Config key: %s deleted", keyToDelete);
                    sprintf(loggerMessage, "Response: %s", response);
                    Logger.info("HttpServer,configHandler()", loggerMessage);
                    httpd_resp_send(req, response, strlen(response));
                }
                else
                {
                    char *configKey;
                    char *rest = queryString;
                    char *value = nullptr;

                    configKey = strtok_r(rest, "=", &rest);
                    if (configKey)
                    {
                        sprintf(loggerMessage, "Configname: %s", configKey);
                        Logger.info("Thing,configHandler()", loggerMessage);
                        value = strtok_r(rest, "=", &rest);
                        if (!value) // Abfrage des Configvalues
                        {
                            EspConfig.getNvsStringValue(configKey, readValue);
                            if (readValue == NULL)
                            {
                                sprintf(response, "ConfigKey %s not found!", configKey);
                                Logger.info("HttpServer,configHandler()", loggerMessage);
                                httpd_resp_send(req, response, strlen(response));
                            }
                            else
                            {
                                sprintf(response, "Config key: %s, value: %s", configKey, readValue);
                                sprintf(loggerMessage, "Response: %s", response);
                                Logger.info("HttpServer,configHandler()", loggerMessage);
                                httpd_resp_send(req, response, strlen(response));
                            }
                        }
                        else // Setzen des Config-Entry
                        {
                            sprintf(loggerMessage, "Value: %s", value);
                            Logger.info("Thing,configHandler()", loggerMessage);
                            EspConfig.setNvsStringValue(configKey, value);
                            sprintf(response, "Config %s set to %s", configKey, value);
                            sprintf(loggerMessage, "Response: %s", response);
                            Logger.info("Thing,configHandler()", loggerMessage);
                            httpd_resp_send(req, response, strlen(response));
                        }
                    }
                }
            }
        }
        else
        {
            sprintf(loggerMessage, "httpd_req_get_url_query_str() ERROR: %d", err);
            Logger.error("HttpServer, getConfigHandler()", loggerMessage);
            httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
        }
    }
    else // alle fixen Configs auslesen
    {
        char loggerMessage[LENGTH_LOGGER_MESSAGE];
        EspConfig.getConfig(loggerMessage, LENGTH_LOGGER_MESSAGE - 1);
        httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
        Logger.info("HttpServer, getConfigHandler(), Config:", loggerMessage);
    }
    return ESP_OK;
}

static esp_err_t clearConfigHandler(httpd_req_t *req)
{
    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    sprintf(loggerMessage, "Config cleared!");
    EspConfig.clearConfig();
    httpd_resp_send(req, loggerMessage, strlen(loggerMessage));
    return ESP_OK;
}

static const httpd_uri_t configUri = {
    .uri = "/config",
    .method = HTTP_GET,
    .handler = configHandler,
    .user_ctx = nullptr};

static const httpd_uri_t clearconfig = {
    .uri = "/clearconfig",
    .method = HTTP_GET,
    .handler = clearConfigHandler,
    .user_ctx = nullptr};

static const httpd_uri_t restart = {
    .uri = "/restart",
    .method = HTTP_GET,
    .handler = getRestartHandler,
    .user_ctx = nullptr};

static const httpd_uri_t echo = {
    .uri = "/echo",
    .method = HTTP_GET,
    .handler = echoHandler,
    .user_ctx = nullptr};

void HttpServerClass::addRoute(const httpd_uri *httpdUri)
{
    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    Logger.info("HttpServer, addRoute()", httpdUri->uri);
    esp_err_t err = httpd_register_uri_handler(_server, httpdUri);
    if (err != ESP_OK)
    {
        sprintf(loggerMessage, "httpd_register_uri_handler(), uri: %s, ERROR: %d", httpdUri->uri, err);
        Logger.error("HttpServer, addRoute()", loggerMessage);
    }
}

httpd_handle_t HttpServerClass::startWebserver()
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        // httpd_register_uri_handler(server, &echo);
        httpd_register_uri_handler(server, &configUri);
        httpd_register_uri_handler(server, &clearconfig);
        httpd_register_uri_handler(server, &restart);
        // httpd_register_uri_handler(server, &testmqttrequest);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

// static void stop_webserver(httpd_handle_t server)
// {
//     // Stop the httpd server
//     httpd_stop(server);
// }

// static void disconnect_handler(void* arg, esp_event_base_t event_base,
//                                int32_t event_id, void* event_data)
// {
//     httpd_handle_t* server = (httpd_handle_t*) arg;
//     if (*server) {
//         ESP_LOGI(TAG, "Stopping webserver");
//         stop_webserver(*server);
//         *server = NULL;
//     }
// }

// static void connect_handler(void* arg, esp_event_base_t event_base,
//                             int32_t event_id, void* event_data)
// {
//     httpd_handle_t* server = (httpd_handle_t*) arg;
//     if (*server == NULL) {
//         ESP_LOGI(TAG, "Starting webserver");
//         *server = startWebserver();
//     }
// }

void HttpServerClass::init()
{
    _server = startWebserver();
}

HttpServerClass HttpServer;