/**
 *******************************************************************************
 *
 * @file user_config.h
 *
 * @brief Application configuration definition
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *******************************************************************************
 */
 
#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

#include "stdint.h"
#include "utc_clock.h"
#include "stddef.h"
#include "string.h"
#include "tuya_ble_unix_time.h"


 //#######################################################################################//
//#################################### �ͺŶ��� ##########################################//
//#######################################################################################//
//#define LNH_01					1		//�ι�-ǰ��һ��������
#define PT01K_BK				1		//С�ɰ���������

#if defined LNH_01
#define USER_VERSION 	"LNH_01.0.2"
#define USER_DATA 		"20210721"

//	#define RTC_TIME			1		//�ⲿʱ�Ӽ�ʱ
	#define BATTERY_CHAN		1		//ADC����ص�����ͨ����
	#define FEED_ONE_DELAY 		100		//ιʳһ�ݺ���ʱֹͣ���ʱ��
	#define LOCK_KEY_E			1		//��������
	#define FEED_KEY_E			1		//ιʳ����
	#define FEED_KEY_MUSIC_E	1		//����ιʳ����
	#define NO_LED_E			1		//�޵�
	#define BL_CONTROL_E		1		//����ƿ���

	#define MOTOR_REVERSE		1		//�����ת����
	#define UART_2_INIT			1		//����2��ʼ��
	#define UART_2_PRINTF		1		//����2��ӡ
//	#define UART_1_INIT			1		//����1��ʼ��
//	#define UART_1_PRINTF		1		//����1��ӡ

	#define MOTOR_DELAY			Delay_ms(1000)			//ιʳ������ʱֹͣ���ʱ��
#endif

#if defined PT01K_BK
#define USER_VERSION 	"PT01K_BK.0.3"
#define USER_DATA 		"20210729"

//	#define RTC_TIME			1		//�ⲿʱ�Ӽ�ʱ
	#define BATTERY_CHAN		1		//ADC����ص�����ͨ����
	#define FEED_ONE_DELAY 		100		//ιʳһ�ݺ���ʱֹͣ���ʱ��
	#define BACKLIGHT_CONTROL	1		//����ƿ�������

	#define MOTOR_REVERSE		1		//�����ת����
	#define UART_2_INIT			1		//����2��ʼ��
	#define UART_2_PRINTF		1		//����2��ӡ
//	#define UART_1_INIT			1		//����1��ʼ��
//	#define UART_1_PRINTF		1		//����1��ӡ

	#define NEW_RECORD_IC		1		//��¼��IC���͵�ƽ��Ч


	#define MOTOR_DELAY			Delay_ms(100)			//ιʳ������ʱֹͣ���ʱ��
#endif

//******************************* ���ú궨�� ***********************************************//
#if 0
	#define RTC_TIME			1		//�ⲿʱ�Ӽ�ʱ
	#define BATTERY_CHAN		1		//ADC����ص�����ͨ����
	#define LOCK_KEY_E			1		//��������
	#define FEED_KEY_E			1		//ιʳ����
	#define MOTOR_REVERSE		1		//�����ת����
	#define BACKLIGHT_CONTROL	1		//����ƿ�������
	#define NO_LED_E			1		//�޵�
	
	#define UART_2_INIT			1		//����2��ʼ��
	#define UART_2_PRINTF		1		//����2��ӡ
	#define UART_1_INIT			1		//����1��ʼ��
	#define UART_1_PRINTF		1		//����1��ӡ

	#define FEED_ONE_DELAY 		100		//ιʳһ�ݺ���ʱֹͣ���ʱ��
	#define MOTOR_DELAY			Delay_ms(1000)			//ιʳ������ʱֹͣ���ʱ��
#endif
//******************************* ���ú궨�� ***********************************************//


 /******************************************************************************
  *############################################################################*
  * 							SYSTEM MACRO CTRL                              *
  *############################################################################*
  *****************************************************************************/

//�����Ҫʹ��GPIO���е��ԣ���Ҫ�������
#define GPIO_DBG_MSG					0
//UARTʹ�ܿ��ƺ�
#define UART_PRINTF_EN					1
//����Ӳ�����Կ���
#define DEBUG_HW						0

/*******************************************************************************
 *#############################################################################*
 *								APPLICATION MACRO CTRL                         *
 *#############################################################################*
 *******************************************************************************/
 
//���Ӳ������¿���
#define UPDATE_CONNENCT_PARAM  			1

//��С���Ӽ��
#define BLE_UAPDATA_MIN_INTVALUE		20
//������Ӽ�� 
#define BLE_UAPDATA_MAX_INTVALUE		40
//����Latency
#define BLE_UAPDATA_LATENCY				0
//���ӳ�ʱ
#define BLE_UAPDATA_TIMEOUT				600

//�豸����
#define APP_DFLT_DEVICE_NAME           ("BK3432-GATT")

 //�㲥��UUID����
#define APP_FFC0_ADV_DATA_UUID        "\x03\x03\xF0\xFF"
#define APP_FFC0_ADV_DATA_UUID_LEN    (4)

//ɨ����Ӧ������
#define APP_SCNRSP_DATA        "\x0c\x08\x42\x4B\x33\x34\x33\x32\x2D\x47\x41\x54\x54" //BK3432-GATT"
#define APP_SCNRSP_DATA_LEN     (13)

//�㲥��������
/// Advertising channel map - 37, 38, 39
#define APP_ADV_CHMAP           (0x07)
/// Advertising minimum interval - 100ms (160*0.625ms)
#define APP_ADV_INT_MIN         (80)
/// Advertising maximum interval - 100ms (160*0.625ms)
#define APP_ADV_INT_MAX         (80)
/// Fast advertising interval
#define APP_ADV_FAST_INT        (32)

#define FEED_MAX_NUM	4
#define BLE_SAVE_ADDR	0x80
#define BLE_PLAN_ADDR	0x8000
#define FLASH_SIZE_ONE	512

//bk3432�û��洢��3��nvr����ÿ������512�ֽڣ�������׵�ַ�ֱ���0x80��0x8000��0x8080


enum FEED_STATUS_TYPE {
	FEED_STEP_NONE = 0,
	FEED_STEP_SOUND,
	FEED_STEP_READY,
	FEED_STEP_LEAVE_DET,
	FEED_STEP_WAIT_DET,
	FEED_STEP_OVER,
};

typedef struct {
	uint32_t mark;
	uint32_t rtc_hour;					//ʱ��Сʱ
	uint32_t rtc_minute;				//ʱ�����
	uint32_t record_time;				//¼��ʱ��
	uint32_t led_backlight_pwm;			//���������		
} SAVE_INFO_t;


typedef struct {
	uint8_t hour;
	uint8_t minute;
	uint8_t weight;
	uint8_t music;
} FEED_INFO_t;

typedef struct {
	uint32_t mark;
	FEED_INFO_t plans[FEED_MAX_NUM];
} FEED_PLAN_t;

extern FEED_PLAN_t feed_plan;
extern SAVE_INFO_t save_info;

extern uint8_t lock_flag;				//�豸����־
extern uint8_t key_flag;				//������������
extern uint8_t reset_flag;				//��λ��־
extern uint8_t key_lock;				//��������־
extern uint8_t beep_flag;				//���������־
extern uint8_t keep_dowm_flag;			//����������־

extern uint32_t lock_timeout;			//������ʱʱ��
extern uint32_t rtc_timestamp;			//ʱ���ʱ��

//void ht1621_set_dat(uint8_t addr, uint8_t val);
void beep_test(void);
extern uint8_t feed_status;

void printf_flash_info(void);
void flash_data_init(uint8_t type);
void led_control(uint8_t link_led, uint8_t red_led, uint8_t led_level);
void feed_error_led(void);

/*******************************************************************************
 *#############################################################################*
 *								DRIVER MACRO CTRL                              *
 *#############################################################################*
 ******************************************************************************/

//DRIVER CONFIG
#define UART_DRIVER						0
#define UART2_DRIVER					1
#define GPIO_DRIVER						1
#define AUDIO_DRIVER					0
#define RTC_DRIVER						1
#define ADC_DRIVER						1
#define I2C_DRIVER						0
#define PWM_DRIVER						1

#if RTC_DRIVER
#include "rtc.h"
#endif

#if ADC_DRIVER
#include "adc.h"
#endif

#if PWM_DRIVER
#include "pwm.h"
#endif

#include "string.h"

#include "gpio.h"
#include "rf.h"
#include "uart2.h"
#include "uart.h"
#include "flash.h"
#include "motor.h"
#include "nvds.h"

#include "lcd.h"
#include "function.h"
#include "user_gpio.h"


#endif /* _USER_CONFIG_H_ */
