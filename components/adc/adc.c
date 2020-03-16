#include "driver/adc.h"

#include "adc.h"
#include "proto.h"
#include "utils.h"
#include "driver/gpio.h"

#define DEFAULT_VREF 1100
#define NO_OF_SAMPLES 64

#define ACCEL_CHANNEL (CONFIG_M365_ACCEL_CHANNEL)
#define BRAKE_CHANNEL (CONFIG_M365_BRAKE_CHANNEL)

uint16_t accel_adc_min = 1000;
uint16_t accel_adc_max = 2700;

uint16_t brake_adc_min = 1000;
uint16_t brake_adc_max = 2700;

void adc_init(void) {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ACCEL_CHANNEL, ADC_ATTEN_11db);
    adc1_config_channel_atten(BRAKE_CHANNEL, ADC_ATTEN_11db);
}

uint8_t adc_speed(void) {
    uint32_t adc_reading = 0;
    // Multisampling
    for (uint8_t i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw(ACCEL_CHANNEL);
    }
    adc_reading /= NO_OF_SAMPLES;

    if (adc_reading > accel_adc_max) {
        accel_adc_max = (uint16_t)adc_reading;
    }
    if (adc_reading < accel_adc_min) {
        accel_adc_min = (uint16_t)adc_reading;
    }

    uint8_t accel_output = (uint8_t)map(adc_reading, accel_adc_min, accel_adc_max, PROTO_ADC_MIN, PROTO_ADC_MAX);

    if (accel_output > PROTO_ADC_MAX_THRESHOLD) {
        accel_output = PROTO_ADC_MAX;
    }

    return accel_output;
}

uint8_t adc_brake(void) {
    uint32_t adc_reading = 0;
    // Multisampling
    for (uint8_t i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw(BRAKE_CHANNEL);
    }
    adc_reading /= NO_OF_SAMPLES;

    if (adc_reading > brake_adc_max) {
        brake_adc_max = (uint16_t)adc_reading;
    }
    if (adc_reading < brake_adc_min) {
        brake_adc_min = (uint16_t)adc_reading;
    }

    uint8_t brake_output = (uint8_t)map(adc_reading, brake_adc_min, brake_adc_max, PROTO_ADC_MIN, PROTO_ADC_MAX);

    if (brake_output > PROTO_ADC_MAX_THRESHOLD) {
        brake_output = PROTO_ADC_MAX;
    }

    return brake_output;
}