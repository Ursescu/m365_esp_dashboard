#include "buzzer.h"



void defaultBeep() {
    printf("Default beep\n");
}
/* LEDC esp32 documentation */
void beep(uint32_t freq, uint32_t on_time, uint32_t off_time, uint8_t beeps) {
    printf("Beep\n");
}