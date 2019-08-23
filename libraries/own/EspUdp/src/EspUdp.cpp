#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include <EspUdp.h>
#include <Constants.h>
#include <LoggerTarget.h>
#include <Logger.h>
#include <UdpLoggerTarget.h>

#define SERVERPORT 41235

/*
    Eine gültige IP-Adresse muss aus vier Zahlen, getrennt durch drei Punkte bestehen
*/
bool isValidIp(char *text)
{
    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    int len = strlen(text);
    bool hasWrongChar = false;
    if (len < 7)
    {
        sprintf(loggerMessage, "ipText: %s is too short", text);
    }
    else
    {
        int dots = 0;
        int index = 0;
        while (!hasWrongChar && index < len)
        {
            if (text[index == '.'])
            {
                dots++;
            }
            else if (text[index] < '0' || text[index] > '9')
            {
                hasWrongChar = true;
                sprintf(loggerMessage, "ipText: %s has wrong chars", text);
            }
            index++;
        }
    }
    if (len < 7 || hasWrongChar)
    {
        Logger.error("EspUdp;isValidIp()", loggerMessage);
        return false;
    }
    // sprintf(loggerMessage, "Valid IpAddress: %s", text);
    // Logger.info("EspUdp;isValidIp()", loggerMessage);
    return true;
}

static void udp_server_task(void *pvParameters)
{
    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    char data[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;
    uint8_t lastChar = ' ';

    while (1)
    {

        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(SERVERPORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0)
        {
            sprintf(loggerMessage, "Unable to create socket: errno %d", errno);
            Logger.error("EspUdp; udp_server_task()", loggerMessage);
            break;
        }
        Logger.info("EspUdp; udp_server_task()", "Socket created");

        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0)
        {
            sprintf(loggerMessage, "Socket unable to bind: errno %d", errno);
            Logger.error("EspUdp; udp_server_task()", loggerMessage);
            break;
        }
        sprintf(loggerMessage, "Socket bound, port %d", SERVERPORT);
        Logger.info("EspUdp; udp_server_task()", loggerMessage);

        while (1)
        {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            // Logger.info("EspUdp; udp_server_task()", "Waiting for data");
            struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, data, sizeof(data) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
            // Logger.info("EspUdp; udp_server_task()", "Data received");

            // Error occurred during receiving
            if (len < 0)
            {
                sprintf(loggerMessage, "recvfrom failed: errno %d", errno);
                Logger.error("EspUdp; udp_server_task()", loggerMessage);
                break;
            }
            // Data received
            else
            {
                // Get the sender's ip address as string
                if (source_addr.sin6_family == PF_INET)
                {
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                    // sprintf(loggerMessage, "Sourceaddress of udp-packet: %s", addr_str);
                    // Logger.info("EspUdp; udp_server_task()", loggerMessage);
                }
                else if (source_addr.sin6_family == PF_INET6)
                {
                    inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                }

                data[len] = 0;                        // Null-terminate whatever we received and treat like a string...
                if (data[0] >= '0' && data[0] <= '5') // Verbose bis NoLog
                {
                    int logLevel = data[0] - '0';
                    LoggerTarget *loggerTarget = Logger.getLoggerTarget("ULT");
                    loggerTarget->setLogLevel(logLevel);
                    sprintf(loggerMessage, "Data: %s, UdpLogger LogLevel set to %i", data, logLevel);
                    Logger.info("EspUdp; udp_server_task()", loggerMessage);
                }
                else if ((data[0] >= 'A' && data[0] <= 'Z') || (data[0] >= 'a' && data[0] <= 'z'))
                {
                    if (data[0] != lastChar)
                    {
                        sprintf(loggerMessage, "acknowledged: %s to %s", data, addr_str);
                        Logger.info("EspUdp;echo()", loggerMessage);
                        lastChar = data[0];
                    }
                }
                else
                {
                    sprintf(loggerMessage, "UdpLogger UdpMessage %s is invalid", data);
                    Logger.error("EspUdp; udp_server_task()", loggerMessage);
                }
            }
        }

        if (sock != -1)
        {
            sprintf(loggerMessage, "Shutting down socket and restarting...");
            Logger.error("EspUdp; udp_server_task()", loggerMessage);
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

bool EspUdpClass::sendUdpMessage(const char *ipAddress, const int port, const char *message)
{
    //printf("!!! EspUdp; sendUdpMessage(); Start\n");
    char addr_str[128];
    // int addr_family;
    // int ip_protocol;

    //char loggerMessage[LENGTH_LOGGER_MESSAGE];
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(ipAddress);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    // addr_family = AF_INET;
    // ip_protocol = IPPROTO_IP;

    if (_clientSocket < 0)
    {
        printf("!!!! EspUdp; udp_client_task();clientSocket lower 0: %d\n", _clientSocket);
    }

    inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

    int err = sendto(_clientSocket, message, strlen(message), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0)
    {
        printf("!!!! EspUdp; udp_client_task();Error occurred during sending: errno %d\n", errno);
        return false;
    }
    //printf("!!! EspUdp; udp_client_task(); Message sent: %s to %s:%i", message, ipAddress,port);
    return true;
}

void EspUdpClass::init()
{
    int addr_family;
    int ip_protocol;
    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    xTaskCreate(udp_server_task, "udp_server", 4096, NULL, 5, NULL);

    _clientSocket = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (_clientSocket < 0)
    {
        sprintf(loggerMessage, "Unable to create socket: errno %d", errno);
        Logger.error("EspUdp; init()", loggerMessage);
        return;
    }
    sprintf(loggerMessage, "Clientsocket created!");
    Logger.info("EspUdp; init()", loggerMessage);
}

EspUdpClass EspUdp;