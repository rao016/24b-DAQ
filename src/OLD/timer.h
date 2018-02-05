#ifndef TIMER_H_
#define TIMER_H_

#include <asf.h>
#include <math.h>
#include "sampling.h"

#ifndef F_CPU
#define F_CPU 48000000
#endif

#define CLK_PS_1 0xFFFFFFFF/F_CPU
#define CLK_PS_2 0xFFFFFFFF/F_CPU*2
#define CLK_PS_4 0xFFFFFFFF/F_CPU*4
#define CLK_PS_8 0xFFFFFFFF/F_CPU*8
#define CLK_PS_16 0xFFFFFFFF/F_CPU*16
#define CLK_PS_64 0xFFFFFFFF/F_CPU*64
#define CLK_PS_256 0xFFFFFFFF/F_CPU*256
#define CLK_PS_1024 0xFFFFFFFF/F_CPU*1024

extern bool timer_done;

void init_timer(void);
void config_timer(float rate);
void disable_timer(void);
void reconfig_timer(float rate);
enum tc_clock_prescaler determinePrescale(float rate);
uint8_t prescaleToInt(enum tc_clock_prescaler prescale);
uint32_t determineCounter(enum tc_clock_prescaler prescale, float rate);
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

#endif /* TIMER_H_ */
