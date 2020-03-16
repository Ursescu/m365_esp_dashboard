#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include <stdio.h>
#include "esp_log.h"
#include "sdkconfig.h"

#define DEBUG_BUFF_SIZE (CONFIG_M365_BUFF_SIZE)

/* Get current time macro */
#define GET_TIME() ((uint32_t)(clock() * 1000 / CLOCKS_PER_SEC))

void print_command(const uint8_t *, const uint16_t);

uint32_t map(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

#endif