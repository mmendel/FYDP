/*
 * adc.c
 *
 *  Created on: Nov 16, 2015
 *      Author: Mathew
 */
// Standard includes
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// Driverlib includes
#include "utils.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_types.h"
#include "hw_adc.h"
#include "hw_ints.h"
#include "hw_gprcm.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "pin.h"
#include "adc.h"
#include "adc_user.h"

void ConfigureAdc()
{
#ifdef CC3200_ES_1_2_1
        //
        // Enable ADC clocks.###IMPORTANT###Need to be removed for PG 1.32
        //
        HWREG(GPRCM_BASE + GPRCM_O_ADC_CLK_CONFIG) = 0x00000043;
        HWREG(ADC_BASE + ADC_O_ADC_CTRL) = 0x00000004;
        HWREG(ADC_BASE + ADC_O_ADC_SPARE0) = 0x00000100;
        HWREG(ADC_BASE + ADC_O_ADC_SPARE1) = 0x0355AA00;
#endif
        //
        // Configure ADC timer which is used to timestamp the ADC data samples
        //
        MAP_ADCTimerConfig(ADC_BASE,2^17);

        //
        // Enable ADC timer which is used to timestamp the ADC data samples
        //
        MAP_ADCTimerEnable(ADC_BASE);

        //
        // Enable ADC module
        //
        MAP_ADCEnable(ADC_BASE);
}

void EnableAdcChannel(unsigned char uAdcChannel)
{
    MAP_ADCChannelEnable(ADC_BASE, uAdcChannel);
}

float SampleAdc(unsigned char uAdcChannel)
{
	unsigned long sample = 0;
    if(MAP_ADCFIFOLvlGet(ADC_BASE, uAdcChannel))
    {
        sample = MAP_ADCFIFORead(ADC_BASE, uAdcChannel);
    }

	return SAMPLE_TO_VOLTAGE(sample);
}

