// 
// 
#define CAN_MODULE
//#define WIFI_MODULE

#include "LogoBoard.h"


LogoBoard::LogoBoard()
{
}

LogoBoard::~LogoBoard()
{
}
void LogoBoard:: Init()
{
	I_O_init();

}

void LogoBoard::BTinit(long baudRate)
{
	BTserial.begin(baudRate);
}

void LogoBoard::GSMinit(long baudRate)
{
	GSMserial.begin(baudRate);
}

void LogoBoard::RS485init(long baudRate)
{
	RS485serial.begin(baudRate);
}

#ifdef WIFI_MODULE
void Board::WIFIinit(long baudRate)
{
	WIFIserial.begin(baudRate);
}
#endif



#ifdef CAN_MODULE

void LogoBoard::CANInit(int speed)
{
	MCP_CAN CAN(53);
	CAN.begin(speed);
	pinMode(2, INPUT);
}
#endif

void LogoBoard::Send(Modules modul, String str)
{
	Serial.println("send");
	switch (modul)
	{
	case BT: Serial.println(str);
		break;
#ifdef GSM_MODULE
	case GSM: GSMserial.print(str);
		break;
#endif // GSM_MODULE
#ifdef RS485_MODULE
	case RS485: SEND_485 Serial2.print(str); delay(15); REC_485
		break;
#endif // RS485_MODULE
#ifdef WIFI_MODULE
	case WIFI:Serial3.print(str);
		break;
#endif //WIFI_MODULE
	default:
		break;
	}
}

void LogoBoard::Send(Modules modul, char * arr)
{
}



void LogoBoard::I_O_init()
{
	//inputs
	GpioInit(&inputs[0], IN1, INPUT_PULLUP);
	GpioInit(&inputs[1], IN2, INPUT_PULLUP);
	GpioInit(&inputs[2], IN3, INPUT_PULLUP);
	GpioInit(&inputs[3], IN4, INPUT_PULLUP);

	//outputs
	GpioInit(&outputs[0], RELE1, OUTPUT);
	GpioInit(&outputs[1], RELE2, OUTPUT);
	GpioInit(&outputs[2], OUT12V_1, OUTPUT);
	GpioInit(&outputs[3], OUT12V_2, OUTPUT);
	GpioInit(&outputs[4], OUT12V_3, OUTPUT);
	GpioInit(&outputs[5], OUT12V_4, OUTPUT);

}

void LogoBoard::GpioInit(Gpio_struct *gpio, char pin, char type)
{
	gpio->Pin = pin;
	gpio->Type = type;
	pinMode(pin, type);
	if (type == INPUT || type == INPUT_PULLUP)gpio->Input_delay = 100;
}






