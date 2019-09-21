#ifndef _COMM_H_
#define _COMM_H_

#include "driver/uart.h"
#include "driver/gpio.h"


#define COMM_PIN_TXD  (GPIO_NUM_17)
#define COMM_PIN_RXD  (GPIO_NUM_16)

/* The interval needed to send data */
#define TX_INTERVAL (2000)

/* Timeout for response */
#define RX_TIMEOUT (500)

#define MAX_COMMAND_SIZE (100)

typedef struct {
    uint8_t tx[COMM_BUFF_SIZE];
    uint32_t tx_size;
    uint8_t rx[COMM_BUFF_SIZE];
    uint32_t rx_size;
} comm_chan;



#endif