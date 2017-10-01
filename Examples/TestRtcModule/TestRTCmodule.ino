//#include "GPIO.h"



#include <LogoBoard\GsmModule.h>
#include <LogoBoard\LOGOGPIO.h>
#include <LogoBoard\DateTime.h>
#include <LogoBoard\LogoBoard.h>

LogoBoard board;
DateTime dateTime;
GsmModule gsm;

String buffer;
uint8_t c;
void setup ()
{
Serial.begin (38400);
while (!Serial);
Serial.println ("ok1");
	dateTime.Init ();
	board.Init ();
	
	//board.SerialInit (COMserial, 38400);//9600
									   //board.SerialInit(GSMserial, 9600);	board.SerialInit(RS485serial, 9600);
	Serial.println ( "nastav datum a cas ve formatu \n DD.MM.RRRR-HH:MM:SS\n");

}
void loop ()
{
	do
	{
		while (Serial.available () > 0)
		{
			c = Serial.read ();
			if (c == '\n')
			{
				Serial.println ("ok");
				SetDateTime (buffer);
			}
			else buffer += (char)c;
		}
	} while (c != '\n');

	dateTime.GetDateTime ();
	Serial.println ( dateTime.ToString () + '\n');
	delay (1000);
}

void SetDateTime (String buf)
{
	Serial.println (buf);
	dateTime.SetDateTime (buf);
	dateTime.GetDateTime ();
	Serial.println ( "nastaveny cas:  " + dateTime.ToString ());
}


