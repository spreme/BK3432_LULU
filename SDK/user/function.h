#ifndef	__FUNCTION_H
#define	__FUNCTION_H

#include "user_config.h"
#include "lcd.h"


void get_time(void);
char set_time(void);

void get_meal(uint8_t num);
void get_feed_num(uint8_t num);
void get_weight(uint8_t num);

char set_meal(void);

char set_meal_1(uint8_t num);


void del_meal(uint8_t num);

void disp_voltage(void);

uint8_t key_scan(void);
void key_func(void);

void test_feed_info(void);

void play_record_control(void);
void record_reset_control(void);
void update_backlight(uint8_t light);
void backlight_init(void);

void lock_led(void);

extern uint8_t key_scan_flag;				//��ⰴ����־
extern uint16_t key_set_t;
extern uint16_t key_esc_t;
extern uint16_t key_rec_t;
extern uint16_t key_up_t;

extern uint8_t get_feed_info_flag;		//�鿴ιʳ�ƻ���־
extern uint8_t set_val_flag;			//����������ֵ��־
extern uint8_t set_time_flag;			//��������ʱ���־


extern uint32_t set_key_tick;				//���ð�����ʱ
extern uint32_t dowm_key_tick;				//�°�����ʱ
extern uint32_t up_key_tick;				//�ϰ�����ʱ
extern uint32_t lock_key_tick;				//����������ʱ
extern uint32_t feed_key_tick;				//ιʳ������ʱ
extern uint32_t record_key_tick;			//¼��������ʱ

extern uint32_t ok_key_tick;

#endif
