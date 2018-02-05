#ifndef SPI_COM_H
#define SPI_COM_H

#include <asf.h>

#define SLAVE_SELECT_PIN PIN_PA05
#define BUF_SIZE 25
#define FIRST_BYTE_WAIT 2
#define SPI_SPEED 12000000

extern uint8_t rx_buf[BUF_SIZE];
extern bool trx_complete;

void configure_spi_master(void);
void configure_spi_master_callbacks(void);
void callback_spi_master(struct spi_module *const module);
uint8_t* txrx(uint8_t* tx, uint8_t num_bytes, uint8_t *rx);
uint8_t* txrx_wait(uint8_t *tx, uint8_t num_bytes);
uint8_t* txrx_wait_sel(uint8_t *tx, uint8_t num_bytes, uint8_t *rx);

#endif
