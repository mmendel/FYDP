/*
 * pwm.h
 *
 *  Created on: Nov 16, 2015
 *      Author: Mathew
 */

#ifndef PWM_H_
#define PWM_H_

#define PWM_0 			0
#define PWM_1 			1
#define PWM_2 			2
#define PWM_3 			3

#define PWM_5 			5 //Pin 64
#define PWM_6 			6 //Pin 01
#define PWM_7 			7 //Pin 02

#define INDEX 			0
#define MIDDLE 			1
#define POINTER 		2
#define PINKY 			3

#define NUM_OF_JOINTS 	3
#define PIP				0
#define MCP_VERTICAL	1
#define MCP_HORIZONTAL	2

typedef struct _SMA {
	unsigned char jointId, uPwm, uADCPin, uRef;
	float fU1, fU2, fU3;
	float fE1, fE2, fE3;
} SMA;

typedef struct _Finger {
	unsigned char fingerId;
	SMA pip, mcpv, mcph;
} Finger;

void InitializeHand(Finger* hand, int numFingers);
SMA* FingerToSMA(Finger* hand, unsigned char uFinger, unsigned char uJoint);

#endif /* PWM_H_ */
