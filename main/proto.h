#ifndef _PROTO_H_
#define _PROTO_H_

#include <stdint.h>
#include "comm.h"

#include "freertos/queue.h"

#define CRC0_MASK (0xff00)
#define CRC0_SHIFT (8)

#define CRC1_MASK (0x00ff)
#define CRC1_SHIFT (0)

void add_crc(uint8_t *, uint8_t);
uint16_t proto_crc(const uint8_t *, uint16_t);
void proto_command(comm_chan *);

#endif