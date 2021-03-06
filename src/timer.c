#include "timer.h"

static struct tc_module tc_instance;
static struct tc_config config_tc;
static enum tc_clock_prescaler timer_ps;

bool tdone, timer_done;

/******************************************************************
 *
 * Description: Initializes the 32-bit timer
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void init_timer(void) {
	tc_get_config_defaults(&config_tc);
	config_tc.counter_size = TC_COUNTER_SIZE_32BIT;
	config_tc.wave_generation = TC_WAVE_GENERATION_MATCH_FREQ;
	config_tc.counter_size = TC_COUNTER_SIZE_32BIT;
	config_tc.wave_generation = TC_WAVE_GENERATION_MATCH_FREQ;
}

/******************************************************************
 *
 * Description: Configures the timer for the given rate
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void config_timer(float rate) {
	config_tc.counter_32_bit.value = 0;
	config_tc.clock_prescaler = (timer_ps = determinePrescale(rate));
	config_tc.counter_32_bit.compare_capture_channel[0] = determineCounter(timer_ps, rate);
	tc_init(&tc_instance, TC4, &config_tc);
	tc_register_callback(&tc_instance, timer_callback, TC_CALLBACK_CC_CHANNEL0);
	tc_enable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL0);
	tc_enable(&tc_instance);
	timer_done = false;
    tdone = false;
}

/******************************************************************
 *
 * Description: Disables the timer
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void disable_timer(void) {
	tc_disable(&tc_instance);
}

/******************************************************************
 *
 * Description: Reconfigures the timer for a new rate
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void reconfig_timer(float rate) {
	disable_timer();
	config_timer(rate);
}

/******************************************************************
 *
 * Description: Determines which prescale to use based on the rate
 * Last Modified: 11/1/17
 *
 ******************************************************************/
enum tc_clock_prescaler determinePrescale(float rate) {
	if ((float) 1/rate < (float) CLK_PS_1) return TC_CLOCK_PRESCALER_DIV1;
	else if ((float) 1/rate < (float) CLK_PS_2) return TC_CLOCK_PRESCALER_DIV2;
	else if ((float) 1/rate < (float) CLK_PS_4) return TC_CLOCK_PRESCALER_DIV4;
	else if ((float) 1/rate < (float) CLK_PS_8) return TC_CLOCK_PRESCALER_DIV8;
	else if ((float) 1/rate < (float) CLK_PS_16) return TC_CLOCK_PRESCALER_DIV16;
	else if ((float) 1/rate < (float) CLK_PS_64) return TC_CLOCK_PRESCALER_DIV64;
	else if ((float) 1/rate < (float) CLK_PS_256) return TC_CLOCK_PRESCALER_DIV256;
	else return TC_CLOCK_PRESCALER_DIV1024;
}

/******************************************************************
 *
 * Description: Returns the value of the prescaler
 * Last Modified: 11/1/17
 *
 ******************************************************************/
uint8_t prescaleToInt(enum tc_clock_prescaler prescale) {
	switch (prescale) {
		case TC_CLOCK_PRESCALER_DIV1:
            return 1;
            break;
		case TC_CLOCK_PRESCALER_DIV2:
            return 2;
            break;
		case TC_CLOCK_PRESCALER_DIV4:
            return 4;
            break;
		case TC_CLOCK_PRESCALER_DIV8:
            return 8;
            break;
		case TC_CLOCK_PRESCALER_DIV16:
            return 16;
            break;
		case TC_CLOCK_PRESCALER_DIV64:
            return 64;
            break;
		case TC_CLOCK_PRESCALER_DIV256:
            return 256;
            break;
		case TC_CLOCK_PRESCALER_DIV1024:
            return 1024;
            break;
		default:
            return 0;
            break;
	}
}

/******************************************************************
 *
 * Description: Returns the counter value based on the prescale and
 *  rate
 * Last Modified: 11/1/17
 *
 ******************************************************************/
uint32_t determineCounter(enum tc_clock_prescaler prescale, float rate) {
	return ((uint32_t) ((float) F_CPU/((float) (prescaleToInt(prescale))*rate)));
}

/******************************************************************
 *
 * Description: Timer callback function
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void timer_callback (void) {
    tdone = true;
    if (dataRdy) {
        timer_done = true;
        dataRdy = false;
    }
}

/******************************************************************
 *
 * Description: Delays for a set number of milliseconds using the timer
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void __attribute__((optimize("O0"))) delay_ms(uint32_t ms) {
	config_timer((float) 1000/ms);
	while(!tdone);
	disable_timer();
}

/******************************************************************
 *
 * Description: Delays for a set number of microseconds with the
 *  assumption addition takes one clock cycle
 * Last Modified: 11/1/17
 * TODO: check how many clock cycles addition takes
 *
 ******************************************************************/
void __attribute__((optimize("O0"))) delay_us(uint32_t us) {
	uint32_t i = 0, v = floor(F_CPU/(2*us*1000000));
	while (++i < v);
}
