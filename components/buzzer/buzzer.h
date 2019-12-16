#ifndef _BUZZER_H_
#define _BUZZER_H_
#include <stdint.h>
#include <stdio.h>

void defaultBeep();
void beep(uint32_t, uint32_t, uint32_t, uint8_t);

#endif