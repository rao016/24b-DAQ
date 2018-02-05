#include "adcLib.h"

//Flag to determine if ADC has new data
bool dataRdy = false;
//Temporary storage for data read from the ADC
uint8_t adcData[ADC_BYTES_PER_SAMPLE+4];

/******************************************************************
 *
 * Description: Writes the register to change the ADC sample rate
 *  with the rate from the input
 * Last Modified: 11/1/17
 *
******************************************************************/
void changeSampleRate (uint8_t rate) {
    writeReg(CONFIG1_REG, (rate & 0b00000111) + CONFIG1_REG_INIT);
}

/******************************************************************
 *
 * Description: Sets which channels to 'turn on' on the ADC.  Data
 *  is sent from all channels regardless.
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void change_channel (uint8_t channel) {	
	if ((1) & channel) writeReg(CH_0_SET_REG, CHSET_ON_REG_VAL);
	else writeReg(CH_0_SET_REG, CHSET_OFF_REG_VAL);
	
	if ((1 << 1) & channel) writeReg(CH_1_SET_REG, CHSET_ON_REG_VAL);
	else writeReg(CH_1_SET_REG, CHSET_OFF_REG_VAL);
	
	if ((1 << 2) & channel) writeReg(CH_2_SET_REG, CHSET_ON_REG_VAL);
	else writeReg(CH_2_SET_REG, CHSET_OFF_REG_VAL);
	
	if ((1 << 3) & channel) writeReg(CH_3_SET_REG, CHSET_ON_REG_VAL);
	else writeReg(CH_3_SET_REG, CHSET_OFF_REG_VAL);
	
	if (4 < HIGHEST_CHANNEL) {
		if ((1 << 4) & channel) writeReg(CH_4_SET_REG, CHSET_ON_REG_VAL);
		else writeReg(CH_4_SET_REG, CHSET_OFF_REG_VAL);
		
		if ((1 << 5) & channel) writeReg(CH_5_SET_REG, CHSET_ON_REG_VAL);
		else writeReg(CH_5_SET_REG, CHSET_OFF_REG_VAL);
		
		if (6 < HIGHEST_CHANNEL) {
			if ((1 << 6) & channel) writeReg(CH_6_SET_REG, CHSET_ON_REG_VAL);
			else writeReg(CH_6_SET_REG, CHSET_OFF_REG_VAL);
			
			if ((1 << 7) & channel) writeReg(CH_7_SET_REG, CHSET_ON_REG_VAL);
			else writeReg(CH_7_SET_REG, CHSET_OFF_REG_VAL);
		}
	}
}

/******************************************************************
 *
 * Description: Initializes Registers setting the sample rate and
 *  channels to record on.
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void initReg(uint8_t rate, uint8_t channel) {
    changeSampleRate(rate);
    change_channel(channel);
}

/******************************************************************
 *
 * Description: Reads data from the ADC and stores it in the global
 *  array 'adcData'.  Retries until either 3 attempts or expected
 *  first byte is recieved.
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void readADC(void) {
    static uint8_t read_tx[22] = {READ_ADC,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint8_t i;
    
    do {
        txrx_wait_sel(read_tx, 22, adcData);
    } while (((adcData[1] & 0xF0) != 0xC0) && i++ < 3);
}

/******************************************************************
 *
 * Description: Callback function for pin on ADC saying data is ready
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void drdy_callback(void) {
	if (!timer_done) {
        dataRdy = true;
		readADC();
	}
}

/******************************************************************
 *
 * Description: Initializes all GPIO Pins direction and state
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void initGPIO(void) {
	struct port_config config_port_pin;
	struct extint_chan_conf config_extint_chan;
	
	config_extint_chan.gpio_pin = DRDY_PIN;
	config_extint_chan.gpio_pin_mux = DRDY_PIN_MUX;
	config_extint_chan.gpio_pin_pull = EXTINT_PULL_DOWN;
	config_extint_chan.wake_if_sleeping = true;
	config_extint_chan.filter_input_signal = true;
	config_extint_chan.detection_criteria = EXTINT_DETECT_FALLING;
	
	extint_chan_set_config(DRDY_PIN_LINE, &config_extint_chan);
	extint_register_callback(drdy_callback, DRDY_PIN_LINE, EXTINT_CALLBACK_TYPE_DETECT);
	enableDrdy(false);
	
	config_port_pin.powersave = false;
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	config_port_pin.input_pull = SYSTEM_PINMUX_PIN_PULL_DOWN;
	port_pin_set_config(PWDN_PIN, &config_port_pin);
	port_pin_set_config(START_PIN, &config_port_pin);
	port_pin_set_config(RST_PIN, &config_port_pin);
	port_pin_set_config(SLAVE_SELECT_PIN, &config_port_pin);
	
	port_pin_set_output_level(RST_PIN,true);
	port_pin_set_output_level(SLAVE_SELECT_PIN, false);
	startADC(false);
	reset_ADC();
}

/******************************************************************
 *
 * Description: Resets the ADC using the pwdn pin.  There is also a
 *  rst pin.
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void reset_ADC(void) {
	uint8_t tx[2] = {STOP_CONT_ADC, STOP_ADC};
    turnOn(false);
	//Needs at least 9ms
	delay_ms(100);
	turnOn(true);
	delay_ms(100);
	txrx_wait(tx,2);
}

/******************************************************************
 *
 * Description: Enables or Disables the drdy callback depending on
 *  the input
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void enableDrdy(bool val) {
	val ? extint_chan_enable_callback(DRDY_PIN_LINE, EXTINT_CALLBACK_TYPE_DETECT) : extint_chan_disable_callback(DRDY_PIN_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}

/******************************************************************
 *
 * Description: Turns on or off the ADC depending on the input
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void turnOn(bool val) {
    port_pin_set_output_level(PWDN_PIN, val);
}

/******************************************************************
 *
 * Description: Sets the start pin depending on the input
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void startADC(bool val) {
    port_pin_set_output_level(START_PIN, val);
}

/******************************************************************
 *
 * Description: Writes the input register with the input value.
 *  ADC allows for multiple sequential registers to be written, but
 *  this functionality is ignored.
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void writeReg(uint8_t reg, uint8_t value) {
    uint8_t attempts = 0, tx[3] = {(WRITE_REG + reg), 0, value};
	do {
		txrx_wait(tx, 3);
    } while (value != readReg(reg) && attempts++ < 3);
}

/******************************************************************
 *
 * Description: Reads the input register and returns that value.
 *  ADC allows for multiple sequential registers to be read, but this
 *  functionality is ignored.
 * Last Modified: 11/1/17
 *
 ******************************************************************/
uint8_t readReg(uint8_t reg) {
	uint8_t tx[3] = {(READ_REG + reg), 0, 0};
	if (reg > CONFIG4_REG) return 0;
    txrx_wait(tx, 3);
    return rx_buf[2];
}

/******************************************************************
 *
 * Description: Initializes the ADC.  SPI is configured, GPIO is set,
 *  Registers are set to initialization states.
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void __attribute__((optimize("O0"))) initADC(void) {
	configure_spi_master();
    initGPIO();
    initReg(DATA_RATE_16000,0b00111111);
	writeReg(CONFIG1_REG,CONFIG1_REG_INIT);
	writeReg(CONFIG2_REG,CONFIG2_REG_INIT);
	writeReg(CONFIG3_REG,CONFIG3_REG_INIT);
	writeReg(MISC1_REG,MISC1_REG_INIT);
}

/******************************************************************
 *
 * Description: Determines an appropriate ADC sample rate with the
 *  users desired sample rate as the input.  'FF' is a 'fudge factor'
 *  since we need a little extra time to transfer data between samples
 * Last Modified: 11/1/17
 *
 ******************************************************************/
uint8_t determineADCRate(float rate) {
    if (rate < (float) RATE_1000 * FF) {
        if (rate < (float) RATE_500 * FF) return (rate < (float) RATE_250 * FF) ? DATA_RATE_250 : DATA_RATE_500;
        else return DATA_RATE_1000;
    }
    else {
        if (rate < (float) RATE_4000 * FF) return (rate < (float) RATE_2000 * FF) ? DATA_RATE_2000 : DATA_RATE_4000;
		else return (rate < (float) RATE_8000 * FF) ? DATA_RATE_8000 : DATA_RATE_16000;
    }
}
