#include "buzzer.h"
#include <stdio.h>
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "esp_log.h"

#define TAG "buzzer"

#define LEDC_HS_TIMER LEDC_TIMER_0
#define LEDC_HS_MODE LEDC_HIGH_SPEED_MODE

#define LEDC_HS_CH0_GPIO (CONFIG_M365_BUZZER_GPIO)
#define LEDC_HS_CH0_CHANNEL LEDC_CHANNEL_0

#define LEDC_TEST_DUTY (4000)

#define DEFAULT_BEEP_DURATION (CONFIG_M365_BEEP_DURATION)

#define DEFAULT_BEEP_FREQ (CONFIG_M365_BUZZER_FREQ)

static ledc_timer_config_t ledc_timer;

static ledc_channel_config_t ledc_channel;

void buzzer_init(void) {
    ledc_timer = (ledc_timer_config_t){
        .duty_resolution = LEDC_TIMER_13_BIT,  // resolution of PWM duty
        .freq_hz = DEFAULT_BEEP_FREQ,          // frequency of PWM signal
        .speed_mode = LEDC_HS_MODE,            // timer mode
        .timer_num = LEDC_HS_TIMER,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };

    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    ledc_channel = (ledc_channel_config_t){
        .channel = LEDC_HS_CH0_CHANNEL,
        .duty = LEDC_TEST_DUTY,
        .gpio_num = LEDC_HS_CH0_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_HS_TIMER,
    };
}

static void stop_buzzer(TimerHandle_t timerHandler) {
    ledc_stop(ledc_channel.speed_mode, ledc_channel.channel, 1);
}

void buzzer_default_beep() {
    ledc_channel_config(&ledc_channel);

    uint32_t id = 2;
    TimerHandle_t tmr;

    tmr = xTimerCreate("buzzer", pdMS_TO_TICKS(DEFAULT_BEEP_DURATION), pdFALSE, (void*)id, &stop_buzzer);
    if (xTimerStart(tmr, 10) != pdPASS) {
        /* Failed to create timer task */
        ESP_LOGE(TAG, "Failed to create buzzer timer\n");
        while (1) {
            /* Spin forever */
        }
    }
}

/* LEDC esp32 documentation */
void buzzer_beep(uint32_t freq, uint32_t on_time, uint32_t off_time, uint8_t beeps) {
    ESP_LOGV(TAG, "Beep\n");
}