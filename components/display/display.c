
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <sys/stat.h>

#include "tftspi.h"
#include "tft.h"
#include "driver/gpio.h"

#include "display.h"
#include "proto.h"

#define TAG "display"

static time_t time_now, time_last = 0;

char velocityBuffer[10];

void refreshDisplay() {
    TFT_setFont(USER_FONT, TAHOMA);

    sprintf(velocityBuffer, "%.2f", proto_mainboard_stats.velocity * 1.60934);

    TFT_fillWindow(TFT_BLACK);
    TFT_print(velocityBuffer, 0, 8);

    sprintf(velocityBuffer, "%u", proto_mainboard_stats.battery);

    TFT_print(velocityBuffer, 0, 50);
    // TFT_bmp_image(0, 42, 0, RABBIT, NULL, 0);
    // TFT_bmp_image(0, 84, 0, SKULL, NULL, 0);
    // TFT_bmp_image(0, 126, 0, TURTLE, NULL, 0);

    vTaskDelay(300 / portTICK_PERIOD_MS);
}

void initDisplay() {
    esp_err_t ret;

    // === SET GLOBAL VARIABLES ==========================

    // ===================================================
    // ==== Set maximum spi clock for display read    ====
    //      operations, function 'find_rd_speed()'    ====
    //      can be used after display initialization  ====
    tft_max_rdclock = 1000000;
    // ===================================================

    // ====================================================================
    // === Pins MUST be initialized before SPI interface initialization ===
    // ====================================================================
    TFT_PinsInit();

    // ====  CONFIGURE SPI DEVICES(s)  ====================================================================================

    spi_lobo_device_handle_t spi;

    spi_lobo_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,  // set SPI MISO pin
        .mosi_io_num = PIN_NUM_MOSI,  // set SPI MOSI pin
        .sclk_io_num = PIN_NUM_CLK,   // set SPI CLK pin
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 6 * 1024,
    };
    spi_lobo_device_interface_config_t devcfg = {
        .clock_speed_hz = DEFAULT_SPI_CLOCK,  // Initial clock out at 8 MHz
        .mode = 0,                            // SPI mode 0
        .spics_io_num = -1,                   // we will use external CS pin
        .spics_ext_io_num = PIN_NUM_CS,       // external CS pin
        .flags = LB_SPI_DEVICE_HALFDUPLEX,    // ALWAYS SET  to HALF DUPLEX MODE!! for display spi
    };

    ret = spi_lobo_bus_add_device(SPI_BUS, &buscfg, &devcfg, &spi);
    assert(ret == ESP_OK);
    tft_disp_spi = spi;

    // ==== Test select/deselect ====
    ret = spi_lobo_device_select(spi, 1);
    assert(ret == ESP_OK);
    ret = spi_lobo_device_deselect(spi);
    assert(ret == ESP_OK);

    TFT_display_init();

    tft_font_rotate = 0;
    tft_text_wrap = 0;
    tft_font_transparent = 0;
    tft_font_forceFixed = 0;
    tft_gray_scale = 0;
    TFT_setGammaCurve(DEFAULT_GAMMA_CURVE);
    TFT_setRotation(PORTRAIT_FLIP);
    TFT_invertDisplay(INVERT_ON);
    TFT_setFont(DEFAULT_FONT, NULL);
    TFT_resetclipwin();
    TFT_fillWindow(TFT_BLACK);
}
