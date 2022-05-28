#include "analogExpand.h"
#include "esp_log.h"

void analogExpand_init(analogExpand_t *analogExpand, analogPins_t *analogPins, uint8_t lengthAnalogPins, gpio_num_t *digitalPins, uint8_t lengthDigitalPins, uint32_t resistance)
{
    analogExpand->resistance = resistance;
    analogExpand->lengthAnalogPins = lengthAnalogPins;
    analogExpand->lengthDigitalPins = lengthDigitalPins;
    analogExpand->analogPins = analogPins;
    analogExpand->digitalPins = digitalPins;
    //Инициализация цифровых пинов
    uint64_t pinBitMask = 0;
    for (uint8_t index = 0; index < lengthDigitalPins; index++)
    {
        gpio_pad_select_gpio(digitalPins[index]);
        pinBitMask |= 1ULL << digitalPins[index];
    }
    gpio_config_t configGPIO = {
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = pinBitMask,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 0};
    gpio_config(&configGPIO);
    //Инициализация аналоговых пинов
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc_set_data_inv(ADC_UNIT_1, true);
    for (uint8_t index = 0; index < lengthAnalogPins; index++)
    {
        adc_gpio_init(analogPins[index].adcUnit, analogPins[index].adcChannel);
    }
}

// static int correctAnalogSignal(adc1_channel_t channel){
//     uint16_t averageAnalogSignal = 0;
//     for (uint16_t metering = 0; metering != countMetering; metering++){
//         averageAnalogSignal += adc1_get_raw(channel);
//     }
//     return averageAnalogSignal / countMetering;
// }

uint32_t analogExpand_getResistanceIndexPort(analogExpand_t *analogExpand, uint16_t indexAnalogPins, uint16_t indexDigitalPins)
{
    gpio_set_level(analogExpand->digitalPins[indexDigitalPins], 1);
    uint32_t resistance = 4294967295;
    uint16_t analogSignal = adc1_get_raw(analogExpand->analogPins[indexAnalogPins].adcChannel);
    if (analogSignal)
    {
        resistance = ((analogExpand->resistance * 4096) / analogSignal) - analogExpand->resistance;
    }
    gpio_set_level(analogExpand->digitalPins[indexDigitalPins], 0);
    return resistance;
}
uint32_t analogExpand_getResistancePort(analogExpand_t *analogExpand, uint16_t port)
{
    return analogExpand_getResistanceIndexPort(analogExpand, (port % analogExpand->lengthAnalogPins), (port / analogExpand->lengthAnalogPins));
}
void analogExpand_getResistanceAnalogPorts(analogExpand_t *analogExpand, uint16_t indexDigitalPins, uint32_t *resistance)
{
    gpio_set_level(analogExpand->digitalPins[indexDigitalPins], 1);
    for (uint16_t indexAnalogPins = 0; indexAnalogPins < analogExpand->lengthAnalogPins; indexAnalogPins++)
    {
        uint16_t analogSignal = adc1_get_raw(analogExpand->analogPins[indexAnalogPins].adcChannel);
        if (analogSignal)
        {
            resistance[indexAnalogPins] = ((analogExpand->resistance * 4096) / analogSignal) - analogExpand->resistance;
        }
        else
        {
            resistance[indexAnalogPins] = 4294967295;
        }
    }
    gpio_set_level(analogExpand->digitalPins[indexDigitalPins], 0);
}
void analogExpand_getResistancePorts(analogExpand_t *analogExpand, uint32_t *resistance)
{
    for (uint16_t indexDigitalPins = 0; indexDigitalPins < analogExpand->lengthDigitalPins; indexDigitalPins++)
    {
        analogExpand_getResistanceAnalogPorts(analogExpand, indexDigitalPins, &resistance[indexDigitalPins * analogExpand->lengthAnalogPins]);
    }
}

bool analogExpand_getActivityIndexPort(analogExpand_t *analogExpand, uint16_t indexAnalogPins, uint16_t indexDigitalPins)
{
    uint16_t analogSignalFalse = adc1_get_raw(analogExpand->analogPins[indexAnalogPins].adcChannel);
    gpio_set_level(analogExpand->digitalPins[indexDigitalPins], 1);
    uint16_t analogSignalTrue = adc1_get_raw(analogExpand->analogPins[indexAnalogPins].adcChannel);
    gpio_set_level(analogExpand->digitalPins[indexDigitalPins], 0);
    return !analogSignalFalse && analogSignalTrue;
}
bool analogExpand_getActivityPort(analogExpand_t *analogExpand, uint16_t port)
{
    return analogExpand_getActivityIndexPort(analogExpand, (port % analogExpand->lengthAnalogPins), (port / analogExpand->lengthAnalogPins));
}
void analogExpand_getActivityAnalogPorts(analogExpand_t *analogExpand, uint16_t indexDigitalPins, bool *activity)
{
    for (uint16_t indexAnalogPins = 0; indexAnalogPins < analogExpand->lengthAnalogPins; indexAnalogPins++)
    {
        activity[indexAnalogPins] = analogExpand_getActivityIndexPort(analogExpand, indexAnalogPins, indexDigitalPins);
    }
}
void analogExpand_getActivityPorts(analogExpand_t *analogExpand, bool *activity)
{
    for (uint16_t indexDigitalPins = 0; indexDigitalPins < analogExpand->lengthDigitalPins; indexDigitalPins++)
    {
        analogExpand_getActivityAnalogPorts(analogExpand, indexDigitalPins, &activity[indexDigitalPins * analogExpand->lengthAnalogPins]);
    }
}