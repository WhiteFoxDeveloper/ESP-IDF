#include <stdio.h>
#include "mcp23s17.h"

static void delayMCS(int64_t countMCS)
{
	int64_t time = esp_timer_get_time();
	time += countMCS;
	while (time > esp_timer_get_time())
	{
	};
}

static void writeUINT16(mcp23s17_t *mcp23s17, uint8_t reg, uint16_t data)
{
	uint8_t tx[4] = {mcp23s17->address, reg, (uint8_t)data, (uint8_t)(data >> 8)};
	mcp23s17->configTransactionSPI.tx_buffer = tx;
	mcp23s17->configTransactionSPI.length = 32;
	spi_device_transmit(mcp23s17->configHandleSPI, &mcp23s17->configTransactionSPI);
}

static void writeUINT8(mcp23s17_t *mcp23s17, uint8_t reg, uint8_t data)
{
	uint8_t tx[3] = {mcp23s17->address, reg, data};
	mcp23s17->configTransactionSPI.tx_buffer = tx;
	mcp23s17->configTransactionSPI.length = 24;
	spi_device_transmit(mcp23s17->configHandleSPI, &mcp23s17->configTransactionSPI);
}

static uint16_t readUINT16(mcp23s17_t *mcp23s17, uint8_t reg)
{
	uint8_t tx[4] = {(mcp23s17->address + 1), reg, 0, 0};
	uint8_t rx[4];
	mcp23s17->configTransactionSPI.tx_buffer = tx;
	mcp23s17->configTransactionSPI.rx_buffer = rx;
	mcp23s17->configTransactionSPI.length = 32;
	spi_device_transmit(mcp23s17->configHandleSPI, &mcp23s17->configTransactionSPI);
	return ((rx[3] << 8) | rx[2]);
}

void mcp23s17_init(mcp23s17_t *mcp23s17, mcp23s17_config_t *config)
{
	mcp23s17->address = config->address;

	spi_device_interface_config_t configSPI = {
		.address_bits = 0,
		.command_bits = 0,
		.dummy_bits = 0,
		.mode = 0,
		.duty_cycle_pos = 0,
		.cs_ena_posttrans = 0,
		.cs_ena_pretrans = 0,
		.clock_speed_hz = config->speedClockSPI_hz,
		.spics_io_num = config->pinCS,
		.flags = 0,
		.queue_size = 7,
		.pre_cb = NULL,
		.post_cb = NULL};
	spi_bus_add_device(config->host, &configSPI, &mcp23s17->configHandleSPI);
	mcp23s17->configTransactionSPI.flags = 0;
	mcp23s17->configTransactionSPI.addr = 0;
	mcp23s17->configTransactionSPI.cmd = 0;
	gpio_pad_select_gpio(config->pinCS);
	gpio_pad_select_gpio(config->pinRST);
	gpio_config_t io_conf = {
		.intr_type = GPIO_INTR_DISABLE,
		.pin_bit_mask = 1ULL << config->pinRST,
		.mode = GPIO_MODE_OUTPUT,
		.pull_down_en = 0,
		.pull_up_en = 0};
	gpio_config(&io_conf);
	gpio_set_level(config->pinRST, 1);
	delayMCS(10);
	gpio_set_level(config->pinRST, 0);
	delayMCS(10);
	gpio_set_level(config->pinRST, 1);

	writeUINT8(mcp23s17, IOCON, 0x08);
}

void mcp23s17_setGpioMode(mcp23s17_t *mcp23s17, uint16_t data)
{
	writeUINT16(mcp23s17, IODIRA, data);
}

void mcp23s17_setPullupMode(mcp23s17_t *mcp23s17, uint16_t data)
{
	writeUINT16(mcp23s17, GPPUA, data);
}

void mcp23s17_setInvertMode(mcp23s17_t *mcp23s17, uint16_t data)
{
	writeUINT16(mcp23s17, IPOLA, data);
}

void mcp23s17_writePorts(mcp23s17_t *mcp23s17, uint16_t data)
{
	writeUINT16(mcp23s17, GPIOA, data);
}

uint16_t mcp23s17_readPorts(mcp23s17_t *mcp23s17)
{
	return readUINT16(mcp23s17, GPIOA);
}

// void mcp23S17_SetPin(uint8_t address, uint8_t pin)
// {
// 	_outputCache |= (1 << pin);
// 	mcp23S17_WriteWord(address, GPIOA, _outputCache);
// }
// //*********************************************************************
// // mcp23S17_ClrPin
// //*********************************************************************
// void mcp23S17_ClrPin(uint8_t address, uint8_t pin)
// {
// 	_outputCache &= ~(1 << pin);
// 	mcp23S17_WriteWord(address, GPIOA, _outputCache);
// }
