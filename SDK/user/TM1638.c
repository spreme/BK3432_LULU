#include "tm1638.h"
#include "user_config.h"
#include "utc_clock.h"

#define uchar unsigned char		   
#define uint unsigned int		   
  
uchar  TAB[10]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};  
uchar  KEY[5]={0};							 

/*数码管各自的段码  0    1    2    3    4    5    6    7    8    9    */
uchar GRID_1[10]={0X77,0X60,0X3D,0X79,0X6A,0X6B,0X5F,0X61,0X7F,0X7B};
uchar GRID_2[10]={0XDD,0X11,0X7C,0X75,0XB1,0XE5,0XED,0X15,0XFD,0XF5};
uchar GRID_3[10]={0XE8,0X28,0X8C,0XAC,0X6C,0XE4,0XE4,0XA8,0XEC,0XE4};
uchar GRID_4[10]={0X3B,0X22,0X1E,0X2E,0X27,0X2D,0X3D,0X22,0X3F,0X2F};
uchar GRID_5[10]={0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00};
uchar GRID_6[10]={0X44,0X00,0X44,0X04,0X00,0X04,0X44,0X00,0X44,0X04};
uchar GRID_7[10]={0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00};
uchar LED20[10]={0X1,0X0,0X1,0X1,0X0,0X1,0X1,0X1,0X1,0X1};

uchar GRID[8][10]={ {0X77,0X60,0X3D,0X79,0X6A,0X6B,0X5F,0X61,0X7F,0X7B},
				    {0XDD,0X11,0X7C,0X75,0XB1,0XE5,0XED,0X15,0XFD,0XF5},
				    {0XE8,0X28,0X8C,0XAC,0X6C,0XE4,0XE4,0XA8,0XEC,0XE4},
				    {0X3B,0X22,0X1E,0X2E,0X27,0X2D,0X3D,0X22,0X3F,0X2F},
					{0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00},
					{0X44,0X00,0X44,0X04,0X00,0X04,0X44,0X00,0X44,0X04},
					{0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00},
					{0X1,0X0,0X1,0X1,0X0,0X1,0X1,0X1,0X1,0X1}};


//位码
uchar SEG9[16]={0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
				0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00};								

//TM1638低8段位码 [SEG1-SEG8]
uchar WeiL_tab[]={0xce,0xcc,0xca,0xc8,0xc6,0xc4,0xc2,0xc0};//偶数
//TM1638高段位码  [SEG9-SEG10]
uchar WeiH_tab[]={0xcf,0xcd,0xcb,0xc9,0xc7,0xc5,0xc3,0xc1};//奇数


void delay_nop(unsigned char i)       //延时
{
	unsigned char x;
	for(x=0;x<i;x++)
	{
		__nop();
		__nop();
	}
}


void TM1638_Write(unsigned char	DATA)			//写数据函数
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		gpio_set(CLK,0);

		if(DATA&0X01)
			gpio_set(DIO,1);
		else
			gpio_set(DIO,0);
		
		DATA>>=1;
		gpio_set(CLK,1);		
	}
}


unsigned char TM1638_Read(void)					//读数据函数
{
	unsigned char i;
	unsigned char temp=0;
	gpio_config(DIO, 	INPUT, PULL_NONE);	//设置为输入
	for(i=0;i<8;i++)
	{
		temp>>=1;
		gpio_set(CLK,0);
		delay_nop(2);
		if(DIO)
			temp|=0x80;
		gpio_set(CLK,1);
	}
	delay_nop(5);
	return temp;
}

void TM1638_Write_COM(unsigned char cmd)		//发送命令字
{
//  delay_nop(2);
	gpio_set(STB,0);
	TM1638_Write(cmd);
	gpio_set(STB,1);

}


unsigned char TM1638_Read_key(void)
{
	unsigned char c[4],i,key_value=0;
	gpio_set(STB,0);
	TM1638_Write(0x42);		           //读键扫数据 命令
	for(i=0;i<4;i++)		
		c[i]=TM1638_Read();
	gpio_set(STB,1);					           
	delay_nop(5);
	for(i=0;i<4;i++)								//4个字节数据合成一个字节
		key_value|=c[i]<<i;
	for(i=0;i<8;i++)
		if((0x01<<i)==key_value)
			break;
	return i;
}

void TM1638_Write_DATA(unsigned char add,unsigned char DATA)		//指定地址写入数据
{
	TM1638_Write_COM(0x44);
	gpio_set(STB,0);
	TM1638_Write(0xc0|add);
	TM1638_Write(DATA);
	gpio_set(STB, 1);
}


void Write_oneLED(unsigned char num,unsigned char flag)	//单独控制一个LED函数，num为需要控制的led序号，flag为0时熄灭，不为0时点亮
{
//	if(flag)
//		TM1638_Write_DATA(2*num+1,1);
//	else
//		TM1638_Write_DATA(2*num+1,0);
}


void TM1638_Write_allLED(unsigned char LED_flag)					//控制全部LED函数，LED_flag表示各个LED状态
{
//	unsigned char i;
//	for(i=0;i<8;i++)
//		{
//			if(LED_flag&(1<<i))
//				//Write_DATA(2*i+1,3);
//				TM1638_Write_DATA(2*i+1,1);
//			else
//				TM1638_Write_DATA(2*i+1,0);
//		}
}


void TM1638_init(void)
{
	unsigned char i;
//	TM1638_Write_COM(0x03);
	TM1638_Write_COM(0x89);       //亮度 (0x88-0x8f)8级亮度可调
	TM1638_Write_COM(0x40);       //采用地址自动加1
	
	gpio_set(STB,0);
	TM1638_Write(0xc0);    //设置起始地址

	for(i=0;i<16;i++)	   //传送16个字节的数据
		TM1638_Write(0x00);
	gpio_set(STB,1);
}

void delay_nms(uint n)
{
//	uint i;
//	while(n--)
//	{
//		for(i=0;i<550;i++);
//	}
}

void send_8bit(uchar dat)	 
{
  uchar i;
  for(i=0;i<8;i++)
  {	
    if(dat&0x01) 
			gpio_set(DIO,1);
		else         
	   gpio_set(DIO,0);
	
	gpio_set(CLK,0);
	__nop();
	__nop();
	__nop();
	gpio_set(CLK,1);	 
	dat>>=1;	 
  }
  gpio_set(CLK,0);
  gpio_set(DIO,0);
}

void send_command(uchar com)  
{
  gpio_set(STB,1);						  
  delay_nop(2);
  gpio_set(STB,0);						  
  send_8bit(com);			          
}

uchar read_key(void)			   
{
	unsigned char i,j,k,key_val=0; 
	send_command(0x42);		          
	gpio_config(DIO, 	INPUT, PULL_NONE);	//设置为输入
	for(j=0;j<5;j++)		
	{
		for(i=0;i<8;i++)
		{
			gpio_set(CLK,0);
			KEY[j]=KEY[j]>>1;
			gpio_set(CLK,1);
			if(DIO==1)
			{
				KEY[j]=KEY[j]|0x80;
			}
			delay_nop(2);
		} 
		delay_nop(4);
	}
		gpio_set(CLK,0);
		gpio_set(DIO,0);
		gpio_set(STB,1);
	
	for(k=0;k<5;k++)
		key_val|=KEY[k]<<k;
	for(k=0;k<8;k++)
		if((0x01<<k)==key_val)
			break;
	return k;
}

void Write_DATA(uchar add,uchar DATA)                
{
	send_command(0x44);	
	gpio_set(STB,1);
  delay_nop(2);
	gpio_set(STB,0);
  send_8bit(0xc0|add);
  send_8bit(DATA);
	gpio_set(STB,1);
}

void Write_String(uchar *p,uchar cnt)
{
	for(uchar i=0;i<cnt;i++)
		send_8bit(*p++);
}

void display(void)   
{	
		UTCTimeStruct tm_s;	
		utc_get_time(&tm_s);
		
		send_command(0x40);	
		send_command(0xc0);

//		for(uchar a=0;a<14;a++)
//		{
//			UART_PRINTF("%02X  ",SEG9[a]);
//		}
			
		Write_String(SEG9,14);
	
		gpio_set(STB,1);
}

void init_TM1638(void)
{
	send_command(0x03);
	send_command(0x40);
	send_command(0xc0);
	send_command(0x8a);
	for(uchar i=0;i<16;i++)
	{
		send_8bit(0x00);
	}
	gpio_set(STB,1);
}
