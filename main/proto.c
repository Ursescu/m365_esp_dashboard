#include "proto.h"

#include "adc.h"
#include <time.h>

#define 


#define GET_TIME() ((uint32_t) (clock() * 1000 / CLOCKS_PER_SEC))

uint32_t lastTimestamp;

void add_crc(uint8_t *message, uint8_t size) {
    unsigned long cksm = 0;
    for (int i = 2; i < size - 2; i++)
        cksm += message[i];
    cksm ^= 0xFFFF;
    message[size - 2] = (uint8_t)(cksm & 0xFF);
    message[size - 1] = (uint8_t)((cksm & 0xFF00) >> 8);
}

uint16_t proto_crc(const uint8_t *data, uint16_t size) {
    uint32_t calc = 0;
    uint16_t ret = 0;

    for (uint8_t index = 2; index < size - 2; index++)
        calc += data[index];

    calc ^= 0xffff;
    ret |= (((uint8_t)(calc & 0xff)) << CRC0_SHIFT) & CRC0_MASK;
    ret |= (((uint8_t)((calc & 0xff00) >> 8)) << CRC1_SHIFT) & CRC1_MASK;

    return ret;
}


static uint8_t connected() {

    return 0;
}

static void process_command(comm_chan *channel, QueueHandle_t display_queue) {
    /* Verify crc */



    /* Insert display queue if necessary */

    /* Return if crc status */
}

void proto_command(comm_chan *channel, QueueHandle_t display_queue) {

    static uint8_t messageType = 0;

    uint8_t brake = adc_brake();
    uint8_t speed = adc_speed();

    /* Read the data, verify crc and send data to the display queue */
    if(channel->rx_size > channel->tx_size) {
        printf("Received something\n");
        uint8_t status;

        /* Process incoming data*/
        process_command(channel, display_queue);
    }

    if(!connected()) {
        messageType = 4;
    }
    make_command(channel, messageType);
    messageType++;
    switch (messageType++)
    {
        case 0:
        case 1:
        case 2:
        case 3:

            break;
        case 4:
            break;
        
        default:
            break;
    }
}