#ifndef _PROTO_H_
#define _PROTO_H_

#include <stdint.h>
#include "comm.h"

#include "freertos/queue.h"

#define CRC0_MASK (0xff00)
#define CRC0_SHIFT (8)

#define CRC1_MASK (0x00ff)
#define CRC1_SHIFT (0)

#define PROTO_CONSTANT (1.60934)
#define PROTO_COMMAND_HEADER0 (0x55)
#define PROTO_COMMAND_HEADER1 (0xAA)

typedef struct __stat {
    uint8_t alarmStatus;
    uint8_t averageVelocity;
    uint8_t battery;
    uint8_t beep;
    uint8_t brakeConnected;
    uint8_t cruise;
    uint8_t eco;
    uint8_t ecoMode;
    uint8_t hasConnected;
    uint8_t isConnected;
    uint8_t led;
    uint8_t lock;
    uint8_t night;
    uint8_t odometer;
    uint8_t tail;
    uint8_t temperature;
    uint8_t throttleConnected;
    uint8_t velocity;
} proto_stat;

void proto_add_crc(uint8_t *, uint8_t);
uint8_t proto_verify_crc(uint8_t *, uint8_t);
uint16_t proto_crc(const uint8_t *, uint16_t);
void proto_command(comm_chan *, QueueHandle_t);

#endif