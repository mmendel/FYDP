/*
 * LED.h
 *
 *  Created on: Feb 24, 2016
 *      Author: Mathew
 */

#ifndef LED_H_
#define LED_H_

#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "f2802x_common/include/gpio.h"

#define POWER	 	GPIO_Number_19
#define RUNNING 	GPIO_Number_16
#define ERROR 		GPIO_Number_17
#define DEADMAN 	GPIO_Number_32
#define STREAM 		GPIO_Number_33
#define LED5		GPIO_Number_18

#define ON			0
#define OFF			1

void ConfigureLeds(GPIO_Handle myGpio);
void SetLedState(int led, int state);


#endif /* LED_H_ */

