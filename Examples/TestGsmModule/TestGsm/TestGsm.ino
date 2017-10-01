


#include <LogoBoard\GsmModule.h>
#include <LogoBoard\LOGOGPIO.h>
#include <LogoBoard\DateTime.h>
#include <LogoBoard\LogoBoard.h>
#define DEBUG_MODE

#define sendSmsPin A0
LogoBoard board;
DateTime dateTime;
GsmModule gsm;

char pom, pom1;
int delayInp;
bool blok,blok1,blok2,blok3,blok4,blok5;
char *myNumber = "777622530";
void setup()
{
	Serial.begin (9600);
	while (!Serial);
	dateTime.Init();
	board.Init();
	board.SerialInit(GSMserial, 9600);
	board.SerialInit(RS485serial, 9600);
	//board.CANInit(CAN_500KBPS);
	if (gsm.Init())Serial.println("gsm OK\n\n");
	else Serial.println ("gsm fail\n\n");
}

void loop()
{
	
	if (digitalRead(IN_1)==LOW && blok==false)
	{
		blok = true;
		Serial.println( "POSILAM SMS NA CISLO ");
		Serial.println( myNumber);
		if(gsm.Sms(myNumber, "TEST sms"))Serial.println(" SMS OK");
		else Serial.println( " SMS fail");
	
	}

	if (digitalRead(IN_2) == LOW && blok1 == false)
	{
		blok1 = true;
		Serial.println( "VOLAM CISLO ");
		Serial.println(myNumber + '\n');
		if (gsm.Call(myNumber))Serial.println( " call OK\n");
		else Serial.println( " call fail\n");
	}

	if (digitalRead(IN_4) == LOW && blok2 == false)
	{
		blok2 = true;
		Serial.println( "ZAVESUJI\n");
		if (gsm.HangOut())Serial.println( " hangout OK\n");
		else Serial.println(" hangout fail\n");
	}

	if (digitalRead(IN_3) == LOW && blok3 == false)
	{
		blok3 = true;
		Serial.println("SIGNAL: ");
		Serial.println(gsm.Signal()+'\n');
		
	}
	//if (digitalRead(A3) == LOW && blok4 == false)
	//{
	//	blok4 = true;
	//	Serial.println( "OPERATOR: ");
	//	Serial.println( gsm.Operator() + '\n');
	//}

	//if (digitalRead(A4) == LOW && blok5 == false)
	//{
	//	blok5 = true;
	//	Serial.println ("CAS: ");
	//	Serial.println (gsm.ReadRTC() + '\n');
	//}


}


