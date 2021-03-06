#include "function.h"
#include "wdt.h"
#include "rwip.h"
#include "tm1638.h"

uint8_t lock_flag = 1;				//设备锁标志
uint8_t key_lock = 0;				//锁按键标志
uint8_t key_scan_flag = 0;			//正在检查按键标志
uint8_t key_flag = 0;				//按键触发类型
uint8_t reset_flag = 0;				//复位标志
uint8_t get_feed_info_flag = 0;		//查看喂食计划标志
uint8_t set_time_flag = 0;			//正在设置时间标志
uint8_t set_val_flag = 0;			//正在设置数值标志
uint8_t record_flag = 0;

uint32_t lock_timeout = LOCK_TIMEOUT_TIME;		//锁屏超时时间
uint32_t rtc_timestamp = 1;						//时间戳时间

	
uint8_t display_flag = 0;			//第一次显示标志(第一次显示第一位会导致后面显示异常，传数值进去显示一次第二位解决)
uint8_t display_val = 0;			//第一次显示值
uint8_t beep_flag = 0;				//蜂鸣器响标志
uint8_t keep_dowm_flag = 0;			//按键长按标志

uint8_t set_back_flag = 0;			//设置上一个标志

uint8_t led_flag;
uint8_t unlock_flag = 0;

uint8_t get_meal_flag = 0;

uint8_t key_scan(void)
{	
	uint8_t key_flag_send = 0;
	
	if(key_flag > 0)
	{
		key_scan_flag = 1;
		
//		set_key_tick = 0;				//设置按键计时
//		dowm_key_tick = 0;				//下按键计时
//		up_key_tick = 0;				//上按键计时
//		lock_key_tick = 0;				//锁键按键计时
//		feed_key_tick = 0;				//喂食按键计时
//		
//		ok_key_tick = 0;

		key_flag_send = key_flag;
		key_flag = 0;
		UART_PRINTF("!!!!!!!!!!!!!!! !!!!!!! key_scan:%d \r\n",key_flag_send);
	}
	key_scan_flag = 0;
	return key_flag_send;
}

void clean_key_flag()
{
	key_flag = 0;
	set_key_tick = 0;	
	key_scan_flag = 0;
	dowm_key_tick = 0;				//下按键计时
	up_key_tick = 0;				//上按键计时
	record_key_tick = 0;
	
//	ok_key_tick = 0;
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
			lock_timeout = LOCK_TIMEOUT_TIME;//无操作20s后锁屏
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
				case KEY_OK_S:
				{	
					*dat = val;
					status = val;
					set_back_flag = 2;
					goto back;

				}
				case KEY_RECORD_S:
				{	
					
					*dat = val;
					status = val;
					set_back_flag = 1;
					goto back;
				}
				
				case KEY_DOWN_S:
				{
					
					if(val > min)
						val--;
					else if(val == min)
						val = max;
				}
					break;
									
				case KEY_UP_S:
				{
					
					val++;
					if (val > max)
						val = min;
				}
					break;
			}
			key_flag = 0;
			key_scan_flag = 0;
		}
		
		if (lcd_update_old != lcd_update) 
		{
			lcd_update_old = lcd_update;
			
			SEG9[3]&=0X0;

			if (reverse)
			{
				switch(addr)
				{
					case 0:
					{
						SEG9[0]&=0X80;
					}
						break;
					
					case 1:
					{
						SEG9[2]&=0X00;
					}
						break;						
					
					case 2:
					{
						SEG9[4]&=0X13;
						SEG9[10]&=0XBB;
					}
						break;
					
					case 3:
					{
						SEG9[6]&=0XC0;
						SEG9[7]&=0X0;
					}
						break;
					
					default:break;
				}				
				if (max>9)
				{
					switch(addr+1)
					{
						case 0:
						{
							SEG9[0]&=0X80;	
						}
							break;
						
						case 1:
						{
							SEG9[2]&=0X00;
						}
							break;
						
						case 2:
						{
							SEG9[4]&=0X13;
							SEG9[10]&=0XBB;
						}
							break;
						
						case 3:
						{
							SEG9[6]&=0XC0;
							SEG9[7]&=0X0;
						}
							break;
						
						default:break;
					}
				}	
				reverse = 0;
			}
			else 
			{
				if (max>9)
				{
					switch(addr)
					{
						case 0:
						{
							SEG9[0]|=GRID[0][val/10];
						}
							break;
						
						case 1:
						{
							SEG9[2]|=GRID[1][val/10];
						}
							break;
						
						case 2:
						{
							SEG9[4]|=GRID[2][val/10];
							SEG9[10]|=GRID[5][val/10];
						}
							break;
						
						case 3:
						{
							SEG9[6]|=GRID[3][val/10];
							SEG9[7]|=GRID[7][val/10];
						}
							break;
						
						default:break;
					}
					
					switch(addr+1)
					{
						case 0:
						{
							SEG9[0]|=GRID[0][val%10];
						}
							break;
						
						case 1:
						{
							SEG9[2]|=GRID[1][val%10];
						}
							break;
						
						case 2:
						{
							SEG9[4]|=GRID[2][val%10];
							SEG9[10]|=GRID[5][val%10];
						}
							break;
						
						case 3:
						{
							SEG9[6]|=GRID[3][val%10];
							SEG9[7]|=GRID[7][val%10];
						}
							break;
						
						default:break;
					}
				}
				
				else
				{				
					switch(addr)
					{
						case 0:
						{
							SEG9[0]|=GRID[0][val];
						}
							break;
						
						case 1:
						{
							SEG9[2]|=GRID[1][val];
						}
							break;
						
						case 2:
						{
							SEG9[4]|=GRID[2][val];
							SEG9[10]|=GRID[5][val];
						}
							break;
						
						case 3:
						{
							SEG9[6]|=GRID[3][val];
							SEG9[7]|=GRID[7][val];
						}
							break;
						
						default:break;
					}					
					
					if(display_flag)
					{
						display_flag = 0;
						
						switch(addr+1)
						{
							case 0:
							{
								SEG9[0]|=GRID[0][display_val];	
							}
								break;
						
							case 1:
							{
								SEG9[2]|=GRID[1][display_val];
							}
								break;
							
							case 2:
							{
								SEG9[4]|=GRID[2][display_val];
								SEG9[10]|=GRID[5][display_val];
							}
								break;
							
							case 3:
							{
								SEG9[6]|=GRID[3][display_val];
								SEG9[7]|=GRID[7][display_val];
							}
								break;
							
							default:break;
						}																		
					}
				}
				reverse = 1;
			}
		}
		lcd_update_time++;
		if(lcd_update_time > 25)
		{
			lcd_update_time = 0;
			lcd_update++;
		}
		Delay_ms(20);
		timeout--;
	}
	status = val;
	set_back_flag = 3;
back:
	*dat = val;
	SEG9[3]&=0X0;
	if (max>9)
	{		
		switch(addr)
		{
			case 0:
			{
				SEG9[0]&=0X80;
				SEG9[0]|=GRID[0][*dat/10];
			}
				break;
			
			case 1:
			{
				SEG9[2]&=0X00;
				SEG9[2]|=GRID[1][*dat/10];
			}
				break;
			
			case 2:
			{
				SEG9[4]&=0X13;
				SEG9[4]|=GRID[2][*dat/10];
				SEG9[10]&=0XBB;
				SEG9[10]|=GRID[5][*dat/10];
			}
				break;
			
			case 3:
			{
				SEG9[6]&=0XC0;
				SEG9[6]|=GRID[3][*dat/10];
				SEG9[7]&=0X0;
				SEG9[7]|=GRID[7][*dat/10];
			}
				break;
			
			default:break;
		}
		
		switch(addr+1)
		{
			case 0:
			{
				SEG9[0]&=0X80;
				SEG9[0]|=GRID[0][*dat%10];
			}
				break;
			
			case 1:
			{
				SEG9[2]&=0X00;
				SEG9[2]|=GRID[1][*dat%10];
			}
				break;
			
			case 2:
			{
				SEG9[4]&=0X13;
				SEG9[4]|=GRID[2][*dat%10];
				SEG9[10]&=0XBB;
				SEG9[10]|=GRID[5][*dat%10];
			}
				break;
			
			case 3:
			{
				SEG9[6]&=0XC0;
				SEG9[6]|=GRID[3][*dat%10];
				SEG9[7]&=0X0;
				SEG9[7]|=GRID[7][*dat%10];
			}
				break;
			
			default:break;
		}

	}
	else
	{		
		switch(addr)
		{
			case 0:
			{
				SEG9[0]&=0X80;
				SEG9[0]|=GRID[0][*dat];
			}
				break;
			
			case 1:
			{
				SEG9[2]&=0X00;
				SEG9[2]|=GRID[1][*dat];
			}
				break;
			
			case 2:
			{
				SEG9[4]&=0X13;
				SEG9[4]|=GRID[2][*dat];
				SEG9[10]&=0XBB;
				SEG9[10]|=GRID[5][*dat];
			}
				break;
			
			case 3:
			{
				SEG9[6]&=0XC0;
				SEG9[6]|=GRID[3][*dat];
				SEG9[7]&=0X0;
				SEG9[7]|=GRID[7][*dat];
			}
				break;
			
			default:break;
		}

	}
	
	set_val_flag = 0;			//正在设置数值标志

	return status;
}

int set_num_val(uint8_t addr, int* dat, char min, char max)
{
	char val = *dat;
	int status;
	uint8_t key_value;
	int timeout = 500;
	uint8_t reverse;
	uint8_t lcd_update_old = 0;
	uint8_t lcd_update_time = 0;

	UART_PRINTF("########## enter set_num_val \r\n");
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
			lock_timeout = LOCK_TIMEOUT_TIME;//无操作20s后锁屏
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
				case KEY_OK_S:
				{	
					
					*dat = val;
					status = val;
					set_back_flag = 2;
					goto back;
				}
				case KEY_RECORD_S:
				{	
					
					*dat = val;
					status = val;
					set_back_flag = 1;
					goto back;
				}
				
				case KEY_DOWN_S:
				{
					
					if(val > min)
						val--;
					else if(val == min)
						val = max;
				}
					break;
									
				case KEY_UP_S:
				{
					
					val++;
					if (val > max)
						val = min;
				}
					break;
			}
			key_flag = 0;
			key_scan_flag = 0;
		}
		
		if (lcd_update_old != lcd_update) 
		{
			lcd_update_old = lcd_update;
			SEG9[3]&=0X0;
			SEG9[4]&=0X13;
			SEG9[10]&=0XBB;
			
			SEG9[6]&=0XC0;
			SEG9[7]&=0X0;

			if (reverse)
			{
				switch(addr)
				{
					case 0:
					{
						SEG9[0]&=0X00;	
					}
						break;
					
					case 1:
					{
						SEG9[2]&=0x00;	
					}
						break;						
					
					default:break;
				}				
				if (max>9)
				{
					switch(addr+1)
					{
						case 0:
						{
							SEG9[0]&=0X00;	
						}
							break;
						
						case 1:
						{
							SEG9[2]&=0x00;
						}
							break;
						
						default:break;
					}
				}	
				reverse = 0;
			}
			else 
			{
				if (max>9)
				{
					switch(addr)
					{
						case 0:
						{
							SEG9[0]|=GRID[0][val/10];
						}
							break;
						
						case 1:
						{
							SEG9[2]|=GRID[1][val/10];
						}
							break;
						
						default:break;
					}
					
					switch(addr+1)
					{
						case 0:
						{
							SEG9[0]|=GRID[0][val%10];
						}
							break;
						
						case 1:
						{
							SEG9[2]|=GRID[1][val%10];
						}
							break;
						
						default:break;
					}
				}
				
				else
				{				
					switch(addr)
					{
						case 0:
						{
							SEG9[0]|=GRID[0][val];
						}
							break;
						
						case 1:
						{
							SEG9[2]|=GRID[1][val];
						}
							break;
						
						default:break;
					}					
					
					if(display_flag)
					{
						display_flag = 0;
						
						switch(addr+1)
						{
							case 0:
							{
								SEG9[0]|=GRID[0][display_val];	
							}
								break;
							
							case 1:
							{
								SEG9[2]|=GRID[1][display_val];
							}
								break;
							
							default:break;
						}																		
					}
				}
				reverse = 1;
			}
		}
		lcd_update_time++;
		if(lcd_update_time > 25)
		{
			lcd_update_time = 0;
			lcd_update++;
		}
		Delay_ms(20);
		timeout--;
	}
	status = val;
	set_back_flag = 3;
back:
	*dat = val;
	SEG9[3]&=0X0;
	SEG9[4]&=0X13;
	SEG9[10]&=0XBB;
	
	SEG9[6]&=0XC0;
	SEG9[7]&=0X0;

	if (max>9)
	{		
		switch(addr)
		{
			case 0:
			{
				SEG9[0]&=0X00;
				SEG9[0]|=GRID[0][*dat/10];
			}
				break;
			
			case 1:
			{
				SEG9[2]&=0X00;
				SEG9[2]|=GRID[1][*dat/10];
			}
				break;
			
			default:break;
		}
		
		switch(addr+1)
		{
			case 0:
			{
				SEG9[0]&=0X00;
				SEG9[0]|=GRID[0][*dat%10];
			}
				break;
			
			case 1:
			{
				SEG9[2]&=0X00;
				SEG9[2]|=GRID[1][*dat%10];
			}
				break;
			
			default:break;
		}

	}
	else
	{		
		switch(addr)
		{
			case 0:
			{
				SEG9[0]&=0X00;
				SEG9[0]|=GRID[0][*dat];
			}
				break;
			
			case 1:
			{
				SEG9[2]&=0X00;
				SEG9[2]|=GRID[1][*dat];
			}
				break;
			
			default:break;
		}

	}
	
	set_val_flag = 0;			//正在设置数值标志

	return status;

}

int set_weight_val(uint8_t addr, int* dat, char min, char max)
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
			lock_timeout = LOCK_TIMEOUT_TIME;//无操作20s后锁屏
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
				case KEY_OK_S:
				{	
					
					*dat = val;
					status = val;
					set_back_flag = 2;
					goto back;
				}
				case KEY_RECORD_S:
				{	
					
					*dat = val;
					status = val;
					set_back_flag = 1;
					goto back;
				}
				
				case KEY_DOWN_S:
				{
					
					if(val > min)
						val--;
					else if(val == min)
						val = max;
				}
					break;
									
				case KEY_UP_S:
				{
					
					val++;
					if (val > max)
						val = min;
				}
					break;
			}
			key_flag = 0;
			key_scan_flag = 0;
		}
		
		if (lcd_update_old != lcd_update) 
		{
			lcd_update_old = lcd_update;
			
			SEG9[0]&=0X00;
			SEG9[2]&=0x00;
			SEG9[3]&=0X0;
			
			if (reverse)
			{
				switch(addr)
				{
					case 2:
					{
						SEG9[4]&=0X13;
						SEG9[10]&=0XBB;
					}
						break;
					case 3:
					{
						SEG9[6]&=0XC0;
						SEG9[7]&=0X0;
					}
						break;
					default:break;
				}				
				if (max>9)
				{
					switch(addr+1)
					{
						case 2:
						{
							SEG9[4]&=0X13;
							SEG9[10]&=0XBB;
						}
							break;
						case 3:
						{
							SEG9[6]&=0XC0;
							SEG9[7]&=0X0;
						}						
							break;
						default:break;
					}
				}	
				reverse = 0;
			}
			else 
			{
				if (max>9)
				{
					switch(addr)
					{
						case 2:
						{
							SEG9[4]|=GRID[2][val/10];
							SEG9[10]|=GRID[5][val/10];
						}
							break;
						
						case 3:
						{
							SEG9[6]|=GRID[3][val/10];
							SEG9[7]|=GRID[7][val/10];
						}
							break;
						
						default:break;
					}
					
					switch(addr+1)
					{
						case 2:
						{
							SEG9[4]|=GRID[2][val%10];
							SEG9[10]|=GRID[5][val%10];
						}
							break;
						
						case 3:
						{
							SEG9[6]|=GRID[3][val%10];
							SEG9[7]|=GRID[7][val%10];
						}
							break;
						
						default:break;
					}
				}
				
				else
				{				
					switch(addr)
					{
						case 2:
						{
							SEG9[4]|=GRID[2][val];
							SEG9[10]|=GRID[5][val];
						}
							break;
						
						case 3:
						{
							SEG9[6]|=GRID[3][val];
							SEG9[7]|=GRID[7][val];
						}
							break;
						
						default:break;
					}					
					
					if(display_flag)
					{
						display_flag = 0;
						
						switch(addr+1)
						{
							case 2:
							{
								SEG9[4]|=GRID[2][display_val];
								SEG9[10]|=GRID[5][display_val];
							}
								break;
							
							case 3:
							{
								SEG9[6]|=GRID[3][display_val];
								SEG9[7]|=GRID[7][display_val];
							}
								break;
							
							default:break;
						}																		
					}
				}
				reverse = 1;
			}
		}
		lcd_update_time++;
		if(lcd_update_time > 25)
		{
			lcd_update_time = 0;
			lcd_update++;
		}
		Delay_ms(20);
		timeout--;
	}
	status = val;
	set_back_flag = 3;
back:
	*dat = val;
	
	SEG9[0]&=0X00;
	SEG9[2]&=0x00;
	SEG9[3]&=0X0;
	
	if (max>9)
	{		
		switch(addr)
		{
			case 2:
			{
				SEG9[4]&=0X13;
				SEG9[4]|=GRID[2][*dat/10];
				SEG9[10]&=0XBB;
				SEG9[10]|=GRID[5][*dat/10];
			}
				break;
			
			case 3:
			{
				SEG9[6]&=0XC0;
				SEG9[6]|=GRID[3][*dat/10];
				SEG9[7]&=0X0;
				SEG9[7]|=GRID[7][*dat/10];
			}
				break;
			
			default:break;
		}
		
		switch(addr+1)
		{
			case 2:
			{
				SEG9[4]&=0X13;
				SEG9[4]|=GRID[2][*dat%10];
				SEG9[10]&=0XBB;
				SEG9[10]|=GRID[5][*dat%10];
			}
				break;
			
			case 3:
			{
				SEG9[6]&=0XC0;
				SEG9[6]|=GRID[3][*dat%10];
				SEG9[7]&=0X0;
				SEG9[7]|=GRID[7][*dat%10];
			}
				break;
			
			default:break;
		}

	}
	else
	{		
		switch(addr)
		{
			case 2:
			{
				SEG9[4]&=0X13;
				SEG9[4]|=GRID[2][*dat];
				SEG9[10]&=0XBB;
				SEG9[10]|=GRID[5][*dat];
			}
				break;
			
			case 3:
			{
				SEG9[6]&=0XC0;
				SEG9[6]|=GRID[3][*dat];
				SEG9[7]&=0X0;
				SEG9[7]|=GRID[7][*dat];
			}
				break;
			
			default:break;
		}

	}
	
	set_val_flag = 0;			//正在设置数值标志

	return status;
}



//获取时间显示
void get_time(void)
{
	static uint8_t first_time = 0;
	UTCTimeStruct tm_s;
	
	utc_get_time(&tm_s);
	
	static uint8_t reverse = 1;


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
	
	SEG9[10]&=0X54;
	SEG9[11]&=0X0;
			
	for(uchar i=0;i<4;i++)
	{
		switch(i)
		{
			case 0:
			{
				SEG9[0]&=0X80;
				SEG9[0]|=GRID_1[tm_s.hour/10]; 
			}
				break;
			case 1:
			{
				SEG9[2]&=0X00;
				SEG9[2]|=GRID_2[tm_s.hour%10];
			}	
				break;
			case 2:
			{
				SEG9[4]&=0X13;
				SEG9[4]|=GRID_3[tm_s.minutes/10];
				SEG9[10]&=0XBB;
				SEG9[10]|=GRID_6[tm_s.minutes/10];	
			}
				break;
			case 3:	
			{
				SEG9[6]&=0XC0;
				SEG9[6]|=GRID_4[tm_s.minutes%10];
				SEG9[7]&=0X0;
				SEG9[7]|=LED20[tm_s.minutes%10];			
			}
				break;	
			default:break;
		}
	}
		if(reverse) 
		{				
			SEG9[2]|=0X02;
			SEG9[3]|=0xf;				
			reverse = 0;
		} 
		else 
		{
			SEG9[2]&=0Xfd;
			SEG9[3]&=0X0;
			reverse = 1;
		}

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
				
				if(tm_s.hour/10 == 2)
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
			{
				set_value_flag = 4;
			}
				break;
		}
		
		if(set_value_flag == 0)
		{
			for(uchar i=0;i<4;i++)
			{
				switch(i)
				{
					case 0:
					{
						SEG9[0]&=0X80;
						SEG9[0]|=GRID_1[tm_s.hour/10]; 
					}
						break;
					case 1:
					{
						SEG9[2]&=0X00;
						SEG9[2]|=GRID_2[tm_s.hour%10];
					}	
						break;
					case 2:
					{
						SEG9[4]&=0X13;
						SEG9[4]|=GRID_3[tm_s.minutes/10];
						SEG9[10]&=0XBB;
						SEG9[10]|=GRID_6[tm_s.minutes/10];	
					}
						break;
					case 3:	
					{
						SEG9[6]&=0XC0;
						SEG9[6]|=GRID_4[tm_s.minutes%10];
						SEG9[7]&=0X0;
						SEG9[7]|=LED20[tm_s.minutes%10];			
					}
						break;	
					default:break;
				}
			}

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
	static uint8_t reverse = 1;

	UART_PRINTF("########## enter get_meal \r\n");	
	
	num = num - 1;
	flash_read(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR, sizeof(FEED_PLAN_t), (uint8_t *)&feed_plan);
	printf_flash_info();
	UART_PRINTF("feed_plan %02X:%02X-%02X-%02X", num, feed_plan.plans[num].hour, feed_plan.plans[num].minute, feed_plan.plans[num].weight);
	
	SEG9[10]&=0X54;
	SEG9[11]&=0X0;			

	if (feed_plan.plans[num].weight == 0xff)
	{		
		for(uchar i=0;i<4;i++)
		{
			switch(i)
			{
				case 0:
				{
					SEG9[0]&=0X00;
					SEG9[0]|=GRID_1[0]; 
				}
					break;
				case 1:
				{
					SEG9[2]&=0X00;
					SEG9[2]|=GRID_2[0];
				}	
					break;
				case 2:
				{
					SEG9[4]&=0X13;
					SEG9[4]|=GRID_3[0];
					SEG9[10]&=0XBB;
					SEG9[10]|=GRID_6[0];	
				}
					break;
				case 3:	
				{
					SEG9[6]&=0XC0;
					SEG9[6]|=GRID_4[0];
					SEG9[7]&=0X0;
					SEG9[7]|=LED20[0];			
				}
					break;	
				default:break;
			}
		}

	} 
	else
	{		
		for(uchar i=0;i<4;i++)
		{
			switch(i)
			{
				case 0:
				{
					SEG9[0]&=0X00;
					SEG9[0]|=GRID_1[feed_plan.plans[num].hour/10]; 
				}
					break;
				case 1:
				{
					SEG9[2]&=0X00;
					SEG9[2]|=GRID_2[feed_plan.plans[num].hour%10];
				}	
					break;
				case 2:
				{
					SEG9[4]&=0X13;
					SEG9[4]|=GRID_3[feed_plan.plans[num].minute/10];
					SEG9[10]&=0XBB;
					SEG9[10]|=GRID_6[feed_plan.plans[num].minute/10];	
				}
					break;
				case 3:	
				{
					SEG9[6]&=0XC0;
					SEG9[6]|=GRID_4[feed_plan.plans[num].minute%10];
					SEG9[7]&=0X0;
					SEG9[7]|=LED20[feed_plan.plans[num].minute%10];			
				}
					break;	
				default:break;
			}
		}
			if(reverse) 
			{				
				SEG9[2]|=0X02;
				SEG9[3]|=0x0f;				
				reverse = 0;
			} 
			else 
			{
				SEG9[2]&=0Xfd;
				SEG9[3]&=0X00;
				reverse = 1;
			}				
	}
}

void get_feed_num(uint8_t num)
{
	UART_PRINTF("########## enter get_feed_num \r\n");
	int timeout = 500;
		
	
		SEG9[10]&=0X54;
		SEG9[11]&=0X0;
		SEG9[10]|=0XA8;

		SEG9[3]&=0X0;
		SEG9[4]&=0X13;
		SEG9[10]&=0XBB;
		SEG9[6]&=0XC0;
		SEG9[7]&=0X0;
		for(uchar i=0;i<2;i++)
		{
			switch(i)
			{
				case 0:
				{
					if(num/10 == 0)
					{
						SEG9[0]&=0X00;
					}
					else
					{
						SEG9[0]&=0X00;
						SEG9[0]|=GRID_1[num/10]; 
					}
				}
					break;
				case 1:
				{
					SEG9[2]&=0X00;
					SEG9[2]|=GRID_2[num%10];
				}	
					break;
				default:break;
			}
		}
}

void get_weight(uint8_t num)
{
	UART_PRINTF("########## enter get_weight \r\n");
	int timeout = 500;
	
	num = num - 1;
	flash_read(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR, sizeof(FEED_PLAN_t), (uint8_t *)&feed_plan);
//	printf_flash_info();
	
		
		SEG9[10]&=0X54;
		SEG9[11]|=0XF;
		SEG9[10]|=0X03;
		
		SEG9[0]&=0X00;
		SEG9[2]&=0X00;
		SEG9[3]&=0X0;
		if (feed_plan.plans[num].weight == 0xff)
		{		
			for(uchar i=0;i<2;i++)
			{
				switch(i)
				{
					case 0:
					{
						SEG9[4]&=0X13;
//						SEG9[4]|=GRID_3[0];
						SEG9[10]&=0XBB;
//						SEG9[10]|=GRID_6[0];
					}
						break;
					case 1:	
					{
						SEG9[6]&=0XC0;
						SEG9[6]|=GRID_4[0];
						SEG9[7]&=0X0;
						SEG9[7]|=LED20[0];						
					}
						break;	
					default:break;
				}
			}
		} 
		else
		{
			for(uchar i=0;i<2;i++)
			{
				switch(i)
				{
					case 0:
					{
						if(feed_plan.plans[num].weight/10 == 0)
						{
							SEG9[4]&=0X13;
							SEG9[10]&=0XBB;
						}
						else
						{
							SEG9[4]&=0X13;
							SEG9[4]|=GRID_3[feed_plan.plans[num].weight/10];			
							SEG9[10]&=0XBB;
							SEG9[10]|=GRID_6[feed_plan.plans[num].weight/10];
						}
					}
						break;
					case 1:	
					{
						SEG9[6]&=0XC0;
						SEG9[6]|=GRID_4[feed_plan.plans[num].weight%10];
						SEG9[7]&=0X0;
						SEG9[7]|=LED20[feed_plan.plans[num].weight%10];						
					}
						break;	
					default:break;
				}
			}			
		}
}

//设置喂食计划
char set_meal(void)
{
	FEED_INFO_t feed;
	int ret;
	int ret2;
	int set_value_flag = 0;
	uint8_t meal = 1, num;
	uint8_t flag = 1;
		
	UART_PRINTF("########## enter set_meal \r\n");
	
	flash_read(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR, sizeof(FEED_PLAN_t), (uint8_t *)&feed_plan);
//	printf_flash_info();
//	memcpy(&feed, (uint8_t *) &feed_plan.plans[num], sizeof(FEED_INFO_t));
//	UART_PRINTF("333:%02X:%02X-%02X-%02X\n", num, feed.hour, feed.minute, feed.weight);
	
	while (1)
	{
		rwip_schedule();
		wdt_enable(0x3fff);
			
		switch(set_value_flag)
		{
			case 0:
			{
				UART_PRINTF("############### 111\n");
				printf_flash_info();

//				meal = 1;
				
				ret = meal / 10;
				ret2 = meal % 10;

				display_flag = 1;				//第一次显示第一位会导致后面显示异常，传数值进去显示一次第二位解决
				display_val = ret2;
				UART_PRINTF("%02X  %02X\n",ret, ret2);


				meal = set_num_val(0, &ret, 0, 1);
				meal = meal * 10 + ret2;

				flag = 1;
				UART_PRINTF("set meal h:%02X\n",meal);				
			}
				break;
			
			case 1:
			{
				UART_PRINTF("############### 222\n");
				printf_flash_info();
				ret2 = meal % 10;
				if(meal/10 == 1)
				{
					ret2 = set_num_val(1, &ret2, 0, 0);
				}
				else
				{
					ret2 = set_num_val(1, &ret2, 0, 9);
				}
				meal = ((meal / 10) * 10)+ ret2;
				
				UART_PRINTF("set meal:%02X\n",meal);				
				
				if(meal == 0)
				{
					UART_PRINTF("set meal error!\n");
					set_value_flag -= 2;
				}	
				flag = 1;
			}
				break;
			
			case 2:
			{	
				
				
				if (feed.hour == 0xff)
					feed.hour = 0;
				
				ret = feed.hour / 10;
				ret2 = feed.hour % 10;

//				display_flag = 1;				//第一次显示第一位会导致后面显示异常，传数值进去显示一次第二位解决
//				display_val = ret2;
				UART_PRINTF("%02X  %02X\n",ret, ret2);

				feed.hour = set_val(0, &ret, 0, 2);
				feed.hour = feed.hour * 10 + ret2;
				UART_PRINTF("set feed.hour h:%02X\n",feed.hour);				
			}
				break;
			
			case 3:
			{
				ret2 = feed.hour % 10;
				if(feed.hour/10 == 2)
				{
					ret2 = set_val(1, &ret2, 0, 3);
				}
				else
				{
					ret2 = set_val(1, &ret2, 0, 9);
				}
				feed.hour = ((feed.hour / 10) * 10)+ ret2;
				
				UART_PRINTF("set feed.hour:%02X\n",feed.hour);
				
			}
				break;

			case 4:
			{
				
				if (feed.minute == 0xff)
					feed.minute = 0;
				
				ret = feed.minute / 10;
				ret2 = feed.minute % 10;

				UART_PRINTF("%02X  %02X\n",ret, ret2);

				
				feed.minute = set_val(2, &ret, 0, 5);
				feed.minute = feed.minute * 10 + ret2;
				
				UART_PRINTF("set feed.minute h:%02X\n",feed.minute);
				
			}
				break;

			case 5:
			{
				ret2 = feed.minute % 10;
				ret2 = set_val(3, &ret2, 0, 9);
				feed.minute = ((feed.minute / 10) * 10) + ret2;
				UART_PRINTF("set feed.minute:%02X\n",feed.minute);
				
			}
				break;
			
			case 6:
			{
				
				if (feed.weight == 0xff)
					feed.weight = 1;
				
				ret = feed.weight / 10;
				ret2 = feed.weight % 10;
				
//				display_flag = 1;				//第一次显示第一位会导致后面显示异常，传数值进去显示一次第二位解决
//				display_val = ret2;				
				UART_PRINTF("%02X  %02X\n",ret, ret2);
				
				feed.weight = set_weight_val(2, &ret, 0, 2);
				
				feed.weight = feed.weight * 10 + ret2;

				UART_PRINTF("set feed.weight h:%02X\n",feed.weight);
			}
				break;
			
			case 7:
			{
				ret2 = feed.weight % 10;
				
				if(feed.weight/10 == 2)
				{
					ret2 = set_weight_val(3, &ret2, 0, 4);
				}
				else
				{
					ret2 = set_weight_val(3, &ret2, 0, 9);
				}
				
				feed.weight = ((feed.weight / 10) * 10)+ ret2;
				
				UART_PRINTF("set feed.weight:%02X\n",feed.weight);
			}
				break;
			
			default:
			{
				set_value_flag = 8;
			}
				break;
		}
		
		num = meal - 1;
				
		if(set_back_flag == 1)
		{
//			feed_plan.plans[num] = feed;
			
			if(set_value_flag > 0)
			{
				set_value_flag--;
				
				
				if(set_value_flag < 2)
				{
					SEG9[10]&=0X54;
					SEG9[11]&=0X0;
					SEG9[10]|=0XA8;			
				}
				else if(set_value_flag > 5 && set_value_flag < 8)
				{
					SEG9[10]&=0X54;
					SEG9[11]|=0XF;
					SEG9[10]|=0X03;
				}
				else
				{
					SEG9[10]&=0X54;
					SEG9[11]&=0X0;			
				}

				
				if(set_value_flag <= 5 && set_value_flag >= 2)
				{	
					SEG9[3]&=0X0;
					if (feed_plan.plans[num].weight == 0xff)
					{		
						for(uchar i=0;i<4;i++)
						{
							switch(i)
							{
								case 0:
								{
									SEG9[0]&=0X00;
									SEG9[0]|=GRID_1[0]; 
								}
									break;
								case 1:
								{
									SEG9[2]&=0X00;
									SEG9[2]|=GRID_2[0];
								}	
									break;
								case 2:
								{
									SEG9[4]&=0X13;
									SEG9[4]|=GRID_3[0];
									SEG9[10]&=0XBB;
									SEG9[10]|=GRID_6[0];	
								}
									break;
								case 3:	
								{
									SEG9[6]&=0XC0;
									SEG9[6]|=GRID_4[0];
									SEG9[7]&=0X0;
									SEG9[7]|=LED20[0];			
								}
									break;	
								default:break;
							}
						}
					} 
					else
					{		
						for(uchar i=0;i<4;i++)
						{
							switch(i)
							{
								case 0:
								{
									SEG9[0]&=0X00;
									SEG9[0]|=GRID_1[feed.hour/10]; 
								}
									break;
								case 1:
								{
									SEG9[2]&=0X00;
									SEG9[2]|=GRID_2[feed.hour%10];
								}	
									break;
								case 2:
								{
									SEG9[4]&=0X13;
									SEG9[4]|=GRID_3[feed.minute/10];
									SEG9[10]&=0XBB;
									SEG9[10]|=GRID_6[feed.minute/10];	
								}
									break;
								case 3:	
								{
									SEG9[6]&=0XC0;
									SEG9[6]|=GRID_4[feed.minute%10];
									SEG9[7]&=0X0;
									SEG9[7]|=LED20[feed.minute%10];		
								}
									break;	
								default:break;
							}
						}
					}
				}
				if(set_value_flag <= 1)
				{
					SEG9[4]&=0X13;
					SEG9[10]&=0XBB;
					SEG9[6]&=0XC0;
					SEG9[7]&=0X0;
					for(uchar i=0;i<2;i++)
					{
						switch(i)
						{
							case 0:
							{
								SEG9[0]&=0X00;
								SEG9[0]|=GRID_1[meal/10]; 
							}
								break;
							case 1:
							{
								SEG9[2]&=0X00;
								SEG9[3]&=0X0;
								SEG9[2]|=GRID_2[meal%10];
							}	
								break;
							default:break;
						}
					}
				}
				
			}
			set_back_flag = 0;
		}
		
		else if(set_back_flag >= 2)
		{
			
			set_value_flag++;
						
			if(set_value_flag < 2)
			{
				SEG9[10]&=0X54;
				SEG9[11]&=0X0;
				SEG9[10]|=0XA8;			
			}
			else if(set_value_flag >5 && set_value_flag < 8)
			{
				SEG9[10]&=0X54;
				SEG9[11]|=0XF;
				SEG9[10]|=0X03;
			}
			else
			{
				SEG9[10]&=0X54;
				SEG9[11]&=0X0;			
			}

			
			if(set_value_flag == 2)
			{
				SEG9[3]&=0X0;
				if (feed_plan.plans[num].weight == 0xff)
				{		
					for(uchar i=0;i<4;i++)
					{
						switch(i)
						{
							case 0:
							{
								SEG9[0]&=0X00;
								SEG9[0]|=GRID_1[0]; 
							}
								break;
							case 1:
							{
								SEG9[2]&=0X00;
								SEG9[2]|=GRID_2[0];
							}	
								break;
							case 2:
							{
								SEG9[4]&=0X13;
								SEG9[4]|=GRID_3[0];
								SEG9[10]&=0XBB;
								SEG9[10]|=GRID_6[0];	
							}
								break;
							case 3:	
							{
								SEG9[6]&=0XC0;
								SEG9[6]|=GRID_4[0];
								SEG9[7]&=0X0;
								SEG9[7]|=LED20[0];			
							}
								break;	
							default:break;
						}
					}

				} 
				else
				{	
					if(flag)
					{
						flag = 0;
						printf_flash_info();
						memcpy(&feed, (uint8_t *) &feed_plan.plans[meal - 1], sizeof(FEED_INFO_t));
						UART_PRINTF("333:%02X:%02X-%02X-%02X\n", meal - 1, feed.hour, feed.minute, feed.weight);
					}
					
					for(uchar i=0;i<4;i++)
					{
						switch(i)
						{
							case 0:
							{
								SEG9[0]&=0X00;
								SEG9[0]|=GRID_1[feed.hour/10]; 
							}
								break;
							case 1:
							{
								SEG9[2]&=0X00;
								SEG9[2]|=GRID_2[feed.hour%10];
							}	
								break;
							case 2:
							{
								SEG9[4]&=0X13;
								SEG9[4]|=GRID_3[feed.minute/10];
								SEG9[10]&=0XBB;
								SEG9[10]|=GRID_6[feed.minute/10];	
							}
								break;
							case 3:	
							{
								SEG9[6]&=0XC0;
								SEG9[6]|=GRID_4[feed.minute%10];
								SEG9[7]&=0X0;
								SEG9[7]|=LED20[feed.minute%10];			
							}
								break;	
							default:break;
						}
					}
				}
			}
			if(set_value_flag == 6)
			{
				SEG9[0]&=0X00;
				SEG9[2]&=0X00;
				SEG9[3]&=0X0;

				if (feed_plan.plans[num].weight == 0xff)
				{		
					for(uchar i=0;i<2;i++)
					{
						switch(i)
						{
							case 0:
							{
								SEG9[4]&=0X13;
								SEG9[4]|=GRID_3[0];
								SEG9[10]&=0XBB;
								SEG9[10]|=GRID_6[0];
							}
								break;
							case 1:	
							{
								SEG9[6]&=0XC0;
								SEG9[6]|=GRID_4[0];
								SEG9[7]&=0X0;
								SEG9[7]|=LED20[0];						
							}
								break;	
							default:break;
						}
					}
				} 
				else
				{
					for(uchar i=0;i<2;i++)
					{
						switch(i)
						{
							case 0:
							{
								SEG9[4]&=0X13;
								SEG9[4]|=GRID_3[feed_plan.plans[num].weight/10];			
								SEG9[10]&=0XBB;
								SEG9[10]|=GRID_6[feed_plan.plans[num].weight/10];
							}
								break;
							case 1:	
							{
								SEG9[6]&=0XC0;
								SEG9[6]|=GRID_4[feed_plan.plans[num].weight%10];
								SEG9[7]&=0X0;
								SEG9[7]|=LED20[feed_plan.plans[num].weight%10];						
							}
								break;	
							default:break;
						}
					}			
				}

			}						
			set_back_flag = 0;
		}
				
		if (set_value_flag >= 8)
		{
			UART_PRINTF("222:%02X:%02X-%02X-%02X\n", num+1, feed.hour, feed.minute, feed.weight);

			feed.music = 0x01;
			if (feed.weight == 0)
			{
				feed.hour = 0xff;
				feed.minute = 0xff;
				feed.weight = 0xff;
				feed.music = 0xff;
			}
			UART_PRINTF("111:%02X:%02X-%02X-%02X\n", num+1, feed.hour, feed.minute, feed.weight);
			memcpy((uint8_t *) &feed_plan.plans[num], (uint8_t *) &feed , sizeof(FEED_INFO_t));
			UART_PRINTF("######################### save feed_plan 1\n");
			UART_PRINTF("222:%02X:%02X-%02X-%02X\n", num+1, feed_plan.plans[num].hour, feed_plan.plans[num].minute, feed_plan.plans[num].weight);	

			flash_erase_sector(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR);
			flash_write(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR, sizeof(FEED_PLAN_t), (uint8_t *)&feed_plan);
			
			return ret;
		}				
	}
	return ret;
}


char set_meal_1(uint8_t num)
{
	FEED_INFO_t feed;
	int ret;
	int ret2;
	int set_value_flag = 0;
			
	UART_PRINTF("########## enter set_meal \r\n");

	num = num - 1;
	flash_read(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR, sizeof(FEED_PLAN_t), (uint8_t *)&feed_plan);
	printf_flash_info();
	memcpy(&feed, (uint8_t *) &feed_plan.plans[num], sizeof(FEED_INFO_t));
	UART_PRINTF("333:%02X:%02X-%02X-%02X\n", num, feed.hour, feed.minute, feed.weight);
	
	while (1)
	{
		rwip_schedule();
		wdt_enable(0x3fff);
		
		if(set_value_flag == 0)
		{
			SEG9[3]&=0X0;
			if (feed.weight == 0xff)
			{		
				for(uchar i=0;i<4;i++)
				{
					switch(i)
					{
						case 0:
						{
							SEG9[0]&=0X00;
							SEG9[0]|=GRID_1[0]; 
						}
							break;
						case 1:
						{
							SEG9[2]&=0X00;
							SEG9[2]|=GRID_2[0];
						}	
							break;
						case 2:
						{
							SEG9[4]&=0X13;
							SEG9[4]|=GRID_3[0];
							SEG9[10]&=0XBB;
							SEG9[10]|=GRID_6[0];	
						}
							break;
						case 3:	
						{
							SEG9[6]&=0XC0;
							SEG9[6]|=GRID_4[0];
							SEG9[7]&=0X0;
							SEG9[7]|=LED20[0];			
						}
							break;	
						default:break;
					}
				}

			} 
			else
			{						
				for(uchar i=0;i<4;i++)
				{
					switch(i)
					{
						case 0:
						{
							SEG9[0]&=0X00;
							SEG9[0]|=GRID_1[feed.hour/10]; 
						}
							break;
						case 1:
						{
							SEG9[2]&=0X00;
							SEG9[2]|=GRID_2[feed.hour%10];
						}	
							break;
						case 2:
						{
							SEG9[4]&=0X13;
							SEG9[4]|=GRID_3[feed.minute/10];
							SEG9[10]&=0XBB;
							SEG9[10]|=GRID_6[feed.minute/10];	
						}
							break;
						case 3:	
						{
							SEG9[6]&=0XC0;
							SEG9[6]|=GRID_4[feed.minute%10];
							SEG9[7]&=0X0;
							SEG9[7]|=LED20[feed.minute%10];			
						}
							break;	
						default:break;
					}
				}
			}
		}
			
		switch(set_value_flag)
		{
			case 0:
			{					
				if (feed.hour == 0xff)
					feed.hour = 0;
				
				ret = feed.hour / 10;
				ret2 = feed.hour % 10;

//				display_flag = 1;				//第一次显示第一位会导致后面显示异常，传数值进去显示一次第二位解决
//				display_val = ret2;
				UART_PRINTF("%02X  %02X\n",ret, ret2);

				feed.hour = set_val(0, &ret, 0, 2);
				feed.hour = feed.hour * 10 + ret2;
				UART_PRINTF("set feed.hour h:%02X\n",feed.hour);				
			}
				break;
			
			case 1:
			{
				ret2 = feed.hour % 10;
				if(feed.hour/10 == 2)
				{
					ret2 = set_val(1, &ret2, 0, 3);
				}
				else
				{
					ret2 = set_val(1, &ret2, 0, 9);
				}
				feed.hour = ((feed.hour / 10) * 10)+ ret2;
				
				UART_PRINTF("set feed.hour:%02X\n",feed.hour);
				
			}
				break;

			case 2:
			{
				
				if (feed.minute == 0xff)
					feed.minute = 0;
				
				ret = feed.minute / 10;
				ret2 = feed.minute % 10;

				UART_PRINTF("%02X  %02X\n",ret, ret2);
				
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
				
				ret = feed.weight / 10;
				ret2 = feed.weight % 10;
				
//				display_flag = 1;				//第一次显示第一位会导致后面显示异常，传数值进去显示一次第二位解决
//				display_val = ret2;				
				UART_PRINTF("%02X  %02X\n",ret, ret2);
				
				feed.weight = set_weight_val(2, &ret, 0, 2);
				
				feed.weight = feed.weight * 10 + ret2;

				UART_PRINTF("set feed.weight h:%02X\n",feed.weight);
			}
				break;
			
			case 5:
			{
				ret2 = feed.weight % 10;
				
				if(feed.weight/10 == 2)
				{
					ret2 = set_weight_val(3, &ret2, 0, 4);
				}
				else
				{
					ret2 = set_weight_val(3, &ret2, 0, 9);
				}
				
				feed.weight = ((feed.weight / 10) * 10)+ ret2;
				
				UART_PRINTF("set feed.weight:%02X\n",feed.weight);
			}
				break;
			
			default:
			{
				set_value_flag = 6;
			}
				break;
		}

	
		if(set_back_flag == 1)
		{
			
			if(set_value_flag > 0)
			{
				set_value_flag--;
								
				if(set_value_flag > 3 && set_value_flag < 6)
				{
					SEG9[10]&=0X54;
					SEG9[11]|=0XF;
					SEG9[10]|=0X03;
				}
				else
				{
					SEG9[10]&=0X54;
					SEG9[11]&=0X0;			
				}

				
				if(set_value_flag <= 3)
				{	
					SEG9[3]&=0X0;
					if (feed.weight == 0xff)
					{		
						for(uchar i=0;i<4;i++)
						{
							switch(i)
							{
								case 0:
								{
									SEG9[0]&=0X00;
									SEG9[0]|=GRID_1[0]; 
								}
									break;
								case 1:
								{
									SEG9[2]&=0X00;
									SEG9[2]|=GRID_2[0];
								}	
									break;
								case 2:
								{
									SEG9[4]&=0X13;
									SEG9[4]|=GRID_3[0];
									SEG9[10]&=0XBB;
									SEG9[10]|=GRID_6[0];	
								}
									break;
								case 3:	
								{
									SEG9[6]&=0XC0;
									SEG9[6]|=GRID_4[0];
									SEG9[7]&=0X0;
									SEG9[7]|=LED20[0];			
								}
									break;	
								default:break;
							}
						}
					} 
					else
					{		
						for(uchar i=0;i<4;i++)
						{
							switch(i)
							{
								case 0:
								{
									SEG9[0]&=0X00;
									SEG9[0]|=GRID_1[feed.hour/10]; 
								}
									break;
								case 1:
								{
									SEG9[2]&=0X00;
									SEG9[2]|=GRID_2[feed.hour%10];
								}	
									break;
								case 2:
								{
									SEG9[4]&=0X13;
									SEG9[4]|=GRID_3[feed.minute/10];
									SEG9[10]&=0XBB;
									SEG9[10]|=GRID_6[feed.minute/10];	
								}
									break;
								case 3:	
								{
									SEG9[6]&=0XC0;
									SEG9[6]|=GRID_4[feed.minute%10];
									SEG9[7]&=0X0;
									SEG9[7]|=LED20[feed.minute%10];		
								}
									break;	
								default:break;
							}
						}
					}
				}				
			}
			set_back_flag = 0;
		}
		
		else if(set_back_flag >= 2)
		{
			
			set_value_flag++;
						
			if(set_value_flag >3 && set_value_flag < 6)
			{
				SEG9[10]&=0X54;
				SEG9[11]|=0XF;
				SEG9[10]|=0X03;
			}
			else
			{
				SEG9[10]&=0X54;
				SEG9[11]&=0X0;			
			}

			if(set_value_flag == 4)
			{
				SEG9[0]&=0X00;
				SEG9[2]&=0X00;
				SEG9[3]&=0X0;

				if (feed.weight == 0xff)
				{		
					for(uchar i=0;i<2;i++)
					{
						switch(i)
						{
							case 0:
							{
								SEG9[4]&=0X13;
								SEG9[4]|=GRID_3[0];
								SEG9[10]&=0XBB;
								SEG9[10]|=GRID_6[0];
							}
								break;
							case 1:	
							{
								SEG9[6]&=0XC0;
								SEG9[6]|=GRID_4[0];
								SEG9[7]&=0X0;
								SEG9[7]|=LED20[0];						
							}
								break;	
							default:break;
						}
					}
				} 
				else
				{
					for(uchar i=0;i<2;i++)
					{
						switch(i)
						{
							case 0:
							{
								SEG9[4]&=0X13;
								SEG9[4]|=GRID_3[feed.weight/10];			
								SEG9[10]&=0XBB;
								SEG9[10]|=GRID_6[feed.weight/10];
							}
								break;
							case 1:	
							{
								SEG9[6]&=0XC0;
								SEG9[6]|=GRID_4[feed.weight%10];
								SEG9[7]&=0X0;
								SEG9[7]|=LED20[feed.weight%10];						
							}
								break;	
							default:break;
						}
					}			
				}

			}												
			set_back_flag = 0;
		}
				
		if (set_value_flag >= 6)
		{
			UART_PRINTF("222:%02X:%02X-%02X-%02X\n", num+1, feed.hour, feed.minute, feed.weight);

			feed.music = 0x01;
			if (feed.weight == 0)
			{
				feed.hour = 0xff;
				feed.minute = 0xff;
				feed.weight = 0xff;
				feed.music = 0xff;
			}
			UART_PRINTF("111:%02X:%02X-%02X-%02X\n", num+1, feed.hour, feed.minute, feed.weight);
			memcpy((uint8_t *) &feed_plan.plans[num], (uint8_t *) &feed , sizeof(FEED_INFO_t));
			UART_PRINTF("######################### save feed_plan 1\n");
			UART_PRINTF("222:%02X:%02X-%02X-%02X\n", num+1, feed_plan.plans[num].hour, feed_plan.plans[num].minute, feed_plan.plans[num].weight);	

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
	num =num - 1;
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
//		ht1621_disp(POWER_0, 1);
//		ht1621_disp(POWER_1, 1);
//		ht1621_disp(POWER_2, 1);
//		ht1621_disp(POWER_3, 1);
//		ht1621_disp(POWER_4, 1);
		
		SEG9[8]|=0X01;
		SEG9[8]|=0X04;
		SEG9[8]|=0X20;
		SEG9[9]|=0XF;
		SEG9[10]|=0X10;
		
	}
	else if (voltage < 570) 				//3.5
	{
//		ht1621_disp(POWER_0, 1);
//		ht1621_disp(POWER_1, 0);
//		ht1621_disp(POWER_2, 0);
//		ht1621_disp(POWER_3, 0);
//		ht1621_disp(POWER_4, 0);
		
		SEG9[8]&=~0X01;
		SEG9[8]&=~0X04;
		SEG9[8]&=~0X20;
		SEG9[9]&=~0XF;
		SEG9[10]|=0X10;

		
	} 
	else if (voltage < 610) 				//3.7
	{
//		ht1621_disp(POWER_0, 1);
//		ht1621_disp(POWER_1, 1);
//		ht1621_disp(POWER_2, 0);
//		ht1621_disp(POWER_3, 0);
//		ht1621_disp(POWER_4, 0);
		
		SEG9[8]&=~0X01;
		SEG9[8]&=~0X04;
		SEG9[8]&=~0X20;
		SEG9[9]|=0XF;
		SEG9[10]|=0X10;
		
	} 
	else if (voltage < 640) 				// = 3.9
	{
//		ht1621_disp(POWER_0, 1);
//		ht1621_disp(POWER_1, 1);
//		ht1621_disp(POWER_2, 1);
//		ht1621_disp(POWER_3, 0);
//		ht1621_disp(POWER_4, 0);
		
		SEG9[8]&=~0X01;
		SEG9[8]&=~0X04;
		SEG9[8]|=0X20;
		SEG9[9]|=0XF;
		SEG9[10]|=0X10;
		
	} 
	else if (voltage < 680) 				// = 4.2
	{
//		ht1621_disp(POWER_0, 1);
//		ht1621_disp(POWER_1, 1);
//		ht1621_disp(POWER_2, 1);
//		ht1621_disp(POWER_3, 1);
//		ht1621_disp(POWER_4, 0);
		
		SEG9[8]&=~0X01;
		SEG9[8]|=0X04;
		SEG9[8]|=0X20;
		SEG9[9]|=0X0F;
		SEG9[10]|=0X10;
		
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

void lock_led(void)
{
	static uint8_t reverse = 0;
	if(led_flag > 0)
	{
		if(reverse && (led_flag % 5 == 0))
		{
			SEG9[12]|=0X80;
			reverse = 0;
		}	
		else if(reverse == 0)
		{
			SEG9[12]&=~0X80;
			reverse = 1;
		}	
		led_flag --;						
	}
	
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
		if(key_value == KEY_LOCK_L && lock_flag)	
		{
			clean_key_flag();
						
			lock_flag = 0;
			
			unlock_flag = 1;
			led_flag = 10;
						
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
			case KEY_OK_L:
			{
				if(meal)
					set_meal_1(meal);
				else
					set_time();
								
			}
				break;			
			
			case KEY_FEED_L:	//长按删除喂食条目
			{								
				if (meal) 
				{
					del_meal(meal);
				}
			}
				break;
				
			case KEY_RECORD_L:			//录音
			{								
				record_flag = 1;
			}
				break;

			case KEY_RECORD_L_UP:		//录音结束
			{								
				record_flag = 0;
			}
				break;
			
			case KEY_FEED_S:	//手动喂食
			{				
				feed_info_func.hour = 0;
				feed_info_func.minute = 0;
				feed_info_func.weight = 1;//改
				#ifdef FEED_KEY_MUSIC_E
				feed_info_func.music = 1;
				#else
				feed_info_func.music = 0;
				#endif
				feed_run(&feed_info_func);
			}
				break;
			
			case KEY_UP_S:
			{				
				meal++;
				if(meal > 10)
				{
					meal = 0;
				}
			}
				break;
			
			case KEY_DOWN_S:
			{				
				if(meal > 0)
				{
					meal--;
				}
				else 
				{
					meal = 10;
				}
			}
				break;
									
			case KEY_SET_S:	
			{				
				if(meal && get_meal_flag == 1)
				{
					meal = 0;
				}
			}
				break;			
		}
		
		if (meal) 
		{
			get_feed_info_flag = 1;
			get_meal_flag = 1;
			get_feed_num(meal);
			
				if(key_value == KEY_SET_L)
				{
					get_meal_flag = 2;
					get_meal(meal);
				}		
				else if(key_value == KEY_LOCK_L)
				{
					get_meal_flag = 3;
					get_weight(meal);
				}
		}
		else
		{
			get_feed_info_flag = 0;
			get_meal_flag = 0;
			get_time();
		}
		
		key_flag = 0;
		key_scan_flag = 0;

	}
	else 		//二十秒后没有按键，重新锁键
	{
		if (lock_timeout == 0) 
		{
			UART_PRINTF("########## lock_timeout == 0  lock@@@@@@\r\n");

			get_feed_info_flag = 0;
			set_time_flag = 0;
						
			lock_flag = 1;					//设备锁标志
			
			unlock_flag = 0;
			
			key_scan_flag = 0;
			
			meal = 0;
		}
	}
}
#endif


#ifdef FEED_INFO_AUTO_TEST
#define u8 uint8_t
u8 test_feed_info_flag = 0;

void test_feed_info()
{
	UTCTimeStruct tm_s;

	utc_get_time(&tm_s);
	
	FEED_PLAN_t flash_feed_info;
	
	u8 i;
	u8 hour_test;
	u8 min_test;
	static u8 hour_old = 250;
	static u8 min_old = 25;
	
	if(test_feed_info_flag == 0)		//if(test_feed_info_flag == 0 && tm_s.year > 20)
	{
        #ifdef TY_PROTOCOL
        for(i = 0; i < FEED_MAX_NUM; i++)
        {
//            flash_feed_info.plans[i].week = 0x7F;
            if((tm_s.minutes + (2 * i) + 2) >= 60)
            {
                min_test = tm_s.minutes + (2 * i) + 2 - 60;
                hour_test = tm_s.hour + 1;
                if(hour_test >= 24)
                    hour_test = hour_test - 24;
            }
            else
            {
                min_test =  tm_s.minutes + (2 * i) + 2;  
                hour_test = tm_s.hour;
            }
            flash_feed_info.plans[i].minute = min_test;
            flash_feed_info.plans[i].hour = hour_test;
            flash_feed_info.plans[i].weight = i;
//            flash_feed_info.plans[i].allow_feed = 1;
	    }
		#else
        for(i = 0; i < FEED_MAX_NUM; i++)
        {
            if((tm_s.minutes + (2 * i) + 2) >= 60)
            {
                min_test = tm_s.minutes + (2 * i) + 2 - 60;
                hour_test = tm_s.hour + 1;
                if(hour_test >= 24)
                    hour_test = hour_test - 24;
            }
            else
            {
                min_test =  tm_s.minutes + (2 * i) + 2;  
                hour_test = tm_s.hour;
            }
//            flash_feed_info.plans[i].year = tm_s.year;
//            flash_feed_info.plans[i].month = tm_s.month;
//            flash_feed_info.plans[i].mday = 0x7F;
            flash_feed_info.plans[i].hour = hour_test;
            flash_feed_info.plans[i].minute = min_test;
//            printf("min_test:%d \n",min_test);

			flash_feed_info.plans[i].weight = 2 * (10 - i) + 1;
			
//            flash_feed_info.plans[i].weight_l = i * 10 + 10;
//            flash_feed_info.plans[i].weight_h = 0;
//            
//            flash_feed_info.plans[i].mode = 0x11;
//            flash_feed_info.plans[i].num = i + 1;
			
            flash_feed_info.plans[i].music = 1;
	    }        
        #endif
		
		delay_ms(100);
		test_feed_info_flag = 1;
		
		hour_old = hour_test;
		min_old = min_test;
		
		
		flash_erase_sector(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR);
		flash_write(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR, sizeof(FEED_PLAN_t), (uint8_t *)&flash_feed_info);
		
//		user_write_flash(FLASH_FEED_INFO_ADDR,(u8 *) &flash_feed_info, sizeof(FLASH_FEED_INFO_E));
		
//		#ifdef TY_PROTOCOL
//		send_feed_info();
//		#else
//		for(i = 0; i < FEED_MAX_NUM; i++)
//		{
//			cmd_result(0x02, (u8 *) &flash_feed_info.feed_info[i], sizeof(struct feed_infos));
//		}
//		#endif
		
//    	printf_rtc();
//	    printf_feed_info();

		UART_PRINTF("%04d-%02d-%02d %02d:%02d:%02d \r\n",
			tm_s.year, tm_s.month, tm_s.day,
			tm_s.hour, tm_s.minutes, tm_s.seconds);

		uint8_t a;	
		for(a = 0; a < 10; a++)
		{
			UART_PRINTF("%02X-%02X-%02X\n",flash_feed_info.plans[a].hour,flash_feed_info.plans[a].minute,flash_feed_info.plans[a].weight);	
		}

		
	}
    else if(hour_old <= tm_s.hour && min_old <= tm_s.minutes)
    {
//        printf_rtc();
        test_feed_info_flag = 0;
    }
    
}
#endif


void printf_flash_info(void)
{
	uint8_t a;
	
	for(a = 0; a < 10; a++)
	{
		UART_PRINTF("%02X:%02X-%02X-%02X\n",a+1, feed_plan.plans[a].hour,feed_plan.plans[a].minute,feed_plan.plans[a].weight);	
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
