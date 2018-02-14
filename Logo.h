#pragma once
#ifndef _LOGO_h
#define _LOGO_h
#include <TimerOne.h>
#include <SPI.h>
#include "GsmModule.h"
#include "LogoBoard.h"
#include "DateTime.h"
//#include <AT24CX.h>
#include "EEPROM.h"
#include <DallasTemperature.h> 
#include <OneWire.h>
//#define DEBUG
//in1   A2
//in2   A3
//in3   A4
//in4   A5
//SW TX   11
//SW rX   10
// 485 SEL  12
//TX  20
//RX  21
//OUT 1   31
//OUT 2   30
//OUT 3   29
//OUT 4   28
//OUT 5   27
//OUT 6   26



#pragma region makra

#define CONTROLCOMBLUE
#ifdef CONTROLCOMBLUE// opro komunikaci s programem pouzit bluetooth
#define COMCONTROL swSerial
#define COMDEBUG Serial

#else // opro komunikaci s programem pouzit usb konektor
#define COMCONTROL Serial//
#define COMDEBUG swSerial
#define COMGSM Serial1

#endif // CONTROLCOMBLUE
#define INPUTS_CONTROL outputs[0].IsInputControl||outputs[1].IsInputControl||outputs[2].IsInputControl||outputs[3].IsInputControl||outputs[4].IsInputControl||outputs[5].IsInputControl

#pragma endregion

#pragma region typedefs
typedef struct In
{
	char func_index;
	char div1;
	char outs[6];
	char div2;
	char tel[10];
	char div3;
	char sms[21];
	char div4;
	char nmb;
	char div5;
	char state;
	boolean blockSendOn;
	boolean blockSendOff;
	boolean isCallingGsm;//priznak ze prozvani
	boolean isSendingSms;//priznak ze poslal sms
}In;

typedef struct
{
	int startTime;
	int stopTime;
}MinuteSpan;

typedef struct
{
	long timeOfDelay;
	long timeOfPulse;
}ControlTimes;

typedef struct Out
{
	boolean IsTimeControl =0;
	boolean IsInputControl=0;
	boolean IsExtControl=0;
	boolean IsUseSwitchClk=0;
	boolean IsUseProgTmr=0;
	boolean IsUseThermostat=0;
	int Temperature=0;
	char TempHysteresis=0;
	int TempAlarmHi=0;
	int TempAlarmLo=0;
	boolean IsAlarmHi=0;
	boolean IsAlarmLo=0;
	char ktere_cidlo=0;
	boolean IsTrvale=0;
	boolean IsNastCas=0;
	boolean IsSwitchOn=0;
	boolean IsSwitchOff=0;
	boolean IsAnyChange=0;
	MinuteSpan minutespans[4];
	ControlTimes controlTimes;
	char state=0;
	char nmb=0;
	boolean blockSendOn=0;
	boolean blockSendOff=0;
	char next_time_msg[6] = { 0,0,0,0,0,0 };
}Out;

typedef struct
{
    boolean isEnabled;
	char telNumber[3][10];

}GsmData;

GsmData gsmData;

typedef enum { empty, first, second }enumFlag;
#pragma endregion

#pragma region promenne
char rxBuffer[100];
char rozdelenyString[7][7];
char divider[30];
const unsigned char inpNmbs[4] = { A2,A3,A4,A5 };
const unsigned char outNmbs[6] = { 4,5,6,7,8,9 };
int nmbOfSubstr;
In inputs[4];
Out outputs[6];
int outTimers[6];
enumFlag whichTime[6];
boolean outTimersFlg[6];
boolean recMsg = false;


int rxBufferIndex;
int recChar;
boolean sendDateTimeFlg;
boolean sendOutsFlg;
int minutes;
int eepromPtr;
//pro fci setdatetime
ts pomTs;
//pro fci decode data
char pom_strgs[6][12];
//char procenta[8] = { 0,0,0,0,0,0,0,0 };
//char pomlcky[6] = { 0,0,0,0,0,0 };
char maskIn;
//pro gsm
char casProzvaneni;//jak dlouho bude prozvanet
int casBlokovaniSms;
char currCallingInput;//ktery vstup zrovna prozvani
char currSendingSmsInput;//ktery vstup zrovna poslal sms
char callingNumber;//index volajiciho cisla ze seznamu
///dallas
int teploty_new[2];
int teploty_old[2];
#define ONE_WIRE_BUS A1
#define TEMPERATURE_PRECISION 9 // Lower resolution



int numberOfDevices; // Number of temperature devices found

DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address
DeviceAddress addr[2];
#pragma endregion

#endif

