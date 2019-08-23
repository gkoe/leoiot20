#include "UdpLoggerTarget.h"

#include <tcpip_adapter.h>

#include <Constants.h>
#include <EspConfig.h>
#include <EspTime.h>
#include <Logger.h>
#include <string.h>
#include <EspUdp.h>
#include <EspConfig.h>

/**
 * Ermittelt aus der aktuellen IP-Adresse die Broadcastadresse, die im letzten Byte
 * FF enthält.
 */
void getBroadcastIp(ip4_addr_t *broadcastIpPtr)
{
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	tcpip_adapter_ip_info_t ipInfo;
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
	uint32_t broadcastAddress = ipInfo.ip.addr;

	// sprintf(loggerMessage, "ESP-IP: %u", broadcastAddress);
	// Logger.info("getBroadcastIp();  IP", loggerMessage);
	broadcastAddress = broadcastAddress % (16777216) + 4278190080u; //(255*256*256*256);
	broadcastIpPtr->addr = broadcastAddress;
	sprintf(loggerMessage, "Broadcast-IP: %s", ip4addr_ntoa(broadcastIpPtr));
	Logger.info("Udplogger Broadcast IP", loggerMessage);
}

UdpLoggerTarget::UdpLoggerTarget(const char *name, int logLevel)
	: LoggerTarget(name, logLevel)
{
	strcpy(_name, name);
	EspConfig.getNvsStringValue("loggerip", _ipAddress);
	if (strlen(_ipAddress) <= 0)
	{
		ip4_addr_t broadcastIp;
		getBroadcastIp(&broadcastIp);
		strcpy(_ipAddress, ip4addr_ntoa(&broadcastIp));
	}
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	sprintf(loggerMessage, "Udp-TargetAddress: %s, Port: %d created", _ipAddress, _port);
	Logger.info("Udplogger Constructor", loggerMessage);
}

void UdpLoggerTarget::log(const char *logLevelText, const char *tag, const char *message)
{
	_id++;
	char logMessage[LENGTH_LOGGER_MESSAGE];
	// char timeText[LENGTH_MIDDLE_TEXT];
	long time = EspTime.getTime();
	// Serial.printf("*ULT Thingname: %s\n", Logger.getThingName());
	sprintf(logMessage, "%ld;%ld;%s;%s;%s;%s", time, _id, EspConfig.getThingName(), logLevelText, tag, message);

	if (!EspUdp.sendUdpMessage(_ipAddress, _port, logMessage))
	{
		printf("!!!! ULT, Error in endPacket()!");
		return;
	}
	else
	{
		//printf("!!!! ULT Sent Udp-Logmessage to %s:%d, Text: %s\n", _ipAddress, _port, logMessage); //No Logger else infinity loop
	}
}

int UdpLoggerTarget::getPort()
{
	return _port;
}