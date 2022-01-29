int set_val(uint8_t addr, int dat, char min, char max)
{
	char val = dat;
	int status;
	uint8_t key_value;
	int timeout = 700;
	uint8_t reverse;
	uint8_t lcd_update_old = 0;
	uint8_t lcd_update_time = 0;

	UART_PRINTF("########## enter set_val \r\n");
//	set_val_flag = 1;						//ÕýÔÚÉèÖÃÊýÖµ±êÖ¾
//	Delay_ms(200);
	while (timeout) 
	{
		rwip_schedule();
		wdt_enable(0x3fff);

//		MCU_IDLE();
//		UART_PRINTF("timeout:%d \r\n",timeout);

		key_value = key_scan();

		if (key_value) 
		{			
			lock_timeout = LOCK_TIMEOUT_TIME;//ÎÞ²Ù×÷20sºóËøÆÁ
			clean_key_flag();
//			timeout = 500;
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
					dat = val;
					status = val;
					set_back_flag = 2;
					
					if(set_time_flag)
					{
						set_val_flag = 2;
						goto back;
					}
					else if(set_meal_flag)
					{
						set_val_flag = 2;
						goto back;
					}
				}
					break;
				
				case KEY_RECORD_S:
				{						
					dat = val;
					status = val;
					set_back_flag = 1;
					goto back;
				}
					break;
				
				case KEY_DOWN_S:
				{					
					if(val > min)
						val--;
					else if(val == min)
						val = max;
					
					timeout = 500;
				}
					break;
									
				case KEY_UP_S:
				{					
					val++;
					if (val > max)
						val = min;
					
					timeout = 500;
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
					
					default:
					break;
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
	
	set_val_flag = 0;			//ÕýÔÚÉèÖÃÊýÖµ±êÖ¾

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
	set_val_flag = 1;						//ÕýÔÚÉèÖÃÊýÖµ±êÖ¾
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
			lock_timeout = LOCK_TIMEOUT_TIME;//ÎÞ²Ù×÷20sºóËøÆÁ
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
	
	set_val_flag = 0;			//ÕýÔÚÉèÖÃÊýÖµ±êÖ¾

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
	set_val_flag = 1;						//ÕýÔÚÉèÖÃÊýÖµ±êÖ¾
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
			lock_timeout = LOCK_TIMEOUT_TIME;//ÎÞ²Ù×÷20sºóËøÆÁ
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
	
	set_val_flag = 0;			//ÕýÔÚÉèÖÃÊýÖµ±êÖ¾

	return status;
}



//»ñÈ¡Ê±¼äÏÔÊ¾
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

//ÉèÖÃÊ±¼ä
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
				display_flag = 1;				//µÚÒ»´ÎÏÔÊ¾µÚÒ»Î»»áµ¼ÖÂºóÃæÏÔÊ¾Òì³££¬´«ÊýÖµ½øÈ¥ÏÔÊ¾Ò»´ÎµÚ¶þÎ»½â¾ö
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

//»ñÈ¡Î¹Ê³¼Æ»®
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

//ÉèÖÃÎ¹Ê³¼Æ»®
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

				display_flag = 1;				//µÚÒ»´ÎÏÔÊ¾µÚÒ»Î»»áµ¼ÖÂºóÃæÏÔÊ¾Òì³££¬´«ÊýÖµ½øÈ¥ÏÔÊ¾Ò»´ÎµÚ¶þÎ»½â¾ö
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

//				display_flag = 1;				//µÚÒ»´ÎÏÔÊ¾µÚÒ»Î»»áµ¼ÖÂºóÃæÏÔÊ¾Òì³££¬´«ÊýÖµ½øÈ¥ÏÔÊ¾Ò»´ÎµÚ¶þÎ»½â¾ö
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
				
//				display_flag = 1;				//µÚÒ»´ÎÏÔÊ¾µÚÒ»Î»»áµ¼ÖÂºóÃæÏÔÊ¾Òì³££¬´«ÊýÖµ½øÈ¥ÏÔÊ¾Ò»´ÎµÚ¶þÎ»½â¾ö
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

//				display_flag = 1;				//µÚÒ»´ÎÏÔÊ¾µÚÒ»Î»»áµ¼ÖÂºóÃæÏÔÊ¾Òì³££¬´«ÊýÖµ½øÈ¥ÏÔÊ¾Ò»´ÎµÚ¶þÎ»½â¾ö
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
				
//				display_flag = 1;				//µÚÒ»´ÎÏÔÊ¾µÚÒ»Î»»áµ¼ÖÂºóÃæÏÔÊ¾Òì³££¬´«ÊýÖµ½øÈ¥ÏÔÊ¾Ò»´ÎµÚ¶þÎ»½â¾ö
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

