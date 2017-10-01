

#include "Logo.h"

void setup ()
{
	board.I_O_init ();
	Serial.begin (19200);
	while (!Serial); // wait for serial port to connect. Needed for native USB port only
	//SPI.begin ();
	Serial.println ("serial ok");
	COM.begin (38400);
	delay (100);
	Timer1.initialize ();
	Timer1.attachInterrupt (TimerTick);
	COM.println ("COM ok");
	dateTime.Init ();
	//Serial.println (outputs[0].controlTimes.timeOfDelay);
	//Serial.println (outputs[1].controlTimes.timeOfDelay);
	//Serial.println (outputs[2].controlTimes.timeOfDelay);
	//Serial.println (outputs[3].controlTimes.timeOfDelay);
	//Serial.println (outputs[4].controlTimes.timeOfDelay);
	//Serial.println (outputs[5].controlTimes.timeOfDelay);
}

void loop ()
{
	if (sendDateTimeFlg)
	{
		sendDateTimeFlg = false;
		dateTime.GetDateTime ();
		if(dateTime.dateTimeStr.sec==0)minutes = dateTime.GetMinutes ();	
		COM.println (dateTime.ToString ());
	}

	//komunikace prijem zpravy
	while (COM.available ())
	{
		Timer1.stop ();
		rxBuffer = COM.readStringUntil ('\n');
		recMsg = true;
		Timer1.start ();
	}
	//pokud dosla zprava
	if (recMsg)
	{
		recMsg = false;
		COM.println ("recok");//ack
		delay (100);
		RozdelString (rxBuffer, '#');
		switch (rxBuffer[0])
		{
		case 'T': dateTime.SetDateTime (rozdelenyString); break;
		case 'D': DecodeData (); break;
		default:break;
		}
		rxBuffer.remove (0);//vynuluj buffer
	}
	//end komunikace
	/***************************/

	/***************************/
	//obsluha vstupu a vystupu
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
					ChangeOutput (j, outNmbs[j], outputs[j].state);//zapis na vystup
					whichTime[j] = second;
					outTimers[j] = outputs[j].controlTimes.timeOfPulse;
				}
				else if (whichTime[j] == second)//druhy cas
				{
					ChangeOutput (j, outNmbs[j], outputs[j].state);//zapis na vystup
					whichTime[j] = first;
					outTimers[j] = outputs[j].controlTimes.timeOfDelay;
				}
			}
			else ChangeOutput (j, outNmbs[j], outputs[j].state);//zmen stav na trvalo
		}

		if (outputs[j].IsUseSwitchClk == true)
		{//pokud docasoval timer pro vystup proved akci

			for (int k = 0; k < 5; k++)
			{
				if(outputs[j].minutespans[k].startTime==minutes)ChangeOutput (j, outNmbs[j], HIGH);
				else if (outputs[j].minutespans[k].stopTime == minutes)ChangeOutput (j, outNmbs[j], LOW);
			}

		}

	}
	/****************************************************************/
		//kontrola stavu na vstupech
	for (int i = 0; i < 4; i++)
	{
		if (digitalRead (inpNmbs[i]) == LOW)//je seply?
		{
			delay (100);//zakmit
			unsigned char state;
			if (!inputs[i].blockSendOn)//posli hi stav vstupu pokud nebyl odeslan
			{
				COM.print (i, 1); COM.println ("in1");
				inputs[i].blockSendOn = true;
				inputs[i].blockSendOff = false;
				for (int j = 0; j < 6; j++)
				{
					switch (inputs[i].func_index)
					{
					case 3:
					case 0: state = HIGH; break;
					case 1: state = LOW; break;
					case 2: state = outputs[j].state ^ 1; break;
					case 4: break;
					}

					//pro sepnuti/vypnuti vystupu
					if (inputs[i].func_index < 4)
					{
						if (inputs[i].outs[j] == '1')
						{
							if (outputs[j].state != state)
							{
								Serial.println (inputs[i].outs[j]);
								ChangeOutput (j, outNmbs[j], state);//jen pri zmene
							}
							outputs[j].state = state;//uloz novy stav

							COM.println (outNmbs[j]);
						}
					}
					//pro spusteni casovani
					if (inputs[i].func_index == 3)
					{
						//kdyz je vystupu nastavene ovladani casem a zaroven je prirazeny ctenemu vstupu
						if (outputs[j].IsUseProgTmr&&inputs[i].outs[j] == '1')
						{
							COM.print (j);
							COM.print ("_okx_");
							if (outputs[j].IsNastCas)whichTime[j] = first;
							outTimers[j] = outputs[j].controlTimes.timeOfDelay;
							COM.println (outTimers[j]);
							outTimersFlg[j] = false;
						}
					}



				}
				//delay (500)
			}
			//zkontroluj co je nastavene pro vystup a proved operaci
			;
		}
		//posli lo stav vstupu pokud nebyl odeslan
		else if (inputs[i].blockSendOff == false)
		{
			COM.print (i, 1); COM.println ("in0");
			inputs[i].blockSendOn = false;
			inputs[i].blockSendOff = true;
		}
	}
}

void ChangeOutput (char out, char nmb, char state)//cislo vystupu,cislo pinu,stav
{

	outputs[nmb].state = state;
	digitalWrite (nmb, state);//zapis stav na vystup
	COM.print (out, 10); COM.print ("out"); COM.println (outputs[nmb].state, 10);//posli zpravu
	delay (10);
}

void DecodeData ()
{
	char idxs[13]; char idx = 0;
	for (char i = 0; i < 4; i++)
	{
		idx = 0;
		Serial.print ("IN");// Serial.print (i);
		for (char j = 0; j < rozdelenyString[i].length (); j++)
		{
			if (rozdelenyString[i][j] == ':')
			{

				idxs[idx] = j;
				Serial.print ("-");
				Serial.print (idxs[idx], 1);
				idx++;
				nmbOfSubstr++;

			}
		}
		Serial.println ();
		inputs[i].func_index = rozdelenyString[i].substring (idxs[0] + 1, idxs[1]).toInt ();
		Serial.print (inputs[i].func_index, 1);
		Serial.print ("  ");
		String s_pom = rozdelenyString[i].substring (idxs[1] + 1, idxs[2]);
		for (char k = 0; k < 6; k++)
		{
			inputs[i].outs[k] = s_pom[k];
			Serial.print (inputs[i].outs[k]);
			Serial.print ("  ");
		}
		inputs[i].tel = rozdelenyString[i].substring (idxs[2] + 1, idxs[3]);
		Serial.print (inputs[i].tel);
		Serial.print ("  ");

		String ss = rozdelenyString[i].substring (idxs[3] + 1, idxs[4]);
		Serial.print (ss);
		ss.toCharArray (inputs[i].sms, ss.length ());
		Serial.print (ss.length ());
		Serial.print ("  ");
		Serial.println (inputs[i].sms);



	}
	//outputs
	char pomIndex;
	for (char i = 4; i < 10; i++)
	{
		pomIndex = i - 4;
		idx = 0;
		Serial.print ("OUT = "); //Serial.print (i-4);
		for (char j = 0; j < rozdelenyString[i].length (); j++)
		{
			if (rozdelenyString[i][j] == ':')
			{

				idxs[idx] = j;
				Serial.print ("-");
				Serial.print (idxs[idx], 1);
				idx++;
				nmbOfSubstr++;

			}

		}
		Serial.println ();
		outputs[pomIndex].IsTimeControl = rozdelenyString[i].substring (idxs[0] + 1, idxs[1]).toInt ();
		outputs[pomIndex].IsInputControl = rozdelenyString[i].substring (idxs[1] + 1, idxs[2]).toInt ();
		outputs[pomIndex].IsExtControl = rozdelenyString[i].substring (idxs[2] + 1, idxs[3]).toInt ();
		outputs[pomIndex].IsUseSwitchClk = rozdelenyString[i].substring (idxs[3] + 1, idxs[4]).toInt ();
		outputs[pomIndex].IsUseProgTmr = rozdelenyString[i].substring (idxs[4] + 1, idxs[5]).toInt ();
		//ProgTimer
		outputs[pomIndex].IsTrvale = rozdelenyString[i].substring (idxs[5] + 1, idxs[6]).toInt ();
		outputs[pomIndex].IsNastCas = rozdelenyString[i].substring (idxs[6] + 1, idxs[7]).toInt ();
		outputs[pomIndex].IsSwitchOn = rozdelenyString[i].substring (idxs[7] + 1, idxs[8]).toInt ();
		outputs[pomIndex].IsSwitchOff = rozdelenyString[i].substring (idxs[8] + 1, idxs[9]).toInt ();
		outputs[pomIndex].IsAnyChange = rozdelenyString[i].substring (idxs[9] + 1, idxs[10]).toInt ();

		//Serial.print (outputs[i].IsTimeControl, 1);
		//Serial.print ("  ");
		//Serial.print (outputs[i].IsInputControl, 1);
		//Serial.print ("  ");
		//Serial.print (outputs[i].IsExtControl, 1);
		//Serial.print ("  ");
		//Serial.print (outputs[i].IsUseSwitchClk, 1);
		//Serial.print ("  ");
		//Serial.print (outputs[i].IsUseProgTmr, 1);
		//Serial.print (")(");
		//Serial.print (outputs[i].IsTrvale, 1);
		//Serial.print ("  ");
		//Serial.print (outputs[i].IsNastCas, 1);
		//Serial.print ("  ");
		//Serial.print (outputs[i].IsSwitchOn, 1);
		//Serial.print ("  ");
		//Serial.print (outputs[i].IsSwitchOff, 1);
		//Serial.print ("  ");
		//Serial.print (outputs[i].IsAnyChange, 1);
		//Serial.println ();
		String s1 = rozdelenyString[i].substring (idxs[10] + 1, idxs[11]);
		//Serial.print ("//");
		Serial.println (s1);
		String strgs[5];
		char idxs2[6];
		char idx2 = 0, idx3 = 0;
		char pomlcky[5];
		for (char k = 0; k < s1.length (); k++)
		{
			if (s1[k] == '%')idxs2[idx2++] = k;
			else if (s1[k] == '-')pomlcky[idx3++] = k;
		}



		outputs[pomIndex].controlTimes.timeOfDelay = s1.substring (idxs2[0] + 1, pomlcky[0]).toInt ();
		outputs[pomIndex].controlTimes.timeOfPulse = s1.substring (pomlcky[0] + 1, idxs2[1]).toInt ();
		Serial.print ("--");
		Serial.println (outputs[pomIndex].controlTimes.timeOfDelay);
		s1 = rozdelenyString[i].substring (idxs[11] + 1, idxs[12]);
		idx2 = idx3 = 0;
		for (char k = 0; k < s1.length (); k++)
		{
			if (s1[k] == '%')idxs2[idx2++] = k;
			else if (s1[k] == '-')pomlcky[idx3++] = k;
		}

		for (char k = 0; k < idx3; k++)
		{
			outputs[pomIndex].minutespans[k].startTime = s1.substring (idxs2[k] + 1, pomlcky[k]).toInt ();
			outputs[pomIndex].minutespans[k].stopTime = s1.substring (pomlcky[k] + 1, idxs2[k + 1]).toInt ();
			Serial.print (outputs[pomIndex].minutespans[k].startTime);
			Serial.print (":");
			Serial.println (outputs[pomIndex].minutespans[k].stopTime);
		}
		//String s_pom = rozdelenyString[i].substring (idxs[1] + 1, idxs[2]);
		//for (char k = 0; k < 6; k++)
		//{
		//	inputs[i].outs[k] = s_pom[k] - 0x30;
		//	Serial.print (inputs[i].outs[k]);
		//	Serial.print ("  ");
		//}
		//inputs[i].tel = rozdelenyString[i].substring (idxs[2] + 1, idxs[3]);
		//Serial.print (inputs[i].tel);
		//Serial.print ("  ");

		//String ss = rozdelenyString[i].substring (idxs[3] + 1, idxs[4]);
		//Serial.print (ss);
		//ss.toCharArray (inputs[i].sms, ss.length ());
		//Serial.print (ss.length ());
		//Serial.print ("  ");
		//Serial.println (inputs[i].sms);



	}

}

void RozdelString (String s, char c)
{
	int idxs[12], idx = 0;
	nmbOfSubstr = 0;
	//Serial.println (s.length ());
	for (int i = 0; i < s.length (); i++)
	{
		if (s[i] == c)
		{

			idxs[idx] = i;
			Serial.print ("-");//debug
			Serial.println (idxs[idx], 1);//debug
			idx++;
			nmbOfSubstr++;

		}

	}
	Serial.println (nmbOfSubstr, 1);//debug
	for (int i = 0; i < nmbOfSubstr - 1; i++)
	{
		rozdelenyString[i] = s.substring (idxs[i] + 1, idxs[i + 1]);
		Serial.println (rozdelenyString[i]);//debug
	}

}

void TimerTick ()
{
	sendDateTimeFlg = true;
	for (int i = 0; i < 6; i++)
	{
		if (outTimers[i] > 0)
		{
			Serial.print (i);
			Serial.print ("-");
			Serial.println (outTimers[i]);
			if (--outTimers[i] == 0)
			{
				outTimersFlg[i] = true;
				Serial.print (i);
				Serial.println ("flg");
			}
		}
	}
}

void SetDateTime ()
{
	//dateTime.
}

void GetDateTime ()
{

}
