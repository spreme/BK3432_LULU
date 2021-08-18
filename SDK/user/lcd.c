#include "user_config.h"
#include "lcd.h"


#define HT_BISA_COM         0x29
#define HT_LCD_OFF          0x02
#define HT_LCD_ON           0x03
#define HT_SYS_DIS          0x00
#define HT_SYS_EN           0x01

#define HT_TIMER_DIS        0x04
#define HT_TIMER_EN         0x06
#define HT_WDT_DIS          0x05
#define HT_WDT_EN           0x07
#define HT_TONE_DIS         0x08
#define HT_TONE_EN          0x09

#define HT_IRQ_DIS         0x80
#define HT_IRQ_EN          0x88

/* lcd_buf用于保存lcd指段，如果是一个8字（包括高4位和低4位）则占用一个字节
 如果是其他字段则占用一个字节，但只能表示4位 */
static uint8_t lcd_buf[8] = {0};

static void ht1621_set_bith(uint8_t val, uint8_t cnt)
{
	uint8_t i;
	for (i=0; i<cnt; i++) {
		if ((val&0x80) == 0)
			gpio_set(HT1621_DAT, 0);
		else
			gpio_set(HT1621_DAT, 1);
		
		gpio_set(HT1621_WR, 0);
//		__nop();__nop();
		__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();
		gpio_set(HT1621_WR, 1);
//		__nop();__nop();
		__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();
		val <<= 1;
	}
}

static void ht1621_set_bitl(uint8_t val, uint8_t cnt)
{
	uint8_t i;
	for (i=0; i<cnt; i++) {
		if ((val&0x01) == 0)
			gpio_set(HT1621_DAT, 0);
		else
			gpio_set(HT1621_DAT, 1);
		gpio_set(HT1621_WR, 0);
//		__nop();__nop();
		__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();
		gpio_set(HT1621_WR, 1);
//		__nop();__nop();
//		__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();
		val >>= 1;
	}
}

static uint8_t ht1621_get_dat(uint8_t addr)
{
	uint8_t i, val = 0;

	gpio_set(HT1621_CS, 0);
	ht1621_set_bith(0xc0, 3);
	ht1621_set_bith(addr<<2, 6);

	for (i=0; i<4; i++) {
//		HT1621_RD = 0;
//		__nop();__nop();
//		__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();
//		HT1621_RD = 1;
//		__nop();__nop();
//		__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();
//		__nop();__nop();__nop();__nop();
		val <<= 1;
		val |= HT1621_DAT;
	}
	gpio_set(HT1621_CS, 1);
/*	uart_send_buf(&val, 1);
	val = HT1621_DAT;
	uart_send_buf(&val, 1);*/
	
	gpio_set(HT1621_WR, 1);
	gpio_set(HT1621_DAT, 1);
//	gpio_set(HT1621_RD, 1);
	
	return val;
}

static void ht1621_set_cmd(uint8_t cmd)
{
	gpio_set(HT1621_CS, 0);
	ht1621_set_bith(0x80, 3);
	ht1621_set_bith(cmd, 9);
	gpio_set(HT1621_CS, 1);
	gpio_set(HT1621_WR, 1);
	gpio_set(HT1621_DAT, 1);
//	gpio_set(HT1621_RD, 1);
}

//static void ht1621_set_dat(uint8_t addr, uint8_t val)
void ht1621_set_dat(uint8_t addr, uint8_t val)
{
	gpio_set(HT1621_CS, 0);
	ht1621_set_bith(0xa0, 3);
	ht1621_set_bith(addr<<2, 6);
	ht1621_set_bitl(val, 4);
	gpio_set(HT1621_CS, 1);
	gpio_set(HT1621_WR, 1);
	gpio_set(HT1621_DAT, 1);
//	gpio_set(HT1621_RD, 1);
}

void ht1621_init(void)
{ 
	gpio_set(HT1621_CS, 1);
	gpio_set(HT1621_WR, 1);
	gpio_set(HT1621_DAT, 1);
//	gpio_set(HT1621_RD, 1);
	Delay_ms(5);
	ht1621_set_cmd(HT_BISA_COM);
	ht1621_set_cmd(HT_SYS_EN);
//	ht1621_set_cmd(HT_TIMER_DIS);
//	ht1621_set_cmd(HT_WDT_DIS);
//	ht1621_set_cmd(HT_TONE_DIS);
//	ht1621_set_cmd(HT_IRQ_DIS);
	ht1621_set_cmd(HT_LCD_ON);
}
/*
void ht1621_down(void)
{
	ht1621_set_cmd(HT_LCD_OFF);
	ht1621_set_cmd(HT_SYS_DIS);
}
*/
void ht1621_clean(void)
{  
	uint8_t i;
	uint8_t addr = 0;

	for (i=0; i<32; i++) {
		ht1621_set_dat(addr, 0x00);
		addr++;
	}

	ht1621_get_dat(9);
	ht1621_get_dat(8);
}

/*-------------------------------*/
uint8_t task_delay_flash = FLASH_PERIOD;
uint8_t lcd_update = 0;

void seg_flash_task(void)
{
	lcd_update++;
	if (lcd_update >= 60) {
		lcd_update = 0;
	}
}


#if 0
static uint8_t code ht1621_font [] = {
	/* 0     1     2     3     4     5     6     7     8     9   */
	0xfa, 0x0a, 0xbc, 0x9e, 0x4e, 0xd6, 0xf6, 0x8a, 0xfe, 0xde,
	/* A     b     C     c     d     E     F     H     h     L   */
	0xee, 0x76, 0xf0, 0x34, 0x3e, 0xF4, 0xE4, 0x6e, 0x66, 0x70,
	/* N     n     o     P     r     t     U     -     */
	0xea, 0x26, 0x36, 0xec, 0x24, 0x74, 0x7a, 0x04, 0x00
};
#elif LNH_01
static uint8_t ht1621_font [] = {
	/* 0     1     2     3     4     5     6     7     8     9   */
	0xfa, 0x60, 0xbc, 0xf4, 0x66, 0xd6, 0xde, 0x70, 0xfe, 0xf6,
	/* -     */
	0x04, 0x00
};
#elif PT01K_BK
static uint8_t ht1621_font [] = {
	/* 0     1     2     3     4     5     6     7     8     9   */
	0x5f, 0x50, 0x3d, 0x79, 0x72, 0x6b, 0x6f, 0x51, 0x7f, 0x7b,
	/* -     */
	0x02, 0x00
};
#elif F04
static uint8_t ht1621_font1 [] = {
	/* 0     1     2     3     4     5     6     7     8     9   */
	0x5f, 0x06, 0x6b, 0x2f, 0x36, 0x3d, 0x7d, 0x07, 0x7f, 0x3f,
	/* -     */
	0x04, 0x00
};
static uint8_t ht1621_font2 [] = {
	/* 0     1     2     3     4     5     6     7     8     9   */
	0xaf, 0x06, 0x6d, 0x4f, 0xc6, 0xcb, 0xeb, 0x0e, 0xef, 0xcf,
	/* -     */
	0x40, 0x00
};
static uint8_t ht1621_font3 [] = {
	/* 0     1     2     3     4     5     6     7     8     9   */
	0xf5, 0x60, 0xb6, 0xf2, 0x63, 0xd3, 0xd7, 0x70, 0xf7, 0xf3,
	/* -     */
	0x02, 0x00
};
#else
static uint8_t ht1621_font [] = {
	/* 0     1     2     3     4     5     6     7     8     9   */
	0xf5, 0x05, 0xd3, 0x97, 0x27, 0xb6, 0xf6, 0x15, 0xf7, 0xb7,
	/* A     b     C     c     d     E     F     H     h     L   */
	0x77, 0xe6, 0xf0, 0xc2, 0xc7, 0xF2, 0x72, 0x67, 0x66, 0xe0,
	/* N     n     o     P     r     t     U     -     */
	0x75, 0x46, 0xc6, 0x73, 0x42, 0xe2, 0xe5, 0x02, 0x00
};
#endif

#ifdef LNH_01
void ht1621_disp_dat(uint8_t addr, uint8_t dat)
{
	uint8_t addr_l = 0, addr_h = 0;
	uint8_t data_l = 0, data_h = 0;
	uint8_t index = 0;

	addr_l = addr * 2;
	addr_h = addr_l + 1;
	switch (dat) {
	case '-':
		index = 10;
		break;
	default :
		index = 11;
		break;
	}
	
	if (dat>='0' && dat<='9') {
		index = dat-'0';
	}
	
	lcd_buf[addr] &= 0x01;
	lcd_buf[addr] |= ht1621_font[index];
	
	data_h = lcd_buf[addr] >> 4;
	data_l = lcd_buf[addr];

//	UART_PRINTF("addr_h-l:%02X-02X \r\n", addr_h,addr_l);
//	UART_PRINTF("data_h-l:%02X-02X \r\n", data_h,data_l);
	ht1621_set_dat(addr_h, data_h);
	ht1621_set_dat(addr_l, data_l);
}

void ht1621_disp(uint8_t seg, uint8_t disp)
{
	uint8_t lcd_buf_offset = 0;
	uint8_t seg_mask = 0;
	uint8_t addr;

	switch (seg) {
	case POWER_0:
		lcd_buf_offset = 7;
		seg_mask = 0x08;
		addr = 14;
		break;
	case POWER_1:
		lcd_buf_offset = 1;
		seg_mask = 0x01;
		addr = 2;
		break;
	case POWER_2:
		lcd_buf_offset = 0;
		seg_mask = 0x01;
		addr = 0;
		break;
	case POWER_3:
		lcd_buf_offset = 7;
		seg_mask = 0x01;
		addr = 14;
		break;
	case POWER_4:
		lcd_buf_offset = 7;
		seg_mask = 0x02;
		addr = 14;
		break;
	
	case MEAL_0:
		lcd_buf_offset = 6;
		seg_mask = 0x10;
		addr = 13;
		break;
	case MEAL_1:
		lcd_buf_offset = 6;
		seg_mask = 0x01;
		addr = 12;
		break;
	case MEAL_2:
		lcd_buf_offset = 6;
		seg_mask = 0x02;
		addr = 12;
		break;
	case MEAL_3:
		lcd_buf_offset = 6;
		seg_mask = 0x04;
		addr = 12;
		break;
	case MEAL_4:
		lcd_buf_offset = 6;
		seg_mask = 0x08;
		addr = 12;
		break;
		
	case LOCK:
		lcd_buf_offset = 6;
		seg_mask = 0x20;
		addr = 13;
		break;
	case LOCK_CLOSE:
		lcd_buf_offset = 6;
		seg_mask = 0x40;
		addr = 13;
		break;
	case LOCK_OPEN:
		lcd_buf_offset = 6;
		seg_mask = 0x80;
		addr = 13;
		break;
		
	case COL:
		lcd_buf_offset = 2;
		seg_mask = 0x01;
		addr = 4;
		break;
	
	case PORTION:
		lcd_buf_offset = 4;
		seg_mask = 0x01;
		addr = 8;
		break;
	default:
		return;
	}
	if (disp)
		lcd_buf[lcd_buf_offset] |= seg_mask;
	else
		lcd_buf[lcd_buf_offset] &= ~seg_mask;
	
	if(seg_mask >= 0x10)
	{
		ht1621_set_dat(addr, (lcd_buf[lcd_buf_offset] >> 4));	
	}
	else
	{
		ht1621_set_dat(addr, lcd_buf[lcd_buf_offset]);
	}
}
#endif

#ifdef PT01K_BK
void ht1621_disp_dat(uint8_t addr, uint8_t dat)
{
	uint8_t addr_l = 0, addr_h = 0;
	uint8_t data_l = 0, data_h = 0;
	uint8_t index = 0;

	addr_l = (addr * 2) + 1;
	addr_h = addr_l + 1;
	switch (dat) {
	case '-':
		index = 10;
		break;
	default :
		index = 11;
		break;
	}
	
	if (dat>='0' && dat<='9') {
		index = dat-'0';
	}
	
	lcd_buf[addr] &= 0x80;
	lcd_buf[addr] |= ht1621_font[index];
	
	data_h = lcd_buf[addr] >> 4;
	data_l = lcd_buf[addr] & 0xf;

//	UART_PRINTF("lcd_buf[%d]:0x%02X \n", addr, lcd_buf[addr]);
//	UART_PRINTF("addr_h:%d 	data_h:0x%02X\n", addr_h, data_h);
//	UART_PRINTF("addr_l:%d 	data_l:0x%02X\n", addr_l, data_l);
	ht1621_set_dat(addr_h, data_h);
	ht1621_set_dat(addr_l, data_l);
}

void ht1621_disp(uint8_t seg, uint8_t disp)
{
	uint8_t lcd_buf_offset = 0;
	uint8_t seg_mask = 0;
	uint8_t addr;
	
	switch (seg) {
	case POWER_0:
		lcd_buf_offset = 6;
		seg_mask = 0x10;
		addr = 0;
		break;
	case POWER_1:
		lcd_buf_offset = 0;
		seg_mask = 0x80;
		addr = 2;
		break;
	case POWER_2:
		lcd_buf_offset = 6;
		seg_mask = 0x80;
		addr = 0;
		break;
	case POWER_3:
		lcd_buf_offset = 6;
		seg_mask = 0x40;
		addr = 0;
		break;
	case POWER_4:
		lcd_buf_offset = 6;
		seg_mask = 0x20;
		addr = 0;
		break;
	
	case MEAL_0:
		lcd_buf_offset = 2;
		seg_mask = 0x80;
		addr = 6;
		break;
	case MEAL_1:
		lcd_buf_offset = 3;
		seg_mask = 0x80;
		addr = 8;
		break;
	case MEAL_2:			//*
		lcd_buf_offset = 5;
		seg_mask = 0x80;
		addr = 12;
		break;
	case MEAL_3:			//*
		lcd_buf_offset = 5;
		seg_mask = 0x40;
		addr = 12;
		break;
	case MEAL_4:
		lcd_buf_offset = 5;
		seg_mask = 0x04;
		addr = 11;
		break;
		
	case LOCK:					//* 多了MEAL_3
		lcd_buf_offset = 5;
		seg_mask = 0x20;
		addr = 12;
		break;
	case LOCK_CLOSE:			//*
		lcd_buf_offset = 5;
		seg_mask = 0x10;
		addr = 12;
		break;
	case LOCK_OPEN:
		lcd_buf_offset = 5;
		seg_mask = 0x01;
		addr = 11;
		break;
		
	case COL:
		lcd_buf_offset = 1;
		seg_mask = 0x80;
		addr = 4;
		break;
	
	case PORTION:
		lcd_buf_offset = 5;
		seg_mask = 0x02;
		addr = 11;
		break;
	}
	if (disp)
		lcd_buf[lcd_buf_offset] |= seg_mask;
	else
		lcd_buf[lcd_buf_offset] &= ~seg_mask;
	
	if((addr % 2) == 0)
	{
		seg_mask = lcd_buf[lcd_buf_offset] >> 4;
	}
	else
	{
		seg_mask = lcd_buf[lcd_buf_offset] & 0xf;
	}
	
//	UART_PRINTF("addr:%d 	lcd_buf[%d]:0x%02X\n", addr, lcd_buf_offset, lcd_buf[lcd_buf_offset]);

//	ht1621_set_dat(addr, lcd_buf[lcd_buf_offset]);
	ht1621_set_dat(addr, seg_mask);

}


#endif
#ifdef F04
void ht1621_disp_dat(uint8_t addr, uint8_t dat)
{
	uint8_t addr_l = 0, addr_h = 0;
	uint8_t data_l = 0, data_h = 0;
	uint8_t index = 0;

	addr = addr + 4;
	addr_l = (addr * 2) ;
	addr_h = addr_l + 1;
	switch (dat) {
	case '-':
		index = 10;
		break;
	default :
		index = 11;
		break;
	}
	
	if (dat>='0' && dat<='9') {
		index = dat-'0';
	}
	
	lcd_buf[addr] &= 0x08;
	lcd_buf[addr] |= ht1621_font3[index];
	
	data_h = lcd_buf[addr] >> 4;
	data_l = lcd_buf[addr] & 0xf;

//	UART_PRINTF("lcd_buf[%d]:0x%02X \n", addr, lcd_buf[addr]);
//	UART_PRINTF("addr_h:%d 	data_h:0x%02X\n", addr_h, data_h);
//	UART_PRINTF("addr_l:%d 	data_l:0x%02X\n", addr_l, data_l);
	ht1621_set_dat(addr_h, data_h);
	ht1621_set_dat(addr_l, data_l);
}

void ht1621_disp_dat_feed_num(uint8_t addr, uint8_t dat)
{
	uint8_t addr_l = 0, addr_h = 0;
	uint8_t data_l = 0, data_h = 0;
	uint8_t index = 0;

	addr--;
	switch(addr)
	{
		case 0:
			addr_l = 4 ;
			break;
		case 1:
			addr_l = 2 ;
			break;
		case 2:
			addr_l = 6 ;
			break;
		case 3:
			addr_l = 0 ;
			break;
		default:
			return;
	}
	
	addr_h = addr_l + 1;
	switch (dat) {
		case '-':
			index = 10;
			break;
		default :
			index = 11;
			break;
	}
	
	if (dat>='0' && dat<='9') {
		index = dat-'0';
	}
	
	if(addr == 1 || addr == 3)
	{
		lcd_buf[addr] &= 0x80;
	}
	else
	{
		lcd_buf[addr] &= 0x10;
	}
	lcd_buf[addr] |= ht1621_font3[index];
	
	data_h = lcd_buf[addr] >> 4;
	data_l = lcd_buf[addr] & 0xf;

//	UART_PRINTF("lcd_buf[%d]:0x%02X \n", addr, lcd_buf[addr]);
//	UART_PRINTF("addr_h:%d 	data_h:0x%02X\n", addr_h, data_h);
//	UART_PRINTF("addr_l:%d 	data_l:0x%02X\n", addr_l, data_l);
	ht1621_set_dat(addr_h, data_h);
	ht1621_set_dat(addr_l, data_l);
}

void ht1621_disp(uint8_t seg, uint8_t disp)
{
	uint8_t lcd_buf_offset = 0;
	uint8_t seg_mask = 0;
	uint8_t addr;
	
	switch (seg) {
//	case POWER_0:
//		lcd_buf_offset = 6;
//		seg_mask = 0x10;
//		addr = 0;
//		break;
//	case POWER_1:
//		lcd_buf_offset = 0;
//		seg_mask = 0x80;
//		addr = 2;
//		break;
//	case POWER_2:
//		lcd_buf_offset = 6;
//		seg_mask = 0x80;
//		addr = 0;
//		break;
//	case POWER_3:
//		lcd_buf_offset = 6;
//		seg_mask = 0x40;
//		addr = 0;
//		break;
//	case POWER_4:
//		lcd_buf_offset = 6;
//		seg_mask = 0x20;
//		addr = 0;
//		break;
	
	case MEAL_0:
		lcd_buf_offset = 5;
		seg_mask = 0x08;
		addr = 10;
		break;
	case MEAL_1:
		lcd_buf_offset = 2;
		seg_mask = 0x10;
		addr = 5;
		break;
	case MEAL_2:			//*
		lcd_buf_offset = 1;
		seg_mask = 0x80;
		addr = 3;
		break;
	case MEAL_3:			//*
		lcd_buf_offset = 3;
		seg_mask = 0x10;
		addr = 7;
		break;
	case MEAL_4:
		lcd_buf_offset = 0;
		seg_mask = 0x80;
		addr = 1;
		break;
		
//	case LOCK:					//* 多了MEAL_3
//		lcd_buf_offset = 5;
//		seg_mask = 0x20;
//		addr = 12;
//		break;
//	case LOCK_CLOSE:			//*
//		lcd_buf_offset = 5;
//		seg_mask = 0x10;
//		addr = 12;
//		break;
//	case LOCK_OPEN:
//		lcd_buf_offset = 5;
//		seg_mask = 0x01;
//		addr = 11;
//		break;
		
	case COL:
		lcd_buf_offset = 6;
		seg_mask = 0x08;
		addr = 12;
		break;
	
	case PORTION:
		lcd_buf_offset = 4;
		seg_mask = 0x08;
		addr = 8;
		break;
	}
	if (disp)
		lcd_buf[lcd_buf_offset] |= seg_mask;
	else
		lcd_buf[lcd_buf_offset] &= ~seg_mask;
	
	if((addr % 2) == 0)
	{
		seg_mask = lcd_buf[lcd_buf_offset] >> 4;
	}
	else
	{
		seg_mask = lcd_buf[lcd_buf_offset] & 0xf;
	}
	
//	UART_PRINTF("addr:%d 	lcd_buf[%d]:0x%02X\n", addr, lcd_buf_offset, lcd_buf[lcd_buf_offset]);

//	ht1621_set_dat(addr, lcd_buf[lcd_buf_offset]);
	ht1621_set_dat(addr, seg_mask);

}


#endif


