/**
****************************************************************************************
*
* @file gpio.c
*
* @brief icu initialization and specific functions
*
* Copyright (C) Beken 2009-2016
*
* $Rev: $
*
****************************************************************************************
*/

#include <stddef.h>     // standard definition
#include "BK3432_reg.h"
#include "gpio.h"
#include "timer.h"      // timer definition

#include "rwip.h"       // SW interface
#include "h4tl.h"
#if (NVDS_SUPPORT)
#include "nvds.h"                    // NVDS Definitions
#endif
#include "dbg.h"
#include "icu.h"
#include "rf.h"
#include "app.h"
#include "app_task.h"
#include "uart.h"
#include "user_config.h"
#include "ke_timer.h"
#include "adc.h"

volatile unsigned long * GPIO_CFG[] = 
{
    &REG_APB5_GPIOA_CFG,
    &REG_APB5_GPIOB_CFG,
    &REG_APB5_GPIOC_CFG,
    &REG_APB5_GPIOD_CFG,
    &REG_APB5_GPIOE_CFG
};


volatile unsigned long * GPIO_DATA[] = 
{
    &REG_APB5_GPIOA_DATA,
    &REG_APB5_GPIOB_DATA,
    &REG_APB5_GPIOC_DATA,
    &REG_APB5_GPIOD_DATA,
    &REG_APB5_GPIOE_DATA
};

unsigned int GPIO_CFG_BACKUP[5];
unsigned int GPIO_DATA_BACKUP[5];


static GPIO_INT_CALLBACK_T gpio_int_cb = NULL; 

void gpio_config(uint8_t gpio, Dir_Type dir, Pull_Type pull)
{
    uint32_t  temp_cfg, temp_data;
    uint8_t port = (gpio&0xf0)>>4;
    uint8_t  pin = gpio&0xf;
    temp_cfg = *(GPIO_CFG[port]);
    temp_data = *(GPIO_DATA[port]);
 
    if(dir == OUTPUT)	
    {
        temp_cfg |= (1<<pin);
        temp_cfg &= ~(1<<(pin+8));
		
        temp_data &= ~(1<<pin<<16);
    }
    else if(dir== INPUT)
    {
        temp_cfg |= 1<<pin;
        temp_cfg |= (1<<(pin+8));
        temp_data |= (1<<pin)<<16;
    }
	else if(dir == HIRESI)		
	{
		temp_cfg |= 1<<pin;
        temp_cfg |= (1<<(pin+8));
		temp_data  &= ~(1<<pin<<16);	
	}
    else
    {
        temp_cfg &= ~(1<<pin);
        temp_cfg |= (1<<(pin+8));
	  	temp_data  |= (1<<pin<<16);
    }

    switch(pull)
    {
    case PULL_HIGH:
        temp_cfg |= (1<<pin<<16);
        temp_cfg &= ~(1<<pin<<24);
        break;
    case PULL_LOW:
        temp_cfg &= ~(1<<pin<<16);
        temp_cfg |= (1<<pin<<24);
        break;
    case PULL_NONE:
        temp_cfg &= ~(1<<pin<<16);
        temp_cfg &= ~(1<<pin<<24);
        break;
    }
    *(GPIO_CFG[port]) = temp_cfg;
    *(GPIO_DATA[port]) = temp_data;
}

uint8_t gpio_get_input(uint8_t gpio)
{
    uint32_t temp;
    uint8_t port = (gpio&0xf0)>>4;
    uint8_t  pin = gpio&0xf;

    temp = *(GPIO_DATA[port]);
    temp = ((temp >> 0x08)&0xff)>>pin;
	
    return temp&0x01;
}

void gpio_set(uint8_t gpio, uint8_t val)
{
    uint32_t temp;
    uint8_t port = (gpio&0xf0)>>4;
    uint8_t  pin = gpio&0xf;
    
    temp = *(GPIO_DATA[port]);
    if(val)
    {
        temp |= 1<<pin;
    }
    else
    {
        temp &= ~(1<<pin);
    }

    *(GPIO_DATA[port]) = temp;
}

#if GPIO_DBG_MSG
void gpio_debug_msg_init()
{

	*(GPIO_CFG[1]) = 0X0000ff;		    	
	*(GPIO_DATA[1]) = 0;
	REG_AHB0_ICU_DIGITAL_PWD |= (0X01 << 4);
}
#endif

#define INT_HIGH_EDGE	1
#define INT_LOW_EDGE	0

static void jk_gpio_int_config(uint8_t gpio, uint8_t trigger)
{
	if(trigger == INT_HIGH_EDGE)
		REG_APB5_GPIO_WUATOD_TYPE |= 0<<(8*(gpio>>4)+(gpio&0x0f)); //0<<2 = 0	//上升沿触发
	else                              
		REG_APB5_GPIO_WUATOD_TYPE |= 1<<(8*(gpio>>4)+(gpio&0x0f)); //1<<2 = 4	//下降沿触发
	
	REG_APB5_GPIO_WUATOD_STAT |= 1<<(8*(gpio>>4)+(gpio&0x0f));
	Delay_ms(2);
	REG_APB5_GPIO_WUATOD_ENABLE |= 1<<(8*(gpio>>4)+(gpio&0x0f));
	REG_AHB0_ICU_DEEP_SLEEP0 |= 1<<(8*(gpio>>4)+(gpio&0x0f));
}

static void jk_gpio_int_en(void)
{
	REG_AHB0_ICU_INT_ENABLE |= (0x01 << 9);
}


static uint32_t suble_gpio_irq_pin_change_format(uint32_t pin)
{
    uint32_t zero_count = 0;
    for(int idx=0; idx<32; idx++) {
        pin = pin>>1;
        if(pin == 0) {
            break;
        }
        zero_count++;
    }
    return (((zero_count/8)*0x10) + (zero_count%8));
}
static void gpio_irq_handler(uint32_t pin)
{
//	UART_PRINTF("key pin:%d \r\n",pin);

	pin = suble_gpio_irq_pin_change_format(pin);
	
	lock_timeout = LOCK_TIMEOUT_TIME;
			
	if(keep_dowm_flag == 0)
	{
		switch(pin)
		{
			case RECORD_KEY:
			UART_PRINTF("RECORD_KEY \r\n");
			if(lock_flag == 0)
				ke_timer_set(REC_KEY_TASK, TASK_APP, 10);
			
			break;

			case SET_KEY:
			UART_PRINTF("SET_KEY \r\n");
			ke_timer_set(KEY_SCAN_TASK, TASK_APP, 10);

			break;

			case DOWN_KEY:
			UART_PRINTF("DOWN_KEY \r\n");
			ke_timer_set(KEY_SCAN_TASK, TASK_APP, 10);
			
			break;

			case UP_KEY:
			UART_PRINTF("UP_KEY \r\n");
			ke_timer_set(KEY_SCAN_TASK, TASK_APP, 10);
			
			break;

			#ifdef LOCK_KEY_E
			case LOCK_KEY:
			{
				UART_PRINTF("LOCK_KEY \r\n");
				ke_timer_set(KEY_SCAN_TASK, TASK_APP, 10);
			}
			break;
			#endif

			#ifdef FEED_KEY_E
			case FEED_KEY:
			UART_PRINTF("FEED_KEY \r\n");
			ke_timer_set(KEY_SCAN_TASK, TASK_APP, 10);
			break;
			#endif

			default:
				break;
		}

		if(lock_flag == 0 && beep_flag == 0)
		{
			beep_test();
			beep_flag = 1;
		}
	
	}
	lock_timeout = LOCK_TIMEOUT_TIME;
	
	
#if defined KEY_DOWN_HIGHT
	jk_gpio_int_config(RECORD_KEY, INT_HIGH_EDGE);
	jk_gpio_int_config(SET_KEY, INT_HIGH_EDGE);
	jk_gpio_int_config(DOWN_KEY, INT_HIGH_EDGE);
	jk_gpio_int_config(UP_KEY, INT_HIGH_EDGE);
	
	#ifdef FEED_KEY_E
	jk_gpio_int_config(FEED_KEY, INT_HIGH_EDGE);
	#endif
	#ifdef LOCK_KEY_E
	jk_gpio_int_config(LOCK_KEY, INT_HIGH_EDGE);
	#endif
#else
	jk_gpio_int_config(RECORD_KEY, INT_LOW_EDGE);
	jk_gpio_int_config(SET_KEY, INT_LOW_EDGE);
	jk_gpio_int_config(DOWN_KEY, INT_LOW_EDGE);
	jk_gpio_int_config(UP_KEY, INT_LOW_EDGE);
	
	#ifdef FEED_KEY_E
	jk_gpio_int_config(FEED_KEY, INT_LOW_EDGE);
	#endif
	#ifdef LOCK_KEY_E
	jk_gpio_int_config(LOCK_KEY, INT_LOW_EDGE);
	#endif
#endif
	
	jk_gpio_int_en();

}

void gpio_init(void)
{
	//output
	gpio_config(MOTOR_PIN_P, OUTPUT, PULL_NONE);
	gpio_config(SOUND_PLAY, OUTPUT, PULL_NONE);
	gpio_config(SOUND_REC, OUTPUT, PULL_NONE);
	
	gpio_config(HT1621_DAT, OUTPUT, PULL_NONE);
	gpio_config(HT1621_WR, OUTPUT, PULL_NONE);
	gpio_config(HT1621_CS, OUTPUT, PULL_NONE);
	gpio_set(BL_EN, 0);
	gpio_config(BL_EN, OUTPUT, PULL_LOW);
	gpio_set(HT1621_DAT, 1);
	gpio_set(HT1621_WR, 1);
	gpio_set(HT1621_CS, 1);
	gpio_set(BL_EN, 0);

	gpio_set(MOTOR_PIN_P, 0);
	gpio_set(SOUND_PLAY, PLAYER_OFF);
	gpio_set(SOUND_REC, RECORD_OFF);

	#ifndef NO_LED_E
	gpio_config(LINK_LED, OUTPUT, PULL_NONE);
	gpio_config(PWR_LED, OUTPUT, PULL_NONE);
	led_control(LED_OFF,LED_OFF,0);
	#endif
	
	//input
	gpio_config(MOTOR_DET, INPUT, PULL_HIGH);
	gpio_config(CHARGE_DET, INPUT, PULL_NONE);

	#ifdef MOTOR_REVERSE
	gpio_config(MOTOR_PIN_N, OUTPUT, PULL_NONE);
	gpio_set(MOTOR_PIN_N, 0);
	#endif

	gpio_config(BUZZER_EN, OUTPUT, PULL_LOW);
	gpio_set(BUZZER_EN, 0);

	//interrupt
#if defined KEY_DOWN_HIGHT
	gpio_config(RECORD_KEY, INPUT, PULL_LOW);
	gpio_config(SET_KEY, INPUT, PULL_LOW);
	gpio_config(DOWN_KEY, INPUT, PULL_LOW);
	gpio_config(UP_KEY, INPUT, PULL_LOW);

	jk_gpio_int_config(RECORD_KEY, INT_HIGH_EDGE);
	jk_gpio_int_config(SET_KEY, INT_HIGH_EDGE);
	jk_gpio_int_config(DOWN_KEY, INT_HIGH_EDGE);
	jk_gpio_int_config(UP_KEY, INT_HIGH_EDGE);

	#ifdef FEED_KEY_E
	gpio_config(FEED_KEY, INPUT, PULL_LOW);
	jk_gpio_int_config(FEED_KEY, INT_HIGH_EDGE);
	#endif
	#ifdef LOCK_KEY_E
	gpio_config(LOCK_KEY, INPUT, PULL_LOW);
	jk_gpio_int_config(LOCK_KEY, INT_HIGH_EDGE);
	#endif

#else
	gpio_config(RECORD_KEY, INPUT, PULL_HIGH);
	gpio_config(SET_KEY, INPUT, PULL_HIGH);
	gpio_config(DOWN_KEY, INPUT, PULL_HIGH);
	gpio_config(UP_KEY, INPUT, PULL_HIGH);

	jk_gpio_int_config(RECORD_KEY, INT_LOW_EDGE);
	jk_gpio_int_config(SET_KEY, INT_LOW_EDGE);
	jk_gpio_int_config(DOWN_KEY, INT_LOW_EDGE);
	jk_gpio_int_config(UP_KEY, INT_LOW_EDGE);
	
	#ifdef FEED_KEY_E
	gpio_config(FEED_KEY, INPUT, PULL_HIGH);
	jk_gpio_int_config(FEED_KEY, INT_LOW_EDGE);
	#endif
	#ifdef LOCK_KEY_E
	gpio_config(LOCK_KEY, INPUT, PULL_HIGH);
	jk_gpio_int_config(LOCK_KEY, INT_LOW_EDGE);
	#endif
#endif
	
	jk_gpio_int_en();
	
	gpio_cb_register(gpio_irq_handler);
	
}


void gpio_triger(uint8_t gpio)
{
	gpio_set(gpio, 1);
	gpio_set(gpio, 0);
}


void gpio_cb_register(GPIO_INT_CALLBACK_T cb)
{
	if(cb)
	{
		gpio_int_cb = cb;
	}
}

void gpio_isr(void)
{
	REG_APB5_GPIO_WUATOD_ENABLE = 0x00000000; 
    REG_AHB0_ICU_INT_ENABLE &= (~(0x01 << 9));
    //gpio_triger(0x14);
//    UART_PRINTF("1\r\n");

	//triger int callback
	if(gpio_int_cb)
	{
//		UART_PRINTF("pin111:%X\r\n",REG_APB5_GPIO_WUATOD_STAT);
		(*gpio_int_cb)(REG_APB5_GPIO_WUATOD_STAT);
	}
	REG_APB5_GPIO_WUATOD_STAT = 0xffffffff;
}

void gpio_test_init(void)
{
	gpio_config(GPIOC_0, FLOAT, PULL_HIGH);
	gpio_config(GPIOC_0, FLOAT, PULL_HIGH);
	gpio_config(GPIOC_0, FLOAT, PULL_HIGH);
	gpio_config(GPIOC_0, FLOAT, PULL_HIGH);
	gpio_config(GPIOC_0, FLOAT, PULL_HIGH);
}

void gpio_sleep(void)
{
	GPIO_CFG_BACKUP[0] = REG_APB5_GPIOA_CFG;
	GPIO_CFG_BACKUP[1] = REG_APB5_GPIOB_CFG;
	GPIO_CFG_BACKUP[2] = REG_APB5_GPIOC_CFG;
	GPIO_CFG_BACKUP[3] = REG_APB5_GPIOD_CFG;
	GPIO_CFG_BACKUP[4] = REG_APB5_GPIOE_CFG;
	
	GPIO_DATA_BACKUP[0] = REG_APB5_GPIOA_DATA;
	GPIO_DATA_BACKUP[1] = REG_APB5_GPIOB_DATA;
	GPIO_DATA_BACKUP[2] = REG_APB5_GPIOC_DATA;
	GPIO_DATA_BACKUP[3] = REG_APB5_GPIOD_DATA;
	GPIO_DATA_BACKUP[4] = REG_APB5_GPIOE_DATA;

	
	REG_APB5_GPIOA_CFG = 0x0000ffff;
	REG_APB5_GPIOA_DATA = 0x0000ffff;
	
	REG_APB5_GPIOB_CFG = 0x0000ffff;
	REG_APB5_GPIOB_DATA = 0x0000ffff;
	
	REG_APB5_GPIOC_CFG = 0x0000ffff;
	REG_APB5_GPIOC_DATA = 0x0000ffff;
	
	REG_APB5_GPIOD_CFG = 0x0000ffff;
	REG_APB5_GPIOD_DATA = 0x0000ffff;
	
	REG_APB5_GPIOE_CFG = 0x0000ffff;
	REG_APB5_GPIOE_DATA = 0x0000ffff;
}

void gpio_wakeup(void)
{
	REG_APB5_GPIOA_CFG = GPIO_CFG_BACKUP[0];
	REG_APB5_GPIOB_CFG = GPIO_CFG_BACKUP[1];
	REG_APB5_GPIOC_CFG = GPIO_CFG_BACKUP[2];
	REG_APB5_GPIOD_CFG = GPIO_CFG_BACKUP[3];
	REG_APB5_GPIOE_CFG = GPIO_CFG_BACKUP[4];
	
	REG_APB5_GPIOA_DATA = GPIO_DATA_BACKUP[0];
	REG_APB5_GPIOB_DATA = GPIO_DATA_BACKUP[1];	
	REG_APB5_GPIOC_DATA = GPIO_DATA_BACKUP[2];
	REG_APB5_GPIOD_DATA = GPIO_DATA_BACKUP[3];
	REG_APB5_GPIOE_DATA = GPIO_DATA_BACKUP[4];
}



void DEBUG_MSG(uint8_t x)
{

#if (GPIO_DBG_MSG)    
    REG_APB5_GPIOD_DATA &= 0xffffffdf;
    REG_APB5_GPIOB_DATA = x&0xFF;
    
    if(x&0x20)
        REG_APB5_GPIOD_DATA |= 0X02;
    else
        REG_APB5_GPIOD_DATA &= ~0X02;
        
    REG_APB5_GPIOD_DATA |= 0x00000020;

#else
    return;
#endif
}




