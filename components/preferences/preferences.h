#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_

#include "utils.h"

#define NVS_NAMESPACE "M365"

#pragma pack(push, 1)
typedef struct {
    uint8_t size;
    uint8_t max_speed;
    uint8_t min_speed;
    char name[10];
} Preferences;
#pragma pack(pop)

bool preferences_init(void);
bool preferences_save(void);
void preferences_print(void);

extern Preferences dashboard_preferences;

#endif