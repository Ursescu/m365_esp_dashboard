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
#include "esp_log.h"

#include "soc/rtc.h"
#include "soc/rtc_cntl_reg.h"

#include <stdio.h>
#include <string.h>

#define TAG "main"
#define PROCESS_CPU 0
#define COMM_CPU 1

#include "comm.h"
#include "buzzer.h"
#include "proto.h"
#include "display.h"
#include "utils.h"
#include "adc.h"

/* Communication channel */
comm_chan uart_channel;

/* Sempahore to control the order of tasks */
SemaphoreHandle_t rx_sem = NULL;
SemaphoreHandle_t comm_sem = NULL;

QueueHandle_t display_queue;

static void tx_task(TimerHandle_t xTimer) {
    /* Tx task 
     * Send data at known time interval
     */

    if (uart_channel.tx_size > 0)
        comm_send(&uart_channel);

    /* Command written, now let the rx_task, wait for response */
    xSemaphoreGive(rx_sem);
}

static void rx_task() {
    /* Rx task
     * Listen for data after the tx is done
     */

    while (1) {
        /* Wait to receive signal from tx_task */
        xSemaphoreTake(rx_sem, portMAX_DELAY);

        /* Receive data from channel */
        comm_recv(&uart_channel);

        // print_command(uart_channel.rx, uart_channel.rx_size);

        /* Pass control to comm_task */
        xSemaphoreGive(comm_sem);
    }
}

static void comm_task() {
    /* Communication task
     * This is the place where the incoming commands are verified
     * and next command is set
     */

    while (1) {
        /* Wait to receive signal from rx_task */
        xSemaphoreTake(comm_sem, portMAX_DELAY);

        /*
         * Using half duplex UART will cause to read from the tx,
         * flushig UART Buffer seems to be too slow to clear the buffer before rx
         */

        /* Process command and prepare the sending command */
        proto_command(&uart_channel, display_queue);

        /* Done processing command */
    }
}

static void display_task() {
    /* Dsiplay task
     * This will update the info from the BMS and ESC
     * The timing it's not that important for this task, update when
     * nothing else is doing work
     */

    display_init();

    while (1) {
        /* Get from queue and process it */

        /* data = queue get */

        display_refresh();
        /* update(data) */
    }
}

void app_main() {
    /* Esp-idf entry point */

    /* Debug, show frequency */
    rtc_cpu_freq_config_t old_config;
    rtc_clk_cpu_freq_get_config(&old_config);

    ESP_LOGI(TAG, "Main app, running freq %d", old_config.freq_mhz);
    ESP_LOGI(TAG, "Core id: %d", xPortGetCoreID());
    /* End debug */

    /* Init ADC */
    adc_init();

    /* Init Buzzer */
    buzzer_init();

    /* Init communication channel */
    comm_init();

    /* Init semaphores used to dictate the tasks */
    rx_sem = xSemaphoreCreateBinary();
    comm_sem = xSemaphoreCreateBinary();

    if (rx_sem == NULL || comm_sem == NULL) {
        /* Failed to init semaphore */
        ESP_LOGE(TAG, "Failed to create semaphores\n");
        while (1) {
            /* Spin forever */
        }
    }

    /* This is the main task to send data.
     * I need to be sure that command is prepared until next period. (20 ms)
     */
    int id = 1;
    TimerHandle_t tmr;

    tmr = xTimerCreate("tx_task", pdMS_TO_TICKS(TX_INTERVAL), pdTRUE, (void*)id, &tx_task);
    if (xTimerStart(tmr, 10) != pdPASS) {
        /* Failed to create timer task */
        ESP_LOGE(TAG, "Failed to create timer\n");
        while (1) {
            /* Spin forever */
        }
    }

    // processing is done on core 0
    xTaskCreatePinnedToCore(comm_task, "comm_task", 1024 * 4, NULL, configMAX_PRIORITIES, NULL, PROCESS_CPU);

    // // update the output on core 0
    xTaskCreatePinnedToCore(display_task, "disp_task", 1024 * 4, NULL, configMAX_PRIORITIES - 1, NULL, PROCESS_CPU);

    // rx is done on core 1
    xTaskCreatePinnedToCore(rx_task, "rx_task", 1024 * 4, NULL, configMAX_PRIORITIES, NULL, COMM_CPU);
}
