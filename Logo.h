#pragma once
#ifndef _LOGO_h
#define _LOGO_h
#include <TimerOne.h>
#include <SPI.h>
#include "LogoBoard.h"
#include "DateTime.h"
#include <AT24CX.h>

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

#pragma region instance trid
LogoBoard board;
SoftwareSerial swSerial (11, 10);
DateTime dateTime;
#pragma endregion

#pragma region makra
#define COM swSerial

#define INPUTS_CONTROL outputs[0].IsInputControl||outputs[1].IsInputControl||outputs[2].IsInputControl||outputs[3].IsInputControl||outputs[4].IsInputControl||outputs[5].IsInputControl

#pragma endregion

#pragma region typedefs
typedef struct In
{
	char func_index;
	char outs[6];
	String tel;
	char sms[20];
	char nmb;
	boolean blockSendOn;
	boolean blockSendOff;
}In;

typedef struct
{
	int startTime;
	int stopTime;
}MinuteSpan;

typedef struct
{
	int timeOfDelay;
	int timeOfPulse;
}ControlTimes;

typedef struct Out
{
	boolean IsTimeControl;
	boolean IsInputControl;
	boolean IsExtControl;
	boolean IsUseSwitchClk;
	boolean IsUseProgTmr;
	boolean IsTrvale;
	boolean IsNastCas;
	boolean IsSwitchOn;
	boolean IsSwitchOff;
	boolean IsAnyChange;
	MinuteSpan minutespans[5];
	ControlTimes controlTimes;
	char state;
	char nmb;
}Out;

typedef enum { empty, first, second }enumFlag;
#pragma endregion

#pragma region promenne
String rxBuffer = " ";/*#:3:100000:123456789:ahoj :#:3:010000:000000000::#:4:000000:123456789:ahoj:#:0:000111:000000000::#:1:0:0:0:1:0:1:1:0:0:%4-2%:%240-0%184-6%:#:1:1:0:0:1:1:0:1:0:0:%6-0%:%:#:0:0:0:0:0:0:0:0:0:0:%0-0%:%:#:0:1:0:0:0:0:0:0:0:0:%0-0%:%:#:1:1:0:0:1:0:1:1:0:0:%7441-120%:%:#:1:0:0:1:0:0:0:0:0:0:%0-0%:%300-9%:#*///";
String rozdelenyString[10];
unsigned char inpNmbs[4] = { A2,A3,A4,A5 };
unsigned char outNmbs[6] = { 4,5,6,7,8,9 };
int nmbOfSubstr;
In inputs[4];
Out outputs[6];
int outTimers[6];
enumFlag whichTime[6];
boolean outTimersFlg[6];
boolean recMsg = false;
int rxBufferIndex;
boolean sendDateTimeFlg;
int minutes;
#pragma endregion

#endif

