/* 
 *Main file
 */

#include "freertos/FreeRTOS.h"
#include "freertos/portable.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp32/clk.h"

#include "soc/rtc.h"
#include "soc/rtc_cntl_reg.h"

#include <stdio.h>
#include <string.h>

#define PROCESS_CPU 0
#define COMM_CPU 1

#define COMM_BUFF_SIZE (1024)
#include "comm.h"

// Communication channel
comm_chan uart_channel;

/* Sempahore to control the order of tasks */
SemaphoreHandle_t rx_sem = NULL;
SemaphoreHandle_t comm_sem = NULL;

static void tx_task(TimerHandle_t xTimer) {
    printf("Send task\n");
    uart_write_bytes(UART_NUM_1, (const char*)uart_channel.tx, uart_channel.tx_size);

    /* Command written, now let the rx_task, wait for response */
    xSemaphoreGive(rx_sem);
}

static void rx_task() {
    while (1) {
        // Wait to receive signal from tx_task
        xSemaphoreTake(rx_sem, portMAX_DELAY);

        const int rx_size = uart_read_bytes(UART_NUM_1, uart_channel.rx, COMM_BUFF_SIZE, RX_TIMEOUT / portTICK_RATE_MS);
        uart_channel.rx_size = rx_size;

        /* Pass control to comm_task */
        xSemaphoreGive(comm_sem);
    }
}

static void comm_task() {
    /* Just to test command */
    uint32_t cnt = 0;
    uint8_t command[MAX_COMMAND_SIZE] = {
        'P',
        'r',
        'o',
        'c',
        ' ',
        0,
        '\n',
        0,
    };
    const uint8_t cnt_pos = 5;
    const uint8_t command_size = 7;
    /* End test */

    while (1) {
        // Wait to receive signal from rx_task
        xSemaphoreTake(comm_sem, portMAX_DELAY);

        /* Process command and prepare the sending command */
        if (uart_channel.rx_size > 0) {
            command[cnt_pos] = uart_channel.rx[0];
            memcpy(uart_channel.tx, command, command_size);
            uart_channel.tx_size = command_size;
        }
        /* Done processing command */
    }
}

static inline void initUart() {
    // Init serial
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    // Init uart 1
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, COMM_PIN_TXD, COMM_PIN_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UART_NUM_1, COMM_BUFF_SIZE * 2, 0, 0, NULL, 0);
}

void app_main() {
    rtc_cpu_freq_config_t old_config;
    rtc_clk_cpu_freq_get_config(&old_config);

    printf("Main app -- freq %d\n", old_config.freq_mhz);
    printf("Core id: %d\n", xPortGetCoreID());

    const char dummy[COMM_BUFF_SIZE] = "Salut!\n";

    initUart();

    int id = 1;
    TimerHandle_t tmr;

    /* Dummy data */
    memcpy(uart_channel.tx, dummy, 30);
    uart_channel.tx_size = 7;

    rx_sem = xSemaphoreCreateBinary();
    comm_sem = xSemaphoreCreateBinary();

    if (rx_sem == NULL || comm_sem == NULL) {
        /* Failed to init semaphore */
        while (1) {
            /* Spin forever */
        }
    }

    /* This is the main task to send data.
     * I need to be sure that command is prepared until next period. (20 ms)
     */
    tmr = xTimerCreate("tx_task", pdMS_TO_TICKS(TX_INTERVAL), pdTRUE, (void*)id, &tx_task);
    if (xTimerStart(tmr, 10) != pdPASS) {
        printf("Timer start error");
    }

    // processing is done on core 0
    xTaskCreatePinnedToCore(comm_task, "comm", 1024 * 4, NULL, 1, NULL, PROCESS_CPU);

    // communication is done on core 1
    xTaskCreatePinnedToCore(rx_task, "rx_task", 1024 * 4, NULL, configMAX_PRIORITIES, NULL, COMM_CPU);

    // xTaskCreatePinnedToCore(tx_task, "tx_task", 1024 * 4, NULL, configMAX_PRIORITIES - 1, NULL, COMM_CPU);
}
