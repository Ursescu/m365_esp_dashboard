#define NUM_LEDS 192
#include "ws2812_control.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define GREEN 0xFF0000
#define RED 0x00FF00
#define BLUE 0x0000FF

#define WHITE 0xFFFFFF
#define BLACK 0x000000

#define DELAY 300

struct led_state new_state;

void three_flashes(uint32_t flashDelay, uint32_t betweenDelay) {
    for (uint8_t repeat = 0; repeat < 6; repeat++) {
        for (uint32_t i = 0; i < NUM_LEDS; i++) {
            if (repeat < 3) {
                if (i < NUM_LEDS / 2) {
                    new_state.leds[i] = RED;
                } else {
                    new_state.leds[i] = BLACK;
                }
            } else {
                if (i < NUM_LEDS / 2) {
                    new_state.leds[i] = BLACK;
                } else {
                    new_state.leds[i] = BLUE;
                }
            }
        }
        ws2812_write_leds(new_state);
        vTaskDelay(flashDelay / portTICK_PERIOD_MS);
        for (uint32_t i = 0; i < NUM_LEDS; i++) {
            new_state.leds[i] = BLACK;
        }
        ws2812_write_leds(new_state);
        if (repeat == 2 || repeat == 5) {
            vTaskDelay(betweenDelay / portTICK_PERIOD_MS);
        } else {
            vTaskDelay(flashDelay / portTICK_PERIOD_MS);
        }
    }
}
void three_half_flashes() {
    uint32_t flashDelay = 70;
    uint32_t betweenDelay = 200;
    uint32_t temp = NUM_LEDS / 4;

    for (uint8_t repeat = 0; repeat < 4; repeat++) {
        for (uint32_t i = 0; i < NUM_LEDS; i++) {
            if (repeat == 0 || repeat == 1) {
                if (i < NUM_LEDS / 2) {
                    if ((i % NUM_LEDS / 2) % temp < temp / 2) {
                        new_state.leds[i] = RED;
                    } else {
                        new_state.leds[i] = BLACK;
                    }
                } else {
                    if ((i % NUM_LEDS / 2) % temp < temp / 2) {
                        new_state.leds[i] = BLUE;
                    } else {
                        new_state.leds[i] = BLACK;
                    }
                }
            } else if (repeat == 2 || repeat == 3) {
                if (i < NUM_LEDS / 2) {
                    if ((i % NUM_LEDS / 2) % temp < temp / 2) {
                        new_state.leds[i] = BLACK;
                    } else {
                        new_state.leds[i] = RED;
                    }
                } else {
                    if ((i % NUM_LEDS / 2) % temp < temp / 2) {
                        new_state.leds[i] = BLACK;
                    } else {
                        new_state.leds[i] = BLUE;
                    }
                }
            }
        }
        ws2812_write_leds(new_state);
        vTaskDelay(flashDelay / portTICK_PERIOD_MS);

        if (repeat == 1 || repeat == 3) {
            vTaskDelay(betweenDelay / portTICK_PERIOD_MS);
        } else {
            for (uint32_t i = 0; i < NUM_LEDS; i++) {
                new_state.leds[i] = BLACK;
            }
            ws2812_write_leds(new_state);
            vTaskDelay(flashDelay / portTICK_PERIOD_MS);
        }
    }
}

void leds_refresh() {
    ws2812_control_init();

    while (1) {
        uint8_t i;
        for (i = 0; i < 5; i++) {
            three_flashes(50, 150);
        }

        for (i = 0; i < 20; i++) {
            three_flashes(10, 50);
        }
        for (i = 0; i < 5; i++) {
            three_half_flashes();
        }
        continue;

        for (uint32_t i = 0; i < NUM_LEDS / 2; i++) {
            new_state.leds[i] = RED;
            new_state.leds[i + NUM_LEDS / 2] = BLUE;
        }

        ws2812_write_leds(new_state);
        vTaskDelay(DELAY / portTICK_PERIOD_MS);

        // memset(&new_state, 0, sizeof(new_state));

        // for (uint8_t i = 0; i < NUM_LEDS; i++)
        // {
        //     new_state.leds[i] = a;
        //     a = (a + 20) % 0xFFFFFF;
        // }

        // ws2812_write_leds(new_state);
        // vTaskDelay(DELAY / portTICK_PERIOD_MS);

        // for (uint8_t i = 0; i < NUM_LEDS / 2; i++)
        // {
        //     new_state.leds[i] = RED;
        //     new_state.leds[i + NUM_LEDS / 2] = BLUE;
        // }
        for (uint32_t i = 0; i < NUM_LEDS / 2; i++) {
            new_state.leds[i] = BLUE;
            new_state.leds[i + NUM_LEDS / 2] = RED;
        }

        ws2812_write_leds(new_state);
        vTaskDelay(DELAY / portTICK_PERIOD_MS);
    }
}