


#include "Logo.h"

#pragma region instance trid
LogoBoard board;
SoftwareSerial swSerial (11, 10);
DateTime dateTime;
GsmModule gsmModul;
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire (ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors (&oneWire);
#pragma endregion
void setup ()
{
	delay (2000);
	pinMode (inpNmbs[0], INPUT_PULLUP);
	pinMode (inpNmbs[1], INPUT_PULLUP);
	pinMode (inpNmbs[2], INPUT_PULLUP);
	pinMode (inpNmbs[3], INPUT_PULLUP);
	pinMode (outNmbs[0], OUTPUT);
	pinMode (outNmbs[1], OUTPUT);
	pinMode (outNmbs[2], OUTPUT);
	pinMode (outNmbs[3], OUTPUT);
	pinMode (outNmbs[4], OUTPUT);
	pinMode (outNmbs[5], OUTPUT);

	//COMCONTROL.begin (38400);
	COMDEBUG.begin (38400);
	while (!COMDEBUG);

	COMCONTROL.begin (38400);
	//while (!COMCONTROL);

	COMGSM.begin (9600);
	while (!COMGSM);

	eepromPtr = 0;
	EEPROM.get (eepromPtr, inputs);
	eepromPtr = sizeof (inputs);
	EEPROM.get (eepromPtr, outputs);
	eepromPtr = sizeof (outputs);
	EEPROM.get (eepromPtr, gsmData);
	//dallas
	 //Start up the library
	sensors.begin ();

	// Grab a count of devices on the wire
	numberOfFDallasDevices = sensors.getDeviceCount ();
	for (int i = 0; i < numberOfFDallasDevices; i++)
	{
		// Search the wire for address
		sensors.getAddress (tempDeviceAddress, i);
		sensors.getAddress (addr[i], i);
		// set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
		sensors.setResolution (tempDeviceAddress, TEMPERATURE_PRECISION);
	}

	for (int i = 0; i < 4; i++)
	{
		inputs[i].blockSendOn = false;
		inputs[i].state = 0;
		inputs[i].isCallingGsm = false;
		inputs[i].isSendingSms = false;
		inputs[i].isWaitingCall = false;
		inputs[i].counter = -1;
	}
	Timer1.initialize ();
	Timer1.attachInterrupt (TimerTick);
	dateTime.Init ();
	for (int i = 0; i < 6; i++)
	{
		outputs[i].state = 0;
		outputs[i].blockSendOn = 0;
		//outputs[i].IsTimeControl = 0;
		//outputs[i].IsInputControl = 0;
		//outputs[i].IsUseProgTmr = 0;
		//outputs[i].IsUseSwitchClk = 0;
		//outputs[i].IsUseThermostat = 0;

	}
	teploty_new[0] = 0;
	teploty_new[1] = 0;
	casProzvaneni = 0;

	gsmModul.Init ();

	gsmModul.Signal ();
	dateTime.GetDateTime ();
	SaveEvent (0, 0);
}


void loop ()
{

	//posli cas a datum
	if (sendDateTimeFlg)SendDateTime ();
	//komunikace prijem GSM
	if (gsmData.isEnabled && COMGSM.available ()) GsmReceive ();
	////komunikace prijem zpravy
	if (COMCONTROL.available ())
	{
		//COMDEBUG.println ("start");
		ComControlReceive ();
		//COMDEBUG.println ("stop");
	}


	//obsluha vstupu a vystupu
	ObsluhaVystupu ();

	if (sendOutsFlg)SendOuts ();

	/****************************************************************/
	//kontrola stavu na vstupech
	ObsluhaVstupu ();
	if (sendOutsFlg)SendIns ();

}

void SendOuts ()
{
	sendOutsFlg = false;
	COMCONTROL.print ("out");
	COMCONTROL.print (outputs[0].state, 10);//posli zpravu
	COMCONTROL.print (outputs[1].state, 10);//posli zpravu
	COMCONTROL.print (outputs[2].state, 10);//posli zpravu
	COMCONTROL.print (outputs[3].state, 10);//posli zpravu
	COMCONTROL.print (outputs[4].state, 10);//posli zpravu
	COMCONTROL.println (outputs[5].state, 10);//posli zpravu
	delay (100);
}

void SendIns ()
{
	COMCONTROL.print ("in");
	COMCONTROL.print (inputs[0].state, 10);
	COMCONTROL.print (inputs[1].state, 10);
	COMCONTROL.print (inputs[2].state, 10);
	COMCONTROL.println (inputs[3].state, 10);
	delay (50);
}

void ExecuteGsmCommad (char nmbOut, char outState)
{
	switch (nmbOut)
	{
	case '1': ChangeOutput (0, (outState == 'n') ? HIGH : LOW, (outState == 'n') ? ON_SMS : OFF_SMS); break;
	case '2': ChangeOutput (1, (outState == 'n') ? HIGH : LOW, (outState == 'n') ? ON_SMS : OFF_SMS); break;
	case '3': ChangeOutput (2, (outState == 'n') ? HIGH : LOW, (outState == 'n') ? ON_SMS : OFF_SMS); break;
	case '4': ChangeOutput (3, (outState == 'n') ? HIGH : LOW, (outState == 'n') ? ON_SMS : OFF_SMS); break;
	case '5': ChangeOutput (4, (outState == 'n') ? HIGH : LOW, (outState == 'n') ? ON_SMS : OFF_SMS); break;
	case '6': ChangeOutput (5, (outState == 'n') ? HIGH : LOW, (outState == 'n') ? ON_SMS : OFF_SMS); break;
	case '?': SendStatus (); SaveEvent (GSM_QEST, 0); break;
	}
	if (gsmData.isResponse && nmbOut != '?')SendStatus ();
}

void SendStatus ()
{
	for (int i = 0; i < sizeof (rxBuffer); i++, rxBuffer[i] = 0);
	//COMDEBUG.println ("sms");
	//vystupy
	strncpy (rxBuffer, "re1 ", 4);
	if (outputs[0].state == HIGH)strcat (rxBuffer, "zap\n");
	else strcat (rxBuffer, "vyp\n");
	strcat (rxBuffer, "re2 ");
	if (outputs[1].state == HIGH)strcat (rxBuffer, "zap\n");
	else strcat (rxBuffer, "vyp\n");
	strcat (rxBuffer, "out3 ");
	if (outputs[2].state == HIGH)strcat (rxBuffer, "zap\n");
	else strcat (rxBuffer, "vyp\n");
	strcat (rxBuffer, "out4 ");
	if (outputs[3].state == HIGH)strcat (rxBuffer, "zap\n");
	else strcat (rxBuffer, "vyp\n");
	strcat (rxBuffer, "out5 ");
	if (outputs[4].state == HIGH)strcat (rxBuffer, "zap\n");
	else strcat (rxBuffer, "vyp\n");
	strcat (rxBuffer, "out6 ");
	if (outputs[5].state == HIGH)strcat (rxBuffer, "zap\n");
	else strcat (rxBuffer, "vyp\n");
	//vstupy
	strcat (rxBuffer, "in1 ");
	if (inputs[0].state == HIGH)strcat (rxBuffer, "zap\n");
	else strcat (rxBuffer, "vyp\n");
	strcat (rxBuffer, "in2 ");
	if (inputs[1].state == HIGH)strcat (rxBuffer, "zap\n");
	else strcat (rxBuffer, "vyp\n");
	strcat (rxBuffer, "in3 ");
	if (inputs[2].state == HIGH)strcat (rxBuffer, "zap\n");
	else strcat (rxBuffer, "vyp\n");
	strcat (rxBuffer, "in4 ");
	if (inputs[3].state == HIGH)strcat (rxBuffer, "zap\n");
	else strcat (rxBuffer, "vyp\n");
	//teploty
	if (!numberOfFDallasDevices)teploty_new[0] = teploty_new[1] = 0;

	char tepl[11];
	sprintf (tepl, "t1 = %2u,%1u\n", teploty_new[0] / 10, teploty_new[0] % 10);
	strcat (rxBuffer, tepl);
	char tepl2[11];
	sprintf (tepl2, "t2 = %2u,%1u", teploty_new[1] / 10, teploty_new[1] % 10);
	strcat (rxBuffer, tepl2);
	strcat (rxBuffer, "\0");
	gsmModul.Sms (gsmData.telNumber, rxBuffer);
	//COMDEBUG.println (rxBuffer);
	casBlokovaniSms = 15;
}

void SaveEvent (int ev, char nmb_i_o)
{
	event.yy = dateTime.dateTimeStr.year - 2000;
	event.mnt = dateTime.dateTimeStr.mon;
	event.day = dateTime.dateTimeStr.mday;
	event.hr = dateTime.dateTimeStr.hour;
	event.min = dateTime.dateTimeStr.min;
	event.ss = dateTime.dateTimeStr.sec;
	event.evnt = ev | nmb_i_o;
	unsigned char ee_ptr_events;
	EEPROM.get (EE_EVENT_POINTER, ee_ptr_events);
	EEPROM.put (START_POINT + (ee_ptr_events * sizeof (Event)), event);
	if (++ee_ptr_events > 50)ee_ptr_events = 0;
	EEPROM.put (EE_EVENT_POINTER, ee_ptr_events);

}

void ClearEventList ()
{
	Event pomEvent;
	pomEvent.yy = 0xFF;
	pomEvent.mnt = 0xFF;
	pomEvent.day = 0xFF;
	pomEvent.hr = 0xFF;
	pomEvent.min = 0xFF;
	pomEvent.ss = 0xFF;
	pomEvent.evnt = 0xFFFF;

	for (int i = 0; i <= 50; i++)
	{
		EEPROM.put (START_POINT + i * sizeof (Event), pomEvent);
	}
	EEPROM.put (EE_EVENT_POINTER, 0x00);
}

void VypisPametUdalosti ()
{
	delay (100);
	COMCONTROL.print ("event");
	signed char ptr;	//ukazatel v bufferu 50ti udalosti
	EEPROM.get (EE_EVENT_POINTER, ptr);//nacteni hodnoty z eeprom
									   //COMDEBUG.print (ptr,10);
									   //ptr;//snizeni o 1
	char pom_ptr = ptr;//pomocna pro dalsi pouziti

					   //COMDEBUG.println (ptr,10);
	char pom_buf[17];
	do
	{
		int ptr2 = START_POINT + (ptr * sizeof (Event));
		//COMDEBUG.println (ptr2,10);
		EEPROM.get (ptr2, event);
		//      
		//sprintf (pom_buf, ">%02x %02x %02x %02x %02x %02x %04X", event.yy, event.mnt, event.day, event.hr, event.min, event.ss, event.evnt);
		//COMDEBUG.println (pom_buf);
		if (event.yy != 0xFF)
		{
			sprintf (pom_buf, ">%u %u %u %u %02u %02u %04X", event.yy, event.mnt, event.day, event.hr, event.min, event.ss, event.evnt);
			COMCONTROL.print (pom_buf);
		}

		ptr--;
	} while (ptr >= 0);
	//COMDEBUG.println ("dalsi");
	ptr = 50;
	int ptr2;
	do
	{
		ptr2 = START_POINT + (ptr * sizeof (Event));
		EEPROM.get (ptr2, event);
		//COMDEBUG.println (ptr2, 10);
		if (event.yy != 0xFF)
		{
			//char pom_buf[17];
			sprintf (pom_buf, ">%u %u %u %u %02u %02u %04X", event.yy, event.mnt, event.day, event.hr, event.min, event.ss, event.evnt);
			COMCONTROL.print (pom_buf);
		}
		ptr--;
	} while (ptr > pom_ptr);
	COMCONTROL.println (">");


}

void ChangeOutput (char out, char state, int ev)//cislo vystupu,cislo pinu,stav
{
	outputs[out].state = state;
	digitalWrite (outNmbs[out], state);//zapis stav na vystup
	SaveEvent (ev, out);
	sendOutsFlg = true;
	//COMDEBUG.println (ev, 16);
	//COMDEBUG.println (out, 10);
	//delay (100);
}

void DecodeData (char *str, char indx_i_o, char i_o)
{
	noInterrupts ();
	//pro cas
	char pom2[8];//pomocny string casy
				 //indexy dvojtecek    
	int idxs[15];
	int idxPtr = 0;

	//inputs
	idxPtr = 0;

	////outputs

	char idx2 = 0, idx3 = 0;
	idxPtr = 4;
	//COM.println ("decode");
	//flagy
	outputs[indx_i_o].IsTimeControl = str[idxPtr++] - 0x30;//4
	outputs[indx_i_o].IsInputControl = str[idxPtr++] - 0x30;//5
	outputs[indx_i_o].IsExtControl = str[idxPtr++] - 0x30;//6
	outputs[indx_i_o].IsUseSwitchClk = str[idxPtr++] - 0x30;//7
	outputs[indx_i_o].IsUseProgTmr = str[idxPtr++] - 0x30;//8
	//Thermostat
	outputs[indx_i_o].IsUseThermostat = str[idxPtr++] - 0x30;//9
	memcpy (&pom2[0], &str[idxPtr], 3);//10
	pom2[3] = 0;
	outputs[indx_i_o].Temperature = atoi (&pom2[0]);
	idxPtr += 3;
	//COMDEBUG.print (outputs[indx_i_o].Temperature, 10); COMDEBUG.print (",");

	memcpy (&pom2[0], &str[idxPtr], 3);//13
	pom2[3] = 0;
	outputs[indx_i_o].TempHysteresis = atoi (&pom2[0]);
	idxPtr += 3;
	//COMDEBUG.print (outputs[indx_i_o].TempHysteresis, 10); COMDEBUG.print (",");

	memcpy (&pom2[0], &str[idxPtr], 3);//16
	outputs[indx_i_o].TempAlarmHi = atoi (&pom2[0]);
	pom2[3] = 0;
	idxPtr += 3;

	memcpy (&pom2[0], &str[idxPtr], 3);//19
	outputs[indx_i_o].TempAlarmLo = atoi (&pom2[0]);
	pom2[3] = 0;
	idxPtr += 3;

	outputs[indx_i_o].IsAlarmHi = str[idxPtr++] - 0x30;//22

	outputs[indx_i_o].IsAlarmLo = str[idxPtr++] - 0x30;//23

	outputs[indx_i_o].ktere_cidlo = str[idxPtr++] - 0x30;//24

	//ProgTimer
	outputs[indx_i_o].IsTrvale = str[idxPtr++] - 0x30;//25
	outputs[indx_i_o].IsNastCas = str[idxPtr++] - 0x30;//26
	outputs[indx_i_o].IsSwitchOn = str[idxPtr++] - 0x30;//27
	outputs[indx_i_o].IsSwitchOff = str[idxPtr++] - 0x30;//28
	outputs[indx_i_o].IsAnyChange = str[idxPtr++] - 0x30;//29

	//controlTimes.timeOfDelay
	memcpy (&pom2[0], &str[idxPtr], 4);//30
	pom2[4] = 0;
	idxPtr += 4;

	outputs[indx_i_o].controlTimes.timeOfDelay = atoi (pom2);

	//controlTimes.timeOfPulse
	memcpy (&pom2[0], &str[idxPtr], 4);//34
	pom2[4] = 0;
	idxPtr += 4;
	outputs[indx_i_o].controlTimes.timeOfPulse = atoi (pom2);
	////minutespans 0
	////controlTimes.minutespans[0].startTime
	memcpy (&pom2[0], &str[idxPtr], 4);//38
	pom2[4] = 0;
	idxPtr += 4;
	outputs[indx_i_o].minutespans[0].startTime = atoi (pom2);

	//COMDEBUG.print (pom2); COMDEBUG.print (">>");
	////controlTimes.minutespans[0].stopTime
	memcpy (&pom2[0], &str[idxPtr], 4);//42
	pom2[4] = 0;
	idxPtr += 4;
	outputs[indx_i_o].minutespans[0].stopTime = atoi (pom2);

	//COMDEBUG.print (pom2); COMDEBUG.print (">>>>");
	////minutespans 1
	////controlTimes.minutespans[1].startTime
	memcpy (&pom2[0], &str[idxPtr], 4);//46
	pom2[4] = 0;
	idxPtr += 4;
	outputs[indx_i_o].minutespans[1].startTime = atoi (pom2);
	//COMDEBUG.print (pom2); COMDEBUG.print (">>");

	////controlTimes.minutespans[1].stopTime
	memcpy (&pom2[0], &str[idxPtr], 4);//48
	pom2[4] = 0;
	idxPtr += 4;
	outputs[indx_i_o].minutespans[1].stopTime = atoi (pom2);

	//COMDEBUG.print (pom2); COMDEBUG.print (">>>>");

	////minutespans 2
	////controlTimes.minutespans[2].startTime
	memcpy (&pom2[0], &str[idxPtr], 4);//52
	pom2[4] = 0;
	idxPtr += 4;
	outputs[indx_i_o].minutespans[2].startTime = atoi (pom2);

	//COMDEBUG.print (outputs[indx_i_o].minutespans[2].startTime,10); COMDEBUG.print (">>");

	////controlTimes.minutespans[2].stopTime
	memcpy (&pom2[0], &str[idxPtr], 4);//56
	pom2[4] = 0;
	idxPtr += 4;
	/*COMDEBUG.print (">>");
	COMDEBUG.print (pom2);
	COMDEBUG.print ("<<");*/
	outputs[indx_i_o].minutespans[2].stopTime = atoi (pom2);

	//COMDEBUG.print (pom2); COMDEBUG.print (">>>>");
	//minutespans 3
	//controlTimes.minutespans[3].startTime
	memcpy (&pom2[0], &str[idxPtr], 4);//60
	pom2[4] = 0;
	idxPtr += 4;
	outputs[indx_i_o].minutespans[3].startTime = atoi (pom2);

	//COMDEBUG.print (pom2); COMDEBUG.print (">>");

	//controlTimes.minutespans[3].stopTime
	memcpy (&pom2[0], &str[idxPtr], 4);//64
	pom2[4] = 0;
	idxPtr += 4;
	outputs[indx_i_o].minutespans[3].stopTime = atoi (pom2);
	//COMDEBUG.print (pom2); COMDEBUG.print (">>");
	interrupts ();
}



void RozdelString (char* src, char c)
{
	int i = 0, idxs[14], idx = 0;
	nmbOfSubstr = 0;
	while (*src)
	{
		if (*src == c)
		{
			divider[idx] = i;
			idx++;
			nmbOfSubstr++;
		}
		i++;
		src++;
	}
	if (rxBuffer[0] == 'T')
	{
		for (char j = 0; j < nmbOfSubstr - 1; j++)
		{
			memcpy (rozdelenyString[j], &rxBuffer[divider[j] + 1], divider[j + 1] - divider[j] - 1);
			rozdelenyString[j][divider[i + 1] - divider[i] - 1] = 0;
		}
		pomTs.mday = atoi (rozdelenyString[0]);
		pomTs.mon = atoi (rozdelenyString[1]);
		pomTs.year = atoi (rozdelenyString[2]);
		pomTs.hour = atoi (rozdelenyString[3]);
		pomTs.min = atoi (rozdelenyString[4]);
		pomTs.sec = atoi (rozdelenyString[5]);
	}
}

void TimerTick ()
{
	sendDateTimeFlg = true;


	if (casProzvaneni > 0)
	{
		casProzvaneni--;

		if (casProzvaneni <= 1)
		{
			casProzvaneni = 0;
			//COMDEBUG.println ("ath");

			gsmModul.HangOut ();
			inputs[currCallingInput].isCallingGsm = false;
			delay (100);
		}
		/*	else
		{*/
		//COMDEBUG.println (casProzvaneni, 10);
		//}
	}

	if (casZpozdeniSms)
	{
		//COMDEBUG.println (casZpozdeniSms, 10);
		if (--casZpozdeniSms == 0)SendLocalSms (currCallingInput);//
	}
	for (int i = 0; i < 4; i++)
	{
		if (inputs[i].isWaitingCall)
		{
			//COMDEBUG.println ("aaa");
			if (inputs[i].counter == -1)inputs[i].counter = 10;
			if (inputs[i].counter > 0)
			{
				inputs[i].counter--;
				//COMDEBUG.println (inputs[i].counter,10);
				if (inputs[i].counter == 0)
				{

					inputs[i].isWaitingCall = false;
					inputs[i].counter = -1;
					gsmModul.Call (inputs[i].tel);
					casProzvaneni = 30;
					inputs[i].isCallingGsm = true;
					currCallingInput = i;
				}
			}
		}
	}
	if (casBlokovaniSms)
	{
		//COM.println (casBlokovaniSms);
		if (--casBlokovaniSms == 0)inputs[currSendingSmsInput].isSendingSms = false;
	}
	for (int i = 0; i < 6; i++)
	{
		if (outTimers[i] > 0)
		{
			if (--outTimers[i] == 0)
			{
				outTimersFlg[i] = true;
				COMCONTROL.print (i, 10);
				COMCONTROL.println ("flg");
			}
			else
			{
				COMCONTROL.println (String (i) + "tmr-" + String (outTimers[i]) + '<');
			}

			delay (100);
		}
	}
}


void CtiTeploty ()
{
	static char cnt;
	//unsigned long time = millis ();
	sensors.requestTemperatures (); // Send the command to get temperatures

	for (int i = 0; i < 2; i++)//numberOfDevices
	{

		// Search the wire for address
		if (sensors.getAddress (tempDeviceAddress, i))
		{
			teploty_new[i] = (int)(sensors.getTempC (tempDeviceAddress) * 10); //sensors.getTempCByIndex(i);
																			   //COM.println (teploty_new[i]);
		}

		if (teploty_old[i] != teploty_new[i])
		{
			teploty_old[i] = teploty_new[i];
		}

	}
	//unsigned long cas = millis () - time;
	//COMDEBUG.println (cas, 10);
}

void VypisPamet ()
{
	char buf[22];
	for (int i = 0; i < 4; i++)
	{
		sprintf (buf, "f%u-%c-outs%c%c%c%c%c%c-nmb%c\n", i, inputs[i].func_index, inputs[i].outs[0], inputs[i].outs[1]
			, inputs[i].outs[2], inputs[i].outs[3], inputs[i].outs[4], inputs[i].outs[5], inputs[i].nmb);
		COMDEBUG.println (buf);
		//COM.print ("f");
		//COM.print (i);
		//COM.print ("-");
		//COM.print ();
		//COM.print ("-outs");
		//COM.print (inputs[i].outs[0]);
		//COM.print (inputs[i].outs[1]);
		//COM.print (inputs[i].outs[2]);
		//COM.print (inputs[i].outs[3]);
		//COM.print (inputs[i].outs[4]);
		//COM.print (inputs[i].outs[5]);
		//COM.print ("-nmb");
		//COM.println (inputs[i].nmb, 10);
		//sprintf (buf, "%s", inputs[i].tel);
		//COM.print ("tel-");
		//COM.println (buf);
		//sprintf (buf, "%s", inputs[i].sms);
		//COM.print ("sms-");
		//COM.println (buf);
	}

	for (int j = 0; j < 6; j++)
	{
		sprintf (buf, "tc%u ic%u ec%u sc%u pt%u", outputs[j].IsTimeControl, outputs[j].IsInputControl, outputs[j].IsExtControl, outputs[j].IsUseSwitchClk);
		//COM.print (" tc");
		//COM.print (outputs[j].IsTimeControl , 10);
		//COM.print (" ic");
		//COM.print (outputs[j].IsInputControl, 10);
		//COM.print (" ec");
		//COM.print (outputs[j].IsExtControl, 10);
		//COM.print (" sc");
		//COM.print (outputs[j].IsUseSwitchClk, 10);
		COMDEBUG.print (buf);
#define COM COMDEBUG
		COM.print (" pt");
		COM.print (outputs[j].IsUseProgTmr, 10);
		COM.print (" th");
		COM.print (outputs[j].IsUseThermostat, 10);
		COM.print (" T");
		COM.print (outputs[j].Temperature, 10);
		COM.print (" H");
		COM.print (outputs[j].TempHysteresis, 10);
		COM.print (" ah");
		COM.print (outputs[j].TempAlarmHi, 10);
		COM.print (" al");
		COM.print (outputs[j].TempAlarmLo, 10);
		COM.print (" ci");
		COM.print (outputs[j].ktere_cidlo, 10);
		COM.print ("-");
		COM.print (outputs[j].IsTrvale, 10);
		COM.print ("-");
		COM.print (outputs[j].IsNastCas, 10);
		COM.print ("-");
		COM.print (outputs[j].IsSwitchOn, 10);
		COM.print ("-");
		COM.print (outputs[j].IsSwitchOff, 10);
		COM.print ("-");
		COM.print (outputs[j].IsAnyChange, 10);
		COM.println ();
		COM.print (" msp");
		COM.print (outputs[j].minutespans[0].startTime, 10);
		COM.print ("-");
		COM.print (outputs[j].minutespans[0].stopTime, 10);
		COM.print ("_");
		COM.print (outputs[j].minutespans[1].startTime, 10);
		COM.print ("-");
		COM.print (outputs[j].minutespans[1].stopTime, 10);
		COM.print ("_");
		COM.print (outputs[j].minutespans[2].startTime, 10);
		COM.print ("-");
		COM.print (outputs[j].minutespans[2].stopTime, 10);
		COM.print ("_");
		COM.print (outputs[j].minutespans[3].startTime, 10);
		COM.print ("-");
		COM.print (outputs[j].minutespans[3].stopTime, 10);
		COM.print (" ctm");
		COM.print (outputs[j].controlTimes.timeOfDelay, 10);
		COM.print ("_");
		COM.print (outputs[j].controlTimes.timeOfPulse, 10);
		COM.print (" st");
		COM.print (outputs[j].state, 10);
		COM.print (" nmb");
		COM.print (outputs[j].nmb, 10);
		COM.println ();
#undef COM
	}
}

//COM.println ("gsm");
//COM.println (gsmData.isEnabled);
//COM.println (gsmData.telNumber[0]);
//COM.println (gsmData.telNumber[1]);
//COM.println (gsmData.telNumber[2]);

//}

void GsmReceive ()
{
	String gsm_string;
	signed int start, end;
	char telnmb[10];//pomocna pro tel cislo
	while (COMGSM.available ())
	{
		gsm_string = COMGSM.readString ();
	}

	COMDEBUG.println (gsm_string);
	start = gsm_string.indexOf ("SQ:", 5);
	if (start != -1)
	{
		//COMDEBUG.println (">>");
		//
		//start = gsm_string.indexOf ("SQ:", 5) + 4;
		start += 4;
		end = start + 2;
		gsmSignal = gsm_string.substring (start, end);
		//COMDEBUG.println (gsmSignal);
	}
	else
	{
		start = gsm_string.indexOf ("+420");
		if (start != -1)
		{

			memcpy (telnmb, &gsm_string[start + 4], 10);
			//COMDEBUG.println (telnmb);

			//telnmb.toCharArray (pom, 10);
		}
	}


	if (gsm_string.indexOf ("CARR") != -1 /*&& gsmData.isRinging*/)//no carrier
	{
		//COMDEBUG.println ("nocarr");
		gsmData.isRinging = false;
		gsmData.isActivated = false;
		//COMDEBUG.println (gsmData.isFound);
		//COMDEBUG.println (gsmData.isResponse);
		if (gsmData.isFound && gsmData.isResponse)
		{
			gsmData.isFound = false;
			delay (100);
			SendStatus ();
		}
	}
	else if (gsm_string.indexOf ("RING") != -1 && gsmData.isRinging == false) gsmData.isRinging = true;


	if (gsmData.isRinging && !gsmData.isActivated && strncmp (telnmb, gsmData.telNumber, 9) == 0)
	{
		//COMDEBUG.println ("ring");
		//COMDEBUG.println (gsmData.outNmb,2);
		gsmData.isFound = true;
		gsmData.isActivated = true;

		if (outputs[gsmData.outNmb].IsExtControl)
		{
			if (outputs[gsmData.outNmb].state == HIGH)outputs[gsmData.outNmb].state = LOW;
			else outputs[gsmData.outNmb].state = HIGH;
			ChangeOutput (gsmData.outNmb, outputs[gsmData.outNmb].state, (outputs[gsmData.outNmb].state == HIGH) ? ON_RNG : OFF_RNG);
		}
	}


	else if (strncmp (telnmb, gsmData.telNumber, 9) == 0)
	{
		gsmData.isFound = true;
		//COMDEBUG.println ("found");
		if (gsm_string.indexOf ("T:") != -1)
		{
			//COMDEBUG.println ("+CMT");
			start = gsm_string.length () - 5;
			String command = gsm_string.substring (start, start + 3);
			char pom2[4];//pomocna pro prikaz
			command.toCharArray (pom2, 4);
			//COMDEBUG.println (pom2);
			char nmbx = pom2[0];
			if (pom2[0] != '?')
			{
				nmbx -= 0x30;
				nmbx -= 1;
			}
			if (outputs[nmbx].IsExtControl)
			{
				ExecuteGsmCommad (pom2[0], pom2[2]);
				//if (gsmData.isResponse)SendStatus ();
			}
		}
	}

}

void ComControlReceive ()
{
	int tout = 0;
	char ii = 0;
	Timer1.stop ();
	char pom_buf[75];
	char pomocna = 0;//test
	while (1)
	{

		while (COMCONTROL.available ())
		{
			recChar = COMCONTROL.read ();
			rxBuffer[rxBufferIndex] = (char)recChar;
			if (recChar == '\n')
			{
				COMDEBUG.println ("recmsg");
				recMsg = true;
				break;
			}
			rxBufferIndex++;
		}
		if (rxBufferIndex > 99)break;
		if (recMsg)
		{
	/*		COMDEBUG.println (rxBufferIndex, 10);
			COMDEBUG.println (rxBuffer);*/
			boolean recError;
			pomocna++;
			recMsg = false;
			//COM.println (rxBufferIndex, 10);

			//checksum
			char checksum = 0;
			for (int i = 0; i < rxBufferIndex - 2; i++)
			{
				checksum += rxBuffer[i];
			}
			checksum &= 0xFF;
			checksum = 0 - checksum;

			//COMDEBUG.println (rxBuffer[rxBufferIndex - 1], 10);
			//COMDEBUG.println (checksum, 10);
			if (checksum != rxBuffer[rxBufferIndex - 1])
			{
				rxBufferIndex = 0; recError = true;
			}
			//end checksum
			if (recError) 
			{
				recError = false; COMCONTROL.println ("Err");
			}
			else
			{
				COMCONTROL.println ("Ok");
				memcpy (pom_buf, rxBuffer, rxBufferIndex - 1);
				pom_buf[rxBufferIndex-1] = '\0';

				//COMDEBUG.println (pom_buf);
				if (pom_buf[0] == 'T')
				{
					RozdelString (pom_buf, '#');
					dateTime.SetDateTime (pomTs);
					rxBufferIndex = 0;
					COMCONTROL.println ("recTok");
					break;
				}

				else if (pom_buf[0] == 'C')// vymaz pamet udalosti
				{
					rxBufferIndex = 0;
					//COMDEBUG.println ("xx");
					ClearEventList ();
					//VypisPametUdalosti ();
					break;
				}

				else if (pom_buf[0] == 'U')//pamet udalosti
				{
					//COMDEBUG.println ("pamet");
					VypisPametUdalosti ();
					rxBufferIndex = 0;
					break;
				}



				else if (pom_buf[0] == 'D')
				{
					//COM.println ("rec2");
					char c = pom_buf[3] - 0x30;
					if (pom_buf[2] == 'I')//data vstupu
					{

						memcpy (&inputs[c], &pom_buf[5], rxBufferIndex - 3);
						inputs[c].tel[9] = '\0';
						inputs[c].sms[20] = '\0';
						rxBufferIndex = 0;
					}
					else if (pom_buf[2] == 'O')//data vystupu
					{
						//memcpy (&outputs[c], &pom_buf[5], rxBufferIndex - 3);
						//RozdelString (pom_buf, ':');
						DecodeData (pom_buf, c, pom_buf[2]);
						//COM.println ("rec x");
					}

					else if (pom_buf[2] == 'G')//GSM data
					{
						//COMDEBUG.println ("ok");
						gsmData.isEnabled = pom_buf[3] - 0x30;
						gsmData.isResponse = pom_buf[4] - 0x30;
						//COMDEBUG.println (gsmData.isEnabled); COMDEBUG.println (gsmData.isResponse);
						memcpy (&gsmData.telNumber, &pom_buf[5], 10);
						//memcpy (&gsmData.telNumber[1], &pom_buf[15], 10);
						//memcpy (&gsmData.telNumber[2], &pom_buf[25], 10);
						gsmData.telNumber[9] = '\0';
						/*				gsmData.telNumber[1][9] = '\0';
						gsmData.telNumber[2][9] = '\0';*/
						gsmData.outNmb = pom_buf[15] - 0x30;
					}


					if (rxBuffer[rxBufferIndex - 3] == 'E')
					{


						rxBufferIndex = 0;
						delay (100);

						//COMDEBUG.println ("recDok");
						COMCONTROL.println ("recDok");
						VypisPamet ();
						eepromPtr = 0;
						EEPROM.put (eepromPtr, inputs);
						eepromPtr = sizeof (inputs);
						EEPROM.put (eepromPtr, outputs);
						eepromPtr = sizeof (outputs);
						EEPROM.put (eepromPtr, gsmData);
						break;
					}
					rxBufferIndex = 0;
				}


				else if (pom_buf[0] == 'M')//M = manual control
				{
					rxBufferIndex = 0;

					if (pom_buf[2] == 'O')
					{
						COMCONTROL.println ("O ctrl");
						char ox = pom_buf[4] - 0x30;
						char st = pom_buf[6] - 0x30;
						ChangeOutput (ox, st, (st == HIGH) ? ON_MAN : OFF_MAN);
					}
					break;
				}
			}

		}


	}
	//eepromPtr = 0;
	//EEPROM.put (eepromPtr, inputs);
	//eepromPtr = sizeof (inputs);
	//EEPROM.put (eepromPtr, outputs);
	Timer1.start ();
}

void SendDateTime ()
{
	static char cnt = 0;
	char pomstr[10];
	//COMDEBUG.println ("tick");
	if (++cnt > 30)
	{
		cnt = 0;
		gsmModul.Signal ();
		//sendDateTimeFlg = false;
	}
	//else
	//{
	sendDateTimeFlg = false;
	if (numberOfFDallasDevices > 0)CtiTeploty ();

	sprintf (pomstr, "%2u,%1u %2u,%1u", teploty_new[0] / 10, teploty_new[0] % 10, teploty_new[1] / 10, teploty_new[1] % 10);
	dateTime.GetDateTime ();
	if (dateTime.dateTimeStr.sec == 0)minutes = dateTime.GetMinutes ();
	COMCONTROL.println ("dt>" + dateTime.ToString () + '<' + pomstr + '<' + gsmSignal);
	delay (10);
	//}
}

void ObsluhaVystupu ()
{
	for (int j = 0; j < 6; j++)
	{
		//pro vystupy ovladane casem
		if (outTimersFlg[j] == true)
		{//pokud docasoval timer pro vystup proved akci
			outputs[j].state ^= 1;//preklop stav na vystupu
			outTimersFlg[j] = false;//vynuluj flag
			if (outputs[j].IsNastCas)//pokud ma po nastavenem case zmenit stav
			{
				if (whichTime[j] == first)//prvni cas
				{
					ChangeOutput (j, outputs[j].state, (outputs[j].state == HIGH) ? ON_SWATCH : OFF_SWATCH);//zapis na vystup
					whichTime[j] = second;
					outTimers[j] = outputs[j].controlTimes.timeOfPulse;
				}
				else if (whichTime[j] == second)//druhy cas
				{
					ChangeOutput (j, outputs[j].state, (outputs[j].state == HIGH) ? ON_SWATCH : OFF_SWATCH);//zapis na vystup
					whichTime[j] = first;
					outTimers[j] = outputs[j].controlTimes.timeOfDelay;
				}
			}
			else//zmen stav na trvalo
			{
				//swSerial.println ("tmrchng");
				delay (10);
				ChangeOutput (j, outputs[j].state, (outputs[j].state == HIGH) ? ON_SWATCH : OFF_SWATCH);
			}
		}
		//spinaci hodiny
		if (outputs[j].IsUseSwitchClk == true)
		{//
			for (int k = 0; k < 4; k++)
			{
				if (outputs[j].minutespans[k].startTime == minutes && !outputs[j].blockSendOn)
				{

					outputs[j].blockSendOn = true;
					outputs[j].blockSendOff = false;
					ChangeOutput (j, HIGH, ON_SWCLOCK);

				}

				else if (outputs[j].minutespans[k].stopTime == minutes && !outputs[j].blockSendOff)
				{
					outputs[j].blockSendOn = false;
					outputs[j].blockSendOff = true;
					ChangeOutput (j, LOW, OFF_SWCLOCK);
				}
			}

		}

		//teploty
		if (outputs[j].IsUseThermostat)
		{

			if (outputs[j].ktere_cidlo == 1)
			{

				if (teploty_new[0] >= outputs[j].Temperature)
				{

					if (!outputs[j].blockSendOn)
					{
						//COM.print (j, 10);
						outputs[j].blockSendOn = true;
						outputs[j].blockSendOff = false;
						ChangeOutput (j, LOW, OFF_TEMP1);
					}

				}
				else if (teploty_new[0] < outputs[j].Temperature - outputs[j].TempHysteresis && !outputs[j].blockSendOff)
				{
					ChangeOutput (j, HIGH, ON_TEMP1);
					outputs[j].blockSendOn = false;
					outputs[j].blockSendOff = true;
				}

			}

			else if (outputs[j].ktere_cidlo == 2)
			{
				if (teploty_new[1] >= outputs[j].Temperature && !outputs[j].blockSendOn)
				{
					ChangeOutput (j, LOW, OFF_TEMP2);
					outputs[j].blockSendOn = true;
					outputs[j].blockSendOff = false;
				}
				else if (teploty_new[1] < outputs[j].Temperature - outputs[j].TempHysteresis && !outputs[j].blockSendOff)
				{
					ChangeOutput (j, HIGH, ON_TEMP2);
					outputs[j].blockSendOn = false;
					outputs[j].blockSendOff = true;
				}

			}

		}
	}
}

void ObsluhaVstupu ()
{
	for (int i = 0; i < 4; i++)
	{

		if (digitalRead (inpNmbs[i]) == LOW)//je seply?
		{
			delay (10);//zakmit
			unsigned char state;
			if (inputs[i].blockSendOn == false)//posli hi stav vstupu pokud nebyl odeslan
			{
				inputs[i].blockSendOn = true;
				inputs[i].blockSendOff = false;
				inputs[i].state = 1;
				SendIns ();
				delay (10);//pockat kvuli odeslani stavu vystupu
				for (int j = 0; j < 6; j++)
				{
					switch (inputs[i].func_index - 0x30)
					{
					case 3:
					case 0: state = HIGH; break;
					case 1: state = LOW; break;
					case 2: state = outputs[j].state ^ 1; break;
					case 4: break;
					}
					//pro sepnuti/vypnuti vystupu
					if (inputs[i].func_index - 0x30 < 3)
					{
						if (inputs[i].outs[j] == '1')
						{
							if (outputs[j].state != state)
							{

								ChangeOutput (j, state, (state == HIGH) ? ON_I : OFF_I);//jen pri zmene
							}
							outputs[j].state = state;//uloz novy stav
						}
					}
					//pro spusteni casovani
					if (inputs[i].func_index - 0x30 == 3)
					{
						//kdyz je vystupu nastavene ovladani casem a zaroven je prirazeny ctenemu vstupu
						if (outputs[j].IsUseProgTmr&&inputs[i].outs[j] == '1')
						{
							ChangeOutput (j, state, ON_SWATCH);
							if (outputs[j].IsNastCas)whichTime[j] = first;
							outTimers[j] = outputs[j].controlTimes.timeOfDelay;
							outTimersFlg[j] = false;
						}
					}

				}
				//pro obsluhu GSM
				if (inputs[i].func_index - 0x30 >= 4)
				{
					//sms
					if (inputs[i].func_index - 0x30 == 4)
					{
						SendLocalSms (i);
						SaveEvent (SEND_SMS, i);
					}
					//prozvonit
					else if (inputs[i].func_index - 0x30 == 5)
					{
						if (!inputs[i].isCallingGsm)
						{
							//COM.println (inputs[i].tel);
							gsmModul.Call (inputs[i].tel);
							casProzvaneni = 35;
							inputs[i].isCallingGsm = true;
							currCallingInput = i;
							SaveEvent (RING, i);
							//COMDEBUG.println (RING, 16);
							//COMDEBUG.println (i, 10);
						}

					}
					//sms + prozvonit
					else if (inputs[i].func_index - 0x30 == 6)
					{
						inputs[i].isWaitingCall = true;
						//Timer1.stop ();
						SendLocalSms (i);
						SaveEvent (RING_SMS, i);

					}
				}
			}

			//zkontroluj co je nastavene pro vystup a proved operaci
		}
		//posli lo stav vstupu pokud nebyl odeslan
		else if (inputs[i].blockSendOff == false)
		{
			inputs[i].state = 0;
			SendIns ();
			inputs[i].blockSendOn = false;
			inputs[i].blockSendOff = true;
			delay (10);
		}
	}
}

void SendLocalSms (char nmb)
{
	if (!inputs[nmb].isSendingSms)
	{
		currSendingSmsInput = nmb;
		//inputs[nmb].isSendingSms = true;
		COMDEBUG.println (inputs[nmb].sms);
		gsmModul.Sms (inputs[nmb].tel, inputs[nmb].sms);
		casBlokovaniSms = 20;
	}
}



