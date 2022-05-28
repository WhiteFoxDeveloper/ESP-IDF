#ifndef MAIN_ENCODER_H_
#define MAIN_ENCODER_H_

#include <stdint.h>
#include "driver/gpio.h"

#define rattleTime 500

typedef struct
{
    gpio_num_t pinKey;
    gpio_num_t pinS1;
    gpio_num_t pinS2;

    uint8_t _reset;
    uint8_t _last;
    int _counter;
    int _counterPress;
    uint8_t _press;
    uint8_t _command;
    int64_t intKeyTime;
} encoder_t;

void setupEncoder(encoder_t *encoder, gpio_num_t pinKey, gpio_num_t pinS1, gpio_num_t pinS2);
uint8_t getCommandEncoder(encoder_t *encoder);

#endif
