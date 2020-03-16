#include "proto.h"

#include "adc.h"
#include "utils.h"
#include "buzzer.h"
#include "comm.h"

#include <time.h>

#define TAG "proto"

/* Protocol status structure 
 * Maintaining the state between commands
 */
proto_stat proto_mainboard_stats;

uint32_t last_valid_message_time;

uint8_t proto_verify_crc(uint8_t *message, uint8_t size) {
    uint32_t cksm = 0;
    for (uint8_t i = 2; i < size - 2; i++)
        cksm += message[i];
    cksm ^= 0xFFFF;

    if (cksm == message[size - 2] + (message[size - 1] << 8)) {
        return 1;
    }
    return 0;
}

void proto_add_crc(uint8_t *message, uint8_t size) {
    uint32_t cksm = 0;
    for (uint8_t i = 2; i < size - 2; i++)
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
    /* Some way to measure the timeout, in case connection dropped */

    if (GET_TIME() - last_valid_message_time >= COMM_TIMEOUT) {
        /* Something happened, reset the connection */
        return 0;
    }

    return 1;
}

static void print_stats() {
    ESP_LOGV("State \n");
    ESP_LOGV("Tail: %hhu \n", proto_mainboard_stats.tail);
    ESP_LOGV("Eco: %hhu \n", proto_mainboard_stats.eco);
    ESP_LOGV("Led: %hhu \n", proto_mainboard_stats.led);
    ESP_LOGV("Night: %hhu \n", proto_mainboard_stats.night);
    ESP_LOGV("proto_mainboard_Beep: %hhu \n", proto_mainboard_stats.beep);
    ESP_LOGV("Eco: %hhu \n", proto_mainboard_stats.ecoMode);
    ESP_LOGV("Cruise: %hhu \n", proto_mainboard_stats.cruise);
    ESP_LOGV("Lock: %hhu \n", proto_mainboard_stats.lock);
    ESP_LOGV("Battery: %hhu \n", proto_mainboard_stats.battery);
    ESP_LOGV("Velocity: %hhu \n", proto_mainboard_stats.velocity);
    ESP_LOGV("AverageVelocity: %hhu \n", proto_mainboard_stats.averageVelocity);
    ESP_LOGV("Odometer: %hhu \n", proto_mainboard_stats.odometer);
    ESP_LOGV("Temperature: %hhu \n", proto_mainboard_stats.temperature);
}

static void process_command(const uint8_t *command, uint16_t size) {
    /* Verify crc */
    if (!proto_verify_crc((uint8_t *)command, size))
        return;

    /* Update last valid message */
    last_valid_message_time = GET_TIME();

    switch (command[3]) {
        case 0x21:
            switch (command[4]) {
                case 0x01:
                    if (command[4] == 0x61)
                        proto_mainboard_stats.tail = command[5];
                    break;
                case 0x64:
                    proto_mainboard_stats.eco = command[6];
                    proto_mainboard_stats.led = command[7];
                    proto_mainboard_stats.night = command[8];
                    proto_mainboard_stats.beep = command[9];
                    break;
            }
            break;
        case 0x23:
            switch (command[5]) {
                case 0x7B:
                    proto_mainboard_stats.ecoMode = command[6];
                    proto_mainboard_stats.cruise = command[8];
                    break;
                case 0x7D:
                    proto_mainboard_stats.tail = command[6];
                    break;
                case 0xB0:
                    proto_mainboard_stats.alarmStatus = command[8];
                    proto_mainboard_stats.lock = command[10];
                    proto_mainboard_stats.battery = command[14];
                    proto_mainboard_stats.velocity = (command[16] + (command[17] * 256)) / 1000 / PROTO_CONSTANT;
                    proto_mainboard_stats.averageVelocity = (command[18] + (command[19] * 265)) / 1000 / PROTO_CONSTANT;
                    proto_mainboard_stats.odometer = (command[20] + (command[21] * 256) + (command[22] * 256 * 256)) / 1000 / PROTO_CONSTANT;
                    proto_mainboard_stats.temperature = ((command[28] + (command[29] * 256)) / 10 * 9 / 5) + 32;
                    break;
            }
    }

    print_stats();

    if (proto_mainboard_stats.beep == 1 && !proto_mainboard_stats.alarmStatus) {
        buzzer_default_beep();
    }
    if(proto_mainboard_stats.beep == 2) {
        buzzer_default_beep();
        proto_mainboard_stats.beep = 1;
    }
    else if(proto_mainboard_stats.beep == 3){
        buzzer_default_beep();
        proto_mainboard_stats.beep = 1;
    }

    if (proto_mainboard_stats.alarmStatus) {
        buzzer_default_beep();
        proto_mainboard_stats.alarmStatus = 0;
    }
}

static void process_buffer(comm_chan *channel, QueueHandle_t display_queue) {
    /* 
     * Hack to eliminate what it's send on tx
     * Start reading from rx buffer at position tx_size (suppose no packet was lost in tx)
     */
    uint16_t buffer_size = channel->rx_size - channel->tx_size;
    uint8_t *buffer = channel->rx + channel->tx_size;

    if (buffer_size <= 0 || buffer_size > COMM_BUFF_SIZE) {
        /* Nothing to receive */
        return;
    }

    /* Multiple commands in same buffer */
    uint8_t *command_ptr = buffer;
    uint16_t command_size = 0;

    uint16_t command_no = 0;
    uint16_t index = 0;

    while (index < buffer_size) {
        if (index &&
            buffer[index - 1] == PROTO_COMMAND_HEADER0 &&
            buffer[index] == PROTO_COMMAND_HEADER1) {
            /* Command header detected */
            if (command_no > 0) {
                /* If command_no is 0 it means that the header is for the first command */
                command_size = index - 1 - command_size;
                process_command(command_ptr, command_size);
                command_ptr = buffer + index - 1;
            }
            command_no++;
        } else if (index == buffer_size - 1) {
            /* Process last packet */
            command_size = index - command_size + 1;
            process_command(command_ptr, command_size);
        }
        index++;
    }
}

void proto_command(comm_chan *channel, QueueHandle_t display_queue) {
    static uint8_t messageType = 0;

    uint8_t brake = adc_brake();
    uint8_t speed = adc_speed();

    /* Process what's received and update the display */
    process_buffer(channel, display_queue);

    if (!connected()) {
        ESP_LOGD(TAG, "Not connected \n");
        messageType = 4;
    }

    /* Iterate over all message types and collect informations */
    switch (messageType++) {
        case 0:
            /* Write speed and brake */
        case 1:
            /* Write speed and brake */
        case 2:
            /* Write speed and brake */
        case 3: {
            /* I don't know what this command does, seems to be the way to actually write the speed and brake values */
            uint8_t command[] = {0x55, 0xAA, 0x7, 0x20, 0x65, 0x0, 0x4, speed, brake, 0x0, proto_mainboard_stats.beep, 0x0, 0x0};
            proto_add_crc(command, sizeof(command));
            comm_copy_tx_chan(channel, command, sizeof(command));
            if (proto_mainboard_stats.beep)
                proto_mainboard_stats.beep = 0;
            break;
        }
        case 4: {
            uint8_t command[] = {0x55, 0xAA, 0x9, 0x20, 0x64, 0x0, 0x6, speed, brake, 0x0, proto_mainboard_stats.beep, 0x72, 0x0, 0x0, 0x0};
            proto_add_crc(command, sizeof(command));
            comm_copy_tx_chan(channel, command, sizeof(command));
            if (proto_mainboard_stats.beep)
                proto_mainboard_stats.beep = 0;
            break;
        }
        case 5: {
            uint8_t command[] = {0x55, 0xAA, 0x6, 0x20, 0x61, 0xB0, 0x20, 0x02, speed, brake, 0x0, 0x0};
            proto_add_crc(command, sizeof(command));
            comm_copy_tx_chan(channel, command, sizeof(command));
            break;
        }
        case 6: {
            uint8_t command[] = {0x55, 0xAA, 0x6, 0x20, 0x61, 0x7B, 0x4, 0x2, speed, brake, 0x0, 0x0};
            proto_add_crc(command, sizeof(command));
            comm_copy_tx_chan(channel, command, sizeof(command));
            break;
        }
        case 7: {
            uint8_t command[] = {0x55, 0xAA, 0x6, 0x20, 0x61, 0x7D, 0x2, 0x2, speed, brake, 0x0, 0x0};
            proto_add_crc(command, sizeof(command));
            comm_copy_tx_chan(channel, command, sizeof(command));
            messageType = 0;
            break;
        }
    }
}
