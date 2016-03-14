/*
 * LED.c
 *
 *  Created on: Feb 24, 2016
 *      Author: Mathew
 */

#include "LED.h"

GPIO_Handle myGpioCopy;

void ConfigureLeds(GPIO_Handle myGpioIn)
{
	myGpioCopy = myGpioIn;

	// Set to general purpose
    GPIO_setMode(myGpioCopy, POWER, 0);
    GPIO_setMode(myGpioCopy, RUNNING, 0);
    GPIO_setMode(myGpioCopy, ERROR, 0);
    GPIO_setMode(myGpioCopy, DEADMAN, 0);
    GPIO_setMode(myGpioCopy, STREAM, 0);
    GPIO_setMode(myGpioCopy, LED5, 0);

    // Set all as outpus
    GPIO_setDirection(myGpioCopy, POWER, GPIO_Direction_Output);
    GPIO_setDirection(myGpioCopy, RUNNING, GPIO_Direction_Output);
    GPIO_setDirection(myGpioCopy, ERROR, GPIO_Direction_Output);
    GPIO_setDirection(myGpioCopy, DEADMAN, GPIO_Direction_Output);
    GPIO_setDirection(myGpioCopy, STREAM, GPIO_Direction_Output);
    GPIO_setDirection(myGpioCopy, LED5, GPIO_Direction_Output);

    // Disable all
    GPIO_setHigh(myGpioCopy, POWER);
    GPIO_setHigh(myGpioCopy, RUNNING);
    GPIO_setHigh(myGpioCopy, ERROR);
    GPIO_setHigh(myGpioCopy, DEADMAN);
    GPIO_setHigh(myGpioCopy, STREAM);
    GPIO_setHigh(myGpioCopy, LED5);
}

void SetLedState(int led, int state)
{
	if (state == OFF)
	    GPIO_setHigh(myGpioCopy, led);
	else
	    GPIO_setLow(myGpioCopy, led);
}

void LedPosistion(float angle)
{
	int step = (int)angle/12;
	SetLedState(POWER, 1 < step);
	SetLedState(RUNNING, 2 < step);
	SetLedState(ERROR, 3 < step);
	SetLedState(DEADMAN,4 < step);
	SetLedState(STREAM, 5 < step);
	SetLedState(LED5, 6 < step);

}

