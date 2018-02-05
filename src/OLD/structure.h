#ifndef structure_h
#define structure_h

#include <stdio.h>
#include <stdint.h>
#include <asf.h>
#include <math.h>
#include "adcLib.h"
#include "timer.h"

#define MAX_RATE 16000
#define MIN_RATE CLK_PS_1024/pow(2,32)
#define FULL_RESPONSE 2
#define INVALID_RESPONSE 1
#define OK_RESPONSE 0

typedef struct dataSet {
	uint16_t channels;
	uint32_t num;
	float rate;
	struct dataSet *next;
} dSet;

extern dSet *queue;

uint8_t add(uint32_t n, float rate, uint16_t c);
uint8_t rm(void);
uint8_t dec(void);
dSet* findSet(uint32_t n);
dSet* qryDSet(uint32_t ss, char *buf, uint32_t buf_len);

#endif /* structure_h */
