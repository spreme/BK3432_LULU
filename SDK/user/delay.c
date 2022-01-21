#include <delay.h>

void delay(unsigned char z)
{
	unsigned int i,j;
	for(i=z;i>0;i--)
		for(j=110;j>0;j--);
}
void delay_ms(unsigned int a)		//@12.000MHz
{
	unsigned char i, j;
  while(a--)
	{
		i = 2;
		j = 239;
		do
		{
			while (--j);
		} while (--i);
  }
}
