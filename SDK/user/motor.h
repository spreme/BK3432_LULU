#ifndef __MOTOR_H
#define __MOTOR_H

#include "user_config.h"


extern uint16_t motor(uint32_t count, uint8_t * err_type);	//喂食函数
extern char feed_run(FEED_INFO_t * info);
extern void feed_scan(FEED_INFO_t * info);


extern uint8_t feed_error;				//喂食错误 1(堵粮) 2（设备异常） 4（无粮）
extern uint8_t feed_required;			//执行喂食类型
extern FEED_INFO_t  feed_info_func;		//准备喂食的喂食数据
extern uint8_t feed_detect_again;		//再次喂食标志
extern uint8_t check_feed_flag;			//检测喂食计划标志
extern uint8_t feed_one_flag;			//按键喂食标志

extern char food_count;					//红外检测粮食下落份数
extern uint8_t door_open;				//关门错误标志（=2 关门出错，停止关门）







#endif

