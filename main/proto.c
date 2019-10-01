#include "proto.h"

#include "adc.h"

#include <time.h>

/* Get current time macro */
#define GET_TIME() ((uint32_t)(clock() * 1000 / CLOCKS_PER_SEC))

#define PROTO_CONSTANT (1.60934)

/* Protocol status structure 
 * Maintaining the state between commands
 */
proto_stat stats;

uint32_t last_valid_message_time;

uint8_t proto_verify_crc(uint8_t *message, uint8_t size) {
    unsigned long cksm = 0;
    for (int i = 0; i < size - 2; i++)
        cksm += message[i];
    cksm ^= 0xFFFF;

    if (cksm == message[size - 2] + (message[size - 1] << 8)) {
        return 1;
    }
    return 0;
}

void proto_add_crc(uint8_t *message, uint8_t size) {
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

// {0x55, 0xAA, 0x4, 0x20, 0x3, 0x7C, 0x0, 0x0, 0x5C, 0xFF};

static uint8_t connected() {
    /* Some way to measure the timeout, in case connection dropped */

    if (GET_TIME() - last_valid_message_time >= COMM_TIMEOUT) {
        /* Something happened, reset the connection */
        return 0;
    }

    return 1;
}

static void process_command(comm_chan *channel, QueueHandle_t display_queue) {
    /* Verify crc */
    if (!proto_verify_crc(channel->rx, channel->rx_size))
        return;

    /* Update last good message */
    last_valid_message_time = GET_TIME();

    const char *message = channel->rx;

    switch (message[1]) {
        case 0x21:
            switch (message[2]) {
                case 0x01:
                    if (message[2] == 0x61)
                        stats.tail = message[3];
                    break;
                case 0x64:
                    stats.eco = message[4];
                    stats.led = message[5];
                    stats.night = message[6];
                    stats.beep = message[7];
                    break;
            }
            break;
        case 0x23:
            switch (message[3]) {
                case 0x7B:
                    stats.ecoMode = message[4];
                    stats.cruise = message[6];
                    break;
                case 0x7D:
                    stats.tail = message[4];
                    break;
                case 0xB0:
                    stats.alarmStatus = message[6];
                    stats.lock = message[8];
                    stats.battery = message[12];
                    stats.velocity = (message[14] + (message[15] * 256)) / 1000 / PROTO_CONSTANT;
                    stats.averageVelocity = (message[16] + (message[17] * 265)) / 1000 / PROTO_CONSTANT;
                    stats.odometer = (message[18] + (message[19] * 256) + (message[20] * 256 * 256)) / 1000 / PROTO_CONSTANT;
                    stats.temperature = ((message[26] + (message[27] * 256)) / 10 * 9 / 5) + 32;
                    if (stats.alarmStatus)
                        tone(input.buzzer, 20, 400);
                    break;
            }
    }

    /* Send some data to display */
    /* To be implemented */
}

void proto_command(comm_chan *channel, QueueHandle_t display_queue) {
    static uint8_t messageType = 0;

    uint8_t brake = adc_brake();
    uint8_t speed = adc_speed();

    /* Read the data, verify crc and send data to the display queue */
    if (channel->rx_size > channel->tx_size) {
        printf("Received something\n");
        uint8_t status;

        /* Process incoming data*/
        process_command(channel, display_queue);
    }

    if (!connected()) {
        messageType = 4;
    }

    /* Iterate over all message types and collect informations */
    switch (messageType++) {
        case 0:
            /* Just for sake of completeness */
        case 1:
            /* Just for sake of completeness */
        case 2:
            /* Just for sake of completeness */
        case 3: {
            /* I don't know what this command does, seems to be the way to actually write the speed and brake values */
            uint8_t command[] = {0x55, 0xAA, 0x7, 0x20, 0x65, 0x0, 0x4, speed, brake, 0x0, stats.beep, 0x0, 0x0};
            proto_add_crc(command, sizeof(command));
            comm_copy_tx_chan(channel, sizeof(command));
            break;
        }
        case 4: {
            uint8_t command[] = {0x55, 0xAA, 0x9, 0x20, 0x64, 0x0, 0x6, speed, brake, 0x0, stats.beep, 0x72, 0x0, 0x0, 0x0};
            proto_add_crc(command, sizeof(command));
            comm_copy_tx_chan(channel, sizeof(command));
            break;
        }
        case 5: {
            uint8_t command[] = {0x55, 0xAA, 0x6, 0x20, 0x61, 0xB0, 0x20, 0x02, speed, brake, 0x0, 0x0};
            proto_add_crc(command, sizeof(command));
            comm_copy_tx_chan(channel, sizeof(command));
            break;
        }
        case 6: {
            uint8_t command[] = {0x55, 0xAA, 0x6, 0x20, 0x61, 0x7B, 0x4, 0x2, speed, brake, 0x0, 0x0};
            proto_add_crc(command, sizeof(command));
            comm_copy_tx_chan(channel, sizeof(command));
            break;
        }
        case 7: {
            uint8_t command[] = {0x55, 0xAA, 0x6, 0x20, 0x61, 0x7D, 0x2, 0x2, speed, brake, 0x0, 0x0};
            proto_add_crc(command, sizeof(command));
            comm_copy_tx_chan(channel, sizeof(command));
            messageType = 0;
            break;
        }
    }
}