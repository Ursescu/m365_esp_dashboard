#ifndef _COMM_H_
#define _COMM_H_

#include "driver/uart.h"
#include "driver/gpio.h"

#define COMM_PIN_TXD (GPIO_NUM_17)
#define COMM_PIN_RXD (GPIO_NUM_16)

/* The interval needed to send data */
#define TX_INTERVAL (20)

/* Timeout for response */
#define RX_TIMEOUT (5)

#define COMM_BUFF_SIZE (BUFF_SIZE)

typedef struct {
    uint8_t tx[COMM_BUFF_SIZE];
    uint32_t tx_size;
    uint8_t rx[COMM_BUFF_SIZE];
    uint32_t rx_size;
} comm_chan;

#endif