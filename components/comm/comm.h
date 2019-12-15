#ifndef _COMM_H_
#define _COMM_H_

#include "driver/uart.h"
#include "driver/gpio.h"
#include <string.h>
#include "sdkconfig.h"

#define COMM_PIN_TXD (CONFIG_M365_TX_GPIO)
#define COMM_PIN_RXD (CONFIG_M365_RX_GPIO)

/* The interval needed to send data */
#define TX_INTERVAL (CONFIG_M365_TX_INTERVAL)

/* Timeout for response */
#define RX_TIMEOUT (CONFIG_M365_RX_TIMEOUT)

/* Timeout connection */
#define COMM_TIMEOUT (1000)

#define COMM_BUFF_SIZE (CONFIG_M365_BUFF_SIZE)

#define UART_DEVICE (CONFIG_M365_UART_DEVICE)

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