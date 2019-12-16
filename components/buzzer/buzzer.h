#ifndef _BUZZER_H_
#define _BUZZER_H_
#include <stdint.h>

void buzzer_init(void);

void buzzer_default_beep(void);
void buzzer_beep(uint32_t, uint32_t, uint32_t, uint8_t);

#endif