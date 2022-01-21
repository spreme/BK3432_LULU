#ifndef _DS12C887_H_
#define _DS12C887_H_

#include "define.h"

//uchar shi,fen,miao,nian,yue,ri,week;

#define DC12C887_Sec_Address  0
#define DC12C887_Sec_Alarm_Address  1
#define DC12C887_Min_Address  2
#define DC12C887_Min_Alarm_Address  3
#define DC12C887_Hour_Address 4
#define DC12C887_Hour_Alarm_Address 5
#define DC12C887_Week_Address 6
#define DC12C887_Day_Address 7
#define DC12C887_Month_Address 8
#define DC12C887_Year_Address 9

void DS12C887_write_ds(unsigned char add,unsigned char date);
unsigned char DS12C887_read_ds(unsigned char add);
//void DS12C887_init();
//void DS12C887_read_time();	
//void DS12C887_write_week();
void DS12C887_write_time(unsigned char shi,unsigned char fen,unsigned char miao);

#endif

