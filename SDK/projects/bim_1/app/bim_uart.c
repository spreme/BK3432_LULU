#include <stddef.h>     // standard definition
#include "timer.h"      // timer definition
#include "bim_uart.h"       // uart definition
#include "BK3432_reg.h"
#include <stdlib.h>    // standard lib functions
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>   // boolean definition
#include "string.h"
#include <stdlib.h>
#include  <stdarg.h>
#include  <stdio.h>
#include "bim_app.h"
#include "bim_flash.h"






#define Uart_Write_Byte(v)               (REG_APB3_UART_PORT=v)
#define UART_TX_FIFO_COUNT               (REG_APB3_UART_FIFO_STAT&0xff)
#define UART_RX_FIFO_COUNT               ((REG_APB3_UART_FIFO_STAT>>8)&0xff)
#define UART_TX_FIFO_FULL                (REG_APB3_UART_FIFO_STAT&0x00010000)
#define UART_TX_FIFO_EMPTY               (REG_APB3_UART_FIFO_STAT&0x00020000)
#define UART_RX_FIFO_FULL                (REG_APB3_UART_FIFO_STAT&0x00040000)
#define UART_RX_FIFO_EMPTY               (REG_APB3_UART_FIFO_STAT&0x00080000)
#define UART_TX_WRITE_READY              (REG_APB3_UART_FIFO_STAT&0x00100000)
#define UART_RX_READ_READY               (REG_APB3_UART_FIFO_STAT&0x00200000)
#define bit_UART_TXFIFO_NEED_WRITE        0x01
#define bit_UART_RXFIFO_NEED_READ         0x02
#define bit_UART_RXFIFO_OVER_FLOW         0x04
#define bit_UART_RX_PARITY_ERROR          0x08
#define bit_UART_RX_STOP_ERROR            0x10
#define bit_UART_TX_PACKET_END            0x20
#define bit_UART_RX_PACKET_END            0x40
#define bit_UART_RXD_WAKEUP_DETECT        0x80




#define uart_tx_en    0x1      // 0: Disable TX, 1: Enable TX 
#define uart_rx_en    0x1      // 0: Disable RX, 1: Enable RX
#define irda_mode     0x0      // 0: UART  MODE, 1: IRDA MODE
#define data_len      0x3      // 0: 5 bits, 1: 6 bits, 2: 7 bits, 3: 8 bits
#define parity_en     0x1      // 0: NO Parity, 1: Enable Parity
#define parity_mode   0x1      // 0: Odd Check, 1: Even Check 
#define stop_bits     0x0      // 0: 1 stop-bit, 1: 2 stop-bit 
#define uart_clks     16000000 // UART's Main-Freq, 26M 
#define baud_rate     115200 // UART's Baud-Rate,  1M 
#define BIM_REG_PL_RD(addr)              (*(volatile uint32_t *)(addr))

int bim_uart_printf_null(const char *fmt,...)
{
	return 0;
}

#if 0
int bim_uart_printf(const char *fmt,...)
{
	return 0;
}

int bim_uart_putchar(char * st)
{
	uint8_t num = 0;

	while (*st)
	{
		if(UART_TX_WRITE_READY)
		{
			REG_APB3_UART_PORT = *st;
			st++;
			num++;
		}
	}

	return num;
}
#endif
void uart_send(unsigned char *buff, int len)
{
	while (len--)
	{
		while(!UART_TX_WRITE_READY);
		REG_APB3_UART_PORT = *buff++ ;
	}
}
#if 0
uint8_t bim_uart_fifo_empty_getf(void)
{
	uint32_t localVal = BIM_REG_PL_RD(0x00806308);
	return ((localVal & ((uint32_t)0x00020000)) >> 17);
}

void bim_uart_send_byte( char data)
{
	while (!bim_uart_fifo_empty_getf());

	REG_APB3_UART_PORT = data ;
}
#endif
void bim_uart_write( char *buff)
{
#if 0
	uint8_t len = strlen((char*)buff);
	while (len--)
		bim_uart_send_byte(*buff++);
#endif
}




void bim_printf(char *title, uint32_t v, uint8_t radix)
{
#if 0
	uint8_t	tmpLen;
	char buf[128];
	uint32_t err;

	tmpLen = strlen( (char*)title);
	memcpy( buf, title, tmpLen );
	err = (uint32_t)(v);
	itoa_my( err, &buf[tmpLen], radix);
	bim_uart_write(&buf[0]);
#endif
}


void bim_uart_init(uint32_t baudrate)
{

	unsigned int baud_divisor ;
	REG_AHB0_ICU_UARTCLKCON   &= ~(0x1 << 0) ;  // Enable Uart's Clocks
	baud_divisor = 0x8a;//uart_clks/baud_rate ;
	baud_divisor = baud_divisor-1 ;
	REG_APB3_UART_CFG  = (baud_divisor<<8) +
	                     (stop_bits   <<7) +
	                     //(parity_mode <<6) +
	                     //(parity_en   <<5) +
	                     (data_len    <<3) +
	                     (irda_mode   <<2) +
	                     (uart_rx_en  <<1) +
	                     uart_tx_en       ;
	REG_APB3_UART_FIFO_CFG  = 0x00001001 ;  // Set Fifo threshold 8
	REG_APB3_UART_INT_ENABLE = ((0x01 << 1) | (0x01 << 6) | (0x01 << 7));  //need read / stop end /rxd wake up
	REG_APB3_UART_FLOW_CFG  = 0x00000000 ;  // No Flow Control
	REG_APB3_UART_WAKE_CFG  =  ((0x01 << 0 )| (0x01 << 20) |  (0x01 << 21)| (0x01 << 22));  // No Wake Control

	REG_APB5_GPIOA_CFG  &= ~((0x3<<BIT_GPIO_PULL_UP)  + (0x3<<BIT_GPIO_PERI_EN));
	REG_APB5_GPIOA_CFG  |= ((0x3<<BIT_GPIO_PULL_UP));
	REG_APB5_GPIOA_CFG  |=   (0x3<<BIT_GPIO_OUT_EN_N);

	REG_APB5_GPIOA_DATA &= ~ (0x3<<BIT_GPIO_INPUT_EN);

}



#if 1 //
void bim_uart_deinit( void )
{
	REG_AHB0_ICU_UARTCLKCON   = 0x1 ;  // disable Uart's Clocks

	REG_APB5_GPIOA_CFG  &= ~((0x3<<BIT_GPIO_PULL_UP)  + (0x3<<BIT_GPIO_PERI_EN));
	REG_APB5_GPIOA_CFG  |= ((0x3<<BIT_GPIO_PERI_EN));

}


uint8_t erase_fenable;
uint8_t bim_uart_rx_buf[BIM_UART0_RX_FIFO_MAX_COUNT];
uint8_t bim_uart_cmd[16];
uint8_t bim_uart_data[4096+8];
//uint8_t read_data[512];
uint8_t uart_download_status=0;
uint16_t uart_buff_write;

void bim_uart_isr(void)
{
	unsigned int IntStat;

	IntStat = REG_APB3_UART_INT_STAT;
	if(IntStat & 0x42)
	{
		while((REG_APB3_UART_FIFO_STAT & (0x01 << 21)))
		{
			bim_uart_rx_buf[uart_buff_write++] = ((REG_APB3_UART_PORT>>8)&0xff);
			if( BIM_UART0_RX_FIFO_MAX_COUNT == uart_buff_write )
			{
				uart_buff_write = 0;
			}
		}

	}
	REG_APB3_UART_INT_STAT=IntStat;
}


void bim_uart_data_callback( uint8_t * buff, uint16_t len)
{
	static uint8_t cmd_status=0;
	static uint16_t index=0,index_cnt=0;

	static uint16_t length;
	static uint16_t scmd_length;
	static uint32_t write_addr;
	static uint32_t read_addr;
	static uint8_t read_buff[256];


	while(len>0)
	{
		switch(cmd_status)
		{

		case UART_CMD_STATE_HEAD:
		{
			if(buff[0]==0x01)
			{
				cmd_status=UART_CMD_STATE_OPCODE_ONE;
			}
			else
				cmd_status=UART_CMD_STATE_HEAD;
		}
		break;
		case UART_CMD_STATE_OPCODE_ONE:
		{
			if( buff[0]==0xe0 )
				cmd_status=UART_CMD_STATE_OPCODE_TWO;
			else
				cmd_status=UART_CMD_STATE_HEAD;


		}
		break;
		case UART_CMD_STATE_OPCODE_TWO:
		{
			if( buff[0]==0xfc )
				cmd_status=UART_CMD_STATE_LENGTH;
			else
				cmd_status=UART_CMD_STATE_HEAD;
		}
		break;
		case UART_CMD_STATE_LENGTH:
		{
			length=buff[0];

			if(0xff==buff[0])
			{
				cmd_status=UART_CMD_STATE_CMD_FLASH;
			}
			else if( buff[0]>0 && buff[0]!=0xff )
			{
				cmd_status=UART_CMD_STATE_CMD;
				index=0;
			}
			else
				cmd_status=UART_CMD_STATE_HEAD;


		}
		break;

		case UART_CMD_STATE_CMD:
		{
			bim_uart_cmd[index++]=buff[0];

			if(index==length)
			{

				uart_cmd_dispath(bim_uart_cmd, length);
				cmd_status=UART_CMD_STATE_HEAD;
			}
		}
		break;

		case UART_CMD_STATE_CMD_FLASH:
		{
			if( buff[0]==0xf4 )
				cmd_status=UART_CMD_STATE_LENGTH_FLASH_LEN0;
			else
				cmd_status=UART_CMD_STATE_HEAD;
		}
		break;
		case UART_CMD_STATE_LENGTH_FLASH_LEN0:
		{
			cmd_status=UART_CMD_STATE_LENGTH_FLASH_LEN1;
			scmd_length=buff[0];

		}
		break;
		case UART_CMD_STATE_LENGTH_FLASH_LEN1:
		{

			scmd_length += (buff[0]<<8);

			if(scmd_length>0)
				cmd_status=UART_CMD_STATE_LENGTH_FLASH_SCMD;
			else
				cmd_status=UART_CMD_STATE_HEAD;

			index=0;
			index_cnt=0;
		}
		break;

		case UART_CMD_STATE_LENGTH_FLASH_SCMD:
		{
			bim_uart_data[index++]=buff[0];

			/*
			 if(bim_uart_data[0]==0x0a && index==scmd_length )
			 {
			     int32_t addr = SEC_IMAGE_ALLOC_END_FADDR ;

			     bim_fflash_wr_protect_8k();
				bim_fflash_erase_one_block(addr);

				addr -= FLASH_ONE_BLOCK_SIZE;
				bim_fflash_erase_one_block(addr);

				addr -= FLASH_ONE_BLOCK_SIZE;
				bim_fflash_erase_one_block(addr);

			     addr -= FLASH_ONE_BLOCK_SIZE;
				bim_fflash_erase_one_block(addr);

				addr -= FLASH_SEC_SIZE;
				for(;addr >= SEC_IMAGE_ALLOC_START_STACK_FADDR;addr-= FLASH_SEC_SIZE )
				{
					bim_erase_section(addr,FLASH_SEC_SIZE);
				}

			     operate_flash_cmd_response(0x0a,0,3,&bim_uart_data[1]);
			     cmd_status=UART_CMD_STATE_HEAD;
			 }*/



			if(bim_uart_data[0]==0x09 && index==scmd_length)//read id
			{
				read_addr = bim_uart_data[1]|(bim_uart_data[2]<<8)|(bim_uart_data[3]<<16)|(bim_uart_data[4]<<24) ;

				if(read_addr<0x27ff0)
				{
					bim_uart_data[5]=scmd_length-5;
					operate_flash_cmd_response(0x09,6,7,&bim_uart_data[1]);
				}
				else
				{
					bim_flash_read(FLASH_SPACE_TYPE_MAIN, read_addr/4, 16, read_buff);
					for(uint8_t i=0; i<16; i++)
						bim_uart_data[5+i]=read_buff[i];

					operate_flash_cmd_response(0x09,0,4102,&bim_uart_data[1]);

				}
				cmd_status=UART_CMD_STATE_HEAD;

			}
			else if(bim_uart_data[0]==0x0f && index==scmd_length)//erase
			{
				int32_t addr = bim_uart_data[2]|(bim_uart_data[3]<<8)|(bim_uart_data[4]<<16)|(bim_uart_data[5]<<24) ;

				if(addr<0x1000)
				{
					operate_flash_cmd_response(0x0f,6,0x07,&bim_uart_data[1]);
				}
				else
				{
					if(erase_fenable==1)
					{
						//if(addr>=0x20000)
						// 128k
						//bim_fflash_wr_protect_128k();
						//else
						// 32k
						//bim_fflash_wr_protect_8k();
						erase_fenable=0;
					}


					if(bim_uart_data[1]==0x20)
					{
						//bim_erase_section(addr,FLASH_SEC_SIZE);
						bim_flash_erase(FLASH_SPACE_TYPE_MAIN, addr/4, 1024);

					}
					else if(bim_uart_data[1]==0xd8)
					{
						//bim_fflash_erase_one_block(addr);
						bim_flash_erase(FLASH_SPACE_TYPE_MAIN, addr/4, 8*1024);

					}
					operate_flash_cmd_response(0x0f,0,0x07,&bim_uart_data[1]);
				}
				cmd_status=UART_CMD_STATE_HEAD;
			}
			else if(bim_uart_data[0]==0x06 && index==scmd_length)//write bt addr
			{
				write_addr = bim_uart_data[1]|(bim_uart_data[2]<<8)|(bim_uart_data[3]<<16)|(bim_uart_data[4]<<24) ;

				if(write_addr<0x27ff0)
				{
					bim_uart_data[5]=scmd_length-5;
					operate_flash_cmd_response(0x06,6,7,&bim_uart_data[1]);
				}
				else
				{
					bim_flash_write(FLASH_SPACE_TYPE_MAIN, write_addr/4, (scmd_length-5), &bim_uart_data[5]);
					bim_uart_data[5]=scmd_length-5;
					operate_flash_cmd_response(0x06,0,7,&bim_uart_data[1]);

				}
				cmd_status=UART_CMD_STATE_HEAD;

			}
			else if( bim_uart_data[0]==0x07 )//write code data
			{

				if(index >= (256*(index_cnt+1)+5) )
				{
					write_addr = bim_uart_data[1]|(bim_uart_data[2]<<8)|(bim_uart_data[3]<<16)|(bim_uart_data[4]<<24) ;


					if(write_addr<0x1000)
					{

						index_cnt++;
						if(index==scmd_length )
						{
							bim_uart_data[1]=0xf0;
							bim_uart_data[2]=0x1f;
							operate_flash_cmd_response(0x07,6,6,&bim_uart_data[1]);
							cmd_status=UART_CMD_STATE_HEAD;
							index_cnt=0;
						}
					}
					else
					{
						bim_flash_write(FLASH_SPACE_TYPE_MAIN, (write_addr+256*(index_cnt))/4, 256, &bim_uart_data[5+256*(index_cnt)]);

						index_cnt++;

						if(index==scmd_length )
						{
							operate_flash_cmd_response(0x07,0,6,&bim_uart_data[1]);
							cmd_status=UART_CMD_STATE_HEAD;
							index_cnt=0;
						}
					}
				}
			}
			else if(index==scmd_length)
				cmd_status=UART_CMD_STATE_HEAD;
		}
		break;

		}
		len--;
		buff++;
	}
}


uint32_t crc32_table[256];
int make_crc32_table(void)
{
	uint32_t c;
	int i = 0;
	int bit = 0;
	for(i = 0; i < 256; i++)
	{
		c = (uint32_t)i;
		for(bit = 0; bit < 8; bit++)
		{
			if(c&1)
			{
				c = (c>>1)^(0xEDB88320);
			}
			else
			{
				c = c >> 1;
			}
		}
		crc32_table[i] = c;

	}
	return 0;
}

uint32_t make_crc32(uint32_t crc,unsigned char *string,uint32_t size)
{
	while(size--)
	{
		crc = (crc >> 8)^(crc32_table[(crc^*string++)&0xff]);
	}
	return crc;
}

void bim_delay_ms(unsigned int tt)
{
	volatile unsigned int i, j;
	while(tt--)
	{
		for (j = 0; j < 1000/10; j++)
		{
			for (i = 0; i < 12; i++)
			{
				;
			}
		}
	}
}



void uart_cmd_dispath(uint8_t *buff,uint8_t len)
{
	uint8_t payload[16];
	uint8_t read_data[256];
	uint32_t  calcuCrc=0xffffffff;

	uint32_t read_flash_addr;
	uint32_t uart_clk_div,baudrate_set;
	uint32_t crc_start_addr,crc_end_addr;


	switch(buff[0])
	{
	case LINK_CHECK_CMD:
		uart_download_status=1;
		payload[0]=0x00;
		cmd_response(LINK_CHECK_CMD+1,5,payload);
		erase_fenable=1;
		break;

	case CRC_CHECK_CMD:
		crc_start_addr = ( buff[1]|(buff[2]<<8)|(buff[3]<<16)|(buff[4]<<24) );
		crc_end_addr = ( buff[5]|(buff[6]<<8)|(buff[7]<<16)|(buff[8]<<24) );

		//bim_fflash_wp_ALL();

		make_crc32_table();


		read_flash_addr=crc_start_addr;

		for(uint16_t i = 0; i < (crc_end_addr-crc_start_addr+1)/256; i++)
		{
			bim_flash_read(FLASH_SPACE_TYPE_MAIN, read_flash_addr/4, 256, read_data);

			calcuCrc = make_crc32(calcuCrc,read_data,256);

			read_flash_addr+= 256;
		}
		payload[0]= calcuCrc;
		payload[1]= calcuCrc>>8;
		payload[2]= calcuCrc>>16;
		payload[3]= calcuCrc>>24;
		cmd_response(CRC_CHECK_CMD,8,payload);

		break;
	case STAY_ROM_CMD:
		uart_download_status=1;
		payload[0]=buff[1];
		cmd_response(STAY_ROM_CMD,5,payload);

		break;
	case SET_BAUDRATE_CMD:

		baudrate_set =  buff[1]|(buff[2]<<8)|(buff[3]<<16)|(buff[4]<<24) ;

		uart_clk_div    = 16000000/baudrate_set;

		REG_APB3_UART_CFG  = ((uart_clk_div-1)<<8) +
		                     (stop_bits   <<7) +
		                     (data_len    <<3) +
		                     (irda_mode   <<2) +
		                     (uart_rx_en  <<1) +
		                     uart_tx_en;



		bim_delay_ms(buff[5]*2);

		payload[0]=buff[1];
		payload[1]=buff[2];
		payload[2]=buff[3];
		payload[3]=buff[4];
		payload[4]=buff[5];
		cmd_response(SET_BAUDRATE_CMD,9,payload);
		bim_icu_init(ICU_CLK_64M);
		break;
	case SET_RESET_CMD:
		if(buff[1]==0xa5)
		{
			bim_wdt_enable(0X10);
			while(1);
		}

		break;
	}
}

void cmd_response( uint8_t cmd, uint8_t length, uint8_t *payload )
{
	uint8_t response_buff[16],i;

	if(length<4)
		return;

	response_buff[0]=0x04;
	response_buff[1]=0x0e;
	response_buff[2]=length;
	response_buff[3]=0x01;
	response_buff[4]=0xe0;
	response_buff[5]=0xfc;
	response_buff[6]=cmd;

	for(i=0; i<length-4; i++)
		response_buff[7+i]=payload[i];

	uart_send(response_buff,length+3);

}

void operate_flash_cmd_response( uint8_t cmd, uint8_t status,uint16_t length, uint8_t *payload )
{
	uint8_t response_buff[4200];
	uint16_t    i;

	if(length<2)
		return;

	response_buff[0]=0x04;
	response_buff[1]=0x0e;
	response_buff[2]=0xff;
	response_buff[3]=0x01;
	response_buff[4]=0xe0;
	response_buff[5]=0xfc;
	response_buff[6]=0xf4;

	response_buff[7]=(length&0xff);
	response_buff[8]=length>>8;
	response_buff[9]=cmd;
	response_buff[10]=status;


	for(i=0; i<(length-2); i++)
		response_buff[11+i]=payload[i];

	uart_send(response_buff,length+9);

}




#endif



