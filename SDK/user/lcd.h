#ifndef	__LCD_H
#define	__LCD_H

#include "user_config.h"

#define POWER_0	0x00
#define POWER_1	0x01
#define POWER_2	0x02
#define POWER_3	0x03
#define POWER_4	0x04

#define MEAL_0	0x05
#define MEAL_1	0x06
#define MEAL_2	0x07
#define MEAL_3	0x08
#define MEAL_4	0x09

#define LOCK				0x0a
#define LOCK_CLOSE			0x0b
#define LOCK_OPEN			0x0c

#define COL	0x0d

#define PORTION	0x0e

void ht1621_disp_dat(uint8_t addr, uint8_t dat);
void ht1621_disp(uint8_t seg, uint8_t disp);
void ht1621_init(void);
void ht1621_down(void);
void ht1621_clean(void);
void ht1621_set_dat(uint8_t addr, uint8_t val);


#define FLASH_PERIOD 10
extern uint8_t task_delay_flash;
extern uint8_t lcd_update;
void seg_flash_task(void);

#endif
