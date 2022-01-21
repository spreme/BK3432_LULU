//#include "ds12c887.h"

//void DS12C887_write_ds(unsigned char add,unsigned char date)
//{
//	dscs=0;
//	dsas=1;
//	dsds=1;
//	dsrw=1;
//	DS12C887_DATE=add;
//	dsas=0;
//	dsrw=0;
//	DS12C887_DATE=date;
//	dsrw=1;
//	dsas=1;
//	dscs=1;

//}
//unsigned char DS12C887_read_ds(unsigned char add)
//{
//	uchar ds_date;
//	dsas=1;
//	dsds=1;
//	dsrw=1;
//	dscs=0;
//	DS12C887_DATE=add;
//	dsas=0;
//	dsds=0;
//	DS12C887_DATE=0xff;
//	ds_date=P0;
//	dsds=1;
//	dsas=1;
//	dscs=1;
//	return ds_date;
//}
//void DS12C887_init()
//{
//	DS12C887_write_ds(0x0a,0x20); //REGISTER A  0010 0000 
//	                                            
//	DS12C887_write_ds(0x0b,0x26); //REGISTER B  0010 0110
//}
//void DS12C887_write_sfm(uchar add,uchar date)
//{
//	uchar shi,ge;
//	shi=date/10;
//	ge=date%10;
//	DS12C887_write_com(0x80+0x40+add);
//	DS12C887_write_date(0x30+shi);
//	DS12C887_write_date(0x30+ge);
//}

//void DS12C887_write_time(unsigned char shi,unsigned char fen,unsigned char miao)
//{
//	DS12C887_write_ds(DC12C887_Sec_Address,miao); //秒
//	DS12C887_write_ds(DC12C887_Min_Address,fen);  //分钟
//	DS12C887_write_ds(DC12C887_Hour_Address,shi); //小时
//	DS12C887_write_ds(DC12C887_Week_Address,5);  //星期
//	DS12C887_write_ds(DC12C887_Day_Address,25);  //日期
//	DS12C887_write_ds(DC12C887_Month_Address,4);   //月份
//	DS12C887_write_ds(DC12C887_Year_Address,20);   //年
//	//DS12C887_write_ds(3,30);  
//	//DS12C887_write_ds(5,8);
//}

///*void DS12C887_write_week()
//{

//	DS12C887_write_com(0x80+0x0c);
//	switch(week)
//	{
//		case 1:
//			  DS12C887_write_date('M');delay(5);
//				DS12C887_write_date('O');delay(5);
//				DS12C887_write_date('N');
//				break;
//			case 2:
//				DS12C887_write_date('T');delay(5);
//				DS12C887_write_date('U');delay(5);
//				DS12C887_write_date('E');
//				break;

//				case 3:
//				DS12C887_write_date('W');delay(5);
//				DS12C887_write_date('E');delay(5);
//				DS12C887_write_date('D');
//				break;

//					case 4:
//					DS12C887_write_date('T');delay(5);
//				DS12C887_write_date('H');delay(5);
//				DS12C887_write_date('U');
//				break;
//					case 5:
//						DS12C887_write_date('F');delay(5);
//				DS12C887_write_date('R');delay(5);
//				DS12C887_write_date('T');
//				break;
//					case 6:
//						DS12C887_write_date('S');delay(5);
//				DS12C887_write_date('A');delay(5);
//				DS12C887_write_date('T');
//				break;
//					case 7:
//						DS12C887_write_date('S');delay(5);
//				DS12C887_write_date('U');delay(5);
//				DS12C887_write_date('N');
//				break;
//	}
//}*/
