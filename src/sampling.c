#include "sampling.h"

//Data buffer variables
uint8_t dataBuf[BUFFER_LENGTH];
uint32_t bufLen;

//Status variable for the state of the system (sampling or not)
startS ss = STOP;

//Corruption variables due to data not being read out fast enough
bool corrupt_sample_set = false;
long corruption_amount = 0;

/******************************************************************
 *
 * Description: Initializes all variables for sampline sets
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void sampling_init(void) {
	bufLen = 0;
}

/******************************************************************
 *
 * Description: Returns the length of the data buffer
 * Last Modified: 11/1/17
 *
 ******************************************************************/
uint16_t get_buf_len(void) {
	return bufLen;
}

/******************************************************************
 *
 * Description: Checks to see if sampling is continuing, complete,
 *  or if another sampling set exists to execute
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void status_check(void) {
	uint8_t temp, s[2] = {STOP_ADC,START_ADC};
	
    if ((temp = dec()) == NULL) stop();
    else if (temp == 2) {
		setRate(queue->rate);
		change_channel(queue->channels);
		txrx_wait(s,2);
	}
}

/******************************************************************
 *
 * Description: Starts sampling routine
 * Last Modified: 11/1/17
 *
 ******************************************************************/
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

/******************************************************************
 *
 * Description: Stops sampling routine
 * Last Modified: 11/1/17
 *
 ******************************************************************/
startS stop(void) {
	interruptEnable(false);
    return ss = STOP;
}

/******************************************************************
 *
 * Description: Sets the sampling rate for both the ADC and microcontroller
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void setRate(float rate) {
    changeSampleRate(determineADCRate(rate));
    reconfig_timer(rate);
    interruptEnable(true);
}

/******************************************************************
 *
 * Description: Enables the interrupts as needed
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void interruptEnable(bool en) {
	startADC(en);
	enableDrdy(en);
    if(en) ss = GO;
    else {
        ss = STOP;
		disable_timer();
    }
}

/******************************************************************
 *
 * Description: Reads data from the ADC buffer to the data buffer
 * Last Modified: 11/1/17
 *
 ******************************************************************/
uint32_t readData(void) {
    uint32_t i;
    
	if (queue != NULL && ss != STOP) {
        // Timer function checks if data is ready before setting the timer_done flag
        if (timer_done) {
            system_interrupt_enter_critical_section();
            timer_done = false;
            if (bufLen > (BUFFER_LENGTH - ADC_BYTES_PER_SAMPLE)) {
                //Set data corrupt flag
                corrupt_sample_set = true;
                corruption_amount += ADC_BYTES_PER_SAMPLE+4;
                return queue->num;
            }
            for (i = 4; i < ADC_BYTES_PER_SAMPLE+4; i++) dataBuf[bufLen++] = adcData[i];
            status_check();
            system_interrupt_leave_critical_section();
        }
        return (queue != NULL) ? queue->num : 0;
    }
    else return 0;
}

/******************************************************************
 *
 * Description: Gets data ready to send over USB
 * Last Modified: 11/1/17
 *
 ******************************************************************/
uint32_t send_ADC_data(void* dest, uint16_t numBytes) {
	uint8_t* destPtr = (uint8_t*) dest;
	uint32_t i;
	
	if (bufLen < ADC_BYTES_PER_SAMPLE || numBytes < bufLen) return 0;
	
    for (i = 0; numBytes >= ADC_BYTES_PER_SAMPLE && i < bufLen; numBytes--) *destPtr++ = dataBuf[i++];
    bufLen = 0;
    
    if (corrupt_sample_set) {
        if (numBytes <= corruption_amount) {
            *destPtr += numBytes;
            corruption_amount -= numBytes;
            numBytes = 0;
            i += numBytes;
        }
        else {
            *destPtr += corruption_amount;
            numBytes -= corruption_amount;
            corruption_amount = 0;
            i += corruption_amount;
        }
    }

	return i;
}

bool is_corrupt(void) {
    return corrupt_sample_set;
}
