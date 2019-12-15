#include "utils.h"

uint8_t debug[DEBUG_BUFF_SIZE + 1] = {
    0,
};

void print_command(const uint8_t *data, const uint16_t size) {
    if ((size * 2) > DEBUG_BUFF_SIZE) {
        printf("%s <error>: Debug buffer to small %d > %d\n", __func__, size, DEBUG_BUFF_SIZE);

        return;
    }

    for (uint8_t index = 0; index < size; index++) {
        sprintf((char *)(debug + (index * 2)), "%02x", data[index]);
    }

    debug[size*2] = 0; // Null terminator
    printf("%s <debug>: Received size %d : %s \n", __func__, size, debug);
}
