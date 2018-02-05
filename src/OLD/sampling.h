#ifndef SAMPLING_H
#define SAMPLING_H

#include <asf.h>
#include "adcLib.h"
#include "structure.h"
#include "timer.h"
#include "spi_com.h"

#define BUFFER_LENGTH 10000
#define NUM_BUFFERS 2

typedef enum startStop {
    START,
    STOP,
    GO
}startS;

extern startS ss;

void sampling_init(void);
uint16_t get_buf_len(void);
void status_check(void);
startS start(void);
startS stop(void);
void setRate(float rate);
void interruptEnable(bool en);
uint32_t readData(void);
void timer_callback (void);
uint32_t send_ADC_data(void* dest, uint16_t numBytes);

#endif
