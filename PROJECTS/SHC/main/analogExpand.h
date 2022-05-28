#ifndef MAIN_ANALOGEXPAND_H_
#define MAIN_ANALOGEXPAND_H_

#include <stdint.h>
#include "driver/gpio.h"
#include "driver/adc.h"

//#define countMetering 16

typedef struct
{
    adc_unit_t adcUnit;
    adc_channel_t adcChannel;
} analogPins_t;

typedef struct
{
    analogPins_t *analogPins;
    uint8_t lengthAnalogPins;
    gpio_num_t *digitalPins;
    uint8_t lengthDigitalPins;
    uint32_t resistance;
} analogExpand_t;

void analogExpand_init(analogExpand_t *analogExpand, analogPins_t *analogPins, uint8_t lengthAnalogPins, gpio_num_t *digitalPins, uint8_t lengthDigitalPins, uint32_t resistance);

uint8_t analogExpand_getPercentIndexPort(analogExpand_t *analogExpand, uint16_t indexAnalogPins, uint16_t indexDigitalPins);
uint8_t analogExpand_getPercentPort(analogExpand_t *analogExpand, uint16_t port);
void analogExpand_getPercentAnalogPorts(analogExpand_t *analogExpand, uint16_t indexDigitalPins, uint8_t *percents);
void analogExpand_getPercentPorts(analogExpand_t *analogExpand, uint8_t *percents);

uint32_t analogExpand_getResistanceIndexPort(analogExpand_t *analogExpand, uint16_t indexAnalogPins, uint16_t indexDigitalPins);
uint32_t analogExpand_getResistancePort(analogExpand_t *analogExpand, uint16_t port);
void analogExpand_getResistanceAnalogPorts(analogExpand_t *analogExpand, uint16_t indexDigitalPins, uint32_t *resistance);
void analogExpand_getResistancePorts(analogExpand_t *analogExpand, uint32_t *resistance);

bool analogExpand_getActivityIndexPort(analogExpand_t *analogExpand, uint16_t indexAnalogPins, uint16_t indexDigitalPins);
bool analogExpand_getActivityPort(analogExpand_t *analogExpand, uint16_t port);
void analogExpand_getActivityAnalogPorts(analogExpand_t *analogExpand, uint16_t indexDigitalPins, bool *activity);
void analogExpand_getActivityPorts(analogExpand_t *analogExpand, bool *activity);
#endif