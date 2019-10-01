/* HTTPS GET Example using plain mbedTLS sockets
 *
 * Contacts the howsmyssl.com API via TLS v1.2 and reads a JSON
 * response.
 *
 * Adapted from the ssl_client1 example in mbedtls.
 *
 * Original Copyright (C) 2006-2016, ARM Limited, All Rights Reserved, Apache 2.0 License.
 * Additions Copyright (C) Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD, Apache 2.0 License.
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
#include "protocol_examples_common.h"
#include "tcpip_adapter.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "esp_http_client.h"

#include "esp_tls.h"

#include <Logger.h>
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <EspStation.h>
#include <HttpServer.h>
#include <EspConfig.h>
#include <EspTime.h>
#include <EspMqttClient.h>
#include <SystemService.h>
#include <EspUdp.h>
#include <UdpLoggerTarget.h>

/* Constants that aren't configurable in menuconfig */
#define WEB_PORT "443"
#define WEB_SERVER "leonding.synology.me"
#define WEB_URL "https://leonding.synology.me"

// https://leonding.synology.me/esplogs
// Basic Z2VyYWxkOnBpS2xhODdTaWU1Nw==

// #define WEB_SERVER "www.howsmyssl.com"
// #define WEB_URL "https://www.howsmyssl.com/a/check"

static const char *TAG = "example";

const char *test_root_ca =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n"
    "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
    "DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n"
    "SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n"
    "GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"
    "AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n"
    "q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n"
    "SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n"
    "Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n"
    "a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n"
    "/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n"
    "AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n"
    "CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n"
    "bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n"
    "c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n"
    "VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n"
    "ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n"
    "MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n"
    "Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n"
    "AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n"
    "uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n"
    "wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n"
    "X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n"
    "PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n"
    "KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n"
    "-----END CERTIFICATE-----\n";

// LetsEncryptRoot.cer
const char *root_ca =
    "-----BEGIN CERTIFICATE-----\n\
MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\
MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\
DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\
PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\
Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\
AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\
rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\
OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\
xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\
7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\
aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\
HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\
SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\
ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\
AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\
R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\
JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\
Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\
\n-----END CERTIFICATE-----\n";

const char *after_root_ca = "";

extern "C"
{
    void app_main(void);
}

/* Root cert for howsmyssl.com, taken from server_root_cert.pem
   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null
   The CA root cert is the last cert given in the chain of certs.
   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
// extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
// extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

static void https_get_task(void *pvParameters)
{
    char buf[512];
    int ret, len;

    while (1)
    {
        // esp_tls_cfg_t cfg = {};
        // cfg.cacert_pem_buf = (const unsigned char *)test_root_ca;
        // cfg.cacert_pem_bytes = strlen(test_root_ca);

        esp_http_client_config_t config = {};
        config.url = "https://gerald:piKla87Sie57@leonding.synology.me/esplogs/logentries";
        config.auth_type = HTTP_AUTH_TYPE_BASIC;
        config.cert_pem = nullptr;

        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_err_t err = esp_http_client_perform(client);

        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "Status = %d, content_length = %d",
                     esp_http_client_get_status_code(client),
                     esp_http_client_get_content_length(client));
        }
        esp_http_client_cleanup(client);


        // struct esp_tls *tls = esp_tls_conn_http_new(WEB_URL, nullptr);

        // // struct esp_tls *tls = esp_tls_conn_http_new(WEB_URL, &cfg);
        // char request[500];
        // sprintf(request, "GET %s\r\n", WEB_URL);

        // if (tls != NULL)
        // {
        //     ESP_LOGI(TAG, "Connection established...");
        //     size_t written_bytes = 0;
        //     do
        //     {

        //         ret = esp_tls_conn_write(tls,
        //                                  request + written_bytes,
        //                                  strlen(request) - written_bytes);
        //         if (ret >= 0)
        //         {
        //             ESP_LOGI(TAG, "%d bytes written", ret);
        //             written_bytes += ret;
        //         }
        //         else if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        //         {
        //             ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x", ret);
        //             break;
        //         }
        //     } while (written_bytes < strlen(request));

        //     ESP_LOGI(TAG, "Reading HTTP response...");

        //     do
        //     {
        //         len = sizeof(buf) - 1;
        //         bzero(buf, sizeof(buf));
        //         ret = esp_tls_conn_read(tls, (char *)buf, len);

        //         if (ret == MBEDTLS_ERR_SSL_WANT_WRITE || ret == MBEDTLS_ERR_SSL_WANT_READ)
        //             continue;

        //         if (ret < 0)
        //         {
        //             ESP_LOGE(TAG, "esp_tls_conn_read  returned -0x%x", -ret);
        //             break;
        //         }

        //         if (ret == 0)
        //         {
        //             ESP_LOGI(TAG, "connection closed");
        //             break;
        //         }

        //         len = ret;
        //         ESP_LOGD(TAG, "%d bytes read", len);
        //         /* Print response directly to stdout as it is read */
        //         for (int i = 0; i < len; i++)
        //         {
        //             putchar(buf[i]);
        //         }
        //     } while (1);
        // }
        // else
        // {
        //     ESP_LOGE(TAG, "Connection failed...");
        // }

        // esp_tls_conn_delete(tls);
        // putchar('\n'); // JSON output doesn't have a newline at end

        // static int request_count;
        // ESP_LOGI(TAG, "Completed %d requests", ++request_count);

        // for (int countdown = 10; countdown >= 0; countdown--)
        // {
        //     ESP_LOGI(TAG, "%d...", countdown);
        //     vTaskDelay(1000 / portTICK_PERIOD_MS);
        // }
        // ESP_LOGI(TAG, "Starting again!");
    }
}

void app_main(void)
{
    printf("==============\n");
    printf("UdpLogger Test\n");
    printf("==============\n");
    EspConfig.init();
    const char *thingName = EspConfig.getThingName();
    Logger.init("SslTest");
    SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget("SLT", LOG_LEVEL_VERBOSE);
    Logger.addLoggerTarget(serialLoggerTarget);
    SystemService.init();
    EspStation.init();
    Logger.info("SslTest, app_main()", "Waiting for connection!");
    while (!EspStation.isStationOnline())
    {
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    HttpServer.init();
    Logger.info("SslTest, app_main()", "HttpServer started");
    EspTime.init();
    EspUdp.init();
    UdpLoggerTarget *udpLoggerTargetPtr = new UdpLoggerTarget("ULT", LOG_LEVEL_VERBOSE);
    Logger.addLoggerTarget(udpLoggerTargetPtr);

    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    EspConfig.getConfig(loggerMessage, LENGTH_LOGGER_MESSAGE - 1);
    Logger.info("SslTest, Config", loggerMessage);
    Logger.info("SslTest, app_main()", "SslTest running!");

    xTaskCreate(&https_get_task, "https_get_task", 8192, NULL, 5, NULL);

    while (true)
    {
        SystemService.checkSystem();
        vTaskDelay(1);
    }
}
