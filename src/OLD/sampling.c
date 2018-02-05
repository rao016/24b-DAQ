#include "sampling.h"

uint8_t dataBuf[BUFFER_LENGTH];
uint32_t bufLen;
startS ss = STOP;

void sampling_init(void) {
	bufLen = 0;
}

uint16_t get_buf_len(void) {
	return bufLen;
}

void status_check(void) {
	uint8_t temp, s[2] = {STOP_ADC,START_ADC};
	
    if ((temp = dec()) == NULL) stop();
    else if (temp == 2) {
		setRate(queue->rate);
		change_channel(queue->channels);
		txrx_wait(s,2);
	}
}

startS start(void) {
	uint8_t i;
	
    if (ss == STOP && queue != NULL) {
		//change_channel(queue->channels);
        setRate(queue->rate);
        timer_done = false;
        dataRdy = false;
		bufLen = 0;
        return START;
    }
    else return ss;
}

startS stop(void) {
	interruptEnable(false);
    return ss = STOP;
}

void setRate(float rate) {
    changeSampleRate(determineADCRate(rate));
    reconfig_timer(rate);
    interruptEnable(true);
}

void interruptEnable(bool en) {
	startADC(en);
	enableDrdy(en);
    if(en) ss = GO;
    else {
        ss = STOP;
		disable_timer();
    }
}

uint32_t readData(void) {
    uint32_t i;
    
	if (queue != NULL && ss != STOP) {
        if (timer_done) {
            system_interrupt_enter_critical_section();
            if (bufLen > (BUFFER_LENGTH - ADC_BYTES_PER_SAMPLE)) {
                //Set data corrupt flag
                timer_done = true;
                return queue->num;
            }
            for (i = 4; i < ADC_BYTES_PER_SAMPLE+4; i++) dataBuf[bufLen++] = adcData[i];
            status_check();
            timer_done = false;
            system_interrupt_leave_critical_section();
        }
        return (queue != NULL) ? queue->num : 0;
    }
    else return 0;
}

uint32_t send_ADC_data(void* dest, uint16_t numBytes) {
	uint8_t* destPtr = (uint8_t*) dest;
	uint32_t i;
	
	if (bufLen < ADC_BYTES_PER_SAMPLE || numBytes < bufLen) return 0;
	
    for (i = 0; numBytes >= ADC_BYTES_PER_SAMPLE && i < bufLen; numBytes--) *destPtr++ = dataBuf[i++];
    bufLen = 0;

	return i;
}
