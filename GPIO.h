// GPIO.h

#ifndef _GPIO_h
#define _GPIO_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
typedef struct
{
	char Pin;
	char Type;
	char New_state;
	char Prew_state;
	long Time_pulse;
	int Input_delay;
}Gpio_struct;

/*Gpio_struct outputs[8];
Gpio_struct inputs[8];
Gpio_struct gpios[16];*/

//void GpioInit(Gpio_struct *gpio, char pin, char type);

#endif

