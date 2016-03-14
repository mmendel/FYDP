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
#define POT_MAX_ANGLE		90
/*
#define VOLTAGE_MIN_ANGLE	0.8292308
#define VOLTAGE_MAX_ANGLE	2.131502
*/
#define VOLTAGE_MIN_ANGLE	0.277
#define VOLTAGE_MAX_ANGLE	1.22

#define PIP_ANGLE_MIN 		0
#define PIP_ANGLE_MAX		90

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

#define VOLTAGE_TO_ANGLE(x) (float)(POT_MIN_ANGLE + (x-VOLTAGE_MIN_ANGLE)*(POT_MAX_ANGLE - POT_MIN_ANGLE)/(VOLTAGE_MAX_ANGLE - VOLTAGE_MIN_ANGLE))

#define MAX_SMA_CURRENT  	0.9
#define SMA_RESISTANCE		0.47	//ohms/inch 18.5ohms/m
#define SMA_LENGTH			11.8
#define MAX_VOLTAGE_LIMIT 	5//MAX_SMA_CURRENT*SMA_RESISTANCE*SMA_LENGTH
#define	MAX_VOLTAGE			5
#define MIN_VOLTAGE 		0

#define VOLTAGE_TO_DUTY_CYCLE(x) (float)(x*100/(MAX_VOLTAGE - MIN_VOLTAGE))

#define F1 					5.0
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
