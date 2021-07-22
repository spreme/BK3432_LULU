

#ifndef _ADC_H_
#define _ADC_H_



#include <stdbool.h>          // standard boolean definitions
#include <stdint.h>           // standard integer functions

#define ENABLE_ADC   			0



void adc_init(uint8_t mode,uint8_t channel);
void adc_isr(void);
uint16_t adc_get_value(uint8_t chanle);

#endif //



