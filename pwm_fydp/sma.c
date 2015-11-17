/*
 * sma.c
 *
 *  Created on: Nov 16, 2015
 *      Author: Mathew
 */

#include "sma.h"

// Standard includes
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//****************************************************************************
//
//! Initialzes a SMA
//!
//! \param PWM_Pin, pwm number for control output
//! \param ADC_Pin, adc pin for reading pot values

//!
//! This function  Pinmux
//!    1. Stores sma pin maping
//!    2. Inits control variables to 0
//!
//! \return created SMA.
//
//****************************************************************************
SMA CreateSMA(unsigned char uId, unsigned char uPWM_Pin, unsigned char uADC_Pin){

	SMA sma = {  .jointId = uId, .uPwm = uPWM_Pin, .uADCPin = uADC_Pin, .uRef = 0,
				 .fE1 = 0, .fE2 = 0, .fE3 = 0,
				 .fU1 = 0, .fU2 = 0, .fU3 = 0
	};
	return sma;
}

//****************************************************************************
//
//! Initialzes the Robotic Hand Maping
//!
//! \param hand, array of fingers in hand
//! \param numFingers, count of total fingers

//!
//! This function  Pinmux
//!    1. Initialized Finger -> SMA/Pot pint mapings
//!
//! \return None.
//
//****************************************************************************
void InitializeHand(Finger* hand, int numFingers)
{
	hand[0].fingerId = INDEX;
	hand[0].pip = CreateSMA(PIP, PWM_5, 0); //TODO: Proper ADC Pins
	hand[0].mcpv = CreateSMA(MCP_VERTICAL, PWM_6, 0); //TODO: Proper ADC Pins
	hand[0].mcph = CreateSMA(MCP_HORIZONTAL, PWM_7, 0); //TODO: Proper ADC Pins
}

//****************************************************************************
//
//! Finds a SMA
//!
//! \param hand, array of fingers in hand
//! \param uFingers, fingers requsted SMA is on
//! \param uJoint, joint requsted SMA is on

//!
//! This function  Pinmux
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
		return &hand[uFinger].pip;
	case MCP_VERTICAL:
		return &hand[uFinger].mcpv;
	case MCP_HORIZONTAL:
		return &hand[uFinger].mcph;
	default:
		return NULL;
	}
}

