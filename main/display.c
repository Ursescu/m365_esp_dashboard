
#include "../components/tft/tftspi.h"
#include "../components/tft/tft.h"
#include "driver/gpio.h"

#define SPI_BUS TFT_HSPI_HOST


void refreshDisplay() {
    TFT_print("Test", CENTER, 0);
}

void initDisplay() {
    tft_disp_type = DISP_TYPE_ST7735;

    _width = 128;   // smaller dimension
    _height = 160;  // larger dimension

    max_rdclock = 8000000;

    TFT_PinsInit();

    spi_lobo_device_handle_t spi;

    spi_lobo_bus_config_t buscfg = {
        .miso_io_num = -1,           // set SPI MISO pin
        .mosi_io_num = GPIO_NUM_26,  // set SPI MOSI pin
        .sclk_io_num = GPIO_NUM_25,  // set SPI CLK pin
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
    };

    spi_lobo_device_interface_config_t devcfg = {
        .clock_speed_hz = 8000000,          // Initial clock out at 8 MHz
        .mode = 0,                          // SPI mode 0
        .spics_io_num = -1,                 // we will use external CS pin
        .spics_ext_io_num = GPIO_NUM_12,    // external CS pin
        .flags = LB_SPI_DEVICE_HALFDUPLEX,  // ALWAYS SET  to HALF DUPLEX MODE!! for display spi
    };

    spi_lobo_bus_add_device(SPI_BUS, &buscfg, &devcfg, &spi);

    TFT_display_init();

    spi_lobo_set_speed(spi, DEFAULT_SPI_CLOCK);

    refreshDisplay();
}