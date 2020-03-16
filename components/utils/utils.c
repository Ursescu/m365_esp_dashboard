#include "utils.h"

#define TAG "utils"

uint8_t debug[DEBUG_BUFF_SIZE + 1] = {
    0,
};

void print_command(const uint8_t *data, const uint16_t size) {
    if ((size * 2) > DEBUG_BUFF_SIZE) {
        ESP_LOGE(TAG, "%s: Debug buffer to small %d > %d\n", __func__, size, DEBUG_BUFF_SIZE);

        return;
    }

    for (uint8_t index = 0; index < size; index++) {
        sprintf((char *)(debug + (index * 2)), "%02x", data[index]);
    }

    debug[size * 2] = 0;  // Null terminator
    ESP_LOGD(TAG, "%s: Received size %d : %s \n", __func__, size, debug);
}

uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
