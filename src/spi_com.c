// A delay is added to the SPI functions such that the end of the
// second byte arrives at least 2us after the start of the first
// byte per page 40 of the ADS1299 datasheet
#include "spi_com.h"

#define CONF_MASTER_SPI_MODULE  SERCOM0

static struct spi_module spi_master_instance;
static struct spi_slave_inst slave;
uint8_t rx_buf[BUF_SIZE];
bool trx_complete = false;

/******************************************************************
 *
 * Description: Configures the SPI
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void configure_spi_master(void) {
    struct spi_config config_spi_master;
    struct spi_slave_inst_config slave_dev_config;
    
    spi_slave_inst_get_config_defaults(&slave_dev_config);
    slave_dev_config.ss_pin = SLAVE_SELECT_PIN;
    spi_attach_slave(&slave, &slave_dev_config);
    
    /* Configure, initialize and enable SERCOM SPI module */
    // PA4 - MOSI, PA5 - SS, PA6 - MISO, PA7 - SCK, 
    spi_get_config_defaults(&config_spi_master);
    config_spi_master.mux_setting = SPI_SIGNAL_MUX_SETTING_O;
    config_spi_master.pinmux_pad0 = PINMUX_PA04D_SERCOM0_PAD0;
    config_spi_master.pinmux_pad1 = PINMUX_UNUSED;
    config_spi_master.pinmux_pad2 = PINMUX_PA06D_SERCOM0_PAD2;
    config_spi_master.pinmux_pad3 = PINMUX_PA07D_SERCOM0_PAD3;
    config_spi_master.transfer_mode = SPI_TRANSFER_MODE_1;
	config_spi_master.receiver_enable = true;
	config_spi_master.master_slave_select_enable = true;
	config_spi_master.generator_source = GCLK_GENERATOR_3;
    
    spi_init(&spi_master_instance, CONF_MASTER_SPI_MODULE, &config_spi_master);
    spi_set_baudrate(&spi_master_instance, SPI_SPEED);
    spi_enable(&spi_master_instance);
	configure_spi_master_callbacks(); //For callback mode
}

/******************************************************************
 *
 * Description: Configures the SPI callback funciton.  For txrx
 *  function only
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void configure_spi_master_callbacks(void) {
	spi_register_callback(&spi_master_instance, callback_spi_master, SPI_CALLBACK_BUFFER_TRANSCEIVED);
	spi_enable_callback(&spi_master_instance, SPI_CALLBACK_BUFFER_TRANSCEIVED);
}

/******************************************************************
 *
 * Description: SPI callback function.  For txrx function only
 * Last Modified: 11/1/17
 *
 ******************************************************************/
void callback_spi_master(struct spi_module *const module) {
	trx_complete = true;
}

/******************************************************************
 *
 * Description: SPI transfer, interrupt callback
 * Last Modified: 11/1/17
 *
 ******************************************************************/
uint8_t* __attribute__((optimize("O0"))) txrx(uint8_t* tx, uint8_t num_bytes, uint8_t *rx) {
    if (num_bytes > BUF_SIZE) return NULL;
    
	spi_lock(&spi_master_instance);
    spi_transceive_buffer_wait(&spi_master_instance, tx, rx, 1);
    if (num_bytes > 1) {
        trx_complete = false;
        delay_us(FIRST_BYTE_WAIT);
        spi_transceive_buffer_job(&spi_master_instance, tx+1, rx+1, num_bytes-1);
    }
	spi_unlock(&spi_master_instance);
    
    return rx_buf;
}

/******************************************************************
 *
 * Description: SPI transfer, waits for completion
 * Last Modified: 11/1/17
 *
 ******************************************************************/
uint8_t* __attribute__((optimize("O0"))) txrx_wait(uint8_t *tx, uint8_t num_bytes) {
	if (num_bytes > BUF_SIZE) return NULL;
    
	spi_lock(&spi_master_instance);
    spi_transceive_buffer_wait(&spi_master_instance, tx, rx_buf, 1);
    if (num_bytes > 1) {
        delay_us(FIRST_BYTE_WAIT);
        spi_transceive_buffer_wait(&spi_master_instance, tx+1, rx_buf+1, num_bytes - 1);
    }
	spi_unlock(&spi_master_instance);
    
    return rx_buf;
}

/******************************************************************
 *
 * Description: SPI transfer, waits for completion.  Receieved data
 *  is stored in the buffer input into the function
 * Last Modified: 11/1/17
 *
 ******************************************************************/
uint8_t* __attribute__((optimize("O0"))) txrx_wait_sel(uint8_t *tx, uint8_t num_bytes, uint8_t *rx) {
    if (num_bytes > BUF_SIZE) return NULL;
    
    spi_lock(&spi_master_instance);
    spi_transceive_buffer_wait(&spi_master_instance, tx, rx, 1);
    if (num_bytes > 1) {
        delay_us(FIRST_BYTE_WAIT);
        spi_transceive_buffer_wait(&spi_master_instance, tx+1, rx+1, num_bytes - 1);
    }
    spi_unlock(&spi_master_instance);
    
    return rx_buf;
}
