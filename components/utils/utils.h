#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include <stdio.h>

extern const char *TAG;

#include "esp_log.h"
#include "sdkconfig.h"

#define DEBUG_BUFF_SIZE (CONFIG_M365_BUFF_SIZE)

void print_command(const uint8_t *, const uint16_t);

uint32_t map(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

#endif