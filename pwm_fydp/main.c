//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     - PWM Application
// Application Overview - The general purpose timers (GPTs) support a 16-bit 
//                        pulse-width modulation (PWM) mode with software-
//                        programmable output inversion of the PWM signal.
//                        The brightness of the LEDs are varied from Off to On 
//                        as PWM output varies.
// Application Details  -
// http://processors.wiki.ti.com/index.php/CC32xx_PWM
// or
// docs\examples\CC32xx_PWM.pdf
//
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup pwm
//! @{
//
//****************************************************************************

// Standard includes
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_apps_rcm.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "rom.h"
#include "rom_map.h"
#include "timer.h"
#include "utils.h"
#include "prcm.h"

#include "pinmux.h"
#include "sma.h"

#include "uart_if.h"
#include "common.h"

#define APPLICATION_VERSION     "1.1.1"
#define DBG_PRINT               Report
#define PWM_MAX_DUTY			254
#define PWM_DUTY_PERCENT_TO_TICK(x) (x*PWM_MAX_DUTY)/100
#define PWM_DUTY_TICK_TO_PERCENT(x) (x*100)/PWM_MAX_DUTY
#define MESSAGE_LENGTH			20
#define NUM_OF_FINGERS 			1

//
// The PWM works based on the following settings:
//     Timer reload interval -> determines the time period of one cycle
//     Timer match value -> determines the duty cycle 
//                          range [0, timer reload interval]
// The computation of the timer reload interval and dutycycle granularity
// is as described below:
// Timer tick frequency = 80 Mhz = 80000000 cycles/sec
// For a time period of 0.5 ms, 
//      Timer reload interval = 80000000/2000 = 40000 cycles
// To support steps of duty cycle update from [0, 255]
//      duty cycle granularity = ceil(40000/255) = 157
// Based on duty cycle granularity,
//      New Timer reload interval = 255*157 = 40035
//      New time period = 0.5004375 ms
//      Timer match value = (update[0, 255] * duty cycle granularity)
//
#define TIMER_INTERVAL_RELOAD   40035 /* =(255*157) */
#define DUTYCYCLE_GRANULARITY   157

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

Finger Hand[NUM_OF_FINGERS];

//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************



/****************************************************************************/
/*                      LOCAL FUNCTION DEFINITIONS                          */
/****************************************************************************/

//****************************************************************************
//
//! Update the dutycycle of the PWM timer
//!
//! \param ulBase is the base address of the timer to be configured
//! \param ulTimer is the timer to be setup (TIMER_A or  TIMER_B)
//! \param ucLevel translates to duty cycle settings (0:255)
//! 
//! This function  
//!    1. The specified timer is setup to operate as PWM
//!
//! \return None.
//
//****************************************************************************
void UpdateDutyCycle(unsigned long ulBase, unsigned long ulTimer,
                     unsigned char ucLevel)
{
    //
    // Match value is updated to reflect the new dutycycle settings
    //
    MAP_TimerMatchSet(ulBase,ulTimer,(ucLevel*DUTYCYCLE_GRANULARITY));
}


int PwmDutySelection(SMA * sma, unsigned char percent)
{
	unsigned char ucLevel = PWM_DUTY_PERCENT_TO_TICK(percent);
	unsigned long ulBase, ulTimer;
	sma->uRef = percent;
	switch(sma->uPwm)
	{
/*
	case PWM_0:
		ulBase = TIMERA0_BASE;
		ulTimer = TIMER_A;
		break;
	case PWM_1:
		ulBase = TIMERA0_BASE;
		ulTimer = TIMER_B;
		break;
	case PWM_2:
		ulBase = TIMERA1_BASE;
		ulTimer = TIMER_A;
		break;
	case PWM_3:
		ulBase = TIMERA1_BASE;
		ulTimer = TIMER_B;
		break;
*/
	case PWM_5:
		ulBase = TIMERA2_BASE;
		ulTimer = TIMER_B;
		break;
	case PWM_6:
		ulBase = TIMERA3_BASE;
		ulTimer = TIMER_A;
		break;
	case PWM_7:
		ulBase = TIMERA3_BASE;
		ulTimer = TIMER_B;
		break;
	default:
		return -1;
	}

	UpdateDutyCycle(ulBase, ulTimer, ucLevel);
	return ucLevel;

}

//****************************************************************************
//
//! Setup the timer in PWM mode
//!
//! \param ulBase is the base address of the timer to be configured
//! \param ulTimer is the timer to be setup (TIMER_A or  TIMER_B)
//! \param ulConfig is the timer configuration setting
//! \param ucInvert is to select the inversion of the output
//! 
//! This function  
//!    1. The specified timer is setup to operate as PWM
//!
//! \return None.
//
//****************************************************************************
void SetupTimerPWMMode(unsigned long ulBase, unsigned long ulTimer,
                       unsigned long ulConfig, unsigned char ucInvert)
{
    //
    // Set GPT - Configured Timer in PWM mode.
    //
    MAP_TimerConfigure(ulBase,ulConfig);
    MAP_TimerPrescaleSet(ulBase,ulTimer,0);

    //
    // Inverting the timer output if required
    //
    MAP_TimerControlLevel(ulBase,ulTimer,ucInvert);
    
    //
    // Load value set to ~0.5 ms time period
    //
    MAP_TimerLoadSet(ulBase,ulTimer,TIMER_INTERVAL_RELOAD);
    
    //
    // Match value set so as to output level 0
    //
    MAP_TimerMatchSet(ulBase,ulTimer,TIMER_INTERVAL_RELOAD);

}

//****************************************************************************
//
//! Sets up the identified timers as PWM to drive the peripherals
//!
//! \param none
//! 
//! This function sets up the folowing 
//!    1. TIMERA2 (TIMER B) as RED of RGB light
//!    2. TIMERA3 (TIMER B) as YELLOW of RGB light
//!    3. TIMERA3 (TIMER A) as GREEN of RGB light
//!
//! \return None.
//
//****************************************************************************
void InitPWMModules()
{
    //
    // Initialization of timers to generate PWM output
    //
    //MAP_PRCMPeripheralClkEnable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);
    //MAP_PRCMPeripheralClkEnable(PRCM_TIMERA1, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralClkEnable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA3, PRCM_RUN_MODE_CLK);

    //
    // TIMERA2 (TIMER B) as RED of RGB light. GPIO 9 --> PWM_5
    //
    SetupTimerPWMMode(TIMERA2_BASE, TIMER_B,
            (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM), 1);
    //
    // TIMERA3 (TIMER B) as YELLOW of RGB light. GPIO 10 --> PWM_6
    //
    SetupTimerPWMMode(TIMERA3_BASE, TIMER_A,
            (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM), 1);
    //
    // TIMERA3 (TIMER A) as GREEN of RGB light. GPIO 11 --> PWM_7
    //
    SetupTimerPWMMode(TIMERA3_BASE, TIMER_B,
            (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM), 1);

    MAP_TimerEnable(TIMERA2_BASE,TIMER_B);
    MAP_TimerEnable(TIMERA3_BASE,TIMER_A);
    MAP_TimerEnable(TIMERA3_BASE,TIMER_B);

}

//****************************************************************************
//
//! Disables the timer PWMs
//!
//! \param none
//! 
//! This function disables the timers used
//!
//! \return None.
//
//****************************************************************************
void DeInitPWMModules()
{
    //
    // Disable the peripherals
    //
    MAP_TimerDisable(TIMERA2_BASE, TIMER_B);
    MAP_TimerDisable(TIMERA3_BASE, TIMER_A);
    MAP_TimerDisable(TIMERA3_BASE, TIMER_B);
    MAP_PRCMPeripheralClkDisable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkDisable(PRCM_TIMERA3, PRCM_RUN_MODE_CLK);
}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

void processCommand(char* com)
{
	unsigned int finger, joint;
	char* pch;

	pch = strtok(NULL, " ");
	if(pch == NULL)
	{
		UART_PRINT("Invalid Commad: %s No Finger Specified \n\r", com);
		return;
	}
	sscanf(pch, "%d", &finger);
	if(finger >= NUM_OF_FINGERS)
	{
		UART_PRINT("Invalid Commad: %s Finger is out of range \n\r", com);
		return;
	}

	pch = strtok(NULL, " ");
	if(pch == NULL)
	{
		UART_PRINT("Invalid Commad: %s No Joint Specified \n\r", com);
		return;
	}
	sscanf(pch, "%d", &joint);
	if(joint >= NUM_OF_JOINTS)
	{
		UART_PRINT("Invalid Commad: %s Joint is out of range \n\r", com);
		return;
	}

	SMA* sma = FingerToSMA(Hand, finger, joint);

	if(strcmp(com, "set") == 0)
	{
		int percent;
		pch = strtok(NULL, " ");
		if(pch == NULL)
		{
			UART_PRINT("Invalid Commad: %s Missing Duty Param \n\r", com);
			return;
		}
		sscanf(pch, "%d", &percent);

		if(percent < 0)
			percent = 0;
		else if(percent >100)
			percent = 100;

		PwmDutySelection(sma, percent);

		UART_PRINT("Finger %d, Joint %d, set to %d \n\r", finger, joint, sma->uRef);
	}
	else if(strcmp(com, "get") == 0)
	{
		UART_PRINT("Finger %d, Joint %d, is set to %d \n\r", finger, joint, sma->uRef);
	}
	else
	{
		UART_PRINT("Invalid Commad: %s \n\r", com);
	}
}


//****************************************************************************
//
//! Demonstrates the controlling of LED brightness using PWM
//!
//! \param none
//! 
//! This function  
//!    1. Pinmux the GPIOs that drive LEDs to PWM mode.
//!    2. Initializes the timer as PWM.
//!    3. Loops to continuously change the PWM value and hence brightness 
//!       of LEDs.
//!
//! \return None.
//
//****************************************************************************
void main()
{
    char comBuf[MESSAGE_LENGTH];
    char *pch;
    //
    // Board Initialisation
    //
    BoardInit();
    
    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();    

    InitializeHand(Hand, NUM_OF_FINGERS);
    
    //
    // Configure the UART
    //
#ifndef NOTERM
    InitTerm();
#endif  //NOTERM

    //
    // Initialize the PWMs used for driving the LEDs
    //
    InitPWMModules();
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\*****************************************\n\r");
    UART_PRINT("	PWM CONTROLLER\n\r");
    UART_PRINT("\********************************************\n\r");
    UART_PRINT("\n\n\n\r");

    while(1)
    {

    	if (GetCmd(comBuf, MESSAGE_LENGTH) > 0){
    		pch = strtok(comBuf, " ");
    		if (pch != NULL)
    			processCommand(pch);
    		memset(comBuf, 0, sizeof(comBuf));
    	}
    }

    //
    // De-Init peripherals - will not reach here...
    //
    //DeInitPWMModules();
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
