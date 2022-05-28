#ifndef MAIN_DS3231_H_
#define MAIN_DS3231_H_

#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include "driver/i2c.h"

#define DS3231_ADDR 208 //!< I2C address

#define DS3231_ADDR_TIME 0x00
#define DS3231_ADDR_ALARM1 0x07
#define DS3231_ADDR_ALARM2 0x0b
#define DS3231_ADDR_CONTROL 0x0e
#define DS3231_ADDR_STATUS 0x0f
#define DS3231_ADDR_AGING 0x10
#define DS3231_ADDR_TEMP 0x11

typedef enum
{
    HZ1 = 0,
    kHZ1024 = 8,
    kHZ4096 = 10,
    kHZ8192 = 18
} ds3231_frequencyHZ;

typedef enum
{
    INTERRUPT_OFF = 0,
    INTERRUPT_ALARM1 = 5,
    INTERRUPT_ALARM2 = 6,
    INTERRUPT_ON = 7
} ds3231_interruptAlarm;

typedef struct
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t weekDay;
    uint8_t day;
    uint8_t mouth;
    uint16_t year;
} timeDate_t;

typedef struct
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
} timeAlarm1_t;

typedef struct
{
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
} timeAlarm2_t;

typedef struct
{
    uint16_t shiftYear;
    i2c_port_t port;
} ds3231_t;

typedef struct
{
    // bool oscillator;
    // bool batteryWave;
    // bool convertTemp;
    // bool interruptFlag
    i2c_port_t port;
    ds3231_frequencyHZ frequencyHz;
    ds3231_interruptAlarm interruptAlarm;
    uint16_t shiftYear;
} ds3231_config_t;

void ds3231_init(ds3231_t *ds3231, ds3231_config_t *config);

void ds3231_setTime(ds3231_t *ds3231, timeDate_t *timeDate, bool formatHour12); // DS3231_ADDR_TIME 7byte
void ds3231_getTime(ds3231_t *ds3231, timeDate_t *timeDate);                    // DS3231_ADDR_TIME 7byte

void ds3231_setAlarm1(ds3231_t *ds3231, timeAlarm1_t *timeAlarm, bool formatHour12, bool formatDayWeek); // DS3231_ADDR_ALARM1 4byte
void ds3231_getAlarm1(ds3231_t *ds3231, timeAlarm1_t *timeAlarm);                                        // DS3231_ADDR_ALARM1 4byte

void ds3231_setAlarm2(ds3231_t *ds3231, timeAlarm2_t *timeAlarm, bool formatHour12, bool formatDayWeek); // DS3231_ADDR_ALARM2 3byte
void ds3231_getAlarm2(ds3231_t *ds3231, timeAlarm2_t *timeAlarm);                                        // DS3231_ADDR_ALARM2 3byte

// void ds3231_getRawTemp(ds3231_t *ds3231, int16_t *temp);
// void ds3231_getTempInteger(ds3231_t *ds3231, int8_t *temp);
// void ds3231_getTempFloat(ds3231_t *ds3231, float *temp);
#endif