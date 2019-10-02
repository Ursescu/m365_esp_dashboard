#include "adc.h"

uint8_t d = 0;

uint8_t adc_speed(void) {

    d += 1;
    if(0x28 + d >= 0xC2) {
        d = 0;
    }
    return 0x28 + d;
}

uint8_t adc_brake(void) {

    return 0x28;
}