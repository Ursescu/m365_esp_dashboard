#ifndef _COMM_H_
#define _COMM_H_

#include "driver/uart.h"
#include "driver/gpio.h"
#include <string.h>

#define COMM_PIN_TXD (GPIO_NUM_17)
#define COMM_PIN_RXD (GPIO_NUM_16)

/* The interval needed to send data */
#define TX_INTERVAL (50)

/* Timeout for response */
#define RX_TIMEOUT (10)

/* Timeout connection */
#define COMM_TIMEOUT (1000)

#define COMM_BUFF_SIZE (CONFIG_M365_BUFF_SIZE)

#define UART_DEVICE (UART_NUM_1)

typedef struct {
    uint8_t tx[COMM_BUFF_SIZE];
    uint32_t tx_size;
    uint8_t rx[COMM_BUFF_SIZE];
    uint32_t rx_size;
} comm_chan;

void comm_init(void);
void comm_send(comm_chan *);
void comm_recv(comm_chan *);

void comm_copy_tx_chan(comm_chan *, uint8_t *, uint32_t);

#endif