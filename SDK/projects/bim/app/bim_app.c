

//#include <stdint.h>
#include "bim_flash.h"
#include "bim_app.h"
#include "BK3432_reg.h"
#include "bim_uart.h"
#include "bim_updataImage.h"



typedef void (*FUNCPTR)(void);

void  updata_memset32(void * dest, uint32_t value, uint32_t size)
{
	uint32_t *_u8_dest = (uint32_t *)dest;
	uint32_t *_u8_end  = (uint32_t *)dest+size*4;

	while (_u8_dest < _u8_end)
	{
		*_u8_dest++ = value;
	}

}


void bim_icu_init(uint8_t clk)
{
	REG_AHB0_ICU_FLASH &= ~(0xff << 16);
	REG_AHB0_ICU_FLASH |= (0x15 << 16);
	REG_AHB0_ICU_CPU_STATUS  = 0x771;

	REG_AHB0_ICU_DIGITAL_PWD = REG_AHB0_ICU_DIGITAL_PWD & (~0X02);

	REG_AHB0_ICU_CORECLKCON = 0X00;  //clk div 0

	REG_AHB0_ICU_CLKSRCSEL = 0X000001F9; //usr 16m, sel=0, dec=7,normal=7

	REG_AHB0_ICU_ANA_CTL &= ~(0X01 << 6);
	
}




void bim_main(void)
{
	extern uint16_t uart_buff_write;
	extern uint8_t bim_uart_rx_buf[BIM_UART0_RX_FIFO_MAX_COUNT];
	extern uint8_t uart_download_status;

	static uint16_t bim_uart_temp,uart_buff_read;
	static int32_t  check_cnt=0;
	static uint8_t read_buf[16];
	static uint16_t rom_ver;

	int32_t main_point = SEC_IMAGE_RUN_STACK_FADDR;

	updata_memset32((uint8_t *)0x00400000, 1, 1);

	bim_icu_init(ICU_CLK_16M);

	bim_flash_init();

	bim_uart_init(115200);

	bim_flash_read(FLASH_SPACE_TYPE_MAIN, SEC_IMAGE_OAD_HEADER_STACK_FADDR/4, 16, read_buf);

	rom_ver = ( read_buf[14]|(read_buf[15]<<8) );

	REG_AHB0_ICU_INT_ENABLE = (0x01 << 5);
	REG_AHB0_ICU_IRQ_ENABLE = 0x03;
	__enable_irq();

	//bim_uart_write("boot start\r\n");
	//REG_APB5_GPIOD_CFG = 0Xff;
	//REG_APB5_GPIOD_DATA = 0X00;

	while(1)
	{

		bim_uart_temp=uart_buff_write;
		if (uart_buff_read < bim_uart_temp)
		{
			//REG_APB5_GPIOD_DATA = 0X00;
			bim_uart_data_callback(bim_uart_rx_buf + uart_buff_read, bim_uart_temp - uart_buff_read);
			uart_buff_read = bim_uart_temp;

		}
		else if (uart_buff_read > bim_uart_temp)
		{
			bim_uart_data_callback(bim_uart_rx_buf + uart_buff_read, sizeof(bim_uart_rx_buf) - uart_buff_read);
			bim_uart_data_callback(bim_uart_rx_buf, bim_uart_temp);
			uart_buff_read = bim_uart_temp;
			//REG_APB5_GPIOD_DATA = 0X00;
		}
		else
		{

			if(uart_download_status==0 && rom_ver!=0xffff)
			{
				if(check_cnt++>50000) //30ms
				{
					check_cnt=0;
					//REG_APB5_GPIOD_DATA = 0XFF;

					break;
				}
			}
		}
	}

	__disable_irq( );
	REG_AHB0_ICU_INT_ENABLE = 0x00;
	REG_AHB0_ICU_IRQ_ENABLE = 0x00;
	updata_memset32((uint8_t *)0x00400000, 0, 1);
	bim_uart_deinit();

	if(1 == bim_select_sec())
	{
		//bim_uart_write("image-main_RUN ADDR = 0x2010\r\n");
		(*(FUNCPTR)main_point)();
	}


}

