#include "motor.h"
#include "user_config.h"
#include "user_gpio.h"
#include "wdt.h"

uint8_t feed_required = 0;			//执行喂食类型
FEED_INFO_t  feed_info_func;		//准备喂食的喂食数据
uint8_t feed_detect_again = 0;		//再次喂食标志
uint8_t check_feed_flag = 0;		//检测喂食计划标志
uint8_t feed_one_flag = 0;			//按键喂食标志


char food_count = 0;
uint16_t time = 0;

uint8_t door_open = 0;				//关门错误标志（=2 关门出错，停止关门）

uint8_t feed_error 		= 0;			//喂食错误标志位：		1（电机超时）2（）3（红外错误）
uint8_t task_run_led = 0;
uint8_t task_run_motor = 0;				//喂食中标志位1（喂食中） 0（空闲）



void motor_contorl(enum MOTOR_CONTROL_E motor_c)
{
//	printf("motor_contorl:%bd\n",motor_c);
	switch(motor_c)
	{
		case MOTOR_PULL:
		{
			gpio_set(MOTOR_PIN_P, 1);
			#ifdef MOTOR_REVERSE
			gpio_set(MOTOR_PIN_N, 0);
			#endif
		}
		break;
		
		case MOTOR_PUSH:
		{
			gpio_set(MOTOR_PIN_P, 0);
			#ifdef MOTOR_REVERSE
			gpio_set(MOTOR_PIN_N, 1);
			#endif
		}
		break;
		
		case MOTOR_STOP:
		{
			gpio_set(MOTOR_PIN_P, 0);
			#ifdef MOTOR_REVERSE
			gpio_set(MOTOR_PIN_N, 0);
			#endif
		}
		break;
		
		default:
			break;			
	}
}

#ifdef DOOR_CONTROL
void door_motor_contorl(enum DOOR_CONTROL_E door_c)
{
	switch(door_c)
	{
		case DOOR_PULL:
		{
			gpio_set(MOTOR_DOOR_G, 1);
			gpio_set(MOTOR_DOOR_B, 0);
		}
		break;
		
		case DOOR_PUSH:
		{
			gpio_set(MOTOR_DOOR_G, 0);
			gpio_set(MOTOR_DOOR_B, 1);
		}
		break;
		
		case DOOR_STOP:
		{
			gpio_set(MOTOR_DOOR_G, 0);
			gpio_set(MOTOR_DOOR_B, 0);
		}
		break;
		
		default:
			break;			
	}
}

uint8_t motor_door_open()
{
	uint16_t timeout = 400;
	
//	gpo_set(IR_LED_CTRL);
	task_run_motor = 1;
	door_open = 0;

	//到位不在前到位位置		开门(门仓到位，压着为高电平)
	if(DOOR_OPEN_DET_ON)
	{
		door_motor_contorl(DOOR_PULL);
	}

	WDT_time = 0;
	while(timeout)
	{
		if(DOOR_OPEN_DET_OFF)				//到达开门到位位置
		{
			Delay_ms(10);
			if(DOOR_OPEN_DET_OFF)
			{
				door_motor_contorl(DOOR_STOP);
				printf("door open success\r\n");
				break;
			}				
		}
		Delay_ms(10);
		
		timeout--;
		if(timeout == 0)
		{
			door_motor_contorl(DOOR_STOP);
			printf("open door timeout!!!\r\n");
			return 1;
		}
	}
	
	door_motor_contorl(DOOR_STOP);
	return 0;
}

uint8_t motor_door_close()
{
	uint16_t timeout = 1300;
		
//	gpo_set(IR_LED_CTRL);
	task_run_motor = 1;

	//到位不在后到位位置		关门
	if(DOOR_CLOSE_DET_ON)
	{
		door_motor_contorl(DOOR_PUSH);
	}
	
	WDT_time = 0;
	while(timeout)
	{
		#ifdef IR_FEED_TUBE
		if(IR_DET)				//落粮对管堵塞
		{
			motor_door_open();
			Delay_ms(1000);
			
			if(IR_DET)			//落粮对管堵塞
			{
				break;
			}
			else
			{
				door_motor_contorl(DOOR_PUSH);
			}
		}		
		#endif
		
		if(DOOR_CLOSE_DET_OFF)				//到达后到位位置
		{
			Delay_ms(10);
			if(DOOR_CLOSE_DET_OFF)
			{
				door_motor_contorl(DOOR_STOP);	
				printf("close door success!!!\r\n");
				break;
			}
		}
		
		if(timeout == 800)
		{
			printf("close door error first time, open the door\r\n");			
			motor_door_open();
			Delay_ms(1000);
			timeout = 790;
			door_motor_contorl(DOOR_PUSH);
		}
		
		if(timeout == 300)
		{
			printf("close door error second time, open the door until next feed time\r\n");			
			motor_door_open();
			door_open = 2;
			task_run_motor = 0;
			return 1;
		}
		
		Delay_ms(10);
		timeout--;
	}
	WDT_time = 0;
	
	door_motor_contorl(DOOR_STOP);
	task_run_motor = 0;

	return 0;
}
#endif



uint16_t motor_run(void)
{
	uint16_t timeout = MOTO_TIMEOUT; //用于判断电机超时
	uint16_t i = 0;
	uint16_t key_cont = 0;
#ifdef MOTOR_DET_ADDR
	uint8_t feed_det_flag = 0;
#endif	
	#ifdef MOTOR_REVERSE
	static uint8_t reverse_flag = 0;
	#endif
	
	//启动电机
	motor_contorl(MOTOR_PULL);

	//先让电机转过一定角度，退出到位开关压着的状态
	key_cont = 0;
	i = 200;

	while(i)
	{
		Delay_ms(10);

		i--;
		if(i == 0)
		{
			timeout = 0;
		}
		if(gpio_get_input(MOTOR_DET))
		{
			break;
		}
	}

	wdt_enable(0xffff);
	//等待电机中断(下降沿触发)到来，中断到来后判断中断引脚电平是否为低(并有延时去抖)
	//低表示限位开关还被电机的旋转机构压着，这时电机停止转动
	while(timeout)
	{
		if(gpio_get_input(MOTOR_DET) == 0) //喂食电机到位
		{			
			Delay_ms(10);	//延时去抖
			
			if(gpio_get_input(MOTOR_DET) == 0)
			{
				key_cont++;
				if(key_cont > 6)
				{
					UART_PRINTF("feed det and out \r\n");
					key_cont = 0;
					wdt_enable(0xffff);
//					reverse_flag = 0;
					#ifndef MOTOR_DET_ADDR
					UART_PRINTF("timeout:%d \r\n",timeout);
					return timeout;
					#else
					feed_det_flag = 1;
					#endif
				}
			}
		}
		#ifdef MOTOR_DET_ADDR
		if(feed_det_flag && gpio_get_input(MOTOR_DET))
		{
			feed_det_flag = 0;
			return timeout;
		}
		#endif
		
		Delay_ms(10);
		timeout--;
	}
	
	#ifdef MOTOR_REVERSE
	if(timeout == 0 && reverse_flag == 0)
	{
		reverse_flag++;
		motor_contorl(MOTOR_PUSH);
		Delay_ms(1000);
		motor_contorl(MOTOR_PULL);
		Delay_ms(500);
		motor_contorl(MOTOR_PUSH);
		Delay_ms(500);
		motor_contorl(MOTOR_PULL);
		Delay_ms(500);
		motor_contorl(MOTOR_PUSH);
		Delay_ms(500);
		motor_contorl(MOTOR_PULL);
//		motor_run();
		return 0xfe;
	}
	else
	{
		reverse_flag = 0;
	}
	#endif
	
	return timeout;

}

#ifdef IR_FEED_TUBE
uint16_t motor(uint32_t count, uint8_t * err_type)
{
	uint8_t errs = 0;
	int feed_cont = count;		//用于判断电机转动次数，每转动一次，喂食一份（大概10g）
	int timeout = MOTO_TIMEOUT; //用于判断电机超时
	int foods;
	int feed_already = count;		//时间喂食份数（大概10g）
	uint8_t time_ir_out = 50;		//堵粮超时，连续堵粮5s
	
//	gpio_set(PWR_LED, LED_OFF);
	
	ledtime_flag = 1;
	task_run_led = 0;
	
	task_run_motor = 1;
	Delay_ms(40);
	*err_type = 0;
	
	P0EXTIE |=  0x08;			//开启红外中断
	
	wdt_enable(0xffff);
	//判断是否完成所有的喂食分量，电机每转动一次喂食一份（大概10g）
	while(feed_cont > 0)
	{
again:
		food_count = 0; 		//饲料计数清零
		WDT_time = 0;
		
		if(IR_DET == 0)
			timeout = motor_run();
		else
		{
			feed_cont++;
			feed_already++;
			timeout = 5;
		}
		//电机超时，电机没转或到位开关探测错误(到位开关没中断)
		if(timeout == 0)
		{
			printf("motor not move or det err\r\n");
			*err_type = ERROR_MOTOR_TIMEOUT;
			task_run_led = 1;
			goto err;
		}

		//延时让食物充分掉落
		foods = food_count;
//		timeout = 5;

//		while(timeout)
//		{
//			Delay_ms(200);

//			if(food_count == foods) break;
//			timeout--;
//		}
		MOTOR_DELAY;
		
		feed_cont--;
		feed_already--;

//		if(food_count == 0 && task_error_ir == 0)
#ifdef	IR_CHECK_FOOD	
		if(food_count == 0)
			feed_already++;
		
		//食物空(食物计数0 且红外对管导通)余粮不足
		if((food_count == 0) && (IR_DET == 0))
		{
			*err_type = ERROR_EMPTY;
			errs++;

			if(errs <= 4)
			{
				*err_type = 0;
				feed_cont++;
				
				printf("FOOD not enought\r\n");
				printf("feed_cont:%bd - count:%bd \r\n", feed_cont, count);
				
				goto again;
			}
			else
			{
				printf("FOOD not enought \r\n");
				feed_error = ERROR_EMPTY;
				feed_cont++;
				*err_type = ERROR_EMPTY;
				errs = 0;
				goto err;
			}
		}
#endif		


		//红外对管被阻挡（送出的食物堆满托盘）
		if(IR_DET)
		{
//			task_error_ir = 1;
			printf("begin ir error\r\n");

			time_ir_out = 50;
			while(time_ir_out)
			{
				motor_contorl(MOTOR_STOP);
				time_ir_out--;
				Delay_ms(100);
				
				if(IR_DET == 0)
					break;
				else
				{
					if(time_ir_out == 0)
					{
						printf("ir error \r\n");
						*err_type = ERROR_IR;
						feed_error = ERROR_IR;
						goto err;
					}
				}
			}							
		}

		if(*err_type)
		{
			if(feed_cont != 0)
			{
				errs++;

				if(errs < 2)
				{
					*err_type = 0;
					goto again;
				}
				else
				{
					goto err;
				}
			}
		}

		//成功喂食一份
		PWR_LED = LED_OFF;
		*err_type = 0;
		errs = 0;
		feed_error = 0;
		wdt_enable(0xffff);
	}
err:
	P0EXTIE &=	~0x08;			//关闭红外中断
	WDT_time = 0;
	
	#ifdef IR_FOOD_TUBE
	if(food_empty_flag > 0)
	{
		*err_type = ERROR_NOT_ENOUGH;
		feed_error = ERROR_NOT_ENOUGH;
	}
	#endif

	//停止电机
	motor_contorl(MOTOR_STOP);
//	IR_LED = 0;
	
	#ifndef DOOR_CONTROL
	task_run_motor = 0;			//喂食中标志位1（喂食中） 0（空闲）
	#endif
	
	PWR_LED = LED_OFF;
	feed_detect_sw = 1;			//喂食完（5分钟）标志位
	task_feed_sw = 0;			//喂食完（5分钟）计时时间清0
	ledtime_flag = 0;			//喂食错误红灯闪烁1分钟标志
	
	return (count - feed_already);
}
#else
uint16_t motor(uint32_t count, uint8_t * err_type)
{
//	int foods;
	int feed_cont = count;		//用于判断电机转动次数，每转动一次，喂食一份（大概10g）
	uint16_t timeout = MOTO_TIMEOUT; //用于判断电机超时
	int feed_already = count;		//时间喂食份数（大概10g）
	
//	gpio_set(PWR_LED, LED_OFF);
	
	task_run_led = 0;
	task_run_motor = 1;
	*err_type = 0;

	wdt_enable(0xffff);
	//判断是否完成所有的喂食分量，电机每转动一次喂食一份（大概10g）
	while(feed_cont > 0)
	{		
		timeout = motor_run();

		UART_PRINTF("timeout:%d\r\n",timeout);
		//电机超时，电机没转或到位开关探测错误(到位开关没中断)
		if(timeout == 0)
		{
			UART_PRINTF("motor not move or det err\r\n");
			*err_type = ERROR_MOTOR_TIMEOUT;
			task_run_led = 1;
			goto err;
		}
		else if(timeout == 0xfe)
		{
			feed_cont++;
			feed_already++;
		}

		MOTOR_DELAY;
		feed_cont--;
		feed_already--;
//		UART_PRINTF("feed_cont--: feed_cont=%d \n",feed_cont);
//		UART_PRINTF("feed_already:%d \n",feed_already);

		//成功喂食一份
//		gpio_set(PWR_LED, LED_OFF);
		*err_type = 0;
		feed_error = 0;
		wdt_enable(0xffff);
	}
err:
	#ifdef IR_FOOD_TUBE
	if(food_empty_flag > 0)
	{
		*err_type = ERROR_NOT_ENOUGH;
		feed_error = ERROR_NOT_ENOUGH;
	}
	#endif
	
	//停止电机
	motor_contorl(MOTOR_STOP);
	
	#ifndef DOOR_CONTROL
	task_run_motor = 0;			//喂食中标志位1（喂食中） 0（空闲）
	#endif
	
//	gpio_set(PWR_LED, LED_OFF);
	
	return (count - feed_already);
}

#endif

//执行喂食：按键手动 手动 定时
char feed_run(FEED_INFO_t * info)
{
	uint16_t weight = 0;
	uint8_t weight_already = 0;
	uint8_t res;
	uint8_t i ;

	weight = info->weight;
	UART_PRINTF("info->weight:%d\n",weight);
	
	if(weight == 0)		//喂食份数为0，退出喂食函数
	{
		return 0;
	}

	wdt_enable(0xffff);
	if(info->music)
	{
		play_record_control();
	
		i = save_info.record_time;			//10s
		if(i > 110)
		{
			i = 0;
		}
	}
	wdt_enable(0xffff);

	while(i)
	{
		i--;
		Delay_ms(100);				//按下单次喂食键后，等一会电机才会转
//		Delay_ms(50);			
	}
	
	#ifdef DOOR_CONTROL
	motor_door_open();				//开仓门
	#endif
	
	//启动电机
	weight_already = motor(weight, &res);

	UART_PRINTF("feeds = %d		already = %d\r\n",weight, weight_already);

	info->weight = (uint8_t) (weight - weight_already);
		
	return res;
}


//扫描eeprom判断是否需要执行定时喂食
void feed_scan(FEED_INFO_t * info)
{
	uint8_t i;
//	uint32_t t_now = 0;
//	tuya_ble_time_struct_data_t t_struct;
	UTCTimeStruct tm_s;
	
	flash_read(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR, sizeof(FEED_PLAN_t), (uint8_t *)&feed_plan);
	printf_flash_info();
//	memset(&t_struct, 0, sizeof(tuya_ble_time_struct_data_t));
	memset(&tm_s, 0, sizeof(UTCTimeStruct));
		
	utc_get_time(&tm_s);
//	utc_update();
//	t_now = utc_get_clock();
//	tuya_ble_utc_sec_2_mytime(t_now, &t_struct, 0);

	UART_PRINTF("%04d-%02d-%02d %02d:%02d:%02d \r\n",
				tm_s.year, tm_s.month, tm_s.day,
				tm_s.hour, tm_s.minutes, tm_s.seconds);

	for(i = 0; i < FEED_MAX_NUM; i++)
	{
		//如果重量为0（数据有问题）
		if(feed_plan.plans[i].weight == 0)
		{
//			printf("continue:%bd\r\n",i);
			continue;
		}

		if(feed_plan.plans[i].hour == tm_s.hour)
		{
			if(feed_plan.plans[i].minute == tm_s.minutes)
			{
				*info = feed_plan.plans[i];
				info->music = 1;
				feed_required = FEED_AUTO;
				
				UART_PRINTF("goto autofeed: week:\r\n");
				break;
			}
		}
	}
	
//	if(rtc_date_change == 2)
//		rtc_date_change = 1;
//	else
//		rtc_date_change = 0;	
}
