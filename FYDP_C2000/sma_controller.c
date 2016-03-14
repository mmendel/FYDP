/*
 * sma_controller.c
 *
 *  Created on: Nov 16, 2015
 *      Author: Mathew
 */

#include "sma_controller.h"
#include "sma.h"

void AngleSaturator(SMA* sma)
{
	int anglemax = 0;
	int anglemin = 0;

	switch(sma->jointId)
	{
	case PIP:
		anglemax = PIP_ANGLE_MAX;
		anglemin = PIP_ANGLE_MIN;
		break;
	case MCP_VERTICAL:
		anglemax = MCPV_ANGLE_MAX;
		anglemin = MCPV_ANGLE_MIN;
		break;
	case MCP_HORIZONTAL:
		anglemax = MCPH_ANGLE_MAX;
		anglemin = MCPH_ANGLE_MIN;
		break;
	}

	if (sma->uRef < anglemin)
		sma->uRef = anglemin;
	else if (sma->uRef > anglemax)
		sma->uRef = anglemax;
}

float VoltageSaturator(float voltage)
{
	if (voltage < MIN_VOLTAGE)
		voltage = MIN_VOLTAGE;
	else if (voltage > MAX_VOLTAGE_LIMIT)
		voltage = MAX_VOLTAGE_LIMIT;
	return voltage;
}

float ControllerStep(SMA* sma, float angle)
{
	sma->fE3 = sma->fE2;
	sma->fE2 = sma->fE1;
	sma->fE1 = sma->uRef - angle;

	sma->fU3 = sma->fU2;
	sma->fU2 = sma->fU1;
	sma->fU1 = VoltageSaturator(1*(F1*sma->fE1 + F2*sma->fE2 + F3*sma->fE3 - G2*sma->fU2 - G3*sma->fU3));
	//sma->fU1 = VoltageSaturator(5*(F1*sma->fE1 + F2*sma->fE2 + F3*sma->fE3 - G2*sma->fU2 - G3*sma->fU3));
	return VOLTAGE_TO_DUTY_CYCLE(sma->fU1);

}

void SetReference(Finger* hand, unsigned char finger, unsigned char joint, uint16_t ref)
{
	SMA* sma = FingerToSMA(hand, finger, joint);
	sma->uRef = ref;
	AngleSaturator(sma);
}

uint16_t GetReference(Finger* hand, unsigned char finger, unsigned char joint)
{
	SMA* sma = FingerToSMA(hand, finger, joint);
	return sma->uRef;
}

void DataStreamStart(Finger* hand, unsigned char finger, unsigned char joint)
{
	SMA* sma = FingerToSMA(hand, finger, joint);
	sma->stream = true;
}

void DataStreamStop(Finger* hand, unsigned char finger, unsigned char joint)
{
	SMA* sma = FingerToSMA(hand, finger, joint);
	sma->stream = false;
}

