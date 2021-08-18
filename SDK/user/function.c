#include "function.h"
#include "wdt.h"
#include "rwip.h"

uint8_t lock_flag = 1;				//设备锁标志
uint8_t key_lock = 0;				//锁按键标志
uint8_t key_scan_flag = 0;			//正在检查按键标志
uint8_t key_flag = 0;				//按键触发类型
uint8_t reset_flag = 0;				//复位标志
uint8_t get_feed_info_flag = 0;		//查看喂食计划标志
uint8_t set_time_flag = 0;			//正在设置时间标志
uint8_t set_val_flag = 0;			//正在设置数值标志

uint32_t lock_timeout = LOCK_TIMEOUT_TIME;		//锁屏超时时间
uint32_t rtc_timestamp = 1;						//时间戳时间

	
uint8_t display_flag = 0;			//第一次显示标志(第一次显示第一位会导致后面显示异常，传数值进去显示一次第二位解决)
uint8_t display_val = 0;			//第一次显示值
uint8_t beep_flag = 0;				//蜂鸣器响标志
uint8_t keep_dowm_flag = 0;			//按键长按标志

uint8_t set_back_flag = 0;			//设置上一个标志

uint8_t key_scan(void)
{	
	uint8_t key_flag_send = 0;
	
	if(key_flag > 0)
	{
		key_scan_flag = 1;
		
////		set_key_tick = 0;				//设置按键计时
//		dowm_key_tick = 0;				//下按键计时
//		up_key_tick = 0;				//上按键计时
////		lock_key_tick = 0;				//锁键按键计时
////		feed_key_tick = 0;				//喂食按键计时

		key_flag_send = key_flag;
//		key_flag = 0;
		UART_PRINTF("!!!!!!!!!!!!!!! !!!!!!! key_scan:%d \r\n",key_flag_send);
	}
//	key_scan_flag = 0;
	return key_flag_send;
}

void clean_key_flag()
{
	key_flag = 0;
	key_scan_flag = 0;
	dowm_key_tick = 0;				//下按键计时
	up_key_tick = 0;				//上按键计时
	record_key_tick = 0;
}

int set_val(uint8_t addr, int* dat, char min, char max)
{
	char val = *dat;
	int status;
	uint8_t key_value;
	int timeout = 500;
	uint8_t reverse;
	uint8_t lcd_update_old = 0;
	uint8_t lcd_update_time = 0;

	UART_PRINTF("########## enter set_val \r\n");
	set_val_flag = 1;						//正在设置数值标志
	Delay_ms(200);
	while (timeout) 
	{
		rwip_schedule();
		wdt_enable(0x3fff);

//		MCU_IDLE();
//		UART_PRINTF("timeout:%d \r\n",timeout);

		key_value = key_scan();

		if (key_value) 
		{			
			lock_timeout = LOCK_TIMEOUT_TIME;
			clean_key_flag();
			timeout = 500;
			if(lock_flag == 0 && key_value < KEY_SET_L_UP)
			{
//				beep_test();
//				lock_timeout = LOCK_TIMEOUT_TIME;
				beep_flag = 0;
			}

			switch (key_value) 
			{
				case KEY_SET_S:
					*dat = val;
					status = val;
					set_back_flag = 2;
					goto back;

				case KEY_RECORD_S:
					*dat = val;
					status = val;
					set_back_flag = 1;
					goto back;
				
//				case KEY_ESC_S:
//					*dat = val;
//					status = -1;
//					goto back;
				
				case KEY_DOWM_L:
					beep_test();
				case KEY_DOWN_S:
					if(val > min)
						val--;
					else if(val == min)
						val = max;
					break;
					
				case KEY_UP_L:
					beep_test();
				case KEY_UP_S:
					val++;
					if (val > max)
						val = min;
					break;
			}
			key_flag = 0;
			key_scan_flag = 0;
		}
		
		if (lcd_update_old != lcd_update) 
		{
			lcd_update_old = lcd_update;
			if (reverse)
			{
				ht1621_disp_dat(addr, 0);
				if (max>9)
					ht1621_disp_dat(addr + 1, 0);
				reverse = 0;
			}
			else 
			{
				if (max>9)
				{
					ht1621_disp_dat(addr,     val/10 + 0x30);
					ht1621_disp_dat(addr + 1, val%10 + 0x30);
				}
				else
				{
					ht1621_disp_dat(addr, val + 0x30);
					if(display_flag)
					{
						display_flag = 0;
						ht1621_disp_dat(addr + 1, display_val + 0x30);
					}
				}
				reverse = 1;
			}
		}
		lcd_update_time++;
		if(lcd_update_time > 15)
		{
			lcd_update_time = 0;
			lcd_update++;
		}
		Delay_ms(10);
		timeout--;
	}
	status = val;
	set_back_flag = 3;
back:
	*dat = val;
	if (max>9)
	{
		ht1621_disp_dat(addr,     *dat/10 + 0x30);
		ht1621_disp_dat(addr + 1, *dat%10 + 0x30);
	}
	else
	{
		ht1621_disp_dat(addr, *dat + 0x30);
	}
	
	set_val_flag = 0;			//正在设置数值标志

	return status;
}

//获取时间显示
void get_time(void)
{
//	uint32_t t_zero = 0;
//	tuya_ble_time_struct_data_t t_struct;
	static uint8_t first_time = 0;
	UTCTimeStruct tm_s;
//	UTCTimeStruct tm_s_get;
	
	utc_get_time(&tm_s);

	if(first_time <= 1)
	{
//		utc_get_time(&tm_s);
		
		flash_read(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR, sizeof(SAVE_INFO_t), (uint8_t *) &save_info);

		UART_PRINTF("first_time: %02d-%02d %02d-%02d \r\n",tm_s.hour,tm_s.minutes, save_info.rtc_hour,save_info.rtc_minute);

		if(tm_s.hour == save_info.rtc_hour && tm_s.minutes == save_info.rtc_minute && first_time == 1)
		{
			first_time = 2;	
		}
		else
		{
	//		rtc_timestamp = (save_info.rtc_timestamp_H << 8) | save_info.rtc_timestamp_L;
	//		utc_set_clock(rtc_timestamp);
			tm_s.year = 1;
			tm_s.month = 1;
			tm_s.day = 1;
			tm_s.hour = 0;
			tm_s.minutes = 0;
			tm_s.seconds = 0;
			
			if(save_info.rtc_hour < 24 && save_info.rtc_minute < 60)
			{
				tm_s.hour = save_info.rtc_hour;
				tm_s.minutes = save_info.rtc_minute;
			}
			else
			{
				tm_s.hour = 0;
				tm_s.minutes = 0;
			}
			
			utc_set_time(&tm_s);
			
			UART_PRINTF("first_time: %04d-%02d-%02d %02d:%02d:%02d \r\n",
				tm_s.year, tm_s.month, tm_s.day,
				tm_s.hour, tm_s.minutes, tm_s.seconds);
			
			first_time = 1;	
		}
	}	


	
//	utc_update();
//	t_zero = utc_get_clock();
//	tuya_ble_utc_sec_2_mytime(t_zero, &t_struct, 0);
	
	ht1621_disp(MEAL_0, 0);
	ht1621_disp(MEAL_1, 0);
	ht1621_disp(MEAL_2, 0);
	ht1621_disp(MEAL_3, 0);
	ht1621_disp(MEAL_4, 0);
	ht1621_disp(PORTION, 0);

	ht1621_disp_dat(0, tm_s.hour/10 + 0x30);
	ht1621_disp_dat(1, tm_s.hour%10 + 0x30);
	ht1621_disp_dat(2, tm_s.minutes/10 + 0x30);
	ht1621_disp_dat(3, tm_s.minutes%10 + 0x30);
	ht1621_disp_dat(4, 0x00);
}



//设置时间
char set_time(void)
{
	int ret;
	int ret2;
	int set_value_flag = 0;
	UTCTimeStruct tm_s;
	
	UART_PRINTF("########## enter set_time \r\n");

	set_time_flag = 1;
	
	utc_get_time(&tm_s);
	
	ht1621_disp(COL, 1);
	
	while(1)
	{
		rwip_schedule();
		wdt_enable(0x3fff);
		
		switch(set_value_flag)
		{
			case 0:
			{
				ret = tm_s.hour / 10;
				ret2 = tm_s.hour % 10;
				display_flag = 1;				//第一次显示第一位会导致后面显示异常，传数值进去显示一次第二位解决
				display_val = ret2;

				tm_s.hour = set_val(0, &ret, 0, 2);
				tm_s.hour = tm_s.hour * 10 + ret2;
			}
			break;
			
			case 1:
			{
				ret2 = tm_s.hour % 10;
				
				if(tm_s.hour == 2)
				{
					ret2 = set_val(1, &ret2, 0, 3);
				}
				else
				{
					ret2 = set_val(1, &ret2, 0, 9);
				}
				tm_s.hour = ((tm_s.hour / 10) * 10)+ ret2;
			}
			break;

			case 2:
			{
				ret = tm_s.minutes / 10;
				ret2 = tm_s.minutes % 10;
				
				tm_s.minutes = set_val(2, &ret, 0, 5);	
				tm_s.minutes = tm_s.minutes * 10 + ret2;				
			}
			break;

			case 3:
			{
				ret2 = tm_s.minutes % 10;
				ret2 = set_val(3, &ret2, 0, 9);
				tm_s.minutes = ((tm_s.minutes / 10) * 10) + ret2;
			}
			break;
			
			default:
				set_value_flag = 4;
			break;
		}
		
		if(set_back_flag == 1)
		{
			if(set_value_flag > 0)
				set_value_flag--;
			set_back_flag = 0;
		}
		else if(set_back_flag >= 2)
		{
			set_value_flag++;
			set_back_flag = 0;
		}

		if (set_value_flag >= 4) 
		{
			
			tm_s.seconds = 0;
			utc_set_time(&tm_s);
			
			save_info.rtc_hour = tm_s.hour;
			save_info.rtc_minute = tm_s.minutes;
			
			UART_PRINTF("save time:%ld-%ld \n",save_info.rtc_hour, save_info.rtc_minute);
			flash_erase_sector(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR);
			flash_write(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR, sizeof(SAVE_INFO_t), (uint8_t *)&save_info);

			set_time_flag = 0;
			return ret;
		}
	}
	set_time_flag = 0;
	return ret;
}

//获取喂食计划
void get_meal(uint8_t num)
{
	UART_PRINTF("########## enter get_meal \r\n");

	num = num - 1;
	flash_read(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR, sizeof(FEED_PLAN_t), (uint8_t *)&feed_plan);
//	printf_flash_info();
	
	ht1621_disp(MEAL_0, 1);
	ht1621_disp(MEAL_1, 0);
	ht1621_disp(MEAL_2, 0);
	ht1621_disp(MEAL_3, 0);
	ht1621_disp(MEAL_4, 0);
	ht1621_disp(MEAL_0 + num + 1, 1);
	ht1621_disp(COL, 1);
	ht1621_disp(PORTION, 1);
	
	if (feed_plan.plans[num].weight == 0xff)
	{
		ht1621_disp_dat(0, '0');
		ht1621_disp_dat(1, '0');
		ht1621_disp_dat(2, '0');
		ht1621_disp_dat(3, '0');
		ht1621_disp_dat(4, '0');
	} 
	else
	{
		ht1621_disp_dat(0, feed_plan.plans[num].hour/10 + 0x30);
		ht1621_disp_dat(1, feed_plan.plans[num].hour%10 + 0x30);
		ht1621_disp_dat(2, feed_plan.plans[num].minute/10 + 0x30);
		ht1621_disp_dat(3, feed_plan.plans[num].minute%10 + 0x30);
		ht1621_disp_dat(4, feed_plan.plans[num].weight + 0x30);
	}
}


//设置喂食计划
char set_meal(uint8_t num)
{
	FEED_INFO_t feed;
	int ret;
	int ret2;
	int set_value_flag = 0;
	
	UART_PRINTF("########## enter set_meal \r\n");

	num = num - 1;
	flash_read(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR, sizeof(FEED_PLAN_t), (uint8_t *)&feed_plan);
//	printf_flash_info();
	memcpy(&feed, (uint8_t *) &feed_plan.plans[num], sizeof(FEED_INFO_t));
	
	UART_PRINTF("333:%02X-%02X-%02X\n",feed.hour,feed.minute,feed.weight);
	ht1621_disp(COL, 1);
	
	while (1)
	{
		rwip_schedule();
		wdt_enable(0x3fff);
		if (feed.hour == 0xff)
			feed.hour = 0;

		switch(set_value_flag)
		{
			case 0:
			{	
				ret = feed.hour / 10;
				ret2 = feed.hour % 10;

				display_flag = 1;				//第一次显示第一位会导致后面显示异常，传数值进去显示一次第二位解决
				display_val = ret2;

				feed.hour = set_val(0, &ret, 0, 2);
				feed.hour = feed.hour * 10 + ret2;
				UART_PRINTF("set feed.hour h:%02X\n",feed.hour);
			}
			break;
			
			case 1:
			{
				ret2 = feed.hour % 10;
				if(feed.hour == 2)
				{
					ret2 = set_val(1, &ret2, 0, 3);
				}
				else
				{
					ret2 = set_val(1, &ret2, 0, 9);
				}
				feed.hour = ((feed.hour / 10) * 10)+ ret2;;
				
				UART_PRINTF("set feed.hour:%02X\n",feed.hour);
			}
			break;

			case 2:
			{
				if (feed.minute == 0xff)
					feed.minute = 0;

				ret = feed.minute / 10;
				ret2 = feed.minute % 10;
				feed.minute = set_val(2, &ret, 0, 5);
				feed.minute = feed.minute * 10 + ret2;
				
				UART_PRINTF("set feed.minute h:%02X\n",feed.minute);
			}
			break;

			case 3:
			{
				ret2 = feed.minute % 10;
				ret2 = set_val(3, &ret2, 0, 9);
				feed.minute = ((feed.minute / 10) * 10) + ret2;
				UART_PRINTF("set feed.minute:%02X\n",feed.minute);
			}
			break;
			
			case 4:
			{
				if (feed.weight == 0xff)
					feed.weight = 1;
				
				ret = feed.weight;
				feed.weight = set_val(4, &ret, 0, 9);
				UART_PRINTF("set feed.weight:%02X\n",feed.weight);
			}
			break;

			default:
				set_value_flag = 5;
			break;
		}
		
		if(set_back_flag == 1)
		{
			if(set_value_flag > 0)
				set_value_flag--;
			set_back_flag = 0;
		}
		else if(set_back_flag >= 2)
		{
			set_value_flag++;
			set_back_flag = 0;
		}
		

		if (set_value_flag >= 5)
		{
			UART_PRINTF("222:%02X-%02X-%02X\n",feed.hour,feed.minute,feed.weight);

			feed.music = 0x01;
			if (feed.weight == 0)
			{
				feed.hour = 0xff;
				feed.minute = 0xff;
				feed.weight = 0xff;
				feed.music = 0xff;
			}
			UART_PRINTF("111:%02X-%02X-%02X\n",feed.hour,feed.minute,feed.weight);
			memcpy((uint8_t *) &feed_plan.plans[num], (uint8_t *) &feed , sizeof(FEED_INFO_t));
			UART_PRINTF("######################### save feed_plan 1\n");
			UART_PRINTF("222:%02X-%02X-%02X\n",feed_plan.plans[num].hour,feed_plan.plans[num].minute,feed_plan.plans[num].weight);	

			flash_erase_sector(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR);
			flash_write(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR, sizeof(FEED_PLAN_t), (uint8_t *)&feed_plan);
			return ret;
		} 

	}
	return ret;
}

//删除喂食计划
void del_meal(uint8_t num)
{	
	UART_PRINTF("########## enter del_meal \r\n");
	memset(&feed_plan.plans[num], 0xff, sizeof(FEED_INFO_t));
	
//	flash_erase(FLASH_SPACE_TYPE_NVR,BLE_PLAN_ADDR,FLASH_SIZE_ONE);
	flash_erase_sector(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR);
	flash_write(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR, sizeof(FEED_PLAN_t), (uint8_t *)&feed_plan);
}

//显示电量
void disp_voltage(void)
{
	uint16_t voltage = 0;

	voltage = adc_get_value(1);

	if(voltage < 440 || voltage >= 680)			//460 = 2.4V
	{
		ht1621_disp(POWER_0, 1);
		ht1621_disp(POWER_1, 1);
		ht1621_disp(POWER_2, 1);
		ht1621_disp(POWER_3, 1);
		ht1621_disp(POWER_4, 1);
	}
	else if (voltage < 570) 				//3.5
	{
		ht1621_disp(POWER_0, 1);
		ht1621_disp(POWER_1, 0);
		ht1621_disp(POWER_2, 0);
		ht1621_disp(POWER_3, 0);
		ht1621_disp(POWER_4, 0);
	} 
	else if (voltage < 610) 				//3.7
	{
		ht1621_disp(POWER_0, 1);
		ht1621_disp(POWER_1, 1);
		ht1621_disp(POWER_2, 0);
		ht1621_disp(POWER_3, 0);
		ht1621_disp(POWER_4, 0);
	} 
	else if (voltage < 640) 				// = 3.9
	{
		ht1621_disp(POWER_0, 1);
		ht1621_disp(POWER_1, 1);
		ht1621_disp(POWER_2, 1);
		ht1621_disp(POWER_3, 0);
		ht1621_disp(POWER_4, 0);
	} 
	else if (voltage < 680) 				// = 4.2
	{
		ht1621_disp(POWER_0, 1);
		ht1621_disp(POWER_1, 1);
		ht1621_disp(POWER_2, 1);
		ht1621_disp(POWER_3, 1);
		ht1621_disp(POWER_4, 0);
	} 

}



void play_record_control(void)
{
	gpio_set(SOUND_PLAY, PLAYER_ON);
	Delay_ms(100);
	gpio_set(SOUND_PLAY, PLAYER_OFF);
}

void record_reset_control(void)
{
	gpio_set(SOUND_REC, RECORD_ON);
	#ifdef NEW_RECORD_IC
	Delay_ms(1300);
	#else
	Delay_ms(600);	
	#endif
	gpio_set(SOUND_REC, RECORD_OFF);
}

void led_control(uint8_t link_led, uint8_t red_led, uint8_t led_level)
{
	#ifndef NO_LED_E
	static uint8_t old_led_level = 0;
	
	if(led_level >= old_led_level || led_level == 0)
	{
//		UART_PRINTF("########## led_level:%d \r\n",led_level);
		old_led_level = led_level;
		
		switch(link_led)
		{
			case LED_ON:
				SET_LED_ON(LINK_LED);

				break;
			
			case LED_OFF:
				SET_LED_OFF(LINK_LED);
				break;
		}
		
		switch(red_led)
		{
			case LED_ON:
				SET_LED_ON(PWR_LED);
				break;
			
			case LED_OFF:
				SET_LED_OFF(PWR_LED);
				break;
		}	
	}
	#endif
}

#ifdef BACKLIGHT_CONTROL
PWM_DRV_DESC timer_desc_3;
void update_backlight(uint8_t light)
{
	timer_desc_3.duty_cycle = light;
	pwm_set_duty(&timer_desc_3);
}
void backlight_init(void)
{	
	timer_desc_3.channel = 4;
	timer_desc_3.mode = 1<<0 | 0<<1 | 0<<2 | 0<<4;
	timer_desc_3.end_value = 6;
	timer_desc_3.duty_cycle = 0;
	pwm_init(&timer_desc_3);
}
void set_backlight(uint8_t *pwm, uint8_t up)
{
	if(up) 
	{
		if(*pwm <= 6)
			*pwm += 1;
	} 
	else 
	{
		if (*pwm > 1)
			*pwm -= 1;
	}
		
	update_backlight(*pwm);
}
#endif
uint8_t key_value = 0;				//按键触发哪个按键
uint8_t meal = 0;					//菜单

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ PT01K_BK
#ifdef PT01K_BK
void key_func(void)
{
	key_value = key_scan();
	
	if(lock_flag)
	{
		if(key_value == KEY_SET_L && lock_flag)					//长按设置键解锁
		{
			clean_key_flag();
			
			ht1621_disp(LOCK_OPEN, 1);
			ht1621_disp(LOCK_CLOSE, 0);
			#ifdef BACKLIGHT_CONTROL
//			update_backlight(save_info.led_backlight_pwm);
			update_backlight(5);
			#else
			gpio_set(BL_EN, 1);
			#endif
			
			led_control(LED_ON,LED_OFF,0);
			
			lock_flag = 0;
			beep_test();						//蜂鸣器播音
			UART_PRINTF("########## lock_flag == 0 \r\n");
			lock_timeout = LOCK_TIMEOUT_TIME;
		}
		else if(key_value == KEY_SET_L_UP)
		{
			lock_flag = 0;
			lock_timeout = LOCK_TIMEOUT_TIME;
		}
		key_flag = 0;
		key_scan_flag = 0;
	}
	else if(key_value)
	{
		clean_key_flag();
		lock_timeout = LOCK_TIMEOUT_TIME;
		if(lock_flag == 0 && key_value < KEY_SET_L_UP)
		{
//			beep_test();
//			lock_timeout = LOCK_TIMEOUT_TIME;
			beep_flag = 0;
		}

		switch (key_value) 	
		{
			case KEY_SET_L:
				beep_test();
				if (meal) 
				{
					set_meal(meal);
				} 
				else 
				{	
					set_time();
				}
				break;
				
			case KEY_FEED_L:	//长按删除喂食条目
				if (meal) 
				{
					del_meal(meal);
				}
				break;
				
//			case KEY_RECORD_L:			//录音
//				SET_LED_ON(PWR_LED);
//				SET_LED_OFF(LINK_LED);
////				record_flag = 1;
//				break;

//			case KEY_RECORD_L_UP:		//录音结束
//				SET_LED_OFF(PWR_LED);
//				SET_LED_ON(LINK_LED);
////				record_flag = 0;
//				break;

			case KEY_SET_S:
				break;
			
			case KEY_FEED_S:	//手动喂食
				feed_info_func.hour = 0;
				feed_info_func.minute = 0;
				feed_info_func.weight = 1;//改
				#ifdef FEED_KEY_MUSIC_E
				feed_info_func.music = 1;
				#else
				feed_info_func.music = 0;
				#endif
				feed_run(&feed_info_func);
				break;
			
//			case KEY_RECORD_S:	//放音
//				SET_LED_OFF(LINK_LED);
//				SET_LED_ON(PWR_LED);
//				play_record_control();				//播音
//				Delay_ms(500);
//				SET_LED_ON(LINK_LED);
//				SET_LED_OFF(PWR_LED);
//				break;
			
			case KEY_UP_L:
				beep_test();
			case KEY_UP_S:
				meal++;
				if(meal > 4)
				{
					meal = 0;
				}
				break;
				
			case KEY_DOWM_L:
				beep_test();
			case KEY_DOWN_S:
				if(meal > 0)
				{
					meal--;
				}
				else 
				{
					meal = 4;
				}
				break;
				
//			case KEY_UP_L:
//				beep_test();
//				#ifdef BACKLIGHT_CONTROL
//				set_backlight(&save_info.led_backlight_pwm, 1);
//				#else
//				gpio_set(BL_EN, 1);
//				#endif

//				Delay_ms(1000);
//				break;
//			
//			case KEY_DOWM_L:
//				beep_test();
//				#ifdef BACKLIGHT_CONTROL
//				set_backlight(&save_info.led_backlight_pwm, 0);
//				#else
//				gpio_set(BL_EN, 0);
//				#endif

//				Delay_ms(1000);
//				break;
				
		}

		if (meal) 
		{
			get_feed_info_flag = 1;
			get_meal(meal);
		} 
		else 
		{
			get_feed_info_flag = 0;
			get_time();
		}
		key_flag = 0;
		key_scan_flag = 0;

	}
	else 		//十秒后没有按键，重新锁键
	{
		if (lock_timeout == 0) 
		{
			UART_PRINTF("########## lock_timeout == 0  lock@@@@@@\r\n");

			get_feed_info_flag = 0;
			set_time_flag = 0;
			lock_flag = 1;					//设备锁标志
			key_scan_flag = 0;
			
			meal = 0;
			ht1621_disp(LOCK_OPEN, 0);
			ht1621_disp(LOCK_CLOSE, 1);
			#ifdef BACKLIGHT_CONTROL
			update_backlight(0);
			#else
			gpio_set(BL_EN, 0);
			#endif

			led_control(LED_OFF,LED_OFF,0);
		}
	}
}


#endif
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ LNH_01
#ifdef LNH_01
void key_func(void)
{
	key_value = key_scan();
	
	if(lock_flag)
	{
		if(key_value == KEY_LOCK_L && lock_flag)					//长按设置键解锁
		{
			clean_key_flag();
			
			ht1621_disp(LOCK_OPEN, 1);
			ht1621_disp(LOCK_CLOSE, 0);
			#ifdef BACKLIGHT_CONTROL
			update_backlight(save_info.led_backlight_pwm);
			#else
			gpio_set(BL_EN, 1);
			#endif
			
			led_control(LED_ON,LED_OFF,0);
			
			lock_flag = 0;
			beep_test();						//蜂鸣器播音
			UART_PRINTF("########## lock_flag == 0 \r\n");
			lock_timeout = LOCK_TIMEOUT_TIME;
		}
		else if(key_value == KEY_LOCK_L_UP)
		{
			lock_flag = 0;
			lock_timeout = LOCK_TIMEOUT_TIME;
		}
		key_flag = 0;
		key_scan_flag = 0;
	}
	else if(key_value)
	{
		clean_key_flag();
		lock_timeout = LOCK_TIMEOUT_TIME;
		if(lock_flag == 0 && key_value < KEY_SET_L_UP)
		{
//			beep_test();
//			lock_timeout = LOCK_TIMEOUT_TIME;
			beep_flag = 0;
		}

		switch (key_value) 	
		{
			case KEY_SET_L:
				beep_test();
				if (meal) 
				{
					set_meal(meal);
				} 
				else 
				{	
					set_time();
				}
				break;
				
			case KEY_FEED_L:	//长按删除喂食条目
				beep_test();
				if (meal) 
				{
					del_meal(meal);
				}
				break;
				
//			case KEY_RECORD_L:			//录音
//				SET_LED_ON(PWR_LED);
//				SET_LED_OFF(LINK_LED);
////				record_flag = 1;
//				break;

//			case KEY_RECORD_L_UP:		//录音结束
//				SET_LED_OFF(PWR_LED);
//				SET_LED_ON(LINK_LED);
////				record_flag = 0;
//				break;

			case KEY_SET_S:
				break;
			
			case KEY_FEED_S:	//手动喂食
				feed_info_func.hour = 0;
				feed_info_func.minute = 0;
				feed_info_func.weight = 1;//改
				#ifdef FEED_KEY_MUSIC_E
				feed_info_func.music = 1;
				#else
				feed_info_func.music = 0;
				#endif
				feed_run(&feed_info_func);
				break;
			
//			case KEY_RECORD_S:	//放音
//				SET_LED_OFF(LINK_LED);
//				SET_LED_ON(PWR_LED);
//				play_record_control();				//播音
//				Delay_ms(500);
//				SET_LED_ON(LINK_LED);
//				SET_LED_OFF(PWR_LED);
//				break;
			
			case KEY_UP_S:
				meal++;
				if(meal > 4)
				{
					meal = 0;
				}
				break;
				
			case KEY_DOWN_S:
				if(meal > 0)
				{
					meal--;
				}
				else 
				{
					meal = 4;
				}
				break;
				
//			case KEY_UP_L:
//				beep_test();
//				#ifdef BACKLIGHT_CONTROL
//				set_backlight(&save_info.led_backlight_pwm, 1);
//				#else
//				gpio_set(BL_EN, 1);
//				#endif

//				Delay_ms(1000);
//				break;
//			
//			case KEY_DOWM_L:
//				beep_test();
//				#ifdef BACKLIGHT_CONTROL
//				set_backlight(&save_info.led_backlight_pwm, 0);
//				#else
//				gpio_set(BL_EN, 0);
//				#endif

//				Delay_ms(1000);
//				break;
				
		}

		if (meal) 
		{
			get_feed_info_flag = 1;
			get_meal(meal);
		} 
		else 
		{
			get_feed_info_flag = 0;
			get_time();
		}
		key_flag = 0;
		key_scan_flag = 0;
	}
	else 		//十秒后没有按键，重新锁键
	{
		if (lock_timeout == 0) 
		{
			UART_PRINTF("########## lock_timeout == 0  lock@@@@@@\r\n");

			get_feed_info_flag = 0;
			set_time_flag = 0;
			lock_flag = 1;					//设备锁标志
			clean_key_flag();
			key_scan_flag = 0;
			
			meal = 0;
			ht1621_disp(LOCK_OPEN, 0);
			ht1621_disp(LOCK_CLOSE, 1);
			#ifdef BACKLIGHT_CONTROL
			update_backlight(0);
			#else
			gpio_set(BL_EN, 0);
			#endif

			led_control(LED_OFF,LED_OFF,0);
		}
	}
}


#endif

void printf_flash_info(void)
{
	uint8_t a;
	
	for(a = 0; a < 4;a++)
	{
		UART_PRINTF("%02X-%02X-%02X\n",feed_plan.plans[a].hour,feed_plan.plans[a].minute,feed_plan.plans[a].weight);	
	}
	UART_PRINTF("record_time:%d\n",save_info.record_time);	
	UART_PRINTF("rtc_timestamp:%d-%d\n",save_info.rtc_hour, save_info.rtc_minute);	
	UART_PRINTF("led_backlight_pwm:%d\n",save_info.led_backlight_pwm);	
}


void feed_error_led()
{
	static uint8_t led_static;
	static uint8_t dc_led_flag = 0;
	
	if(feed_error > 0)
	{
		if(led_static != LED_OFF)
		{
			led_control(LED_OFF,LED_OFF,4);
			led_static = LED_OFF;
		}
		else if(led_static != LED_ON)
		{
			led_control(LED_OFF,LED_ON,4);
			led_static = LED_ON;
		}
	}
	else if(led_static == LED_ON || led_static == LED_OFF)
	{
		led_control(LED_ON,LED_NO,0);
		led_static = LED_NO;
	}
	else if(gpio_get_input(CHARGE_DET) <= 0)
	{
		led_control(LED_OFF,LED_ON,3);
		dc_led_flag = 1;
	}
	else if(gpio_get_input(CHARGE_DET))
	{
		if(dc_led_flag)
		{
			if(lock_flag == 0)
				led_control(LED_ON,LED_OFF,0);
			else
				led_control(LED_NO,LED_OFF,0);
			
			dc_led_flag = 0;
		}
		else
		{
			led_control(LED_NO,LED_OFF,2);
		}
	}

}



