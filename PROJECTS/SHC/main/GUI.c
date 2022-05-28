#include "GUI.h"
#include "fontx.h"
#include "encoder.h"
#include "matrix.h"       //
#include "matrixGUI.h"    //
#include "st7789.h"       //
#include "st7789Matrix.h" //
#include "ds3231.h"
#include "events.h"
#include "esp_log.h"
#include "driver/timer.h"
#include "header.h"

typedef enum
{
    MENU,

    MENU_TRIGGERS,
    MENU_SCRIPTS,
    MENU_KEYS,
    MENU_SENSORS,
    MENU_SETTINGS,

    MENU_TRIGGERS_TIME,
    MENU_TRIGGERS_PERIOD,
    MENU_TRIGGERS_SIGNAL,

    MENU_SCRIPTS_SET,

    MENU_SETTINGS_LANGUAGE,
    MENU_SETTINGS_TIME,
    MENU_SETTINGS_THEME,

    MENU_SCRIPTS_SET_TRIGGERS,
    MENU_SCRIPTS_SET_ACTIONS,

    MENU_TRIGGERS_TIME_SET,
    MENU_TRIGGERS_PERIOD_SET,
    MENU_TRIGGERS_SIGNAL_SET,

    MENU_SCRIPTS_SET_TRIGGERS_TIME,
    MENU_SCRIPTS_SET_TRIGGERS_PERIOD,
    MENU_SCRIPTS_SET_TRIGGERS_SIGNAL
} fScreen_t;
typedef enum
{
    SCROLL,
    CHANGE,
    CHANGE_CHAR
} fMode_t;
typedef enum
{
    PRESS = 1,
    LEFT,
    RIGHT,
    PRESS_LEFT,
    PRESS_RIGHT,
} fCommand_t;

// COLORS:
static uint8_t colorBackground[2] = {(COLOR_BACKGROUND >> 8) & 0xFF, COLOR_BACKGROUND & 0xFF};
static uint8_t colorText[2] = {(COLOR_TEXT >> 8) & 0xFF, COLOR_TEXT & 0xFF};
static uint8_t colorFrame[2] = {(COLOR_FRAME >> 8) & 0xFF, COLOR_FRAME & 0xFF};
static uint8_t colorSign[2] = {(COLOR_SIGN >> 8) & 0xFF, COLOR_SIGN & 0xFF};
static uint8_t colorFalse[2] = {(COLOR_FALSE >> 8) & 0xFF, COLOR_FALSE & 0xFF};
static uint8_t colorTrue[2] = {(COLOR_TRUE >> 8) & 0xFF, COLOR_TRUE & 0xFF};
static uint8_t *colorCells[2] = {colorFalse, colorTrue};

// GENERAL VARIABLES:
static fontx_t EU_RU_32p;
static matrixFont_t matrixFont = {
    .color = colorText,
    .direction = 0,
    .fill = 0,
    .font = &EU_RU_32p,
    .underline = 0};
extern encoder_t encoder;
extern ds3231_t ds3231;
extern matrix_t matrix;
extern st7789_t st7789;
extern st7789Matrix_t st7789Matrix;
extern events_t events;
extern uint32_t eventTime;
extern timeDate_t timeDate;

// PATTERNS:
char maxValue[] = "40950000";
char minValue[] = "00000000";
char *signs[] = {">", "<", ">=", "<=", "="};
char formTimer[] = "23:59:59";
char *cutDaysWeeks[] = {"пн", "вт", "ср", "чт", "пт", "сб", "вс"};
char *daysWeeks[] = {"Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресение"};
patternTimerEditGUI_t patternTimerEdit = {GUI_TIMEREDIT, 2, formTimer, colorBackground, colorSign, &matrixFont};
patternChoiceLineGUI_t patternChoiceLine = {GUI_CHOICELINE, 30, 30, 6, colorCells, colorFrame, colorSign, colorBackground, &matrixFont};
patternTextGUI_t patternText = {GUI_TEXT, colorBackground, &matrixFont};
patternSliderGUI_t patternSlider = {GUI_SLIDER, colorBackground, colorSign, colorTrue, colorFalse};
patternChoiceGUI_t patternChoice = {GUI_CHOICE, colorBackground, colorSign, &matrixFont};
patternScrollerGUI_t patternScroller = {GUI_SCROLLER, colorBackground, colorSign, colorTrue, colorFalse};
patternListGUI_t patternList = {GUI_LIST, colorBackground, colorSign};
patternGroupGUI_t patternGroup = {GUI_GROUP};
patternSwitcherGUI_t patternSwitcher = {GUI_SWITCHER};
patternValueEditGUI_t patternValueEdit = {GUI_VALUEEDIT, colorBackground, colorSign, &matrixFont};
patternChoiceStatusGUI_t patternChoiceStatus = {GUI_CHOICESTATUS, 6, colorCells, 2, colorBackground, &matrixFont};
patternTextEditGUI_t patternTextEdit = {
    ._type = GUI_TEXTEDIT,
    .colorBackground = colorBackground,
    .colorSign = colorFalse,
    .matrixFont = &matrixFont};

// ELEMENTS DATA:
static char *tHeader[] = {
    [MENU] = "Меню:",
    [MENU_TRIGGERS] = "Триггеры:",
    [MENU_SCRIPTS] = "Инструкции:",
    [MENU_KEYS] = "Ключи:",
    [MENU_SENSORS] = "Датчики:",
    [MENU_SETTINGS] = "Настройки:",
    [MENU_TRIGGERS_TIME] = "Тактовые:",
    [MENU_TRIGGERS_PERIOD] = "Цикличные:",
    [MENU_TRIGGERS_SIGNAL] = "Сигнальные:",
    [MENU_SCRIPTS_SET] = "Создание:",
    [MENU_SETTINGS_LANGUAGE] = "Язык:",
    [MENU_SETTINGS_THEME] = "Оформление:",
    [MENU_SETTINGS_TIME] = "Время:",
    [MENU_SCRIPTS_SET_TRIGGERS] = "Триггеры:",
    [MENU_SCRIPTS_SET_ACTIONS] = "Действия:",
    [MENU_TRIGGERS_TIME_SET] = "Создание:",
    [MENU_TRIGGERS_PERIOD_SET] = "Создание:",
    [MENU_TRIGGERS_SIGNAL_SET] = "Создание:",
    [MENU_SCRIPTS_SET_TRIGGERS_TIME] = "Тактовые:",
    [MENU_SCRIPTS_SET_TRIGGERS_PERIOD] = "Цикличные:",
    [MENU_SCRIPTS_SET_TRIGGERS_SIGNAL] = "Сигнальные:"};
static char *tMenu[] = {"Триггеры", "Инструкции", "Ключи", "Датчики", "Настройки"};
static char *tTriggers[] = {"Назад", "Тактовые", "Цикличные", "Сигнальные"};
static char *tScripts[] = {"Назад", "Создать"}; // + SCRIPTS
static char *tKeys[] = {"Назад"};               // + KEYS
static char *tSensors[] = {"Назад"};            // + SENSORS
static char *tSettings[] = {"Назад", "Язык", "Время", "Оформление"};

static char *tTriggersType[] = {"Назад", "Создать"}; // + TRIGGERS
static char *tTriggersSetTime[] = {"Назад", "Удалить", "Отсчет с", "Активность", "Дни недели", "Инверсия"};
static char *tTriggersSetPeriod[] = {"Назад", "Удалить", "Отсчет с", "Активность", "Период", "Дни недели", "Инверсия"};
static char *tTriggersSetSignal[] = {"Назад", "Удалить", "Датчик", "Сравнение", "Значение"};

static char *tScriptsSet[] = {"Назад", "Удалить", "Триггеры", "Действия"};
static char *tScriptsSetTriggersBegin[] = {"Назад"};

static char *tSettingsTime[] = {"Назад", "Время", "День недели"};

uint16_t levelScreen = 0;
fScreen_t indexScreen = MENU;
fMode_t mode = SCROLL;

void GUI_free(void *element)
{
    typeLineGUI_t type = **((typeLineGUI_t **)element);
    switch (type)
    {
    case GUI_TEXT:
        // free(((textGUI_t *)element)->_preText);
        free(element);
        break;
    case GUI_TEXTEDIT:
        // free(((textEditGUI_t *)element)->_code);
        free(element);
        break;
    case GUI_VALUEEDIT:
        free(element);
        break;
    case GUI_TIMEREDIT:
        free(element);
        break;
    case GUI_SLIDER:
        free(element);
        break;
    case GUI_CHOICE:
        free(element);
        break;
    case GUI_CHOICELINE:
        free(element);
        break;
    case GUI_CHOICESTATUS:
        free(element);
        break;
    case GUI_SCROLLER:
        free(element);
        break;
    case GUI_GROUP:
        for (uint16_t index = 0; index < ((groupGUI_t *)element)->countElements; index++)
        {
            GUI_free(((groupGUI_t *)element)->elements[index]);
        }
        free(element);
        break;
    case GUI_LIST:
        for (uint16_t index = 0; index < ((listGUI_t *)element)->countElements; index++)
        {
            GUI_free(((listGUI_t *)element)->elements[index]);
        }
        free(element);
        break;
    case GUI_SWITCHER:
        for (uint16_t index = 0; index < ((switcherGUI_t *)element)->countElements; index++)
        {
            GUI_free(((switcherGUI_t *)element)->elements[index]);
        }
        free(element);
        break;
    case GUI_CONNECTION:
        for (uint16_t index = 0; index < ((connectionGUI_t *)element)->countElements; index++)
        {
            GUI_free(((connectionGUI_t *)element)->elements[index]);
        }
        free(element);
        break;
    default:
        break;
    }
}
char *GUI_getTimerText(uint32_t time)
{
    char *text = malloc(9);
    uint8_t seconds = time % 60;
    uint8_t minutes = (time / 60) % 60;
    uint8_t hours = (time / 3600) % 24;
    text[0] = hours / 10 + '0';
    text[1] = hours % 10 + '0';
    text[2] = ':';
    text[3] = minutes / 10 + '0';
    text[4] = minutes % 10 + '0';
    text[5] = ':';
    text[6] = seconds / 10 + '0';
    text[7] = seconds % 10 + '0';
    text[8] = '\0';
    return text;
}
uint32_t GUI_getTimerTime(char *text)
{
    uint8_t seconds = (text[6] - '0') * 10 + (text[7] - '0');
    uint8_t minutes = (text[3] - '0') * 10 + (text[4] - '0');
    uint8_t hours = (text[0] - '0') * 10 + (text[1] - '0');
    return (hours * 60 + minutes) * 60 + seconds;
}

//==========================================================================================================
groupGUI_t *screens[5];
uint16_t levelScreen;
fScreen_t indexScreen;
void *editElement;

//==========================================BUILD==========================================

#define listX 16
#define listY 64
#define headerX 16
#define headerY 16

void GUI_build_MENU()
{
    levelScreen = 0;
    indexScreen = MENU;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU], headerX, headerY);
    listGUI_t *list = matrixGUI_newList(&patternList, malloc(5 * sizeof(void *)), 5, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    for (uint16_t i = 0; i < 5; i++)
    {
        list->elements[i] = matrixGUI_newText(&patternText, tMenu[i], 0, 0);
    }
    matrixGUI_build(screens[levelScreen]); //////
    st7789Matrix_write();                  //////
}

void GUI_build_MENU_TRIGGERS()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    levelScreen = 1;
    indexScreen = MENU_TRIGGERS;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_TRIGGERS], headerX, headerY);
    listGUI_t *list = matrixGUI_newList(&patternList, malloc(4 * sizeof(void *)), 4, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    for (uint16_t i = 0; i < 4; i++)
    {
        list->elements[i] = matrixGUI_newText(&patternText, tTriggers[i], 0, 0);
    }
}
void GUI_build_MENU_TRIGGERS_TIME()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    levelScreen = 2;
    indexScreen = MENU_TRIGGERS_TIME;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_TRIGGERS_TIME], headerX, headerY);
    uint16_t countElements = 2 + events.lenTrigTime;
    listGUI_t *list = matrixGUI_newList(&patternList, malloc(countElements * sizeof(void *)), countElements, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    uint16_t i = 0;
    while (i < 2)
    {
        list->elements[i] = matrixGUI_newText(&patternText, tTriggersType[i], 0, 0);
        i++;
    }
    while (i < countElements)
    {
        list->elements[i] = matrixGUI_newText(&patternText, events.trigTimeList[i - 2].name, 0, 0);
        i++;
    }
}
void GUI_build_MENU_TRIGGERS_TIME_SET()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    listGUI_t *list = screens[levelScreen]->elements[1];
    if (list->index == 1)
    {
        events_new(TYPE_TRIGGER_TIME);
        list->index = list->countElements;
        ++list->countElements;
        list->elements = realloc(list->elements, list->countElements * sizeof(void *));
        list->elements[list->index] = matrixGUI_newText(&patternText, events.trigTimeList[events.lenTrigTime - 1].name, 0, 0);
    }
    triggerTime_t *trigger = &events.trigTimeList[list->index - 2];
    switcherGUI_t *switcher;
    char *str;
    levelScreen = 3;
    indexScreen = MENU_TRIGGERS_TIME_SET;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_TRIGGERS_TIME_SET], headerX, headerY);
    list = matrixGUI_newList(&patternList, malloc(6 * sizeof(void *)), 6, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    // back and save:
    list->elements[0] = matrixGUI_newText(&patternText, tTriggersSetTime[0], 0, 0);
    // delete:
    list->elements[1] = matrixGUI_newText(&patternText, tTriggersSetTime[1], 0, 0);
    // timer begin:
    switcher = matrixGUI_newSwitcher(&patternSwitcher, malloc(2 * sizeof(void *)), 2, 0, 0, 0);
    switcher->elements[0] = matrixGUI_newText(&patternText, tTriggersSetTime[2], 0, 0);
    str = GUI_getTimerText(trigger->begin);
    switcher->elements[1] = matrixGUI_newTimerEdit(&patternTimerEdit, str, 1, 0, 0, 0);
    list->elements[2] = switcher;
    // timer active:
    switcher = matrixGUI_newSwitcher(&patternSwitcher, malloc(2 * sizeof(void *)), 2, 0, 0, 0);
    switcher->elements[0] = matrixGUI_newText(&patternText, tTriggersSetTime[3], 0, 0);
    str = GUI_getTimerText(trigger->active);
    switcher->elements[1] = matrixGUI_newTimerEdit(&patternTimerEdit, str, 1, 0, 0, 0);
    list->elements[3] = switcher;
    // days weeks:
    switcher = matrixGUI_newSwitcher(&patternSwitcher, malloc(2 * sizeof(void *)), 2, 0, 0, 0);
    switcher->elements[0] = matrixGUI_newText(&patternText, tTriggersSetTime[4], 0, 0);
    switcher->elements[1] = matrixGUI_newChoiceLine(&patternChoiceLine, cutDaysWeeks, 7, malloc(7), 2, 1, 0, 0, 0);
    for (uint16_t i = 0; i < 7; i++)
        ((choiceLineGUI_t *)switcher->elements[1])->positions[i] = (trigger->week & (1 << i)) != 0;
    list->elements[4] = switcher;
    // inversion:
    list->elements[5] = matrixGUI_newChoiceStatus(&patternChoiceStatus, tTriggersSetTime[5], (trigger->config >> 1) & 1, 0, 0);
}
void GUI_build_MENU_TRIGGERS_PERIOD()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    levelScreen = 2;
    indexScreen = MENU_TRIGGERS_PERIOD;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_TRIGGERS_PERIOD], headerX, headerY);
    uint16_t countElements = 2 + events.lenTrigPeriod;
    listGUI_t *list = matrixGUI_newList(&patternList, malloc(countElements * sizeof(void *)), countElements, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    uint16_t i = 0;
    while (i < 2)
    {
        list->elements[i] = matrixGUI_newText(&patternText, tTriggersType[i], 0, 0);
        i++;
    }
    while (i < countElements)
    {
        list->elements[i] = matrixGUI_newText(&patternText, events.trigPeriodList[i - 2].name, 0, 0);
        i++;
    }
}
void GUI_build_MENU_TRIGGERS_PERIOD_SET()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    listGUI_t *list = screens[levelScreen]->elements[1];
    if (list->index == 1)
    {
        events_new(TYPE_TRIGGER_PERIOD);
        list->index = list->countElements;
        ++list->countElements;
        list->elements = realloc(list->elements, list->countElements * sizeof(void *));
        list->elements[list->index] = matrixGUI_newText(&patternText, events.trigPeriodList[events.lenTrigPeriod - 1].name, 0, 0);
    }
    triggerPeriod_t *trigger = &events.trigPeriodList[list->index - 2];
    switcherGUI_t *switcher;
    char *str;
    levelScreen = 3;
    indexScreen = MENU_TRIGGERS_PERIOD_SET;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_TRIGGERS_PERIOD_SET], headerX, headerY);
    list = matrixGUI_newList(&patternList, malloc(7 * sizeof(void *)), 7, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    // back and save:
    list->elements[0] = matrixGUI_newText(&patternText, tTriggersSetPeriod[0], 0, 0);
    // delete:
    list->elements[1] = matrixGUI_newText(&patternText, tTriggersSetPeriod[1], 0, 0);
    // timer begin:
    switcher = matrixGUI_newSwitcher(&patternSwitcher, malloc(2 * sizeof(void *)), 2, 0, 0, 0);
    switcher->elements[0] = matrixGUI_newText(&patternText, tTriggersSetPeriod[2], 0, 0);
    str = GUI_getTimerText(trigger->begin);
    switcher->elements[1] = matrixGUI_newTimerEdit(&patternTimerEdit, str, 1, 0, 0, 0);
    list->elements[2] = switcher;
    // timer active:
    switcher = matrixGUI_newSwitcher(&patternSwitcher, malloc(2 * sizeof(void *)), 2, 0, 0, 0);
    switcher->elements[0] = matrixGUI_newText(&patternText, tTriggersSetPeriod[3], 0, 0);
    str = GUI_getTimerText(trigger->active);
    switcher->elements[1] = matrixGUI_newTimerEdit(&patternTimerEdit, str, 1, 0, 0, 0);
    list->elements[3] = switcher;
    // timer period:
    switcher = matrixGUI_newSwitcher(&patternSwitcher, malloc(2 * sizeof(void *)), 2, 0, 0, 0);
    switcher->elements[0] = matrixGUI_newText(&patternText, tTriggersSetPeriod[4], 0, 0);
    str = GUI_getTimerText(trigger->period);
    switcher->elements[1] = matrixGUI_newTimerEdit(&patternTimerEdit, str, 1, 0, 0, 0);
    list->elements[4] = switcher;
    // days weeks:
    switcher = matrixGUI_newSwitcher(&patternSwitcher, malloc(2 * sizeof(void *)), 2, 0, 0, 0);
    switcher->elements[0] = matrixGUI_newText(&patternText, tTriggersSetPeriod[5], 0, 0);
    switcher->elements[1] = matrixGUI_newChoiceLine(&patternChoiceLine, cutDaysWeeks, 7, malloc(7), 2, 1, 0, 0, 0);
    for (uint16_t i = 0; i < 7; i++)
        ((choiceLineGUI_t *)switcher->elements[1])->positions[i] = (trigger->week & (1 << i)) != 0;
    list->elements[5] = switcher;
    // inversion:
    list->elements[6] = matrixGUI_newChoiceStatus(&patternChoiceStatus, tTriggersSetPeriod[6], (trigger->config >> 1) & 1, 0, 0);
}
void GUI_build_MENU_TRIGGERS_SIGNAL()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    levelScreen = 2;
    indexScreen = MENU_TRIGGERS_SIGNAL;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_TRIGGERS_SIGNAL], headerX, headerY);
    uint16_t countElements = 2 + events.lenTrigSignal;
    listGUI_t *list = matrixGUI_newList(&patternList, malloc(countElements * sizeof(void *)), countElements, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    uint16_t i = 0;
    while (i < 2)
    {
        list->elements[i] = matrixGUI_newText(&patternText, tTriggersType[i], 0, 0);
        i++;
    }
    while (i < countElements)
    {
        list->elements[i] = matrixGUI_newText(&patternText, events.trigSignalList[i - 2].name, 0, 0);
        i++;
    }
}
void GUI_build_MENU_TRIGGERS_SIGNAL_SET()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    listGUI_t *list = screens[levelScreen]->elements[1];
    if (list->index == 1)
    {
        events_new(TYPE_TRIGGER_SIGNAL);
        list->index = list->countElements;
        ++list->countElements;
        list->elements = realloc(list->elements, list->countElements * sizeof(void *));
        list->elements[list->index] = matrixGUI_newText(&patternText, events.trigSignalList[events.lenTrigSignal - 1].name, 0, 0);
    }
    triggerSignal_t *trigger = &events.trigSignalList[list->index - 2];
    switcherGUI_t *switcher;
    choiceGUI_t *choice;
    textGUI_t *text;
    char *str;
    levelScreen = 3;
    indexScreen = MENU_TRIGGERS_SIGNAL_SET;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_TRIGGERS_SIGNAL_SET], headerX, headerY);
    list = matrixGUI_newList(&patternList, malloc(5 * sizeof(void *)), 5, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    // back and save:
    text = matrixGUI_newText(&patternText, tTriggersSetSignal[0], 0, 0);
    list->elements[0] = text;
    // delete:
    text = matrixGUI_newText(&patternText, tTriggersSetSignal[1], 0, 0);
    list->elements[1] = text;
    // sensors:
    switcher = matrixGUI_newSwitcher(&patternSwitcher, malloc(2 * sizeof(void *)), 2, 0, 0, 0);
    switcher->elements[0] = matrixGUI_newText(&patternText, tTriggersSetSignal[2], 0, 0);
    choice = matrixGUI_newChoice(&patternChoice, malloc(sizeof(void *) * events.lenSenAnalog), events.lenSenAnalog, 128, 4, trigger->index, 0, 0);
    for (uint16_t i = 0; i < events.lenSenAnalog; i++)
        choice->variants[i] = events.senAnalogList[i].name;
    switcher->elements[1] = choice;
    list->elements[2] = switcher;
    // sign:
    switcher = matrixGUI_newSwitcher(&patternSwitcher, malloc(2 * sizeof(void *)), 2, 0, 0, 0);
    switcher->elements[0] = matrixGUI_newText(&patternText, tTriggersSetSignal[3], 0, 0);
    switcher->elements[1] = matrixGUI_newChoice(&patternChoice, signs, 5, 32, 4, trigger->config >> 5, 0, 0);
    list->elements[3] = switcher;
    // data:
    switcher = matrixGUI_newSwitcher(&patternSwitcher, malloc(2 * sizeof(void *)), 2, 0, 0, 0);
    str = malloc(9);
    str[8] = '\0';
    matrixGUI_valToStr(trigger->data, str, 8);
    switcher->elements[0] = matrixGUI_newText(&patternText, tTriggersSetSignal[4], 0, 0);
    switcher->elements[1] = matrixGUI_newValueEdit(&patternValueEdit, str, maxValue, minValue, 1, 0, 0, 0);
    list->elements[4] = switcher;
}

void GUI_build_MENU_SCRIPTS()
{
    // deletegroup    
    levelScreen = 1;
    indexScreen = MENU_SCRIPTS;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_SCRIPTS], headerX, headerY);
    uint16_t countElements = 2 + events.lenScripts;
    listGUI_t *list = matrixGUI_newList(&patternList, malloc(countElements * sizeof(void *)), countElements, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    uint16_t i = 0;
    while (i < 2)
    {
        list->elements[i] = matrixGUI_newText(&patternText, tScripts[i], 0, 0);
        i++;
    }
    while (i < countElements)
    {
        list->elements[i] = matrixGUI_newText(&patternText, events.scriptsList[i - 2].name, 0, 0);
        i++;
    }
}
void GUI_build_MENU_SCRIPTS_SET()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    listGUI_t *list = screens[levelScreen]->elements[1];
    if (list->index == 1)
    {
        events_new(TYPE_SCRIPT);
        list->index = list->countElements;
        ++list->countElements;
        list->elements = realloc(list->elements, list->countElements * sizeof(void *));
        list->elements[list->index] = matrixGUI_newText(&patternText, events.scriptsList[events.lenScripts - 1].name, 0, 0);
    }
    levelScreen = 2;
    indexScreen = MENU_SCRIPTS_SET;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_SCRIPTS_SET], headerX, headerY);
    list = matrixGUI_newList(&patternList, malloc(4 * sizeof(void *)), 4, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    for (uint16_t i = 0; i < list->countElements; i++)
    {
        list->elements[i] = matrixGUI_newText(&patternText, tScriptsSet[i], 0, 0);
    }
}
void GUI_build_MENU_SCRIPTS_SET_TRIGGERS()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    indexScreen = MENU_SCRIPTS_SET_TRIGGERS;
    levelScreen = 3;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_SCRIPTS_SET_TRIGGERS], headerX, headerY);
    listGUI_t *list = matrixGUI_newList(&patternList, malloc(4 * sizeof(void *)), 4, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    for (uint16_t i = 0; i < list->countElements; i++)
    {
        list->elements[i] = matrixGUI_newText(&patternText, tTriggers[i], 50, 0);
    }
}
void GUI_build_MENU_SCRIPTS_SET_TRIGGERS_TIME()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    indexScreen = MENU_SCRIPTS_SET_TRIGGERS_TIME;
    levelScreen = 4;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_SCRIPTS_SET_TRIGGERS_TIME], headerX, headerY);
    uint16_t countElements = 1 + events.lenTrigTime;
    listGUI_t *list = matrixGUI_newList(&patternList, malloc(countElements * sizeof(void *)), countElements, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    list->elements[0] = matrixGUI_newText(&patternText, tScriptsSetTriggersBegin[0], 0, 0);
    for (uint16_t i = 1; i < countElements; i++)
    {
        list->elements[i] = matrixGUI_newChoiceStatus(&patternChoiceStatus, events.trigTimeList[i - 1].name, 0, 0, 0);
    }
    uint16_t s = events_getScriptTypeIndex(((listGUI_t *)screens[1]->elements[1])->index - 2, TYPE_TRIGGER_TIME);
    for (uint16_t i = 0; i < events.scriptsList->lenIndexList[TYPE_TRIGGER_TIME]; i++)
    {
        ((choiceStatusGUI_t *)list->elements[events.scriptsList->indexList[s + i] + 1])->index = 1;
    }
}
void GUI_build_MENU_SCRIPTS_SET_TRIGGERS_PERIOD()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    indexScreen = MENU_SCRIPTS_SET_TRIGGERS_PERIOD;
    levelScreen = 4;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_SCRIPTS_SET_TRIGGERS_PERIOD], headerX, headerY);
    uint16_t countElements = 1 + events.lenTrigPeriod;
    listGUI_t *list = matrixGUI_newList(&patternList, malloc(countElements * sizeof(void *)), countElements, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    list->elements[0] = matrixGUI_newText(&patternText, tScriptsSetTriggersBegin[0], 0, 0);
    for (uint16_t i = 1; i < countElements; i++)
    {
        list->elements[i] = matrixGUI_newChoiceStatus(&patternChoiceStatus, events.trigPeriodList[i - 1].name, 0, 0, 0);
    }
    uint16_t s = events_getScriptTypeIndex(((listGUI_t *)screens[1]->elements[1])->index - 2, TYPE_TRIGGER_PERIOD);
    for (uint16_t i = 0; i < events.scriptsList->lenIndexList[TYPE_TRIGGER_PERIOD]; i++)
    {
        ((choiceStatusGUI_t *)list->elements[events.scriptsList->indexList[s + i] + 1])->index = 1;
    }
}
void GUI_build_MENU_SCRIPTS_SET_TRIGGERS_SIGNAL()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    indexScreen = MENU_SCRIPTS_SET_TRIGGERS_SIGNAL;
    levelScreen = 4;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_SCRIPTS_SET_TRIGGERS_SIGNAL], headerX, headerY);
    uint16_t countElements = 1 + events.lenTrigSignal;
    listGUI_t *list = matrixGUI_newList(&patternList, malloc(countElements * sizeof(void *)), countElements, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    list->elements[0] = matrixGUI_newText(&patternText, tScriptsSetTriggersBegin[0], 0, 0);
    for (uint16_t i = 1; i < countElements; i++)
    {
        list->elements[i] = matrixGUI_newChoiceStatus(&patternChoiceStatus, events.trigSignalList[i - 1].name, 0, 0, 0);
    }
    uint16_t s = events_getScriptTypeIndex(((listGUI_t *)screens[1]->elements[1])->index - 2, TYPE_TRIGGER_SIGNAL);
    for (uint16_t i = 0; i < events.scriptsList->lenIndexList[TYPE_TRIGGER_SIGNAL]; i++)
    {
        ((choiceStatusGUI_t *)list->elements[events.scriptsList->indexList[s + i] + 1])->index = 1;
    }
}
void GUI_build_MENU_SCRIPTS_SET_ACTIONS()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    indexScreen = MENU_SCRIPTS_SET_ACTIONS;
    levelScreen = 3;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_SCRIPTS_SET_ACTIONS], headerX, headerY);
    uint16_t countElements = 1 + events.lenActKey;
    listGUI_t *list = matrixGUI_newList(&patternList, malloc(countElements * sizeof(void *)), countElements, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    list->elements[0] = matrixGUI_newText(&patternText, tScriptsSetTriggersBegin[0], 0, 0);
    for (uint16_t i = 1; i < countElements; i++)
    {
        list->elements[i] = matrixGUI_newChoiceStatus(&patternChoiceStatus, events.actKeyList[i - 1].name, 0, 0, 0);
    }
    uint16_t s = events_getScriptTypeIndex(((listGUI_t *)screens[1]->elements[1])->index - 2, TYPE_ACTION_KEY);
    for (uint16_t i = 0; i < events.scriptsList->lenIndexList[TYPE_ACTION_KEY]; i++)
    {
        ((choiceStatusGUI_t *)list->elements[events.scriptsList->indexList[s + i] + 1])->index = 1;
    }
}

void GUI_build_MENU_KEYS()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    levelScreen = 1;
    indexScreen = MENU_KEYS;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_KEYS], headerX, headerY);
    uint16_t countElements = 1 + events.lenActKey;
    listGUI_t *list = matrixGUI_newList(&patternList, malloc(countElements * sizeof(void *)), countElements, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    list->elements[0] = matrixGUI_newText(&patternText, tKeys[0], 0, 0);
    for (uint16_t i = 1; i < countElements; i++)
    {
        list->elements[i] = matrixGUI_newText(&patternText, events.actKeyList[i - 1].name, 0, 0);
    }
}

void GUI_build_MENU_SENSORS()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    levelScreen = 1;
    indexScreen = MENU_SENSORS;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_SENSORS], headerX, headerY);
    uint16_t countElements = 1 + events.lenSenAnalog;
    listGUI_t *list = matrixGUI_newList(&patternList, malloc(countElements * sizeof(void *)), countElements, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    list->elements[0] = matrixGUI_newText(&patternText, tSensors[0], 0, 0);
    for (uint16_t i = 1; i < countElements; i++)
    {
        list->elements[i] = matrixGUI_newText(&patternText, events.senAnalogList[i - 1].name, 0, 0);
    }
}

void GUI_build_MENU_SETTINGS()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    levelScreen = 1;
    indexScreen = MENU_SETTINGS;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_SETTINGS], headerX, headerY);
    listGUI_t *list = matrixGUI_newList(&patternList, malloc(4 * sizeof(void *)), 4, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    for (uint16_t i = 0; i < list->countElements; i++)
    {
        list->elements[i] = matrixGUI_newText(&patternText, tSettings[i], 0, 0);
    }
}
void GUI_build_MENU_SETTINGS_LANGUAGE()
{
}
void GUI_build_MENU_SETTINGS_TIME()
{
    matrixGUI_deleteGroup(screens[levelScreen]);
    levelScreen = 2;
    indexScreen = MENU_SETTINGS_TIME;
    screens[levelScreen] = matrixGUI_newGroup(&patternGroup, malloc(2 * sizeof(void *)), 2);
    listGUI_t *list;
    switcherGUI_t *switcher;
    choiceGUI_t *choice;
    char *str;
    screens[levelScreen]->elements[0] = matrixGUI_newText(&patternText, tHeader[MENU_SETTINGS_TIME], headerX, headerY);
    list = matrixGUI_newList(&patternList, malloc(3 * sizeof(void *)), 3, 32, 6, 8, 3, 0, listX, listY);
    screens[levelScreen]->elements[1] = list;
    // back and save:
    list->elements[0] = matrixGUI_newText(&patternText, tSettingsTime[0], 0, 0);
    // time:
    switcher = matrixGUI_newSwitcher(&patternSwitcher, malloc(2 * sizeof(void *)), 2, 0, 0, 0);
    switcher->elements[0] = matrixGUI_newText(&patternText, tSettingsTime[1], 0, 0);
    str = GUI_getTimerText(eventTime);
    switcher->elements[1] = matrixGUI_newTimerEdit(&patternTimerEdit, str, 1, 0, 0, 0);
    list->elements[1] = switcher;
    // weekDays:
    switcher = matrixGUI_newSwitcher(&patternSwitcher, malloc(2 * sizeof(void *)), 2, 0, 0, 0);
    switcher->elements[0] = matrixGUI_newText(&patternText, tSettingsTime[2], 0, 0);
    choice = matrixGUI_newChoice(&patternChoice, daysWeeks, 7, 176, 4, timeDate.weekDay, 0, 0);
    switcher->elements[1] = choice;
    list->elements[2] = switcher;
}
void GUI_build_MENU_SETTINGS_THEME()
{
}

//==========================================BACK==========================================
void GUI_back_MENU_TRIGGERS()
{
    indexScreen = MENU;
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_back_MENU_TRIGGERS_TIME()
{
    indexScreen = MENU_TRIGGERS;
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_back_MENU_TRIGGERS_TIME_SET()
{
    indexScreen = MENU_TRIGGERS_TIME;
    uint16_t indexTrigger = ((listGUI_t *)screens[2]->elements[1])->index - 2;
    triggerTime_t *trigger = &events.trigTimeList[indexTrigger];

    listGUI_t *list = screens[levelScreen]->elements[1];
    timerEditGUI_t *timerEdit;
    choiceLineGUI_t *choiceLine;
    choiceStatusGUI_t *choiceStatus;
    switcherGUI_t *switcher;

    // timer begin:
    switcher = list->elements[2];
    timerEdit = switcher->elements[1];
    trigger->begin = GUI_getTimerTime(timerEdit->text);
    free(timerEdit->text);
    // timer active:
    switcher = list->elements[3];
    timerEdit = switcher->elements[1];
    trigger->active = GUI_getTimerTime(timerEdit->text);
    free(timerEdit->text);
    // days weeks:
    switcher = list->elements[4];
    choiceLine = switcher->elements[1];
    trigger->week = 0;
    for (uint8_t i = 0; i < 7; i++)
    {
        trigger->week |= choiceLine->positions[i] << i;
    }
    // inversion:
    choiceStatus = list->elements[5];
    trigger->config &= 253;
    trigger->config |= choiceStatus->index << 1;

    // process:
    events_edit(indexTrigger, TYPE_TRIGGER_TIME);
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_back_MENU_TRIGGERS_PERIOD()
{
    indexScreen = MENU_TRIGGERS;
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_back_MENU_TRIGGERS_PERIOD_SET()
{
    indexScreen = MENU_TRIGGERS_PERIOD;
    uint16_t indexTrigger = ((listGUI_t *)screens[2]->elements[1])->index - 2;
    triggerPeriod_t *trigger = &events.trigPeriodList[indexTrigger];

    listGUI_t *list = screens[levelScreen]->elements[1];
    timerEditGUI_t *timerEdit;
    choiceLineGUI_t *choiceLine;
    choiceStatusGUI_t *choiceStatus;
    switcherGUI_t *switcher;

    // timer begin:
    switcher = list->elements[2];
    timerEdit = switcher->elements[1];
    trigger->begin = GUI_getTimerTime(timerEdit->text);
    free(timerEdit->text);
    // timer active:
    switcher = list->elements[3];
    timerEdit = switcher->elements[1];
    trigger->active = GUI_getTimerTime(timerEdit->text);
    free(timerEdit->text);
    // timer period:
    switcher = list->elements[4];
    timerEdit = switcher->elements[1];
    trigger->period = GUI_getTimerTime(timerEdit->text);
    free(timerEdit->text);
    // days weeks:
    switcher = list->elements[5];
    choiceLine = switcher->elements[1];
    trigger->week = 0;
    for (uint8_t i = 0; i < 7; i++)
    {
        trigger->week |= choiceLine->positions[i] << i;
    }
    // inversion:
    choiceStatus = list->elements[6];
    trigger->config &= 253;
    trigger->config |= choiceStatus->index << 1;

    // process:
    events_edit(indexTrigger, TYPE_TRIGGER_PERIOD);
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_back_MENU_TRIGGERS_SIGNAL()
{
    indexScreen = MENU_TRIGGERS;
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_back_MENU_TRIGGERS_SIGNAL_SET()
{
    indexScreen = MENU_TRIGGERS_SIGNAL;
    uint16_t indexTrigger = ((listGUI_t *)screens[2]->elements[1])->index - 2;
    triggerSignal_t *trigger = &events.trigSignalList[indexTrigger];

    listGUI_t *list = screens[levelScreen]->elements[1];
    choiceGUI_t *choice;
    valueEditGUI_t *valueEdit;
    switcherGUI_t *switcher;

    // sensors:
    switcher = list->elements[2];
    choice = switcher->elements[1];
    trigger->index = choice->index;
    // signs:
    switcher = list->elements[3];
    choice = switcher->elements[1];
    trigger->config &= 31;
    trigger->config |= choice->index << 5;
    // data:
    switcher = list->elements[4];
    valueEdit = switcher->elements[1];
    matrixGUI_strToVal((uint64_t *)(&trigger->data), valueEdit->text, 8);
    free(valueEdit->text);

    // process:
    events_edit(indexTrigger, TYPE_TRIGGER_SIGNAL);
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}

void GUI_back_MENU_SCRIPTS()
{
    indexScreen = MENU;
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_back_MENU_SCRIPTS_SET()
{
    indexScreen = MENU_SCRIPTS;
    uint16_t indexScript = ((listGUI_t *)screens[1]->elements[1])->index - 2;
    events_edit(indexScript, TYPE_SCRIPT);
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_back_MENU_SCRIPTS_SET_TRIGGERS()
{
    indexScreen = MENU_SCRIPTS_SET;
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_back_MENU_SCRIPTS_SET_TRIGGERS_TIME()
{
    indexScreen = MENU_SCRIPTS_SET_TRIGGERS;
    uint16_t indexScript = ((listGUI_t *)screens[1]->elements[1])->index - 2;
    uint16_t c = 0;
    listGUI_t *list = screens[levelScreen]->elements[1];
    for (uint16_t i = 1; i <= events.lenTrigTime; i++)
    {
        if (((choiceStatusGUI_t *)list->elements[i])->index)
        {
            ++c;
        }
    }
    uint8_t l[c];
    c = 0;
    for (uint16_t i = 1; i <= events.lenTrigTime; i++)
    {
        if (((choiceStatusGUI_t *)list->elements[i])->index)
        {
            l[c] = i - 1;
            ++c;
        }
    }
    events_writeScriptIndex(indexScript, l, c, TYPE_TRIGGER_TIME);
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_back_MENU_SCRIPTS_SET_TRIGGERS_PERIOD()
{
    indexScreen = MENU_SCRIPTS_SET_TRIGGERS;
    uint16_t indexScript = ((listGUI_t *)screens[1]->elements[1])->index - 2;
    uint16_t c = 0;
    listGUI_t *list = screens[levelScreen]->elements[1];
    for (uint16_t i = 1; i <= events.lenTrigPeriod; i++)
    {
        if (((choiceStatusGUI_t *)list->elements[i])->index)
        {
            ++c;
        }
    }
    uint8_t l[c];
    c = 0;
    for (uint16_t i = 1; i <= events.lenTrigPeriod; i++)
    {
        if (((choiceStatusGUI_t *)list->elements[i])->index)
        {
            l[c] = i - 1;
            ++c;
        }
    }
    events_writeScriptIndex(indexScript, l, c, TYPE_TRIGGER_PERIOD);
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_back_MENU_SCRIPTS_SET_TRIGGERS_SIGNAL()
{
    indexScreen = MENU_SCRIPTS_SET_TRIGGERS;
    uint16_t indexScript = ((listGUI_t *)screens[1]->elements[1])->index - 2;
    uint16_t c = 0;
    listGUI_t *list = screens[levelScreen]->elements[1];
    for (uint16_t i = 1; i <= events.lenTrigSignal; i++)
    {
        if (((choiceStatusGUI_t *)list->elements[i])->index)
        {
            ++c;
        }
    }
    uint8_t l[c];
    c = 0;
    for (uint16_t i = 1; i <= events.lenTrigSignal; i++)
    {
        if (((choiceStatusGUI_t *)list->elements[i])->index)
        {
            l[c] = i - 1;
            ++c;
        }
    }
    events_writeScriptIndex(indexScript, l, c, TYPE_TRIGGER_SIGNAL);
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_back_MENU_SCRIPTS_SET_ACTIONS()
{
    indexScreen = MENU_SCRIPTS_SET;
    uint16_t indexScript = ((listGUI_t *)screens[1]->elements[1])->index - 2;
    uint16_t c = 0;
    listGUI_t *list = screens[levelScreen]->elements[1];
    for (uint16_t i = 1; i <= events.lenActKey; i++)
    {
        if (((choiceStatusGUI_t *)list->elements[i])->index)
        {
            ++c;
        }
    }
    uint8_t l[c];
    c = 0;
    for (uint16_t i = 1; i <= events.lenActKey; i++)
    {
        if (((choiceStatusGUI_t *)list->elements[i])->index)
        {
            l[c] = i - 1;
            ++c;
        }
    }
    events_writeScriptIndex(indexScript, l, c, TYPE_ACTION_KEY);
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}

void GUI_back_MENU_KEYS()
{
    indexScreen = MENU;
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}

void GUI_back_MENU_SENSORS()
{
    indexScreen = MENU;
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}

void GUI_back_MENU_SETTINGS()
{
    indexScreen = MENU;
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_back_MENU_SETTINGS_LANGUAGE()
{
    indexScreen = MENU_SETTINGS;

    // SAVE!!!!!

    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_back_MENU_SETTINGS_TIME()
{
    indexScreen = MENU_SETTINGS;
    listGUI_t *list = screens[levelScreen]->elements[1];
    switcherGUI_t *switcher;
    choiceGUI_t *choice;
    timerEditGUI_t *timer;

    timer_pause(0, 0);

    // timer:
    switcher = list->elements[1];
    timer = switcher->elements[1];
    timeDate.second = (timer->text[6] - '0') * 10 + (timer->text[7] - '0');
    timeDate.minute = (timer->text[3] - '0') * 10 + (timer->text[4] - '0');
    timeDate.hour = (timer->text[0] - '0') * 10 + (timer->text[1] - '0');
    eventTime = (timeDate.hour * 60 + timeDate.minute) * 60 + timeDate.second;
    ds3231_setTime(&ds3231, &timeDate, 0);
    free(timer->text);
    // weekDay:
    switcher = list->elements[2];
    choice = switcher->elements[1];
    timeDate.weekDay = choice->index;

    timer_start(0, 0);

    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_back_MENU_SETTINGS_THEME()
{
    indexScreen = MENU_SETTINGS;

    // SAVE!!!!!

    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
//==========================================================================================================

void GUI_delete_MENU_TRIGGERS_TIME_SET()
{
    indexScreen = MENU_TRIGGERS_TIME;
    listGUI_t *list = screens[2]->elements[1];
    events_delete(list->index - 2, TYPE_TRIGGER_TIME);
    --list->countElements;
    for (uint16_t i = list->index; i < list->countElements; i++)
        list->elements[i] = list->elements[i + 1];
    list->elements = realloc(list->elements, list->countElements);
    list->index = 0;
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_delete_MENU_TRIGGERS_PERIOD_SET()
{
    indexScreen = MENU_TRIGGERS_PERIOD;
    listGUI_t *list = screens[2]->elements[1];
    events_delete(list->index - 2, TYPE_TRIGGER_PERIOD);
    --list->countElements;
    for (uint16_t i = list->index; i < list->countElements; i++)
        list->elements[i] = list->elements[i + 1];
    list->elements = realloc(list->elements, list->countElements);
    list->index = 0;
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}
void GUI_delete_MENU_TRIGGERS_SIGNAL_SET()
{
    indexScreen = MENU_TRIGGERS_SIGNAL;
    listGUI_t *list = screens[2]->elements[1];
    events_delete(list->index - 2, TYPE_TRIGGER_SIGNAL);
    --list->countElements;
    for (uint16_t i = list->index; i < list->countElements; i++)
        list->elements[i] = list->elements[i + 1];
    list->elements = realloc(list->elements, list->countElements);
    list->index = 0;
    matrixGUI_deleteGroup(screens[levelScreen]);
    GUI_free(screens[levelScreen]);
    levelScreen -= 1;
}

void GUI_delete_MENU_SCRIPTS_SET()
{
    listGUI_t *list = screens[1]->elements[1];
    events_delete(list->index - 2, TYPE_SCRIPT);
    --list->countElements;
    for (uint16_t i = list->index; i < list->countElements; i++)
        list->elements[i] = list->elements[i + 1];
    list->elements = realloc(list->elements, list->countElements);
    list->index = 0;
    GUI_back_MENU_SCRIPTS_SET();
}

//==========================================================================================================

void GUI_MENU()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // MENU_TRIGGERS
        GUI_build_MENU_TRIGGERS();
        break;
    case 1: // MENU_SCRIPTS
        matrixGUI_deleteGroup(screens[levelScreen]);
        GUI_build_MENU_SCRIPTS();
        break;
    case 2: // MENU_KEYS
        GUI_build_MENU_KEYS();
        break;
    case 3: // MENU_SENSORS
        GUI_build_MENU_SENSORS();
        break;
    case 4: // MENU_SETTINGS
        GUI_build_MENU_SETTINGS();
        break;
    default:
        break;
    }
}

void GUI_MENU_TRIGGERS()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_TRIGGERS();
        break;
    case 1: // TIME
        GUI_build_MENU_TRIGGERS_TIME();
        break;
    case 2: // PERIOD
        GUI_build_MENU_TRIGGERS_PERIOD();
        break;
    case 3: // SIGNAL
        GUI_build_MENU_TRIGGERS_SIGNAL();
    default:
        break;
    }
}
void GUI_MENU_TRIGGERS_TIME()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_TRIGGERS_TIME();
        break;
    default: // SET
        GUI_build_MENU_TRIGGERS_TIME_SET();
        break;
    }
}
void GUI_MENU_TRIGGERS_TIME_SET()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switcherGUI_t *switcher;
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_TRIGGERS_TIME_SET();
        break;
    case 1: // DELETE
        GUI_delete_MENU_TRIGGERS_TIME_SET();
        break;
    case 5: // inversion
        ++((choiceStatusGUI_t *)list->elements[5])->index;
        break;
    default:
        switcher = list->elements[list->index];
        editElement = switcher->elements[1];
        switcher->index = 1;
        mode = CHANGE;
        break;
    }
}
void GUI_MENU_TRIGGERS_PERIOD()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_TRIGGERS_PERIOD();
        break;
    default: // SET
        GUI_build_MENU_TRIGGERS_PERIOD_SET();
        break;
    }
}
void GUI_MENU_TRIGGERS_PERIOD_SET()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switcherGUI_t *switcher;
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_TRIGGERS_PERIOD_SET();
        break;
    case 1: // DELETE
        GUI_delete_MENU_TRIGGERS_PERIOD_SET();
        break;
    case 6: // inversion
        ((choiceStatusGUI_t *)list->elements[6])->index = !((choiceStatusGUI_t *)list->elements[6])->index;
        break;
    default:
        switcher = list->elements[list->index];
        editElement = switcher->elements[1];
        switcher->index = 1;
        mode = CHANGE;
        break;
    }
}
void GUI_MENU_TRIGGERS_SIGNAL()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_TRIGGERS_SIGNAL();
        break;
    default: // SET
        GUI_build_MENU_TRIGGERS_SIGNAL_SET();
        break;
    }
}
void GUI_MENU_TRIGGERS_SIGNAL_SET()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switcherGUI_t *switcher;
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_TRIGGERS_SIGNAL_SET();
        break;
    case 1: // DELETE
        GUI_delete_MENU_TRIGGERS_SIGNAL_SET();
        break;
    default:
        switcher = list->elements[list->index];
        editElement = switcher->elements[1];
        switcher->index = 1;
        mode = CHANGE;
        break;
    }
}

void GUI_MENU_SCRIPTS()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_SCRIPTS();
        break;
    default: // SET
        GUI_build_MENU_SCRIPTS_SET();
        break;
    }
}
void GUI_MENU_SCRIPTS_SET()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_SCRIPTS_SET();
        break;
    case 1: // DELETE
        GUI_delete_MENU_SCRIPTS_SET();
        break;
    case 2: // TRIGGERS
        GUI_build_MENU_SCRIPTS_SET_TRIGGERS();
        break;
    case 3: // ACTIONS
        GUI_build_MENU_SCRIPTS_SET_ACTIONS();
        break;
    default:
        break;
    }
}
void GUI_MENU_SCRIPTS_SET_TRIGGERS()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_SCRIPTS_SET_TRIGGERS();
        break;
    case 1: // TIME
        GUI_build_MENU_SCRIPTS_SET_TRIGGERS_TIME();
        break;
    case 2: // PERIOD
        GUI_build_MENU_SCRIPTS_SET_TRIGGERS_PERIOD();
        break;
    case 3: // SIGNAL
        GUI_build_MENU_SCRIPTS_SET_TRIGGERS_SIGNAL();
        break;
    default:
        break;
    }
}
void GUI_MENU_SCRIPTS_SET_TRIGGERS_TIME()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_SCRIPTS_SET_TRIGGERS_TIME();
        break;
    default:
        ((choiceStatusGUI_t *)list->elements[list->index])->index = !((choiceStatusGUI_t *)list->elements[list->index])->index;
        break;
    }
}
void GUI_MENU_SCRIPTS_SET_TRIGGERS_PERIOD()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_SCRIPTS_SET_TRIGGERS_PERIOD();
        break;
    default:
        ((choiceStatusGUI_t *)list->elements[list->index])->index = !((choiceStatusGUI_t *)list->elements[list->index])->index;
        // EDIT...
        break;
    }
}
void GUI_MENU_SCRIPTS_SET_TRIGGERS_SIGNAL()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_SCRIPTS_SET_TRIGGERS_SIGNAL();
        break;
    default:
        ((choiceStatusGUI_t *)list->elements[list->index])->index = !((choiceStatusGUI_t *)list->elements[list->index])->index;
        // EDIT...
        break;
    }
}
void GUI_MENU_SCRIPTS_SET_ACTIONS()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_SCRIPTS_SET_ACTIONS();
        break;
    default:
        ((choiceStatusGUI_t *)list->elements[list->index])->index = !((choiceStatusGUI_t *)list->elements[list->index])->index;
        // EDIT...
        break;
    }
}

void GUI_MENU_KEYS()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_KEYS();
        break;
    default:
        // EDIT...
        break;
    }
}

void GUI_MENU_SENSORS()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_SENSORS();
        break;
    default:
        // EDIT...
        break;
    }
}

void GUI_MENU_SETTINGS()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_SETTINGS();
        break;
    case 1: // LANGUAGE
        GUI_build_MENU_SETTINGS_LANGUAGE();
        break;
    case 2: // TIME
        GUI_build_MENU_SETTINGS_TIME();
        break;
    case 3: // THEME
        GUI_build_MENU_SETTINGS_THEME();
        break;
    default:
        break;
    }
}
void GUI_MENU_SETTINGS_LANGUAGE()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_SETTINGS_LANGUAGE();
        break;
    default:
        // EDIT...
        break;
    }
}
void GUI_MENU_SETTINGS_TIME()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switcherGUI_t *switcher;
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_SETTINGS_TIME();
        break;
    default:
        switcher = list->elements[list->index];
        editElement = switcher->elements[1];
        switcher->index = 1;
        mode = CHANGE;
        break;
    }
}
void GUI_MENU_SETTINGS_THEME()
{
    listGUI_t *list = screens[levelScreen]->elements[1];
    switch (list->index)
    {
    case 0: // BACK
        GUI_back_MENU_SETTINGS_THEME();
        break;
    default:
        // EDIT...
        break;
    }
}
//==========================================================================================================
//==========================================================================================================

void GUI_update()
{
    fCommand_t command = getCommandEncoder(&encoder);
    fScreen_t preIndexScreen = indexScreen;
    listGUI_t *list = screens[levelScreen]->elements[1];
    if (command)
    {
        if (mode == SCROLL)
        {
            switch (command)
            {
            case LEFT:
                list->index = list->index == 0 ? list->index : list->index - 1;
                break;
            case RIGHT:
                list->index = list->index == (list->countElements - 1) ? list->index : list->index + 1;
                break;
            case PRESS:
                switch (indexScreen)
                {
                case MENU:
                    GUI_MENU();
                    break;
                case MENU_TRIGGERS:
                    GUI_MENU_TRIGGERS();
                    break;
                case MENU_TRIGGERS_TIME:
                    GUI_MENU_TRIGGERS_TIME();
                    break;
                case MENU_TRIGGERS_TIME_SET:
                    GUI_MENU_TRIGGERS_TIME_SET();
                    break;
                case MENU_TRIGGERS_PERIOD:
                    GUI_MENU_TRIGGERS_PERIOD();
                    break;
                case MENU_TRIGGERS_PERIOD_SET:
                    GUI_MENU_TRIGGERS_PERIOD_SET();
                    break;
                case MENU_TRIGGERS_SIGNAL:
                    GUI_MENU_TRIGGERS_SIGNAL();
                    break;
                case MENU_TRIGGERS_SIGNAL_SET:
                    GUI_MENU_TRIGGERS_SIGNAL_SET();
                    break;

                case MENU_SCRIPTS:
                    GUI_MENU_SCRIPTS();
                    break;
                case MENU_SCRIPTS_SET:
                    GUI_MENU_SCRIPTS_SET();
                    break;
                case MENU_SCRIPTS_SET_TRIGGERS:
                    GUI_MENU_SCRIPTS_SET_TRIGGERS();
                    break;
                case MENU_SCRIPTS_SET_TRIGGERS_TIME:
                    GUI_MENU_SCRIPTS_SET_TRIGGERS_TIME();
                    break;
                case MENU_SCRIPTS_SET_TRIGGERS_PERIOD:
                    GUI_MENU_SCRIPTS_SET_TRIGGERS_PERIOD();
                    break;
                case MENU_SCRIPTS_SET_TRIGGERS_SIGNAL:
                    GUI_MENU_SCRIPTS_SET_TRIGGERS_SIGNAL();
                    break;
                case MENU_SCRIPTS_SET_ACTIONS:
                    GUI_MENU_SCRIPTS_SET_ACTIONS();
                    break;

                case MENU_KEYS:
                    GUI_MENU_KEYS();
                    break;

                case MENU_SENSORS:
                    GUI_MENU_SENSORS();
                    break;

                case MENU_SETTINGS:
                    GUI_MENU_SETTINGS();
                    break;
                case MENU_SETTINGS_LANGUAGE:
                    GUI_MENU_SETTINGS_LANGUAGE();
                    break;
                case MENU_SETTINGS_THEME:
                    GUI_MENU_SETTINGS_THEME();
                    break;
                case MENU_SETTINGS_TIME:
                    GUI_MENU_SETTINGS_TIME();
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }
        else if (mode == CHANGE)
        {
            switcherGUI_t *switcher = list->elements[list->index];
            int8_t d = 1;
            switch (command)
            {
            case LEFT:
                d = -1;
                __attribute__((fallthrough));
            case RIGHT:
                switch (matrixGUI_getType(editElement))
                {
                case GUI_TIMEREDIT:
                    ((timerEditGUI_t *)editElement)->indexSegment += d;
                    break;
                case GUI_TEXTEDIT:
                    ((textEditGUI_t *)editElement)->indexSegment += d;
                    break;
                case GUI_CHOICELINE:
                    ((choiceLineGUI_t *)editElement)->indexVariant += d;
                    break;
                case GUI_CHOICE:
                    ((choiceGUI_t *)editElement)->index += d;
                    break;
                case GUI_VALUEEDIT:
                    ((valueEditGUI_t *)editElement)->indexSegment += d;
                    break;
                default:
                    break;
                }
                break;
            case PRESS_LEFT:
                d = -1;
                __attribute__((fallthrough));
            case PRESS_RIGHT:
                switch (matrixGUI_getType(editElement))
                {
                case GUI_TIMEREDIT:
                    ((timerEditGUI_t *)editElement)->index += d;
                    break;
                case GUI_TEXTEDIT:
                    ((textEditGUI_t *)editElement)->index += d;
                    break;
                case GUI_CHOICELINE:
                    ((choiceLineGUI_t *)editElement)->index += d;
                    break;
                case GUI_VALUEEDIT:
                    ((valueEditGUI_t *)editElement)->index += d;
                    break;
                case GUI_CHOICE:
                    ((choiceGUI_t *)editElement)->index += d;
                    break;
                default:
                    break;
                }
                break;
            case PRESS:
                switcher->index = 0;
                mode = SCROLL;
                break;
            default:
                break;
            }
        }
    }
    if (preIndexScreen != indexScreen)
    {
        int64_t time = esp_timer_get_time();
        matrixGUI_buildGroup(screens[levelScreen]);
        ESP_LOGI("speedGUI", "%lli", (esp_timer_get_time() - time) / 500);
    }
    else
    {
        matrixGUI_updateList(list);
    }
    st7789Matrix_write();
}

void GUI_init()
{
    // fontx:
    fontx_init(&EU_RU_32p, FILE_FONT);
    fontx_open(&EU_RU_32p);
    matrix_fontSelect(&matrixFont);

    // main:
    patternTextEdit.sizeSimbolList = matrixGUI_buildSimbolList(&patternTextEdit.simbolList, "а-яА-Я0-9");
    matrix_drawFillRectWH(0, 0, 240, 320, colorBackground); // fill screen
    GUI_build_MENU();
}
