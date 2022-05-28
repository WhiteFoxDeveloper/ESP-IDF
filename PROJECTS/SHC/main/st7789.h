#ifndef MAIN_ST7789_H_
#define MAIN_ST7789_H_

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "fontx.h"

#define bool _Bool

#define RED 0xf800
#define GREEN 0x07e0
#define BLUE 0x001f
#define BLACK 0x0000
#define WHITE 0xffff
#define GRAY 0x8c51
#define YELLOW 0xFFE0
#define CYAN 0x07FF
#define PURPLE 0xF81F

#define DIRECTION0 0
#define DIRECTION90 1
#define DIRECTION180 2
#define DIRECTION270 3

typedef struct
{
	spi_host_device_t hostSPI;
	int speedSPI;
	gpio_num_t pinCS;
	gpio_num_t pinRST;
	gpio_num_t pinDC;
	gpio_num_t pinBL;
	int width;
	int height;
	int offsetX;
	int offsetY;
} st7789_config_t;

typedef struct
{
	uint16_t width;
	uint16_t height;
	uint16_t offsetX;
	uint16_t offsetY;
	gpio_num_t pinDC;
	gpio_num_t pinBL;
	spi_device_handle_t handleSPI;
	
} st7789_t;

void st7789_spiWrite(uint8_t *data, size_t dataLength);
void st7789_spiWriteCommand(uint8_t command);
void st7789_spiWriteData(uint8_t *data, size_t dataLength);
void st7789_spiWriteDataByte(uint8_t data);
void st7789_spiWriteDataWord(uint16_t data);
void st7789_spiWriteAddress(uint16_t address1, uint16_t address2);
void st7789_spiWriteColor(uint16_t color, uint16_t size);
void st7789_spiWriteColor(uint16_t color, uint16_t size);
void st7789_delayMS(int ms);
uint16_t st7789_convertRGB565(uint16_t r, uint16_t g, uint16_t b);

// commands
void st7789_inversionOff();
void st7789_inversionOn();
void st7789_displayOff();
void st7789_displayOn();
void st7789_sleepOff();
void st7789_sleepOn();
void st7789_partialOff();
void st7789_partialOn();
void st7789_idleOff();
void st7789_idleOn();
void st7789_setColumn(uint16_t x1, uint16_t x2);
void st7789_setRow(uint16_t y1, uint16_t y2);
void st7789_setActiveArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void st7789_memoryWrite();
void st7789_backlightOff();
void st7789_backlightOn();

// lcd init
void st7789_select(st7789_t *dev);
void st7789_init(st7789_config_t *config);
#endif /* MAIN_ST7789_H_ */
