#ifndef __ADC_H
#define __ADC_H

#include <stdint.h>

void ADC_Initial(void);
uint16_t ADC_GetData(uint8_t ADC_Channel);

#endif