#ifndef	_TM1638_H
#define	_TM1638_H

#include	<define.h>

#define	DATA_COMMAND	0X40
#define	DISP_COMMAND	0x80
#define	ADDR_COMMAND	0XC0
#define TM1638_Thous_Bit 0
#define TM1638_Hund_Bit 2
#define TM1638_Ten_Bit 4
#define TM1638_Bit 6
#define TM1638_Sec_Bit 8

void delay_nop(unsigned char i);
void TM1638_Write(unsigned char	DATA);
unsigned char TM1638_Read(void);
void TM1638_Write_COM(unsigned char cmd);
unsigned char TM1638_Read_key(void);
void Write_oneLED(unsigned char num,unsigned char flag);
void TM1638_Write_DATA(unsigned char add,unsigned char DATA);	
void TM1638_Write_allLED(unsigned char LED_flag);
void TM1638_init(void);


#define uchar unsigned char		   
#define uint unsigned int		   
  
extern uchar  TAB[10]; 
extern uchar KEY[5];							

extern uchar GRID_1[10];
extern uchar GRID_2[10];
extern uchar GRID_3[10];
extern uchar GRID_4[10];
extern uchar GRID_5[10];
extern uchar GRID_6[10];
extern uchar GRID_7[10];
extern uchar LED20[10];

extern uchar GRID[8][10];


extern uchar SEG9[16];

extern uchar WeiL_tab[];
extern uchar WeiH_tab[];


void delay_nms(uint n);
void send_8bit(uchar dat);
void send_command(uchar com);
uchar read_key(void);
void Write_DATA(uchar add,uchar DATA);
void Write_String(uchar *p,uchar cnt);
void display(void);
void init_TM1638(void);

#endif

