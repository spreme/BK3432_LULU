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
//#################################### 型号定义 ##########################################//
//#######################################################################################//
//#define LNH_01					1		//廖工-前海一凡按键板
#define PT01K_BK				1		//小派按键带屏版

#if defined LNH_01
#define USER_VERSION 	"LNH_01.0.2"
#define USER_DATA 		"20210721"

//	#define RTC_TIME			1		//外部时钟计时
	#define BATTERY_CHAN		1		//ADC检测电池电量的通道口
	#define FEED_ONE_DELAY 		100		//喂食一份后延时停止电机时间
	#define LOCK_KEY_E			1		//锁键按键
	#define FEED_KEY_E			1		//喂食按键
	#define FEED_KEY_MUSIC_E	1		//按键喂食播音
	#define NO_LED_E			1		//无灯
	#define BL_CONTROL_E		1		//背光灯控制

	#define MOTOR_REVERSE		1		//电机反转功能
	#define UART_2_INIT			1		//串口2初始化
	#define UART_2_PRINTF		1		//串口2打印
//	#define UART_1_INIT			1		//串口1初始化
//	#define UART_1_PRINTF		1		//串口1打印

	#define MOTOR_DELAY			Delay_ms(1000)			//喂食结束延时停止电机时间
#endif

#if defined PT01K_BK
#define USER_VERSION 	"PT01K_BK.0.3"
#define USER_DATA 		"20210729"

//	#define RTC_TIME			1		//外部时钟计时
	#define BATTERY_CHAN		1		//ADC检测电池电量的通道口
	#define FEED_ONE_DELAY 		100		//喂食一份后延时停止电机时间
	#define BACKLIGHT_CONTROL	1		//背光灯控制亮度

	#define MOTOR_REVERSE		1		//电机反转功能
	#define UART_2_INIT			1		//串口2初始化
	#define UART_2_PRINTF		1		//串口2打印
//	#define UART_1_INIT			1		//串口1初始化
//	#define UART_1_PRINTF		1		//串口1打印

	#define NEW_RECORD_IC		1		//新录音IC，低电平有效


	#define MOTOR_DELAY			Delay_ms(100)			//喂食结束延时停止电机时间
#endif

//******************************* 可用宏定义 ***********************************************//
#if 0
	#define RTC_TIME			1		//外部时钟计时
	#define BATTERY_CHAN		1		//ADC检测电池电量的通道口
	#define LOCK_KEY_E			1		//锁键按键
	#define FEED_KEY_E			1		//喂食按键
	#define MOTOR_REVERSE		1		//电机反转功能
	#define BACKLIGHT_CONTROL	1		//背光灯控制亮度
	#define NO_LED_E			1		//无灯
	
	#define UART_2_INIT			1		//串口2初始化
	#define UART_2_PRINTF		1		//串口2打印
	#define UART_1_INIT			1		//串口1初始化
	#define UART_1_PRINTF		1		//串口1打印

	#define FEED_ONE_DELAY 		100		//喂食一份后延时停止电机时间
	#define MOTOR_DELAY			Delay_ms(1000)			//喂食结束延时停止电机时间
#endif
//******************************* 可用宏定义 ***********************************************//


 /******************************************************************************
  *############################################################################*
  * 							SYSTEM MACRO CTRL                              *
  *############################################################################*
  *****************************************************************************/

//如果需要使用GPIO进行调试，需要打开这个宏
#define GPIO_DBG_MSG					0
//UART使能控制宏
#define UART_PRINTF_EN					1
//蓝牙硬件调试控制
#define DEBUG_HW						0

/*******************************************************************************
 *#############################################################################*
 *								APPLICATION MACRO CTRL                         *
 *#############################################################################*
 *******************************************************************************/
 
//连接参数更新控制
#define UPDATE_CONNENCT_PARAM  			1

//最小连接间隔
#define BLE_UAPDATA_MIN_INTVALUE		20
//最大连接间隔 
#define BLE_UAPDATA_MAX_INTVALUE		40
//连接Latency
#define BLE_UAPDATA_LATENCY				0
//连接超时
#define BLE_UAPDATA_TIMEOUT				600

//设备名称
#define APP_DFLT_DEVICE_NAME           ("BK3432-GATT")

 //广播包UUID配置
#define APP_FFC0_ADV_DATA_UUID        "\x03\x03\xF0\xFF"
#define APP_FFC0_ADV_DATA_UUID_LEN    (4)

//扫描响应包数据
#define APP_SCNRSP_DATA        "\x0c\x08\x42\x4B\x33\x34\x33\x32\x2D\x47\x41\x54\x54" //BK3432-GATT"
#define APP_SCNRSP_DATA_LEN     (13)

//广播参数配置
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

//bk3432用户存储有3个nvr区域，每个区域512字节，区域的首地址分别是0x80，0x8000，0x8080


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
	uint32_t rtc_hour;					//时间小时
	uint32_t rtc_minute;				//时间分钟
	uint32_t record_time;				//录音时长
	uint32_t led_backlight_pwm;			//背光灯亮度		
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

extern uint8_t lock_flag;				//设备锁标志
extern uint8_t key_flag;				//按键触发类型
extern uint8_t reset_flag;				//复位标志
extern uint8_t key_lock;				//锁按键标志
extern uint8_t beep_flag;				//蜂鸣器响标志
extern uint8_t keep_dowm_flag;			//按键长按标志

extern uint32_t lock_timeout;			//锁屏超时时间
extern uint32_t rtc_timestamp;			//时间戳时间

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
