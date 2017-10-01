// GsmModule.h

#ifndef _GSMMODULE_h
#define _GSMMODULE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


class GsmModule
{
public:
	GsmModule();
	~GsmModule();
	bool Init();
	bool Call(String number);
	bool Sms(String number, String message);
	bool HangOut();
	String Signal();
	String Operator();
private:
	int timeout;
	String rx_buffer;
	String readSerial();
	bool GsmAck(String str);
};

#endif

