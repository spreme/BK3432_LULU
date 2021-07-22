#include <stdint.h>        // standard integer definition
#include <string.h>        // string manipulation
#include <stddef.h>     // standard definition
#include "BK3432_reg.h"
#include "rf.h"
#include "uart.h"
#include "gpio.h"
#include "adc.h"

void adc_init(uint8_t mode,uint8_t channel)
{
	uint32_t cfg;

	//enable adc clk
	REG_AHB0_ICU_ADCCLKCON &= ~(0x01 << 0);
	//adc div
	REG_AHB0_ICU_ADCCLKCON = (0x5 << 1);
		
	//set special as peripheral func
	gpio_config(GPIOD_0 + channel,FLOAT,PULL_NONE);

	//set adc mode/channel/wait clk
	//cfg  = ( (mode << BIT_ADC_MODE ) | (chanle << BIT_ADC_CHNL) | (0x01 << BIT_ADC_WAIT_CLK_SETTING));
	cfg  = ( (mode << BIT_ADC_MODE )  | (0x01 << BIT_ADC_WAIT_CLK_SETTING));
	REG_APB7_ADC_CFG =  cfg;

	//set adc sample rate/pre div
	cfg |= ((18 << BIT_ADC_SAMPLE_RATE) | (3 << BIT_ADC_PRE_DIV)|(0x0 << BIT_ADC_DIV1_MODE));

	REG_APB7_ADC_CFG =  cfg;
	
	cfg |= (0x0 << BIT_ADC_FILTER);
	REG_APB7_ADC_CFG =  cfg;

	REG_APB7_ADC_CFG |= (0x01 << BIT_ADC_INT_CLEAR);
	//REG_APB7_ADC_CFG |= (0x01 << BIT_ADC_EN);
	
	REG_AHB0_ICU_INT_ENABLE |= (0x01 << 8); 
}

void adc_deinit(uint8_t channel)
{
    gpio_config(0x30 + channel,INPUT,PULL_HIGH);

    REG_APB7_ADC_CFG &= ~(SET_ADC_EN+(0x03 << BIT_ADC_MODE ));
    REG_AHB0_ICU_ADCCLKCON |= (0x01 << 0);
    REG_AHB0_ICU_INT_ENABLE &= ~(0x01 << 8); 
}

uint16_t g_adc_value,adc_flag;

void adc_isr(void)
{
	REG_APB7_ADC_CFG |= (0x01 << BIT_ADC_INT_CLEAR);
    adc_flag=1;	
}

uint16_t adc_get_value(uint8_t chanle)
{
    uint16_t adc_cnt;
	uint16_t g_adc_value;
	
    adc_cnt = 0;
    adc_flag = 0;

    REG_APB7_ADC_CFG |= SET_ADC_EN+(0x01 << BIT_ADC_MODE );

	REG_APB7_ADC_CFG &= ~(0x0F<<BIT_ADC_CHNL);
	REG_APB7_ADC_CFG |= (chanle << BIT_ADC_CHNL);
    
    while (!adc_flag)  
    {
        adc_cnt++;       
        if(adc_cnt > 300)
        {
            UART_PRINTF("adc %d get value timeout\r\n", chanle);
            break;			
        }
        Delay_us(10);
    }
	
    if(adc_flag == 1)
    {
        g_adc_value = (REG_APB7_ADC_DAT);
//        UART_PRINTF("g_adc_value=%d\r\n",g_adc_value);
    }
    
    REG_APB7_ADC_CFG &= ~(SET_ADC_EN+(0x03 << BIT_ADC_MODE )); 
    return g_adc_value;     
}



