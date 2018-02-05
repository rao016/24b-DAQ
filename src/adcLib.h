#ifndef ADCLIB_H
#define ADCLIB_H

#include "spi_com.h"
#include "timer.h"
#include <asf.h>
#include <samd21e18a.h>

/*
 * ADC COMMANDS
 */
#define WAKE_ADC 0x02
#define STANDBY_ADC 0x04
#define RESET_ADC 0x06
#define START_ADC 0x08
#define STOP_ADC 0x0A
#define READ_CONT_ADC 0x10
#define STOP_CONT_ADC 0x11
#define READ_ADC 0x12
#define READ_REG 0x20
#define WRITE_REG 0x40

#define ADC_BYTES_PER_SAMPLE 18

/*
 * ADC REGISTERS
 */
// 9.6.1.1: Device Identification (Read Only)
#define ID_REG 0
// 9.6.1.2: Configuration 1 Register
#define CONFIG1_REG 1 //Sample Rate
// 9.6.1.3: Configuration 2 Register
#define CONFIG2_REG 2
// 9.6.1.4: Configuration 3 Register
#define CONFIG3_REG 3
// 9.6.1.5: Lead-Off Control Register
#define LOFF_REG 4
// 9.6.1.6: Individual Channel Settings Register
#define CH_0_SET_REG 5
#define CH_1_SET_REG 6
#define CH_2_SET_REG 7
#define CH_3_SET_REG 8
#define CH_4_SET_REG 9
#define CH_5_SET_REG 10
#define CH_6_SET_REG 11
#define CH_7_SET_REG 12
// 9.6.1.7: Bias Driver Positive Derivation Register
#define BIAS_SENSP_REG 13
// 9.6.1.8: Bias Driver Negative Derivation Register
#define BIAS_SENSN_REG 14
// 9.6.1.9: Positive Signal Lead-Off Detection Register
#define LOFF_SENSP_REG 15
// 9.6.1.10: Negative Signal Lead-Off Detection Register
#define LOFF_SENSN_REG 16
// 9.6.1.11: Lead-off Flip Register
#define LOFF_FLIP_REG 17
// 9.6.1.12: Lead-off Positive Signal Status Register
#define LOFF_STATP_REG 18
// 9.6.1.13: Lead-Off Negative Signal Status Register
#define LOFF_STATN_REG 19
// 9.6.1.14: GPIO Register
#define GPIO_REG 20
// 9.6.1.15: Miscellaneous 1 Register
#define MISC1_REG 21
// 9.6.1.16: Miscellaneous 2 Register
#define MISC2_REG 22
// 9.6.1.17: Configuration 4 Register
#define CONFIG4_REG 23

/*
 * ADC DATA RATES FOR COMPARISON
 */
#define RATE_16000 16000
#define RATE_8000 8000
#define RATE_4000 4000
#define RATE_2000 2000
#define RATE_1000 1000
#define RATE_500 500
#define RATE_250 250
#define FF 0.8

/*
 * ADC 1 DATA RATES: CONFIG1 Register
 */
#define DATA_RATE_16000 0
#define DATA_RATE_8000 1
#define DATA_RATE_4000 2
#define DATA_RATE_2000 3
#define DATA_RATE_1000 4
#define DATA_RATE_500 5
#define DATA_RATE_250 6

/*
 * ADC INITIALIZATION
 */
#define CHSET_ON_REG_VAL 0b00000000
#define CHSET_OFF_REG_VAL 0b10000001
#define HIGHEST_CHANNEL 6

#define CONFIG1_REG_INIT 0b11010000
#define CONFIG2_REG_INIT 0b11000011
#define CONFIG3_REG_INIT 0b01100000
#define MISC1_REG_INIT 0b00100000
#define CHSET_REG_INIT CHSET_ON_REG_VAL

/*
 * ADC PIN DEFINITIONS
 */
#define PWDN_PIN PIN_PA00
#define RST_PIN PIN_PA01
#define START_PIN PIN_PA02
#define DRDY_PIN PIN_PA03A_EIC_EXTINT3
#define DRDY_PIN_MUX PINMUX_PA03A_EIC_EXTINT3
#define DRDY_PIN_LINE 3//PIN_PA03A_EIC_EXTINT_NUM

extern bool dataRdy;
extern uint8_t adcData[ADC_BYTES_PER_SAMPLE+4];

void changeSampleRate(uint8_t rate);
void change_channel(uint8_t ch);
void initReg(uint8_t rate, uint8_t channel);
void readADC(void);
void drdy_callback(void);
void initGPIO(void);
void reset_ADC(void);
void enableDrdy(bool val);
void turnOn(bool val);
void startADC(bool val);
void writeReg(uint8_t reg, uint8_t value);
uint8_t readReg(uint8_t reg);
void initADC(void);
uint8_t determineADCRate(float rate);

#endif
