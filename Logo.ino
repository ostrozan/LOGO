


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

	COMCONTROL.begin (38400);
	while (!COMCONTROL);
	delay (1000);
	COMDEBUG.begin (38400);
	delay (1000);
	while (!COMDEBUG);
	COMGSM.begin (9600);
	delay (1000);
	while (!COMGSM);
	eepromPtr = 0;
	EEPROM.get (eepromPtr, inputs);
	eepromPtr = sizeof (inputs);
	EEPROM.get (eepromPtr, outputs);
	//dallas
	// Start up the library
	sensors.begin ();

	// Grab a count of devices on the wire
	numberOfDevices = sensors.getDeviceCount ();
	for (int i = 0; i < numberOfDevices; i++)
	{
		// Search the wire for address
		sensors.getAddress (tempDeviceAddress, i);
		sensors.getAddress (addr[i], i);
		// set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
		sensors.setResolution (tempDeviceAddress, TEMPERATURE_PRECISION);
		//addr[0] = tempDeviceAddress;
	}
	for (int i = 0; i < 4; i++)
	{
		inputs[i].blockSendOn = false;
		inputs[i].state = 0;
		inputs[i].isCallingGsm = false;
		inputs[i].isSendingSms = false;
		inputs[i].isWaitingSms = false;
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
	casProzvaneni = 0;

	//VypisPamet ();
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
	if (sendOutsFlg)SendOuts ();

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
}

void ExecuteGsmCommad (char nmbOut, char outState)
{
	switch (nmbOut)
	{
	case '1': ChangeOutput (0, (outState == 'n') ? HIGH : LOW); break;
	case '2': ChangeOutput (1, (outState == 'n') ? HIGH : LOW); break;
	case '3': ChangeOutput (2, (outState == 'n') ? HIGH : LOW); break;
	case '4': ChangeOutput (3, (outState == 'n') ? HIGH : LOW); break;
	case '5': ChangeOutput (4, (outState == 'n') ? HIGH : LOW); break;
	case '6': ChangeOutput (5, (outState == 'n') ? HIGH : LOW); break;
	case '?': SendStatus (); break;
	}
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
	char tepl[11];
	sprintf (tepl, "t1 = %2u,%1u\n", teploty_new[0] / 10, teploty_new[0] % 10);
	strcat (rxBuffer, tepl);
	char tepl2[11];
	sprintf (tepl2, "t2 = %2u,%1u\0", teploty_new[1] / 10, teploty_new[1] % 10);
	strcat (rxBuffer, tepl2);
	gsmModul.Sms (gsmData.telNumber, rxBuffer);
	//COM.println (rxBuffer);
	casBlokovaniSms = 15;
}

void ChangeOutput (char out, char state)//cislo vystupu,cislo pinu,stav
{
	outputs[out].state = state;
	digitalWrite (outNmbs[out], state);//zapis stav na vystup
	sendOutsFlg = true;
	//delay (100);
}

void DecodeData (char *str, char indx_i_o, char i_o)
{
	//pro cas
	char pom2[8];//pomocny string casy
				 //indexy dvojtecek    
	int idxs[15];
	int idxPtr = 0;

	//inputs
	idxPtr = 0;

	////outputs

	char idx2 = 0, idx3 = 0;
	idxPtr = 0;
	//COM.println ("decode");
	//flagy
	outputs[indx_i_o].IsTimeControl = str[divider[0] + 1] - 0x30;
	outputs[indx_i_o].IsInputControl = str[divider[1] + 1] - 0x30;
	outputs[indx_i_o].IsExtControl = str[divider[2] + 1] - 0x30;
	outputs[indx_i_o].IsUseSwitchClk = str[divider[3] + 1] - 0x30;
	outputs[indx_i_o].IsUseProgTmr = str[divider[4] + 1] - 0x30;
	//Thermostat
	outputs[indx_i_o].IsUseThermostat = str[divider[5] + 1] - 0x30;
	memcpy (&pom2[0], &str[divider[6] + 1], divider[7] - divider[6]);
	outputs[indx_i_o].Temperature = atoi (&pom2[0]);
	//COM.print (outputs[indx_i_o].Temperature, 10); COM.print (",");

	memcpy (&pom2[0], &str[divider[7] + 1], divider[8] - divider[7]);
	outputs[indx_i_o].TempHysteresis = atoi (&pom2[0]);
	//COM.print (outputs[indx_i_o].TempHysteresis, 10); COM.print (",");

	memcpy (&pom2[0], &str[divider[8] + 1], divider[9] - divider[8]);
	outputs[indx_i_o].TempAlarmHi = atoi (&pom2[0]);

	memcpy (&pom2[0], &str[divider[9] + 1], divider[10] - divider[9]);
	outputs[indx_i_o].TempAlarmLo = atoi (&pom2[0]);

	outputs[indx_i_o].IsAlarmHi = str[divider[10] + 1] - 0x30;

	outputs[indx_i_o].IsAlarmLo = str[divider[11] + 1] - 0x30;

	outputs[indx_i_o].ktere_cidlo = str[divider[12] + 1] - 0x30;

	//ProgTimer
	outputs[indx_i_o].IsTrvale = str[divider[13] + 1] - 0x30;

	outputs[indx_i_o].IsNastCas = str[divider[14] + 1] - 0x30;

	outputs[indx_i_o].IsSwitchOn = str[divider[15] + 1] - 0x30;
	outputs[indx_i_o].IsSwitchOff = str[divider[16] + 1] - 0x30;
	outputs[indx_i_o].IsAnyChange = str[divider[17] + 1] - 0x30;

	//controlTimes.timeOfDelay
	memcpy (pom2, &str[divider[18] + 1], divider[19] - divider[18] - 1);
	pom2[divider[19] - divider[18] - 1] = 0;

	outputs[indx_i_o].controlTimes.timeOfDelay = atol (pom2);

	//controlTimes.timeOfPulse
	memcpy (pom2, &str[divider[19] + 1], divider[20] - divider[19] - 1);
	pom2[divider[20] - divider[19] - 1] = 0;

	outputs[indx_i_o].controlTimes.timeOfPulse = atol (pom2);
	//minutespans 0
	//controlTimes.minutespans[0].startTime
	memcpy (pom2, &str[divider[20] + 1], divider[21] - divider[20] - 1);
	pom2[divider[21] - divider[20] - 1] = 0;
	outputs[indx_i_o].minutespans[0].startTime = atol (pom2);

	//controlTimes.minutespans[0].stopTime
	memcpy (pom2, &str[divider[21] + 1], divider[22] - divider[21] - 1);
	pom2[divider[22] - divider[21] - 1] = 0;

	outputs[indx_i_o].minutespans[0].stopTime = atol (pom2);

	//minutespans 1
	//controlTimes.minutespans[1].startTime
	memcpy (pom2, &str[divider[22] + 1], divider[23] - divider[22] - 1);
	pom2[divider[23] - divider[22] - 1] = 0;
	outputs[indx_i_o].minutespans[1].startTime = atol (pom2);

	//controlTimes.minutespans[1].stopTime
	memcpy (pom2, &str[divider[23] + 1], divider[24] - divider[23] - 1);
	pom2[divider[24] - divider[23] - 1] = 0;
	outputs[indx_i_o].minutespans[1].stopTime = atol (pom2);


	//minutespans 2
	//controlTimes.minutespans[2].startTime
	memcpy (pom2, &str[divider[24] + 1], divider[25] - divider[24] - 1);
	pom2[divider[25] - divider[24] - 1] = 0;
	outputs[indx_i_o].minutespans[2].startTime = atol (pom2);

	//controlTimes.minutespans[2].stopTime
	memcpy (pom2, &str[divider[25] + 1], divider[26] - divider[25] - 1);
	pom2[divider[26] - divider[25] - 1] = 0;
	outputs[indx_i_o].minutespans[2].stopTime = atol (pom2);

	//minutespans 3
	//controlTimes.minutespans[3].startTime
	memcpy (pom2, &str[divider[26] + 1], divider[27] - divider[26] - 1);
	pom2[divider[27] - divider[26] - 1] = 0;
	outputs[indx_i_o].minutespans[3].startTime = atol (pom2);

	//controlTimes.minutespans[3].stopTime
	memcpy (pom2, &str[divider[27] + 1], divider[28] - divider[27] - 1);
	pom2[divider[28] - divider[27] - 1] = 0;
	outputs[indx_i_o].minutespans[3].stopTime = atol (pom2);

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
			COMDEBUG.println ("ath");

			gsmModul.HangOut ();
			inputs[currCallingInput].isCallingGsm = false;
			delay (100);

			//if (inputs[currCallingInput].isWaitingSms)casZpozdeniSms = 5;

		}
		/*	else
			{*/
		COMDEBUG.println (casProzvaneni, 10);
		//}
	}

	if (casZpozdeniSms)
	{
		COMDEBUG.println (casZpozdeniSms, 10);
		inputs[currCallingInput].isWaitingSms = false;
		if (--casZpozdeniSms == 0)SendLocalSms (currCallingInput);//
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
				COMCONTROL.println (i + "flg");
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
}

//void VypisPamet ()
//{
//	char buf[22];
//	for (int i = 0; i < 4; i++)
//	{
//		sprintf (buf, "f%u-%c-outs%c%c%c%c%c%c-nmb%c\n", i, inputs[i].func_index, inputs[i].outs[0], inputs[i].outs[1]
//			, inputs[i].outs[2], inputs[i].outs[3], inputs[i].outs[4], inputs[i].outs[5], inputs[i].nmb);
//		COMDEBUG.println (buf);
//		//COM.print ("f");
//		//COM.print (i);
//		//COM.print ("-");
//		//COM.print ();
//		//COM.print ("-outs");
//		//COM.print (inputs[i].outs[0]);
//		//COM.print (inputs[i].outs[1]);
//		//COM.print (inputs[i].outs[2]);
//		//COM.print (inputs[i].outs[3]);
//		//COM.print (inputs[i].outs[4]);
//		//COM.print (inputs[i].outs[5]);
//		//COM.print ("-nmb");
//		//COM.println (inputs[i].nmb, 10);
//		//sprintf (buf, "%s", inputs[i].tel);
//		//COM.print ("tel-");
//		//COM.println (buf);
//		//sprintf (buf, "%s", inputs[i].sms);
//		//COM.print ("sms-");
//		//COM.println (buf);
//	}
//
//	for (int j = 0; j < 6; j++)
//	{
//		sprintf (buf, "tc%u ic%u ec%u sc%u pt%u", outputs[j].IsTimeControl, outputs[j].IsInputControl, outputs[j].IsExtControl, outputs[j].IsUseSwitchClk);
//		//COM.print (" tc");
//		//COM.print (outputs[j].IsTimeControl , 10);
//		//COM.print (" ic");
//		//COM.print (outputs[j].IsInputControl, 10);
//		//COM.print (" ec");
//		//COM.print (outputs[j].IsExtControl, 10);
//		//COM.print (" sc");
//		//COM.print (outputs[j].IsUseSwitchClk, 10);
//		COMDEBUG.print (buf);
//#define COM COMDEBUG
//		COM.print (" pt");
//		COM.print (outputs[j].IsUseProgTmr, 10);
//		COM.print (" th");
//		COM.print (outputs[j].IsUseThermostat, 10);
//		COM.print (" T");
//		COM.print (outputs[j].Temperature, 10);
//		COM.print (" H");
//		COM.print (outputs[j].TempHysteresis, 10);
//		COM.print (" ah");
//		COM.print (outputs[j].TempAlarmHi, 10);
//		COM.print (" al");
//		COM.print (outputs[j].TempAlarmLo, 10);
//		COM.print (" ci");
//		COM.print (outputs[j].ktere_cidlo, 10);
//		COM.print ("-");
//		COM.print (outputs[j].IsTrvale, 10);
//		COM.print ("-");
//		COM.print (outputs[j].IsNastCas, 10);
//		COM.print ("-");
//		COM.print (outputs[j].IsSwitchOn, 10);
//		COM.print ("-");
//		COM.print (outputs[j].IsSwitchOff, 10);
//		COM.print ("-");
//		COM.print (outputs[j].IsAnyChange, 10);
//		COM.println ();
//		COM.print (" msp");
//		COM.print (outputs[j].minutespans[0].startTime, 10);
//		COM.print ("-");
//		COM.print (outputs[j].minutespans[0].stopTime, 10);
//		COM.print ("_");
//		COM.print (outputs[j].minutespans[1].startTime, 10);
//		COM.print ("-");
//		COM.print (outputs[j].minutespans[1].stopTime, 10);
//		COM.print ("_");
//		COM.print (outputs[j].minutespans[2].startTime, 10);
//		COM.print ("-");
//		COM.print (outputs[j].minutespans[2].stopTime, 10);
//		COM.print ("_");
//		COM.print (outputs[j].minutespans[3].startTime, 10);
//		COM.print ("-");
//		COM.print (outputs[j].minutespans[3].stopTime, 10);
//		COM.print (" ctm");
//		COM.print (outputs[j].controlTimes.timeOfDelay, 10);
//		COM.print ("_");
//		COM.print (outputs[j].controlTimes.timeOfPulse, 10);
//		COM.print (" st");
//		COM.print (outputs[j].state, 10);
//		COM.print (" nmb");
//		COM.print (outputs[j].nmb, 10);
//		COM.println ();
//#undef COM
//	}

	//COM.println ("gsm");
	//COM.println (gsmData.isEnabled);
	//COM.println (gsmData.telNumber[0]);
	//COM.println (gsmData.telNumber[1]);
	//COM.println (gsmData.telNumber[2]);

//}

void GsmReceive ()
{
	String gsm_string;
	while (COMGSM.available ())
	{
		gsm_string = COMGSM.readString ();
	}

	COMDEBUG.println (gsm_string);
	boolean isFound = false;
	boolean isMaster = false;
	int start = gsm_string.indexOf ("+420");
	String telnmb = gsm_string.substring (start + 4, start + 13);
	//COMDEBUG.println (telnmb);
	char pom[10];//pomocna pro tel cislo
	telnmb.toCharArray (pom, 10);
	//COMDEBUG.println (pom);
	//COMDEBUG.println (gsmData.telNumber[0]);
	//COMDEBUG.println (gsmData.telNumber[1]);

	if (gsm_string.indexOf ("CARR") != -1 /*&& gsmData.isRinging*/)//no carrier
	{
		COMDEBUG.println ("nocarr");
		gsmData.isRinging = false;
		gsmData.isActivated = false;
		if (isFound && gsmData.isResponse)
		{
			delay (100);
			SendStatus ();
		}
	}
	else if (gsm_string.indexOf ("RING") != -1 && gsmData.isRinging == false) gsmData.isRinging = true;


   if (gsmData.isRinging && !gsmData.isActivated && strncmp (pom, gsmData.telNumber, 9) == 0)
	{
		COMDEBUG.println ("ring");
		//COMDEBUG.println (gsmData.outNmb,2);
		gsmData.isActivated = true;

		if (outputs[gsmData.outNmb].IsExtControl)
		{
			if (outputs[gsmData.outNmb].state == HIGH)outputs[gsmData.outNmb].state = LOW;
			else outputs[gsmData.outNmb].state = HIGH;
			ChangeOutput (gsmData.outNmb, outputs[gsmData.outNmb].state);
		}
	}


	else if (strncmp (pom, gsmData.telNumber, 9) == 0)
	{
		isFound = true;
		//COMDEBUG.println ("found");
		if (gsm_string.indexOf ("T:") != -1)
		{
			COMDEBUG.println ("+CMT");
			start = gsm_string.length () - 5;
			String command = gsm_string.substring (start, start + 3);
			char pom2[4];//pomocna pro prikaz
			command.toCharArray (pom2, 4);
			COMDEBUG.println (pom2);
			char nmbx = pom2[0] - 0x30;
			nmbx -= 1;
			if (outputs[nmbx].IsExtControl)
			{
				ExecuteGsmCommad (pom2[0], pom2[2]);
				if (gsmData.isResponse)SendStatus ();
			}
		}




	}

}

void ComControlReceive ()
{
	int tout = 0;
	char ii = 0;
	Timer1.stop ();
	char pom_buf[60];
	char pomocna = 0;//test
	while (1)
	{

		while (COMCONTROL.available ())
		{
			recChar = COMCONTROL.read ();
			rxBuffer[rxBufferIndex++] = (char)recChar;
			if (recChar == '\n')
			{
				recMsg = true;
				break;
			}
		}
		if (rxBufferIndex > 99)break;
		if (recMsg)
		{
			boolean recError;
			pomocna++;
			recMsg = false;
			//COM.println (rxBufferIndex, 10);
			memcpy (pom_buf, rxBuffer, rxBufferIndex - 1);
			pom_buf[rxBufferIndex - 1] = 0;

			COMDEBUG.println (pom_buf);
			if (pom_buf[0] == 'T')
			{
				RozdelString (pom_buf, '#');
				dateTime.SetDateTime (pomTs);
				rxBufferIndex = 0;
				COMCONTROL.println ("recTok");
				break;
			}

			else if (pom_buf[0] == 'D')
			{
				//COM.println ("rec2");
				char c = pom_buf[3] - 0x30;
				if (pom_buf[2] == 'I')//data vstupu
				{
					if (rxBufferIndex == 48)
					{
						memcpy (&inputs[c], &pom_buf[5], rxBufferIndex - 4);
						inputs[c].tel[9] = '\0';
						inputs[c].sms[20] = '\0';
						recError = false;
					}
					else recError = true;

				}
				else if (pom_buf[2] == 'O')//data vystupu
				{
					RozdelString (pom_buf, ':');
					DecodeData (pom_buf, c, pom_buf[2]);
					//COM.println ("rec x");
				}

				else if (pom_buf[2] == 'G')//GSM data
				{
					//COMDEBUG.println ("ok");
					gsmData.isEnabled = pom_buf[3] - 0x30;
					gsmData.isResponse = pom_buf[4] - 0x30;
					COMDEBUG.println (gsmData.isEnabled); COMDEBUG.println (gsmData.isResponse);
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
					if (recError) { COMCONTROL.println ("Err"); COMDEBUG.println ("Err"); }
					else
					{
						//COMDEBUG.println ("recDok");
						COMCONTROL.println ("recDok");
						//VypisPamet ();
						eepromPtr = 0;
						EEPROM.put (eepromPtr, inputs);
						eepromPtr = sizeof (inputs);
						EEPROM.put (eepromPtr, outputs);
					}
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
					ChangeOutput (ox, st);
				}
				break;
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
	static char counter = 0;
	char pomstr[10];
	if (++counter > 20)
	{
		counter = 0;
		gsmModul.Init ();

		////COM.println (gsmModul.Operator());
		//gsmSignal = gsmModul.Signal ();
		sendDateTimeFlg = false;
		delay (10);
	}
	else
	{
		sendDateTimeFlg = false;
		CtiTeploty ();
		sprintf (pomstr, "%2u,%1u %2u,%1u", teploty_new[0] / 10, teploty_new[0] % 10, teploty_new[1] / 10, teploty_new[1] % 10);
		dateTime.GetDateTime ();
		if (dateTime.dateTimeStr.sec == 0)minutes = dateTime.GetMinutes ();
		COMCONTROL.println ("dt>" + dateTime.ToString () + '<' + pomstr + '<' + gsmSignal);
		delay (10);
	}
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
					ChangeOutput (j, outputs[j].state);//zapis na vystup
					whichTime[j] = second;
					outTimers[j] = outputs[j].controlTimes.timeOfPulse;
				}
				else if (whichTime[j] == second)//druhy cas
				{
					ChangeOutput (j, outputs[j].state);//zapis na vystup
					whichTime[j] = first;
					outTimers[j] = outputs[j].controlTimes.timeOfDelay;
				}
			}
			else//zmen stav na trvalo
			{
				//swSerial.println ("tmrchng");
				delay (10);
				ChangeOutput (j, outputs[j].state);
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
					ChangeOutput (j, HIGH);

				}

				else if (outputs[j].minutespans[k].stopTime == minutes && !outputs[j].blockSendOff)
				{
					outputs[j].blockSendOn = false;
					outputs[j].blockSendOff = true;
					ChangeOutput (j, LOW);
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
						ChangeOutput (j, LOW);
					}

				}
				else if (teploty_new[0] < outputs[j].Temperature - outputs[j].TempHysteresis && !outputs[j].blockSendOff)
				{
					ChangeOutput (j, HIGH);
					outputs[j].blockSendOn = false;
					outputs[j].blockSendOff = true;
				}

			}

			else if (outputs[j].ktere_cidlo == 2)
			{
				if (teploty_new[1] >= outputs[j].Temperature && !outputs[j].blockSendOn)
				{
					ChangeOutput (j, LOW);
					outputs[j].blockSendOn = true;
					outputs[j].blockSendOff = false;
				}
				else if (teploty_new[1] < outputs[j].Temperature - outputs[j].TempHysteresis && !outputs[j].blockSendOff)
				{
					ChangeOutput (j, HIGH);
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
					COMDEBUG.println (outputs[i].IsInputControl);
					//if (outputs[i].IsInputControl==false)continue;
					switch (inputs[i].func_index - 0x30)
					{
					case 3:
					case 0: state = HIGH; break;
					case 1: state = LOW; break;
					case 2: state = outputs[j].state ^ 1; break;
					case 4: break;
					}
					//pro sepnuti/vypnuti vystupu
					if (inputs[i].func_index - 0x30 < 4)
					{
						if (inputs[i].outs[j] == '1')
						{
							if (outputs[j].state != state)
							{
								//if (outputs[j].IsInputControl)
									ChangeOutput (j, state);//jen pri zmene
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
							/*							COM.print (j);
							COM.print ("_okx_")*/;
						if (outputs[j].IsNastCas)whichTime[j] = first;
						outTimers[j] = outputs[j].controlTimes.timeOfDelay;
						//COM.println (outTimers[j]);
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
					}
					//prozvonit
					else if (inputs[i].func_index - 0x30 == 5)
					{
						if (!inputs[i].isCallingGsm)
						{
							//COM.println (inputs[i].tel);
							gsmModul.Call (inputs[i].tel);
							casProzvaneni = 30;
							inputs[i].isCallingGsm = true;
							currCallingInput = i;
						}

					}
					//sms + prozvonit
					else if (inputs[i].func_index - 0x30 == 6)
					{
						//inputs[i].isWaitingSms = true;
						Timer1.stop ();
						SendLocalSms (i);
						delay (2000);
						if (!inputs[i].isCallingGsm)
						{
							COMDEBUG.println ("ring");
							gsmModul.Call (inputs[i].tel);
							casProzvaneni = 25;
							inputs[i].isCallingGsm = true;
							currCallingInput = i;
						}
						Timer1.start ();
					}
				}
				//delay (500)
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
		//delay (500);
	}
}

void SendLocalSms (char nmb)
{
	if (!inputs[nmb].isSendingSms)
	{

		currSendingSmsInput = nmb;
		inputs[nmb].isSendingSms = true;
		//COMDEBUG.println ("sms");
		//COMDEBUG.println (inputs[nmb].tel);
		//COMDEBUG.println (inputs[nmb].sms);
		gsmModul.Sms (inputs[nmb].tel, inputs[nmb].sms);
		casBlokovaniSms = 20;
	}
}



