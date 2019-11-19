#pragma once

class EspTimeClass
{

public:
	void init();
	long getTime();
	void getTimeString(char *buffer);
	void getDateString(char *buffer);
	void getDateTimeString(long time, char *buffer);

private:

    const char* _ntpServerName = "0.at.pool.ntp.org";
	unsigned int _ntpPort = 8888;
};

extern EspTimeClass EspTime;


