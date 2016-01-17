/*
 * sma_controller.h
 *
 *  Created on: Nov 16, 2015
 *      Author: Mathew
 */

#ifndef SMA_CONTROLLER_H_
#define SMA_CONTROLLER_H_

#include "sma.h"

#define POT_MIN_ANGLE 		0
#define POT_MAX_ANGLE		360

#define VOLTAGE_MIN_ANGLE	0
#define VOLTAGE_MAX_ANGLE	3.3

#define PIP_ANGLE_MIN 		0
#define PIP_ANGLE_MAX		100

#define MCPV_ANGLE_MIN		0
#define MCPV_ANGLE_MAX		90

#define MCPH_ANGLE_MIN		0
#define MCPH_ANGLE_MAX		20

#define PIP_ANGLE_MIN_POT 	PIP_ANGLE_MIN + POT_MIN_ANGLE
#define PIP_ANGLE_MAX_POT	PIP_ANGLE_MAX + POT_MIN_ANGLE

#define MCPV_ANGLE_MIN_POT	MCPV_ANGLE_MIN + POT_MIN_ANGLE
#define MCPV_ANGLE_MAX_POT	MCPV_ANGLE_MAX + POT_MIN_ANGLE

#define MCPH_ANGLE_MIN_POT	MCPH_ANGLE_MIN + POT_MIN_ANGLE
#define MCPH_ANGLE_MAX_POT	MCPH_ANGLE_MAX + POT_MIN_ANGLE

#define VOLTAGE_TO_ANGLE(x) (float)(POT_MIN_ANGLE + x*(POT_MAX_ANGLE - POT_MIN_ANGLE)/(VOLTAGE_MAX_ANGLE - VOLTAGE_MIN_ANGLE))

#define MAX_SMA_CURRENT  	2.2
#define SMA_RESISTANCE		2
#define MAX_VOLTAGE_LIMIT	MAX_SMA_CURRENT*SMA_RESISTANCE
#define	MAX_VOLTAGE			5
#define MIN_VOLTAGE 		0

#define VOLTAGE_TO_DUTY_CYCLE(x) (float)(x*100/(MAX_VOLTAGE - MIN_VOLTAGE))

#define F1 					1.0
#define F2					0.0
#define F3					0.0
#define G2					0.0
#define G3					0.0

float ControllerStep(SMA* sma, float angle);
void SetReference(Finger* hand, unsigned char finger, unsigned char joint, uint16_t ref);
uint16_t GetReference(Finger* hand, unsigned char finger, unsigned char joint);
void DataStreamStart(Finger* hand, unsigned char finger, unsigned char joint);
void DataStreamStop(Finger* hand, unsigned char finger, unsigned char joint);


#endif /* SMA_CONTROLLER_H_ */
