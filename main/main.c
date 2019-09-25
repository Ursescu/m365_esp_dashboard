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

#include "comm.h"
#include "proto.h"
#include "display.h"

// Communication channel
comm_chan uart_channel;

/* Sempahore to control the order of tasks */
SemaphoreHandle_t rx_sem = NULL;
SemaphoreHandle_t comm_sem = NULL;


QueueHandle_t display_queue = xQueueCreate( 10, sizeof( uint32_t ) );


#define SPEED_POS 7
#define BRAKE_POS 8

uint8_t motor[] = {0x55, 0xAA, 0x9, 0x20, 0x64, 0x0, 0x6, 0x00, 0x00, 0x0, 0x0, 0x72, 0x0, 0x0, 0x0};


// 55 aa 09 20 64 00 06 35 28 00 00 72 00 9d fe
// 55 aa 06 21 64 00 00 05 00 02 6d ff
// 55 aa 02 21 01 00 db ff

// 55 aa 07 20 65 00 04 35 26 00 00 14 ff

// 55 aa 07 20 65 00 04 35 28 00 00 12 ff

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

    uint8_t debug[COMM_BUFF_SIZE * 2] = {0,};
    const uint8_t cnt_pos = 5;
    const uint8_t command_size = 7;
    /* End test */

    while (1) {
        // Wait to receive signal from rx_task
        xSemaphoreTake(comm_sem, portMAX_DELAY);

        /* Process command and prepare the sending command */
        /*
         * Using half duplex UART will cause to read from the tx,
         * flushig UART Buffer seems to be too slow to clear the buffer before rx
         */
        proto_command(&uart_channel, display_queue);

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

    // const char dummy[COMM_BUFF_SIZE] = "Salut!\n";

    initUart();

    int id = 1;
    TimerHandle_t tmr;

    // /* Dummy data */
    // memcpy(uart_channel.tx, dummy, 30);
    // uart_channel.tx_size = 7;

    rx_sem = xSemaphoreCreateBinary();
    comm_sem = xSemaphoreCreateBinary();

    if (rx_sem == NULL || comm_sem == NULL) {
        /* Failed to init semaphore */
        while (1) {
            /* Spin forever */
        }
    }

    motor[SPEED_POS] = 0x40;
    motor[BRAKE_POS] = 0x29;

    // uint16_t crc = proto_crc(motor, sizeof(motor));

    add_crc(motor, sizeof(motor));

    // motor[11] = (crc >> CRC0_SHIFT) & 0xff;
    // motor[12] = (crc >> CRC1_SHIFT) & 0xff;

    memcpy(uart_channel.tx, motor, sizeof(motor));
    uart_channel.tx_size = sizeof(motor);


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
