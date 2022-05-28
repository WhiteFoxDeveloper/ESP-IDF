#include <string.h>
#include <time.h>

#include "ds3231.h"

static uint8_t convertToTime(uint8_t val)
{
    return ((val >> 4) * 10) + (val & 0x0f);
}
static uint8_t convertToTimer(uint8_t val)
{
    return ((val / 10) << 4) + (val % 10);
}

static void writeData(i2c_port_t port, uint8_t reg, const uint8_t *data, uint8_t sizeData)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 208, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write(cmd, data, sizeData, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(port, cmd, 1000 / portTICK_RATE_MS); //"(# / portTICK_RATE_MS)"=maximum wait time. This task will be blocked until all the commands have been sent (not thread-safe - if you want to use one I2C port in different tasks you need to take care of multi-thread issues)
    i2c_cmd_link_delete(cmd);
}
static void readData(i2c_port_t port, uint8_t reg, uint8_t *data, uint8_t sizeData)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 208, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 209, true);
    i2c_master_read(cmd, data, sizeData, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(port, cmd, 1000 / portTICK_RATE_MS); //"(# / portTICK_RATE_MS)"=maximum wait time. This task will be blocked until all the commands have been sent (not thread-safe - if you want to use one I2C port in different tasks you need to take care of multi-thread issues)
    i2c_cmd_link_delete(cmd);
}

void ds3231_init(ds3231_t *ds3231, ds3231_config_t *config)
{
    ds3231->port = config->port;
    ds3231->shiftYear = config->shiftYear;
    uint8_t data = config->interruptAlarm | config->frequencyHz;
    writeData(ds3231->port, DS3231_ADDR_CONTROL, &data, 1);
}

void ds3231_setTime(ds3231_t *ds3231, timeDate_t *timeDate, bool formatHour12) // DS3231_ADDR_TIME 7byte
{
    uint8_t year = timeDate->year; // % 4;
    // ds3231->shiftYear = timeDate->year - year;
    uint8_t data[7] = {
        convertToTimer(timeDate->second),
        convertToTimer(timeDate->minute),
        convertToTimer(timeDate->hour),
        convertToTimer(timeDate->weekDay + 1),
        convertToTimer(timeDate->day + 1),
        convertToTimer(timeDate->mouth + 1),
        convertToTimer(year)};
    data[2] |= formatHour12 ? 64 : 0;
    writeData(ds3231->port, DS3231_ADDR_TIME, data, 7);
}
void ds3231_getTime(ds3231_t *ds3231, timeDate_t *timeDate) // DS3231_ADDR_TIME 7byte
{
    uint8_t data[7];
    readData(ds3231->port, DS3231_ADDR_TIME, data, 7);
    timeDate->second = convertToTime(data[0]);
    timeDate->minute = convertToTime(data[1]);
    timeDate->hour = convertToTime(data[2] & 63);
    timeDate->weekDay = convertToTime(data[3]) - 1;
    timeDate->day = convertToTime(data[4]) - 1;
    timeDate->mouth = convertToTime(data[5]) - 1;
    timeDate->year = convertToTime(data[6]) + ds3231->shiftYear;
}

void ds3231_setAlarm1(ds3231_t *ds3231, timeAlarm1_t *timeAlarm, bool formatHour12, bool formatDayWeek) // DS3231_ADDR_ALARM1 4byte
{
    uint8_t data[4] = {
        convertToTimer(timeAlarm->second),
        convertToTimer(timeAlarm->minute),
        convertToTimer(timeAlarm->hour),
        convertToTimer(timeAlarm->day)};
    data[2] |= formatHour12 ? 64 : 0;
    data[3] |= formatDayWeek ? 64 : 0;
    writeData(ds3231->port, DS3231_ADDR_ALARM1, data, 4);
}
void ds3231_getAlarm1(ds3231_t *ds3231, timeAlarm1_t *timeAlarm) // DS3231_ADDR_ALARM1 4byte
{
    uint8_t data[4];
    readData(ds3231->port, DS3231_ADDR_ALARM1, data, 7);
    timeAlarm->second = convertToTime(data[0]);
    timeAlarm->minute = convertToTime(data[1]);
    timeAlarm->hour = convertToTime(data[2] & 63);
    timeAlarm->day = convertToTime(data[3] & 63);
}

void ds3231_setAlarm2(ds3231_t *ds3231, timeAlarm2_t *timeAlarm, bool formatHour12, bool formatDayWeek) // DS3231_ADDR_ALARM2 3byte
{
    uint8_t data[3] = {
        convertToTimer(timeAlarm->minute),
        convertToTimer(timeAlarm->hour),
        convertToTimer(timeAlarm->day)};
    data[1] |= formatHour12 ? 64 : 0;
    data[2] |= formatDayWeek ? 64 : 0;
    writeData(ds3231->port, DS3231_ADDR_ALARM2, data, 4);
}
void ds3231_getAlarm2(ds3231_t *ds3231, timeAlarm2_t *timeAlarm) // DS3231_ADDR_ALARM2 3byte
{
    uint8_t data[3];
    readData(ds3231->port, DS3231_ADDR_ALARM2, data, 7);
    timeAlarm->minute = convertToTime(data[0]);
    timeAlarm->hour = convertToTime(data[1] & 63);
    timeAlarm->day = convertToTime(data[2] & 63);
}

// esp_err_t ds3231_get_raw_temp(i2c_dev_t *dev, int16_t *temp)
// {
//     uint8_t data[2];

//     esp_err_t res = i2c_dev_read_reg(dev, DS3231_ADDR_TEMP, data, sizeof(data));
//     if (res == ESP_OK)
//         *temp = (int16_t)(int8_t)data[0] << 2 | data[1] >> 6;

//     return res;
// }

// esp_err_t ds3231_get_temp_integer(i2c_dev_t *dev, int8_t *temp)
// {
//     CHECK_ARG(temp);

//     int16_t t_int;

//     esp_err_t res = ds3231_get_raw_temp(dev, &t_int);
//     if (res == ESP_OK)
//         *temp = t_int >> 2;

//     return res;
// }

// esp_err_t ds3231_get_temp_float(i2c_dev_t *dev, float *temp)
// {
//     CHECK_ARG(temp);

//     int16_t t_int;

//     esp_err_t res = ds3231_get_raw_temp(dev, &t_int);
//     if (res == ESP_OK)
//         *temp = t_int * 0.25;

//     return res;
// }