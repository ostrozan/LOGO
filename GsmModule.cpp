// 
// 
// 

#include "GsmModule.h"
#include "LogoBoard.h"

GsmModule::GsmModule()
{
}

GsmModule::~GsmModule()
{
}

bool GsmModule::Init()
{
	GSMserial.print("AT\r\n");
	return GsmAck("OK");
}

bool GsmModule::Call(String number)
{
	GSMserial.print("ATD " + number + ";\r\n");
	return GsmAck("OK");
}

bool GsmModule::Sms(String number, String message)
{
	GSMserial.print(F("AT+CMGF=1\r")); //set sms to text mode  
	rx_buffer = readSerial();
	GSMserial.print("AT+CMGS=\""+number+ "\"\r");  // command to send sms
	rx_buffer = readSerial();
	GSMserial.print(message+"\r");
	delay(100);
	GSMserial.print((char)26);
	//Serial.println("sms " + rx_buffer);//test
	//expect CMGS:xxx   , where xxx is a number,for the sending sms.
	return GsmAck("CMGS");	
}

bool GsmModule::HangOut()
{
	GSMserial.print("ATH\r\n");
	return GsmAck("OK");
}

String GsmModule::Signal()
{
	GSMserial.print("AT+CSQ\r\n");
	if (GsmAck("OK"))return   rx_buffer.substring(rx_buffer.indexOf("+CSQ:")+6, rx_buffer.indexOf("+CSQ:") + 10);
	else return "ERR";
}

String GsmModule::Operator()
{
	GSMserial.print("AT+COPS?\r\n");
	if (GsmAck("OK"))return rx_buffer.substring(rx_buffer.lastIndexOf(',') + 1, rx_buffer.lastIndexOf(',') + 8);
	else return "ERR";
}

String GsmModule::readSerial()
{
	timeout = 0;
	while (!GSMserial.available() && timeout < 12000)
	{
		delay(13);
		timeout++;


	}
	if (GSMserial.available()) {
		return GSMserial.readString();
	}


}

bool GsmModule::GsmAck(String str)
{
	rx_buffer = readSerial();
	if (((rx_buffer.indexOf(str)) != -1))return true;
	else return false;
}


