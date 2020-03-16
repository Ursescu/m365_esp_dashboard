#include "comm.h"

#define TAG "comm"

void comm_init(void) {
    /* Init serial */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    /* Init uart 1 */
    uart_param_config(UART_DEVICE, &uart_config);
    uart_set_pin(UART_DEVICE, COMM_PIN_TXD, COMM_PIN_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UART_DEVICE, COMM_BUFF_SIZE * 2, 0, 0, NULL, 0);
}

void comm_send(comm_chan *channel) {
    uart_write_bytes(UART_DEVICE, (const char *)(channel->tx), channel->tx_size);
}

void comm_recv(comm_chan *channel) {
    uint32_t recv = uart_read_bytes(UART_DEVICE, channel->rx, COMM_BUFF_SIZE, RX_TIMEOUT / portTICK_RATE_MS);

    channel->rx_size = recv;
};

inline void comm_copy_tx_chan(comm_chan *channel, uint8_t *data, uint32_t size) {
    if (size >= COMM_BUFF_SIZE) {
        /* Too much data */
        ESP_LOGE(TAG, "%s: Communication channel buffer to small %d > %d\n", __func__, size, COMM_BUFF_SIZE);
        return;
    }

    channel->tx_size = size;

    /* Copy data to tx channel */
    memcpy(channel->tx, data, size);
}