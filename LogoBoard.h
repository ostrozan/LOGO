// Board.h

#ifndef _LOGOBOARD_h
#define _LOGOBOARD_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
#include "GPIO.h"

//def modules
//#define CAN_MODULE
////#define WIFI_MODULE
//#define RTC_MODULE
//#define GSM_MODULE
//#define RS485_MODULE

#ifdef CAN_MODULE
#include <mcp_can_dfs.h>
#include <mcp_can.h>
#endif // CAN_MODULE

#ifdef WIFI_MODULE
#include <ESP8266wifi.h>
#endif // WIFI_MODULE

#ifdef RTC_MODULE
#include <Wire.h>
#include <ds3231.h>
#include "DateTime.h"
#endif // RTC_MODULE

#ifdef GSM_MODULE
#include "GsmModule.h"
#endif // GSM_MODULE

#ifdef RS485_MODULE
#define PIN_CTRL485 22
#define SEND_485 digitalWrite(PIN_CTRL485, LOW);
#define REC_485 digitalWrite(PIN_CTRL485, HIGH);
#endif // RS485_MODULE

#define IN1 A2
#define IN2 A3
#define IN3 A4
#define IN4 A5
#define RELE1 4
#define RELE2 5
#define OUT12V_1 6
#define OUT12V_2 7
#define OUT12V_3 8
#define OUT12V_4 9




#define BTserial Serial
#define GSMserial Serial
#define RS485serial Serial
#define WIFIserial Serial


#include "SoftwareSerial.h"

//SoftwareSerial swSerial (11, 12);
enum Modules
{
	BT, GSM, RS485, WIFI
};


class LogoBoard
{
public:
#ifdef RS485serial
	//SoftwareSerial swSerial(11, 10);
#endif // RS485serial

	Gpio_struct outputs[6];
	Gpio_struct inputs[4];

	LogoBoard ();
	~LogoBoard ();

	void Init ();
	void BTinit (long baudRate);
	void GSMinit (long baudRate);
	void RS485init (long baudRate);
	void WIFIinit (long baudRate);
	void CANInit (int speed);
	void Send (Modules modul, String str);
	void Send (Modules modul, char* arr);
	void I_O_init ();
	void GpioInit (Gpio_struct *gpio, char pin, char type);

private:

};


#endif

