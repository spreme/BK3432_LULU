/**
 ****************************************************************************************
 *
 * @file arch_main.c
 *
 * @brief Main loop of the application.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ******** ********************************************************************************
 */


/*
 * INCLUDES
 ****************************************************************************************
 */

#include "rwip_config.h" // RW SW configuration

#include "arch.h"      // architectural platform definitions
#include <stdlib.h>    // standard lib functions
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>   // boolean definition
#include "boot.h"      // boot definition
#include "rwip.h"      // RW SW initialization
#include "syscntl.h"   // System control initialization
#include "emi.h"       // EMI initialization
#include "intc.h"      // Interrupt initialization
#include "timer.h"     // TIMER initialization
#include "icu.h"
#include "flash.h"
#include "uart.h"      	// UART initialization
#include "flash.h"     // Flash initialization
//#include "led.h"       // Led initialization
#if (BLE_EMB_PRESENT || BT_EMB_PRESENT)
#include "rf.h"        // RF initialization
#endif // BLE_EMB_PRESENT || BT_EMB_PRESENT

#if (BLE_APP_PRESENT)
#include "app.h"       // application functions
#endif // BLE_APP_PRESENT

#if (NVDS_SUPPORT)
#include "nvds.h"                    // NVDS Definitions
#endif
#include "reg_assert_mgr.h"
#include "BK3432_reg.h"
#include "RomCallFlash.h"
#include "gpio.h"
#include "pwm.h"

#include "app_task.h"
#include "ir.h"
#include "oads.h"
#include "wdt.h"
#include "user_config.h"
#include "app_fcc0.h"
#include "utc_clock.h"
#include "tuya_ble_unix_time.h"
#include "ke_timer.h"
#include "tm1638.h"

//#include "delay.c"
/**
 ****************************************************************************************
 * @addtogroup DRIVERS
 * @{
 *
 *
 * ****************************************************************************************
 */

FEED_PLAN_t feed_plan;
SAVE_INFO_t save_info;
/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

// Creation of uart external interface api
struct rwip_eif_api uart_api;
/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */

static void Stack_Integrity_Check(void);

extern void code_sanity_check(void);

#if (UART_DRIVER)
void uart_rx_handler(uint8_t *buf, uint8_t len);
#endif
#if (UART2_DRIVER)
void uart2_rx_handler(uint8_t *buf, uint8_t len);
#endif

#if ((UART_PRINTF_EN) &&(UART_DRIVER || UART2_DRIVER))
void assert_err(const char *condition, const char * file, int line)
{
	UART_PRINTF("%s,condition %s,file %s,line = %d\r\n",__func__,condition,file,line);

}

void assert_param(int param0, int param1, const char * file, int line)
{
	UART_PRINTF("%s,param0 = %d,param1 = %d,file = %s,line = %d\r\n",__func__,param0,param1,file,line);

}

void assert_warn(int param0, int param1, const char * file, int line)
{
	UART_PRINTF("%s,param0 = %d,param1 = %d,file = %s,line = %d\r\n",__func__,param0,param1,file,line);

}

void dump_data(uint8_t* data, uint16_t length)
{
	UART_PRINTF("%s,data = %d,length = %d,file = %s,line = %d\r\n",__func__,data,length);
}
#else
void assert_err(const char *condition, const char * file, int line)
{

}

void assert_param(int param0, int param1, const char * file, int line)
{

}

void assert_warn(int param0, int param1, const char * file, int line)
{

}

void dump_data(uint8_t* data, uint16_t length)
{

}
#endif //UART_PRINTF_EN


/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */


void platform_reset(uint32_t error)
{
	//void (*pReset)(void);

	UART_PRINTF("error = %x\r\n", error);

	// Disable interrupts
	GLOBAL_INT_STOP();

#if UART_PRINTF_EN
	// Wait UART transfer finished
	#ifdef UART_1_PRINTF
	uart_finish_transfers();
	#endif
	#ifdef UART_2_PRINTF
	uart2_finish_transfers();
	#endif
#endif //UART_PRINTF_EN


	if(error == RESET_AND_LOAD_FW || error == RESET_TO_ROM)
	{
		// Not yet supported
	}
	else
	{
		//Restart FW
		//pReset = (void * )(0x0);
		//pReset();
		wdt_enable(10);
		while(1);
	}
}



void bdaddr_env_init(void)
{
	struct bd_addr co_bdaddr;
	flash_read(FLASH_SPACE_TYPE_MAIN, 0x27ff0/4, 6, &co_bdaddr.addr[0]);
	if(co_bdaddr.addr[0]!=0xff ||co_bdaddr.addr[1]!=0xff||
	        co_bdaddr.addr[2]!=0xff||co_bdaddr.addr[3]!=0xff||
	        co_bdaddr.addr[4]!=0xff||co_bdaddr.addr[5]!=0xff )
	{
		memcpy(&co_default_bdaddr,&co_bdaddr,6);
	}
}


void ble_clk_enable(void)
{
	REG_AHB0_ICU_BLECLKCON =  0;
}

/**
 *******************************************************************************
 * @brief RW main function.
 *
 * This function is called right after the booting process has completed.
 *
 * @return status   exit status
 *******************************************************************************
 */
void user_rtc_isr()
{

}

PWM_DRV_DESC timer_desc_2;

void beep_test(void)
{
	timer_desc_2.duty_cycle = 3;
	pwm_set_duty(&timer_desc_2);
	Delay_ms(100);
	timer_desc_2.duty_cycle = 0;
	pwm_set_duty(&timer_desc_2);
}

void beep_init(void)
{	
	timer_desc_2.channel = 2;
	timer_desc_2.mode = 1<<0 | 0<<1 | 0<<2 | 0<<4;
	timer_desc_2.end_value = 6;
	timer_desc_2.duty_cycle = 0;
	pwm_init(&timer_desc_2);
}

void flash_data_init(uint8_t type)
{
	//复位
	UART_PRINTF("flash_data_init:%d \n",type);
	if(type > 0)
	{
		#if defined NO_RECORD_FUNC || defined KEY_BUZZER_FUNC
		pwm_buzzer_en(6);
		Delay_ms(100);
		pwm_buzzer_en(0);
		Delay_ms(200);
		pwm_buzzer_en(6);
		Delay_ms(100);
		pwm_buzzer_en(0);
		Delay_ms(50);
		pwm_buzzer_en(6);
		Delay_ms(100);
		pwm_buzzer_en(0);
		#else
		gpio_set(SOUND_REC, RECORD_ON);
		#ifdef NEW_RECORD_IC
		Delay_ms(1300);
		#else
		Delay_ms(300);	
		#endif
		gpio_set(SOUND_REC, RECORD_OFF);
		#endif

		memset(&feed_plan, 0, sizeof(FEED_PLAN_t));
		feed_plan.mark = FLASH_KEEP_VAL;
//		flash_erase(FLASH_SPACE_TYPE_NVR,BLE_PLAN_ADDR,FLASH_SIZE_ONE);
		flash_erase_sector(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR);
		UART_PRINTF("reset feed_plan 1\n");
		flash_write(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR, sizeof(FEED_PLAN_t), (uint8_t *)&feed_plan);

		memset(&save_info, 0, sizeof(SAVE_INFO_t));
		save_info.mark = FLASH_KEEP_VAL;
//		flash_erase(FLASH_SPACE_TYPE_NVR,BLE_SAVE_ADDR,FLASH_SIZE_ONE);
		flash_erase_sector(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR);
		UART_PRINTF("reset save_info 1\n");
		save_info.led_backlight_pwm = 5;
		flash_write(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR, sizeof(SAVE_INFO_t), (uint8_t *)&save_info);
		
		
		Delay_ms(500);
		wdt_enable(10);
		while(1);
	}
	else
	{
		memset(&feed_plan, 0, sizeof(FEED_PLAN_t));
		memset(&save_info, 0, sizeof(SAVE_INFO_t));
		
		flash_read(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR, sizeof(SAVE_INFO_t), (uint8_t *) &save_info);
		flash_read(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR, sizeof(FEED_PLAN_t), (uint8_t *) &feed_plan);
		printf_flash_info();
		if(feed_plan.mark != FLASH_KEEP_VAL)
		{
			UART_PRINTF("reset flash  feed_plan\r\n");
			memset(&feed_plan, 0, sizeof(FEED_PLAN_t));
			feed_plan.mark = FLASH_KEEP_VAL;
//			flash_erase(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR, FLASH_SIZE_ONE);
			flash_erase_sector(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR);
			flash_write(FLASH_SPACE_TYPE_NVR, BLE_PLAN_ADDR, sizeof(FEED_PLAN_t), (uint8_t *)&feed_plan);
		}
		
		if(save_info.mark != FLASH_KEEP_VAL)
		{
			UART_PRINTF("reset flash  save_info \r\n");

			memset(&save_info, 0, sizeof(SAVE_INFO_t));
			save_info.mark = FLASH_KEEP_VAL;
//			flash_erase(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR, FLASH_SIZE_ONE);
			flash_erase_sector(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR);
			flash_write(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR, sizeof(SAVE_INFO_t), (uint8_t *)&save_info);
		}
	}
}


extern struct rom_env_tag rom_env;

void rwip_eif_api_init(void);
void rw_main(void)
{
	uint8_t utc_flag = 0;
	
	/*
	 ***************************************************************************
	 * Platform initialization
	 ***************************************************************************
	 */
	
#if SYSTEM_SLEEP
	uint8_t sleep_type = 0;
#endif
	icu_init();

	// Initialize random process	初始化随机过程
	srand(1);

	//get System sleep flag	获取系统睡眠标志
	system_sleep_init();

	// Initialize the exchange memory interface	初始化exchange内存接口
	emi_init();

	// Initialize timer module	初始化定时器模块
	timer_init();

	rwip_eif_api_init();

	// Initialize the Interrupt Controller	初始化中断控制器
	intc_init();
	// Initialize UART component
#if (UART_DRIVER)
	uart_init(115200);
	uart_cb_register(uart_rx_handler);
#endif
#if (UART2_DRIVER)
	uart2_init(115200);
	uart2_cb_register(uart2_rx_handler);
#endif

#if PLF_NVDS
	// Initialize NVDS module
	struct nvds_env_tag env;
	env.flash_read = &flash_read;
	env.flash_write = &flash_write;
	env.flash_erase = &flash_erase;
	nvds_init(env);
#endif

	flash_init();
	rom_env_init(&rom_env);

	/*
	  ***************************************************************************
	  * RW SW stack initialization
	  ***************************************************************************
	  */
	//enable ble clock
	ble_clk_enable();

	// Initialize RW SW stack
	rwip_init(0);

	bdaddr_env_init();

	adc_init(0,1);
	gpio_init();

	REG_AHB0_ICU_INT_ENABLE |= (0x01 << 15); //BLE INT
	REG_AHB0_ICU_IRQ_ENABLE = 0x03;

	rwip_prevent_sleep_set(BK_DRIVER_TIMER_ACTIVE);	//关闭低功耗
	// finally start interrupt handling
	GLOBAL_INT_START();

	UART_PRINTF("start \r\n");

	UART_PRINTF("version:%s \r\n",USER_VERSION);
	UART_PRINTF("data:%s \r\n",USER_DATA);

	beep_init();
//	#ifdef BACKLIGHT_CONTROL
//	backlight_init();
//	#endif
	
	utc_update();
	utc_set_clock(1);
	
	flash_data_init(0);

//	ht1621_init();
//	ht1621_clean();					//清屏
//	ht1621_disp(LOCK, 1);			//显示锁
//	ht1621_disp(LOCK_CLOSE, 1);		//显示锁关闭

	disp_voltage();					//显示电量
	get_time();

	/*
	 ***************************************************************************
	 * Main loop
	 ***************************************************************************
	 */
	 
//	init_TM1638();
	TM1638_init();
	
	while(1)
	{		
		//schedule all pending events
				
		rwip_schedule();
		wdt_enable(0xffff);
		
//		TM1638_Write_DATA(0x00,0xff);
//		TM1638_Write_DATA(0x02,0xff);
//		TM1638_Write_DATA(0x04,0xff);
//		TM1638_Write_DATA(0x06,0xff);

		
		SEG9[4]|=0X13;
		SEG9[5]|=0XF;
	
		
		if(utc_flag == 0)
		{
			utc_flag = 1;
			get_time();
			ke_timer_set(UTC_TASK, TASK_APP, 10);
		}

		if(feed_one_flag)
		{
			UART_PRINTF("feed_one_flag\n");
			if(lock_flag == 0)
			{
				beep_test();
				lock_timeout = LOCK_TIMEOUT_TIME;
			}
			
			feed_info_func.hour = 0;
			feed_info_func.minute = 0;
			feed_info_func.weight = 1;//改
			#ifdef FEED_KEY_MUSIC_E
			feed_info_func.music = 1;
			#else
			feed_info_func.music = 0;
			#endif
			
			feed_error = feed_run(&feed_info_func);
			
			feed_one_flag = 0;
			
			beep_flag = 0;
			
			set_key_tick = 0;				//设置按键计时
			dowm_key_tick = 0;				//下按键计时
			up_key_tick = 0;				//上按键计时
			lock_key_tick = 0;				//锁键按键计时
			feed_key_tick = 0;				//喂食按键计时
			ok_key_tick = 0;
			key_flag = 0;
		}
				
		//按键复位	
		if(reset_flag == 1)						//按键按下没松开
		{
			UART_PRINTF(" reset the mcu!\r\n");
			flash_data_init(1); 				//清空单片机,恢复出厂
			
//			record_reset_control();
			wdt_enable(10);
		}
		
		if(check_feed_flag)						//喂食检测：一分钟置1一次
		{
			feed_scan(&feed_info_func);					//扫描eeprom判断是否需要执行定时喂食
			check_feed_flag = 0;
		}
		else if(feed_detect_again)					//再次喂食检测
		{
			if((feed_error & ERROR_IR) == 0)		//不是红外错误，出粮口正常
			{
				feed_required = FEED_OLD;			//饲料所需
			}
		}
		
		if(feed_required > 0)
		{
			feed_error = feed_run(&feed_info_func);
			
			feed_required = 0;

			if(feed_error & ERROR_IR)			//喂食红外错误
			{
				UART_PRINTF(" feed_error & ERROR_IR \r\n");
				feed_detect_again = 1;			//红外错误：确保喂食错误，再次喂食检测
			}
			else
			{
				feed_detect_again = 0;
			}
		}
		
		key_func();
		
//		test_feed_info();
		
		// Checks for sleep have to be done with interrupt disabled
		//睡眠检查必须在中断被禁用的情况下进行
		GLOBAL_INT_DISABLE();				//全局中断暂停

		oad_updating_user_section_pro();

		if(wdt_disable_flag==1)
		{
			wdt_disable();
		}
#if SYSTEM_SLEEP
		// Check if the processor clock can be gated
		sleep_type = rwip_sleep();
		if((sleep_type & RW_MCU_DEEP_SLEEP) == RW_MCU_DEEP_SLEEP)
		{
			// 1:idel  0:reduce voltage
			if(icu_get_sleep_mode())
			{
				cpu_idle_sleep();
			}
			else
			{
				cpu_reduce_voltage_sleep();
			}
		}
		else if((sleep_type & RW_MCU_IDLE_SLEEP) == RW_MCU_IDLE_SLEEP)
		{
			cpu_idle_sleep();
		}
#endif
		Stack_Integrity_Check();				//堆栈完整性检查
		GLOBAL_INT_RESTORE();					//全局中断复位
	}
}


#if (UART_DRIVER)
static void uart_rx_handler(uint8_t *buf, uint8_t len)
{
	for(uint8_t i=0; i<len; i++)
	{
		UART_PRINTF("0x%x ", buf[i]);
	}
	UART_PRINTF("\r\n");
}
#endif
#if (UART2_DRIVER)
static void uart2_rx_handler(uint8_t *buf, uint8_t len)
{
	for(uint8_t i=0; i<len; i++)
	{
		UART_PRINTF("0x%x ", buf[i]);
	}
	UART_PRINTF("\r\n");
}
#endif



void rwip_eif_api_init(void)
{
	#ifdef UART_1_INIT
	uart_api.read = &uart_read;
	uart_api.write = &uart_write;
	uart_api.flow_on = &uart_flow_on;
	uart_api.flow_off = &uart_flow_off;
	#endif
	
	#ifdef UART_2_INIT
	uart_api.read = &uart2_read;
	uart_api.write = &uart2_write;
	uart_api.flow_on = &uart2_flow_on;
	uart_api.flow_off = &uart2_flow_off;	
	#endif
}

const struct rwip_eif_api* rwip_eif_get(uint8_t type)
{
	const struct rwip_eif_api* ret = NULL;
	switch(type)
	{
	case RWIP_EIF_AHI:
	{
		ret = &uart_api;
	}
	break;
#if (BLE_EMB_PRESENT) || (BT_EMB_PRESENT)
	case RWIP_EIF_HCIC:
	{
		ret = &uart_api;
	}
	break;
#elif !(BLE_EMB_PRESENT) || !(BT_EMB_PRESENT)
	case RWIP_EIF_HCIH:
	{
		ret = &uart_api;
	}
	break;
#endif
	default:
	{
		ASSERT_INFO(0, type, 0);
	}
	break;
	}
	return ret;
}

static void Stack_Integrity_Check(void)
{
	if ((REG_PL_RD(STACK_BASE_UNUSED)!= BOOT_PATTERN_UNUSED))
	{
		while(1)
		{
			UART_PUTCHAR("Stack_Integrity_Check STACK_BASE_UNUSED fail!\r\n");
		}
	}

	if ((REG_PL_RD(STACK_BASE_SVC)!= BOOT_PATTERN_SVC))
	{
		while(1)
		{
			UART_PUTCHAR("Stack_Integrity_Check STACK_BASE_SVC fail!\r\n");
		}
	}

	if ((REG_PL_RD(STACK_BASE_FIQ)!= BOOT_PATTERN_FIQ))
	{
		while(1)
		{
			UART_PUTCHAR("Stack_Integrity_Check STACK_BASE_FIQ fail!\r\n");
		}
	}

	if ((REG_PL_RD(STACK_BASE_IRQ)!= BOOT_PATTERN_IRQ))
	{
		while(1)
		{
			UART_PUTCHAR("Stack_Integrity_Check STACK_BASE_IRQ fail!\r\n");
		}
	}

}


void rom_env_init(struct rom_env_tag *api)
{
	memset(&rom_env,0,sizeof(struct rom_env_tag));
	rom_env.prf_get_id_from_task = prf_get_id_from_task;
	rom_env.prf_get_task_from_id = prf_get_task_from_id;
	rom_env.prf_init = prf_init;
	rom_env.prf_create = prf_create;
	rom_env.prf_cleanup = prf_cleanup;
	rom_env.prf_add_profile = prf_add_profile;
	rom_env.rwble_hl_reset = rwble_hl_reset;
	rom_env.rwip_reset = rwip_reset;
#if SYSTEM_SLEEP
	rom_env.rwip_prevent_sleep_set = rwip_prevent_sleep_set;
	rom_env.rwip_prevent_sleep_clear = rwip_prevent_sleep_clear;
	rom_env.rwip_sleep_lpcycles_2_us = rwip_sleep_lpcycles_2_us;
	rom_env.rwip_us_2_lpcycles = rwip_us_2_lpcycles;
	rom_env.rwip_wakeup_delay_set = rwip_wakeup_delay_set;
#endif
	rom_env.platform_reset = platform_reset;
	rom_env.assert_err = assert_err;
	rom_env.assert_param = assert_param;
	#ifdef UART_1_INIT
	rom_env.Read_Uart_Buf = Read_Uart_Buf;
	rom_env.uart_clear_rxfifo = uart_clear_rxfifo;
	#endif
	#ifdef UART_2_INIT
	rom_env.Read_Uart2_Buf = Read_Uart2_Buf;
	rom_env.uart2_clear_rxfifo = uart2_clear_rxfifo;
	#endif

}

/// @} DRIVERS
