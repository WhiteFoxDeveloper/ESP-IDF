#include <string.h>
#include <math.h>
#include "esp_log.h"

#include "st7789.h"

#define COMMAND_MODE 0
#define DATA_MODE 1

static spi_transaction_t transactionSPI;
static st7789_t *dev;

void st7789_spiWrite(uint8_t *data, size_t dataLength)
{
	transactionSPI.length = dataLength * 8;
	transactionSPI.tx_buffer = data;
	spi_device_transmit(dev->handleSPI, &transactionSPI);
}
void st7789_spiWriteCommand(uint8_t command)
{
	gpio_set_level(dev->pinDC, COMMAND_MODE);
	st7789_spiWrite(&command, 1);
}
void st7789_spiWriteData(uint8_t *data, size_t dataLength)
{
	gpio_set_level(dev->pinDC, DATA_MODE);
	st7789_spiWrite(data, dataLength);
}
void st7789_spiWriteDataByte(uint8_t data)
{
	st7789_spiWriteData(&data, 1);
}
void st7789_spiWriteDataWord(uint16_t data)
{
	uint8_t _data[2] = {(data >> 8) & 0xFF, data & 0xFF};
	st7789_spiWriteData(_data, 2);
}
void st7789_spiWriteAddress(uint16_t address1, uint16_t address2)
{
	uint8_t _data[4] = {(address1 >> 8) & 0xFF, address1 & 0xFF, (address2 >> 8) & 0xFF, address2 & 0xFF};
	st7789_spiWriteData(_data, 4);
}
void st7789_spiWriteColor(uint16_t color, uint16_t size)
{
	uint8_t _data[1024];
	uint16_t index = 0;
	for (uint16_t i = 0; i != size; i++)
	{
		_data[index++] = (color >> 8) & 0xFF;
		_data[index++] = color & 0xFF;
	}
	st7789_spiWriteData(_data, size * 2);
}
void st7789_spiWriteColors(uint16_t *colors, uint16_t size)
{
	uint8_t _data[1024];
	uint16_t index = 0;
	for (uint16_t i = 0; i < size; i++)
	{
		_data[index++] = (colors[i] >> 8) & 0xFF;
		_data[index++] = colors[i] & 0xFF;
	}
	st7789_spiWriteData(_data, size * 2);
}
void st7789_delayMS(int ms)
{
	int64_t timeMCS = esp_timer_get_time() + (1000 * ms);
	while (timeMCS > esp_timer_get_time())
	{
	}
}
uint16_t st7789_convertRGB565(uint16_t r, uint16_t g, uint16_t b)
{
	return (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

//======================================================COMMAND======================================================
void st7789_displayOff()
{
	st7789_spiWriteCommand(0x28);
}
void st7789_displayOn()
{
	st7789_spiWriteCommand(0x29);
}
void st7789_inversionOff()
{
	st7789_spiWriteCommand(0x20);
}
void st7789_inversionOn()
{
	st7789_spiWriteCommand(0x21);
}
void st7789_sleepOff()
{
	st7789_spiWriteCommand(0x11);
}
void st7789_sleepOn()
{
	st7789_spiWriteCommand(0x10);
}
void st7789_partialOff() // NORMAL MODE
{
	st7789_spiWriteCommand(0x13);
}
void st7789_partialOn() // PARTIAL MODE
{
	st7789_spiWriteCommand(0x12);
}
void st7789_idleOff()
{
	st7789_spiWriteCommand(0x38);
}
void st7789_idleOn()
{
	st7789_spiWriteCommand(0x39);
}
void st7789_setColumn(uint16_t x1, uint16_t x2)
{
	st7789_spiWriteCommand(0x2A);
	st7789_spiWriteAddress(x1, x2);
}
void st7789_setRow(uint16_t y1, uint16_t y2)
{
	st7789_spiWriteCommand(0x2B);
	st7789_spiWriteAddress(y1, y2);
}
void st7789_setActiveArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	st7789_setColumn(x1, x2);
	st7789_setRow(y1, y2);
}
void st7789_memoryWrite()
{
	st7789_spiWriteCommand(0x2C);
}
void st7789_backlightOff()
{
	if (dev->pinBL >= 0)
	{
		gpio_set_level(dev->pinBL, 0);
	}
}
void st7789_backlightOn()
{
	if (dev->pinBL >= 0)
	{
		gpio_set_level(dev->pinBL, 1);
	}
}
//======================================================FUNCTIONS======================================================

void st7789_select(st7789_t *st7789)
{
	dev = st7789;
}
void st7789_init(st7789_config_t *config)
{
	memset(&transactionSPI, 0, sizeof(transactionSPI));

	spi_device_interface_config_t interfaceSPI = {
		.address_bits = 0,
		.command_bits = 0,
		.cs_ena_posttrans = 0,
		.cs_ena_pretrans = 0,
		.dummy_bits = 0,
		.duty_cycle_pos = 0,
		.input_delay_ns = 0,
		.post_cb = 0,
		.pre_cb = 0,
		.clock_speed_hz = config->speedSPI,
		.queue_size = 7,
		.mode = 2,
		.flags = SPI_DEVICE_NO_DUMMY};

	if (config->pinCS >= 0)
	{
		gpio_pad_select_gpio(config->pinCS);
		// gpio_reset_pin(config->pinCS);
		gpio_set_direction(config->pinCS, GPIO_MODE_OUTPUT);
		gpio_set_level(config->pinCS, 1);
		interfaceSPI.spics_io_num = config->pinCS;
	}
	else
	{
		interfaceSPI.spics_io_num = -1;
	}

	gpio_pad_select_gpio(config->pinDC);
	// gpio_reset_pin(config->pinDC);
	gpio_set_direction(config->pinDC, GPIO_MODE_OUTPUT);
	gpio_set_level(config->pinDC, 0);
	dev->pinDC = config->pinDC;

	if (config->pinRST >= 0)
	{
		gpio_pad_select_gpio(config->pinRST);
		// gpio_reset_pin(config->pinRST);
		gpio_set_direction(config->pinRST, GPIO_MODE_OUTPUT);
		gpio_set_level(config->pinRST, 1);
		st7789_delayMS(50);
		gpio_set_level(config->pinRST, 0);
		st7789_delayMS(50);
		gpio_set_level(config->pinRST, 1);
		st7789_delayMS(50);
	}

	if (config->pinBL >= 0)
	{
		gpio_pad_select_gpio(config->pinBL);
		// gpio_reset_pin(config->pinBL);
		gpio_set_direction(config->pinBL, GPIO_MODE_OUTPUT);
		gpio_set_level(config->pinBL, 0);
		dev->pinBL = config->pinBL;
	}
	else
	{
		dev->pinBL = -1;
	}

	spi_bus_add_device(config->hostSPI, &interfaceSPI, &dev->handleSPI);

	dev->width = config->width;
	dev->height = config->height;
	dev->offsetX = config->offsetX;
	dev->offsetY = config->offsetY;

	st7789_spiWriteCommand(0x01); // Software Reset
	st7789_delayMS(150);

	st7789_sleepOff();
	st7789_delayMS(255);

	st7789_spiWriteCommand(0x3A); // Interface Pixel Format
	st7789_spiWriteDataByte(0x55);
	st7789_delayMS(10);

	st7789_spiWriteCommand(0x36); // Memory Data Access Control
	st7789_spiWriteDataByte(0x00);

	st7789_setActiveArea(0, 0, dev->width - 1, dev->height - 1);

	st7789_inversionOn();
	st7789_delayMS(10);

	st7789_partialOff();
	st7789_delayMS(10);

	st7789_memoryWrite();
	for (uint16_t y = 0; y < dev->height; y++)
	{
		st7789_spiWriteColor(BLACK, dev->width);
	}

	st7789_displayOn();
	st7789_delayMS(255);

	if (dev->pinBL != -1)
	{
		gpio_set_level(dev->pinBL, 1);
	}
}
