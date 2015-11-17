/*
 * adc.h
 *
 *  Created on: Nov 16, 2015
 *      Author: Mathew
 */

#ifndef ADC_USER_H_
#define ADC_USER_H_

#define ADC_VOLTAGE_LEVEL	1.4
#define SAMPLE_TO_VOLTAGE(x) (((float)((x >> 2) & 0x0FF))*ADC_VOLTAGE_LEVEL)/4096

void ConfigureAdc();
void EnableAdc();
float SampleAdc(unsigned char uAdcChannel);

#endif /* ADC_USER_H_ */
