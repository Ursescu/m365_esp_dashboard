#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#define SPI_BUS TFT_HSPI_HOST

#define RABBIT "/spiffs/rabbit.bmp"
#define SKULL "/spiffs/skull.bmp"
#define TURTLE "/spiffs/turtle.bmp"

#define TAHOMA "/spiffs/fonts/dm.fon"

void display_refresh();

void display_init();

#endif