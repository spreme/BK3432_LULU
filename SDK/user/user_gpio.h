
#ifndef _USER_GPIO_H_
#define _USER_GPIO_H_

#include "user_config.h"

//#######################################################################################//
//#################################### 引脚定义 ##########################################//
//#######################################################################################//

//##################################### PT01K_BK ##################################################//
#if defined PT01K_BK
	#define SOUND_PLAY		0x33				//录音芯片播放脚
	#define SOUND_REC		0x13				//录音芯片录音脚
	#define BUZZER_EN		0x12				//喂食电机正转
		
	#define MOTOR_PIN_P		0x10				//喂食电机正转
	#define MOTOR_PIN_N		0x11				//喂食电机反转
	#define MOTOR_DET		0x34				//喂食电机到位
	
	#define LINK_LED		0x07				//电源灯
	#define PWR_LED			0x06				//红灯

	#define UP_KEY			0x04				//上按键
	#define FEED_KEY		0x07				//喂食按键
	#define DOWN_KEY		0x05				//下按键
	#define SET_KEY			0x06				//设置按键
	#define RECORD_KEY		0x35				//录音按键
	#define OK_KEY			0x01				//确认按键
	#define LOCK_KEY		0x14				//锁按键

	#define STB 		0x00
	#define DIO 		0x03	
	#define CLK 		0x02	
	
//	#define BL_EN	 		0x01				//背光灯
//	#define BL_EN	 		0x14				//背光灯
		
	#define CHARGE_DET		0x32				//外电检测
	#define BAT_DET			0x31				//电池电量检测
#endif


//######################### 喂食类型 ####################################//
#define FEED_NEW 		0x1
#define FEED_AUTO 		0x1
#define FEED_OLD 		0x2
#define FEED_MANUAL 	0x3

//######################### 喂食错误类型 ####################################//
#define ERROR_MOTOR_TIMEOUT		0x01		//电机超时
#define ERROR_EMPTY				0x02		//无粮
#define ERROR_IR				0x04		//红外错误
#define ERROR_NOT_ENOUGH		0x08		//余粮不足
#define ERROR_KEY_COUNT			0x10		//按键次数达到最大值

//######################### 计时时间 ####################################//
#define LED_PERIOD 				40			//400ms	警报灯（红灯）闪烁时间
#define ERROR_PERIOD 			100			//1s	电机错误时间检测
#define FEED_AGAIN_PERIOD 		30000		//300s  喂食错误之后5分钟再试一次
#define MOTO_TIMEOUT			300			//3秒  	喂食电机检测到位超时时间

//######################### 灯状态 ####################################//
#ifdef LED_ON_HIGHT
	#define SET_LED_ON(led_pin) 		gpio_set(led_pin, 1)
	#define SET_LED_OFF(led_pin) 		gpio_set(led_pin, 0)
#else
	#define SET_LED_ON(led_pin) 		gpio_set(led_pin, 0)
	#define SET_LED_OFF(led_pin) 		gpio_set(led_pin, 1)
#endif
enum LED_TYPE_E
{
	LED_NO,
	LED_ON,
	LED_OFF,
};

#ifdef NORMAL_DOOR_DET				//普通到位
#define DOOR_OPEN_DET_OFF  				(gpio_read(IR_DET_D2) == 0)			//
#define DOOR_OPEN_DET_ON   				(gpio_read(IR_DET_D2))
#define DOOR_CLOSE_DET_OFF  			(gpio_read(IR_DET_D) == 0)
#define DOOR_CLOSE_DET_ON   			(gpio_read(IR_DET_D))
#else								//光栅到位
#define DOOR_OPEN_DET_OFF  				(gpio_read(IR_DET_D2))
#define DOOR_OPEN_DET_ON   				(gpio_read(IR_DET_D2) == 0)
#define DOOR_CLOSE_DET_OFF  			(gpio_read(IR_DET_D))
#define DOOR_CLOSE_DET_ON   			(gpio_read(IR_DET_D) == 0)
#endif

#ifdef NEW_RECORD_IC				//新录音ID ，低电平有效触发
#define RECORD_ON  					(0)			//
#define RECORD_OFF  				(1)			//
#define PLAYER_ON  					(0)			//
#define PLAYER_OFF  				(1)			//
#else								//旧录音IC，高电平有效触发
#define RECORD_ON  					(1)			//
#define RECORD_OFF  				(0)			//
#define PLAYER_ON  					(1)			//
#define PLAYER_OFF  				(0)			//
#endif

enum FOOD_STATE_E{
	LED_ENOUGH_FOOD 	= 0,		//粮桶粮食充足
    LED_NO_FOOD			= 1,		//粮桶无粮
    LED_NOT_ENOUGH_FOOD	= 2,		//粮桶粮食不多
};
//######################### 杂项 ####################################//
#define FEED_MAX_NUM 			10 			//定时喂食最大条数
#define FLASH_KEEP_VAL 			122 		//flash存储标识号
//######################### 电机功能定义 ####################################//
enum DOOR_CONTROL_E
{
	DOOR_PULL		= 0x1,			//推门
	DOOR_PUSH   	= 0x2,			//拉门
	DOOR_STOP	   	= 0x3			//停止
};
enum MOTOR_CONTROL_E
{
	MOTOR_PULL		= 0x4,			//电机正转
	MOTOR_PUSH   	= 0x5,			//电机反转
	MOTOR_STOP	   	= 0x6			//电机停止
};


//######################### 按键触发类型 ####################################//
enum KEY_TYPE_E{
	KEY_NO			= 0,		//无按键
    KEY_SET_S,					//按键短按
    KEY_UP_S,			//
    KEY_DOWN_S,			//
    KEY_RECORD_S,		//
    KEY_LOCK_S,			//
    KEY_FEED_S,			//	
	KEY_OK_S,			//
	
    KEY_SET_L,					//按键长按触发
    KEY_UP_L,			//
    KEY_DOWM_L,			//
    KEY_RECORD_L,		//
    KEY_LOCK_L,			//
    KEY_FEED_L,			//	
	KEY_OK_L,			//
	
    KEY_SET_L_UP,				//按键长按松开
    KEY_UP_L_UP,			//
    KEY_DOWN_L_UP,			//
    KEY_RECORD_L_UP,		//
    KEY_LOCK_L_UP,			//
    KEY_FEED_L_UP,			//	
	KEY_OK_L_UP,			//
	
};

#ifndef KEY_SHORT_TIME
#define KEY_SHORT_TIME			10			//按键短按时间	1s
#endif
#ifndef KEY_LONG_TIME
#define KEY_LONG_TIME			15			//按键长按时间	1.5s
#endif
#ifndef LOCK_TIMEOUT_TIME
#define LOCK_TIMEOUT_TIME		200			//无操作20s后锁屏
#endif
#ifndef KEY_LONG_TIME_SET
#define KEY_LONG_TIME_SET		30			//按键长按时间	3s
#endif




#endif /* _USER_GPIO_H_ */
