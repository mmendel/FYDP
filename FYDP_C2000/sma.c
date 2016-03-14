/*
 * sma.c
 *
 *  Created on: Nov 16, 2015
 *      Author: Mathew
 */

#include "sma.h"
#include <stdlib.h>
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "f2802x_common/include/adc.h"

//****************************************************************************
//
//! Initialzes a SMA
//!
//! \param PWM_Pin, pwm number for control output
//! \param ADC_Pin, adc pin for reading pot values

//!
//! This function  CreateSMA
//!    1. Stores sma pin maping
//!    2. Inits control variables to 0
//!
//! \return created SMA.
//
//****************************************************************************
SMA* CreateSMA(unsigned char uId, unsigned char uPWM_Pin, unsigned char uADC_Pin){

	SMA *sma = malloc(sizeof(SMA));
	sma->jointId = uId;
	sma->uPwm = uPWM_Pin;
	sma->uADCPin = uADC_Pin;

	sma->uRef = 0;
	sma->stream = false;
	sma->fU1 = 0;
	sma->fU2 = 0;
	sma->fU3 = 0;
	sma->fE1 = 0;
	sma->fE2 = 0;
	sma->fE3 = 0;

	return sma;
}

//****************************************************************************
//
//! Initialzes the Robotic Hand Maping
//!
//! \param hand, array of fingers in hand

//!
//! This function  InitializeHand
//!    1. Initialized Finger -> SMA/Pot pint mapings
//!
//! \return None.
//
//****************************************************************************
void InitializeHand(Finger* hand)
{
	hand[0].fingerId = INDEX;
	hand[0].pip = CreateSMA(PIP, PWM_0, ADC_ResultNumber_0);
	hand[0].mcpv = CreateSMA(MCP_VERTICAL, PWM_2, ADC_ResultNumber_2);
	hand[0].mcph = CreateSMA(MCP_HORIZONTAL, PWM_NONE, ADC_ResultNumber_2);

	hand[1].fingerId = MIDDLE;
	hand[1].pip = CreateSMA(PIP, PWM_2, ADC_ResultNumber_3);
	hand[1].mcpv = CreateSMA(MCP_VERTICAL, PWM_3, ADC_ResultNumber_4);
	hand[1].mcph = CreateSMA(MCP_HORIZONTAL, PWM_NONE, ADC_ResultNumber_5);

	hand[2].fingerId = RING;
	hand[2].pip = CreateSMA(PIP, PWM_4, ADC_ResultNumber_6);
	hand[2].mcpv = CreateSMA(MCP_VERTICAL, PWM_5, ADC_ResultNumber_7);
	hand[2].mcph = CreateSMA(MCP_HORIZONTAL, PWM_NONE, ADC_ResultNumber_8);

	hand[3].fingerId = PINKY;
	hand[3].pip = CreateSMA(PIP, PWM_6, ADC_ResultNumber_8);
	hand[3].mcpv = CreateSMA(MCP_VERTICAL, PWM_7, ADC_ResultNumber_10);
	hand[3].mcph = CreateSMA(MCP_HORIZONTAL, PWM_NONE, ADC_ResultNumber_11);

}

//****************************************************************************
//
//! Finds a SMA
//!
//! \param hand, array of fingers in hand
//! \param uFingers, fingers requsted SMA is on
//! \param uJoint, joint requsted SMA is on

//!
//! This function  FingerToSMA
//!    1. returns the sma connected to a specifeid joint on a finger
//!
//! \return pointer to requested SMA.
//
//****************************************************************************
SMA* FingerToSMA(Finger* hand, unsigned char uFinger, unsigned char uJoint)
{
	switch(uJoint)
	{
	case PIP:
		return hand[uFinger].pip;
	case MCP_VERTICAL:
		return hand[uFinger].mcpv;
	case MCP_HORIZONTAL:
		return hand[uFinger].mcph;
	default:
		return NULL;
	}
}

