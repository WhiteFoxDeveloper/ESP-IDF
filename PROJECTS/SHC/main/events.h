#ifndef MAIN_EVENTS_H_
#define MAIN_EVENTS_H_

#include "header.h"

//===================================================
#define bool _Bool
#define countSecondDay 86400
#define MASK_TRUE 1
#define MASK_FALSE 254
#define MASK_INVERSION 2
#define MASK_SIGN 224
#define MASK_COMPARE 16

typedef enum
{
    SIGN_EQUAL = 0,     // 000 | =
    SIGN_MORE = 1,      // 001 | >
    SIGN_LESS = 2,      // 010 | <
    SIGN_EQUALMORE = 3, // 011 | <=
    SIGN_EQUALLESS = 4  // 100 | >=
} signEvents_t;
typedef enum
{
    // TRIGGERS:
    TYPE_TRIGGER_TIME,
    TYPE_TRIGGER_PERIOD,
    TYPE_TRIGGER_SIGNAL,
    // ACTIONS:
    TYPE_ACTION_KEY,
    // MAIN:
    TYPE_SCRIPT,
    // SENSORS:
    TYPE_SENSOR_ANALOG,
    // DATA:
    TYPE_MAX,
    TYPE_SCRIPT_MAX = 4
} typeEvents_t;
typedef enum
{
    WEEK_MONDAY = 1,    // 1 << 0 | monday
    WEEK_TOESDAY = 2,   // 1 << 1 | toesday
    WEEK_WEDNESDAY = 4, // 1 << 2 | wednesday
    WEEK_THURSDAY = 8,  // 1 << 3 | thursday
    WEEK_FRIDAY = 16,   // 1 << 4 | friday
    WEEK_SATURDAY = 32, // 1 << 5 | saturday
    WEEK_SUNDAY = 64    // 1 << 6 | sunday
} weekEvents_t;

typedef struct
{
    uint8_t config;
    uint32_t begin;
    uint32_t active;
    uint8_t week;
    char name[NAME_CHAR_MAX];
} triggerTime_t;
typedef struct
{
    uint8_t config;
    uint32_t begin;
    uint32_t active;
    uint32_t period;
    uint8_t week;
    char name[NAME_CHAR_MAX];
} triggerPeriod_t;
typedef struct
{
    uint8_t config;
    uint16_t index;
    uint32_t data;
    char name[NAME_CHAR_MAX];
} triggerSignal_t;
typedef struct
{
    uint8_t config;
    uint8_t lenIndexList[4];
    uint8_t indexList[SCRIPTS_INDEX_MAX];
    char name[NAME_CHAR_MAX];
} script_t;
typedef struct
{
    uint8_t config;
    char name[NAME_CHAR_MAX];
} action_t;
typedef struct
{
    uint8_t config;
    uint32_t data;
    char name[NAME_CHAR_MAX];
} sensor_t;

typedef struct
{
    uint16_t lenTrigTime;
    uint16_t lenTrigPeriod;
    uint16_t lenTrigSignal;
    uint16_t lenActKey;
    uint16_t lenScripts;
    uint16_t lenSenAnalog;
    triggerTime_t trigTimeList[TRIGGERS_TIME_MAX];
    triggerPeriod_t trigPeriodList[TRIGGERS_PERIOD_MAX];
    triggerSignal_t trigSignalList[TRIGGERS_SIGNAL_MAX];
    script_t scriptsList[SCRIPTS_MAX];
    action_t actKeyList[ACTIONS_MAX];
    sensor_t senAnalogList[SENSORS_MAX];
} events_t;

uint16_t events_getScriptTypeIndex(uint16_t indexScript, typeEvents_t type);

void events_deleteScriptIndex(uint16_t indexScript, uint8_t *indexList, uint8_t lengthList, typeEvents_t type);
void events_newScriptIndex(uint16_t indexScript, uint8_t *indexList, uint8_t lengthList, typeEvents_t type);
void events_writeScriptIndex(uint16_t indexScript, uint8_t *indexList, uint8_t lengthList, typeEvents_t type);

void events_init();
void callEvents();

void events_delete(uint16_t index, typeEvents_t type);
void events_new(typeEvents_t type);
void events_edit(uint16_t index, typeEvents_t type);

uint32_t events_callTriggersTime();
uint32_t events_callTriggersPeriod();
void events_callTriggersSignal();
void events_callScripts();

#endif