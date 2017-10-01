





#include <ds3231.h>
#include <config.h>
#include <Wire.h>
#include <GsmModule.h>
#include <GPIO.h>
#include <DateTime.h>
#include <Board.h>
Board board;
DateTime dateTime;
uint8_t tmp, tmp1, tmp2;
int delayInp;
void setup ()
{
	dateTime.Init ();
	board.Init ();
	board.SerialInit (BTserial, 38400);//38400
	//board.SerialInit(GSMserial, 9600);
	board.SerialInit (RS485serial, 38400);
	board.Send (RS485serial, "ok");
}

void loop ()
{

	if (tmp != PINL)
	{
		tmp = tmp1 = PINL;
		PORTC = ByteMirror (tmp1);

		for (int i = 0; i < 8; i++)
		{
			if (!bitRead (tmp, i))
			{
				board.outputs[i].New_state = HIGH;
				if (board.outputs[i].Old_state != board.outputs[i].New_state)
				{
					board.outputs[i].Old_state = board.outputs[i].New_state;
					dateTime.GetDateTime ();
					board.Send (BTserial, dateTime.ToString () + "  relay" + String (i) + "  on\n");
					board.Send (RS485serial, dateTime.ToString () + "  relay" + String (i) + "  on\n");
				}
			}
			else
			{
				board.outputs[i].New_state = LOW;
				if (board.outputs[i].Old_state != board.outputs[i].New_state)
				{
					board.outputs[i].Old_state = board.outputs[i].New_state;
					dateTime.GetDateTime ();
					board.Send (BTserial, dateTime.ToString () + "  relay" + String (i) + "  off\n");
				}
			}
			delay (board.inputs[i].Input_delay);
		}
	}
}

char  ByteMirror (char bt)
{
	bt = (bt & 0xF0) >> 4 | (bt & 0x0F) << 4;
	bt = (bt & 0xCC) >> 2 | (bt & 0x33) << 2;
	bt = (bt & 0xAA) >> 1 | (bt & 0x55) << 1;
	return bt ^ 0xFF;
}
