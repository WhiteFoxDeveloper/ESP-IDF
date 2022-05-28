#include <stdio.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "string.h"

#include "encoder.h"

// static uint8_t cRight = 75; // 01|00|10|11
// static uint8_t cLeft = 135; // 10|00|01|11

static uint8_t IRAM_ATTR getCommand(encoder_t *encoder)
{
  uint8_t command = 0;
  int div = encoder->_counter / 4;
  if (div)
  {
    command = div < 0 ? 2 : 3;
    if (encoder->_press)
    {
      command += 2;
      encoder->_press = 2;
    }
  }
  encoder->_counter = 0;
  return command;
}
static void IRAM_ATTR intKeyEnc(void *args)
{
  encoder_t *encoder = (encoder_t *)args;
  uint8_t command = getCommand(encoder);
  if (!command && encoder->_press == 1)
    command = 1;
  encoder->_command = command;
  encoder->_press = !gpio_get_level(encoder->pinKey);
  // encoder->intKeyTime = esp_timer_get_time() + rattleTime;
}
static void IRAM_ATTR intRotEnc(void *args)
{
  encoder_t *encoder = (encoder_t *)args;
  uint8_t state = (gpio_get_level(encoder->pinS1) << 1) | gpio_get_level(encoder->pinS2);
  if (encoder->_reset && state == 0b11)
  {
    int prevCount = encoder->_counter;
    if (encoder->_last == 0b10)
      encoder->_counter++;
    else if (encoder->_last == 0b01)
      encoder->_counter--;
    if (prevCount != encoder->_counter)
      encoder->_reset = 0;
  }
  if (!state)
    encoder->_reset = 1;
  encoder->_last = state;
}

void setupEncoder(encoder_t *encoder, gpio_num_t pinKey, gpio_num_t pinS1, gpio_num_t pinS2)
{
  encoder->pinKey = pinKey;
  encoder->pinS1 = pinS1;
  encoder->pinS2 = pinS2;

  gpio_pad_select_gpio(encoder->pinKey);
  gpio_pad_select_gpio(encoder->pinS1);
  gpio_pad_select_gpio(encoder->pinS2);
  gpio_config_t io_conf = {
      .intr_type = GPIO_INTR_ANYEDGE,
      .pin_bit_mask = (1ULL << encoder->pinKey) | (1ULL << encoder->pinS1) | (1ULL << encoder->pinS2),
      .mode = GPIO_MODE_INPUT,
      .pull_down_en = 0,
      .pull_up_en = 1};
  gpio_config(&io_conf);

  encoder->_press = !gpio_get_level(encoder->pinKey);
  encoder->_reset = 0;
  encoder->_last = 0;
  encoder->_counter = 0;
  encoder->_command = 0;
  encoder->intKeyTime = esp_timer_get_time();

  gpio_install_isr_service(0);
  gpio_isr_handler_add(encoder->pinKey, intKeyEnc, encoder);
  gpio_isr_handler_add(encoder->pinS1, intRotEnc, encoder);
  gpio_isr_handler_add(encoder->pinS2, intRotEnc, encoder);
}

uint8_t getCommandEncoder(encoder_t *encoder)
{
  uint8_t command = getCommand(encoder);
  if (!command)
    command = encoder->_command;
  encoder->_command = 0;
  return command;
}
