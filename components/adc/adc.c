#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "adc.h"

#define DEFAULT_VREF    1100        
#define NO_OF_SAMPLES   64

#define ACCEL_CHANNEL (CONFIG_M365_ACCEL_CHANNEL)
#define BRAKE_CHANNEL (CONFIG_M365_BRAKE_CHANNEL)

static const adc_channel_t accelChannel = ACCEL_CHANNEL;
static const adc_channel_t brakeChannel = BRAKE_CHANNEL;

uint8_t d = 0;

void adc_init(void) {
    check_efuse();

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(accelChannel, ADC_ATTEN_DB_0);
    adc1_config_channel_atten(brakeChannel, ADC_ATTEN_DB_0);
}

uint8_t adc_speed(void) {

    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw((adc1_channel_t)accelChannel);
    }
    adc_reading /= NO_OF_SAMPLES;

    printf("ADC READ - SPEED %d\n", adc_reading);

    d += 1;
    if(0x28 + d >= 0xC2) {
        d = 0;
    }
    return 0x28 + d;
}

uint8_t adc_brake(void) {
    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw((adc1_channel_t)accelChannel);
    }
    adc_reading /= NO_OF_SAMPLES;

    printf("ADC READ - BREAK %d\n", adc_reading);

    return 0x28;
}