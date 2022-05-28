#include <string.h>
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <stdio.h>
#include <stdint.h>
#include "ds3231.h"
#include "analogExpand.h"

#include "events.h"

// uint32_t events_getTime(formatTime_t *formatTime)
// {
//     return (formatTime->hour * 60 + formatTime->minute) * 60 + formatTime->second;
// }
// void events_getFormatTime(formatTime_t *formatTime, uint32_t time)
// {
//     formatTime->second = time % 60;
//     formatTime->minute = (time / 60) % 60;
//     formatTime->hour = time / 3600;
// }

events_t events;
uint32_t eventTime;
uint32_t nextTrigTime;
uint32_t nextTrigPeriod;
extern timeDate_t timeDate;
extern analogExpand_t analogExpand;
extern gpio_num_t relePins[];

static char *nameFiles[] = {TRIGGERS_TIME_FILE, TRIGGERS_PERIOD_FILE, TRIGGERS_SIGNAL_FILE, ACTIONS_KEY_FILE, SCRIPTS_FILE, SENSORS_ANALOG_FILE};
static uint16_t size[] = {sizeof(triggerTime_t), sizeof(triggerPeriod_t), sizeof(triggerSignal_t), sizeof(action_t), sizeof(script_t), sizeof(sensor_t)};
static uint16_t lengthMax[] = {TRIGGERS_TIME_MAX, TRIGGERS_PERIOD_MAX, TRIGGERS_SIGNAL_MAX, ACTIONS_MAX, SCRIPTS_MAX, SENSORS_MAX};
static uint16_t *length;
static void *list[6];
static char *name[] = {"Òàêòîâûé", "Öèêëè÷íûé", "Ñèãíàëüíûé", "Êëþ÷", "Èíñòðóêöèÿ", "Äàò÷èê"};

uint16_t events_getScriptTypeIndex(uint16_t indexScript, typeEvents_t type)
{
    uint16_t i = 0;
    for (uint16_t t = 0; t < type; t++)
    {
        i += events.scriptsList[indexScript].lenIndexList[t];
    }
    return i;
}

void events_init()
{
    //  write in general data
    list[0] = events.trigTimeList;
    list[1] = events.trigPeriodList;
    list[2] = events.trigSignalList;
    list[3] = events.actKeyList;
    list[4] = events.scriptsList;
    list[5] = events.senAnalogList;
    length = &events.lenTrigTime;

    // getting data from a file
    for (uint16_t type = 0; type < 6; type++)
    {
        // remove(nameFiles[type]);
        FILE *file = fopen(nameFiles[type], "r");
        if (file == NULL)
        {
            // if not file:
            fclose(file);
            file = fopen(nameFiles[type], "w");
            length[type] = 0;
            if (type == TYPE_ACTION_KEY || type == TYPE_SENSOR_ANALOG)
            {
                void *indexSRAM = list[type] + size[type] - NAME_CHAR_MAX;
                char buffer[4];
                for (uint16_t index = 1; index <= lengthMax[type]; index++)
                {
                    sprintf(buffer, "%hu", index);
                    strcpy(indexSRAM, name[type]);
                    strcat(indexSRAM, buffer);

                    indexSRAM += size[type];
                }
                length[type] = lengthMax[type];
            }
            fwrite(&length[type], 2, 1, file);
            fwrite(list[type], size[type], length[type], file);
        }
        else
        {
            // if there is a file:
            fread(&length[type], 2, 1, file);
            fread(list[type], size[type], length[type], file);
            // remove(nameFiles[type]);
        }
        fclose(file);
    }
}

int64_t Itime1, Itime2, Itime3, Itime4;
void callEvents()
{
    uint32_t resistance[12];
    analogExpand_getResistancePorts(&analogExpand, resistance);
    for (uint16_t i = 0; i < 12; i++)
        events.senAnalogList[i].data = resistance[i];
    if (nextTrigTime <= eventTime)
    {
        Itime1 = esp_timer_get_time();
        nextTrigTime = events_callTriggersTime();
        Itime1 = (esp_timer_get_time() - Itime1) / 1000;
        // ESP_LOGI("delayTrigTime", "%lli", (esp_timer_get_time() - time) / 1000);
    }
    if (nextTrigPeriod <= eventTime)
    {
        Itime2 = esp_timer_get_time();
        nextTrigPeriod = events_callTriggersPeriod();
        Itime2 = (esp_timer_get_time() - Itime2) / 1000;
        // ESP_LOGI("delayTrigPeriod", "%lli", (esp_timer_get_time() - time) / 1000);
    }
    Itime3 = esp_timer_get_time();
    events_callTriggersSignal();
    Itime3 = (esp_timer_get_time() - Itime3) / 1000;
    // ESP_LOGI("delayTrigSignal", "%lli", (esp_timer_get_time() - time) / 1000);
    Itime4 = esp_timer_get_time();
    events_callScripts();
    for (uint16_t i = 0; i < 3; i++)
        gpio_set_level(relePins[i], !(events.actKeyList[i].config & MASK_TRUE));
    Itime4 = (esp_timer_get_time() - Itime4) / 1000;
    // ESP_LOGI("delayScripts", "%lli", (esp_timer_get_time() - time) / 1000);
}

// CONTROL SCRIPT INSIDE LIST:
void events_deleteScriptIndex(uint16_t indexScript, uint8_t *indexList, uint8_t lengthList, typeEvents_t type)
{
    script_t *script = &events.scriptsList[indexScript];
    uint16_t i = events_getScriptTypeIndex(indexScript, type);
    uint16_t d = 0;
    uint16_t le = 0;
    uint16_t ie = i + script->lenIndexList[type];
    while (i < ie)
    {
        script->indexList[i] = script->indexList[i + d];
        for (uint16_t l = le; l < lengthList; l++)
        {
            if (script->indexList[i] == indexList[l])
            {
                le = l + 1;
                d += 1;
                ie -= 1;
                if ((i + d) <= ie)
                {
                    script->indexList[i] = script->indexList[i + d];
                }
                else
                {
                    break;
                }
            }
        }
        i++;
    }
    if (d)
    {
        script->lenIndexList[type] -= d;
        uint16_t ie = 0;
        while (type < 3)
        {
            type += 1;
            ie += script->lenIndexList[type];
        }
        i++;
        while (i < ie)
        {
            script->indexList[i] = script->indexList[i + d];
            i++;
        }
    }
}
void events_newScriptIndex(uint16_t indexScript, uint8_t *indexList, uint8_t lengthList, typeEvents_t type)
{
    script_t *script = &events.scriptsList[indexScript];

    uint16_t i = events_getScriptTypeIndex(indexScript, type);
    uint16_t ie = i + script->lenIndexList[type];
    uint16_t d = 0;
    for (uint16_t ii = 0; ii < lengthList; ii++)
    {
        indexList[ii] = indexList[ii + d];
        for (uint16_t l = i; l < ie; l++)
        {
            if (script->indexList[i] == indexList[l])
            {
                i = l + 1;
                d += 1;
                lengthList -= 1;
                if ((ii + d) <= lengthList)
                {
                    indexList[ii] = indexList[ii + d];
                }
                else
                {
                    break;
                }
            }
        }
    }
    if (d)
    {
        //˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜˜
        i = ie;
        for (uint16_t t = type + 1; t < 4; t++)
        {
            i += script->lenIndexList[t];
        }
        i -= 1;
        while (i >= ie)
        {
            script->indexList[i + d] = script->indexList[i];
            i--;
        }

        //˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜ ˜ ˜˜˜˜˜˜˜˜˜˜˜
        ie -= script->lenIndexList[type];
        while (i >= ie && d != 0)
        {
            if (script->indexList[i] < indexList[d - 1])
            {
                script->indexList[i + d] = indexList[d - 1];
                d--;
            }
            else
            {
                script->indexList[i + d] = indexList[i];
                i--;
            }
        }
        if (d != 0)
        {
            script->indexList[ie] = indexList[0];
        }
        //˜˜˜˜˜˜ ˜˜ flash...
    }
}
void events_writeScriptIndex(uint16_t indexScript, uint8_t *indexList, uint8_t lengthList, typeEvents_t type)
{
    script_t *script = &events.scriptsList[indexScript];
    uint16_t s = events_getScriptTypeIndex(indexScript, type);
    uint16_t i;
    if (script->lenIndexList[type] != lengthList)
    {
        int16_t d = lengthList - script->lenIndexList[type];
        uint16_t e = s;
        i = s + script->lenIndexList[type];
        for (uint16_t t = type; t < 4; t++)
        {
            e += script->lenIndexList[t];
        }
        if (script->lenIndexList[type] > lengthList)
        {
            while (i < e)
            {
                script->indexList[i + d] = script->indexList[i];
                ++i;
            }
        }
        else
        {
            while (i < e)
            {
                script->indexList[e + d] = script->indexList[e];
                --e;
            }
        }
        script->lenIndexList[type] = lengthList;
    }
    i = 0;
    while (i < lengthList)
    {
        script->indexList[s + i] = indexList[i];
        ++i;
    }
}

// CONTROL MEMORY:
// alignment breaks the code, if it is not repaired, then the code turns out to be huge
void events_delete(uint16_t index, typeEvents_t type)
{
    if (index < length[type])
    {
        // delete an item by index and move the last item to a location by index
        FILE *file = NULL;
        while (file == NULL)
            file = fopen(nameFiles[type], "r+");
        // edit length:
        length[type] -= 1;
        fwrite(&length[type], 2, 1, file);
        // edit list:
        void *indexSRAM1 = NULL;
        void *indexSRAM2 = NULL;
        switch (type)
        {
        case TYPE_TRIGGER_TIME:
            indexSRAM1 = &((triggerTime_t *)list[type])[index];
            indexSRAM2 = &((triggerTime_t *)list[type])[length[type]];
            break;
        case TYPE_TRIGGER_PERIOD:
            indexSRAM1 = &((triggerPeriod_t *)list[type])[index];
            indexSRAM2 = &((triggerPeriod_t *)list[type])[length[type]];
            break;
        case TYPE_TRIGGER_SIGNAL:
            indexSRAM1 = &((triggerSignal_t *)list[type])[index];
            indexSRAM2 = &((triggerSignal_t *)list[type])[length[type]];
            break;
        case TYPE_ACTION_KEY:
            indexSRAM1 = &((action_t *)list[type])[index];
            indexSRAM2 = &((action_t *)list[type])[length[type]];
            break;
        case TYPE_SCRIPT:
            indexSRAM1 = &((script_t *)list[type])[index];
            indexSRAM2 = &((script_t *)list[type])[length[type]];
            break;
        case TYPE_SENSOR_ANALOG:
            indexSRAM1 = &((sensor_t *)list[type])[index];
            indexSRAM2 = &((sensor_t *)list[type])[length[type]];
            break;
        default:
            return;
            break;
        }
        memcpy(indexSRAM1, indexSRAM2, size[type]);
        fpos_t indexFlash = 2 + (index * size[type]);
        fsetpos(file, &indexFlash);
        fwrite(indexSRAM1, size[type], 1, file);
        fclose(file);
        if (type < TYPE_SCRIPT)
        {
            // file = fopen(nameFiles[TYPE_SCRIPT], "r+");
            for (uint16_t indexScript = 0; indexScript < events.lenScripts; indexScript++)
            {
                if (events.scriptsList[indexScript].lenIndexList[type] != 0)
                {
                    int16_t i = events_getScriptTypeIndex(indexScript, type);
                    int16_t ie = i + events.scriptsList[indexScript].lenIndexList[type];
                    uint8_t d = 0;

                    while (i < ie)
                    {
                        if (events.scriptsList[indexScript].indexList[i] >= index)
                        {
                            d = events.scriptsList[indexScript].indexList[i] == index;
                            break;
                        }
                        i++;
                    }
                    if (events.scriptsList[indexScript].indexList[ie - 1] == length[type])
                    {
                        if (d)
                        {
                            i = ie - 1;
                        }
                        else //˜˜˜˜˜˜˜
                        {
                            ie -= 1;
                            while (i < ie)
                            {
                                --ie;
                                events.scriptsList[indexScript].indexList[ie] = events.scriptsList[indexScript].indexList[ie];
                            }
                            events.scriptsList[indexScript].indexList[ie] = index;
                        }
                    }
                    if (d)
                    {
                        uint16_t m = 0;
                        for (uint16_t t = 0; t < 4; t++)
                        {
                            m += events.scriptsList[indexScript].lenIndexList[t];
                        }
                        m -= 1;
                        while (i < m)
                        {
                            ++i;
                            events.scriptsList[indexScript].indexList[i] = events.scriptsList[indexScript].indexList[i];
                        }
                    }
                }
                events_edit(indexScript, TYPE_SCRIPT);
            }
            // fclose(file);
        }
    }
}
void events_new(typeEvents_t type)
{
    // write element by type in flash memory
    if (lengthMax[type] > length[type])
    {
        void *indexSRAM;
        void *indexNameSRAM;
        switch (type)
        {
        case TYPE_TRIGGER_TIME:
            indexSRAM = &((triggerTime_t *)list[type])[length[type]];
            indexNameSRAM = &((triggerTime_t *)list[type])[length[type]].name;
            break;
        case TYPE_TRIGGER_PERIOD:
            indexSRAM = &((triggerPeriod_t *)list[type])[length[type]];
            indexNameSRAM = &((triggerPeriod_t *)list[type])[length[type]].name;
            break;
        case TYPE_TRIGGER_SIGNAL:
            indexSRAM = &((triggerSignal_t *)list[type])[length[type]];
            indexNameSRAM = &((triggerSignal_t *)list[type])[length[type]].name;
            break;
        case TYPE_ACTION_KEY:
            indexSRAM = &((action_t *)list[type])[length[type]];
            indexNameSRAM = &((action_t *)list[type])[length[type]].name;
            break;
        case TYPE_SCRIPT:
            indexSRAM = &((script_t *)list[type])[length[type]];
            indexNameSRAM = &((script_t *)list[type])[length[type]].name;
            break;
        case TYPE_SENSOR_ANALOG:
            indexSRAM = &((sensor_t *)list[type])[length[type]];
            indexNameSRAM = &((sensor_t *)list[type])[length[type]].name;
            break;
        default:
            return;
            break;
        }
        memset(indexSRAM, 0, size[type]);
        char buffer[4];
        sprintf(buffer, "%hu", length[type] + 1);
        strcpy(indexNameSRAM, name[type]);
        strcat(indexNameSRAM, buffer);

        FILE *file = NULL;
        while (file == NULL)
            file = fopen(nameFiles[type], "r+");

        fpos_t indexFlash = 2 + (size[type] * length[type]);
        length[type] += 1;
        fwrite(&length[type], 2, 1, file);
        fsetpos(file, &indexFlash);
        fwrite(indexSRAM, size[type], 1, file);
        fclose(file);
    }
}
void events_edit(uint16_t index, typeEvents_t type)
{
    if (lengthMax[type] > index)
    {
        void *indexSRAM;
        switch (type)
        {
        case TYPE_TRIGGER_TIME:
            indexSRAM = &((triggerTime_t *)list[type])[index];
            break;
        case TYPE_TRIGGER_PERIOD:
            indexSRAM = &((triggerPeriod_t *)list[type])[index];
            break;
        case TYPE_TRIGGER_SIGNAL:
            indexSRAM = &((triggerSignal_t *)list[type])[index];
            break;
        case TYPE_ACTION_KEY:
            indexSRAM = &((action_t *)list[type])[index];
            break;
        case TYPE_SCRIPT:
            indexSRAM = &((script_t *)list[type])[index];
            break;
        case TYPE_SENSOR_ANALOG:
            indexSRAM = &((sensor_t *)list[type])[index];
            break;
        default:
            return;
            break;
        }
        FILE *file = NULL;
        while (file == NULL)
            file = fopen(nameFiles[type], "r+");
        fpos_t indexFlash = 2 + (size[type] * index);
        fsetpos(file, &indexFlash);
        fwrite(indexSRAM, size[type], 1, file);
        fclose(file);
    }
}

// CALL TRIGGERS LOGIC:
uint32_t events_callTriggersTime()
{
    uint8_t week = 1 << timeDate.weekDay;
    // calls the time trigger checker and return next trigger time
    uint32_t minTime = countSecondDay;
    for (uint16_t indexTrigger = 0; indexTrigger < events.lenTrigTime; indexTrigger++) // MASK_INVERSION
    {
        if (events.trigTimeList[indexTrigger].week & week)
        {
            uint32_t beginTime = events.trigTimeList[indexTrigger].begin;
            uint32_t endTime = events.trigTimeList[indexTrigger].active + beginTime;
            bool flag = events.trigTimeList[indexTrigger].config & MASK_INVERSION;
            if ((beginTime <= eventTime) && (endTime > eventTime))
            {
                flag = !flag;
                minTime = endTime < minTime ? endTime : minTime;
            }
            else if (beginTime > eventTime)
                minTime = beginTime < minTime ? beginTime : minTime;
            events.trigTimeList[indexTrigger].config = flag ? events.trigTimeList[indexTrigger].config | MASK_TRUE : events.trigTimeList[indexTrigger].config & MASK_FALSE;
        }
    }
    if (minTime >= countSecondDay)
        return 0;
    return minTime;
}
uint32_t events_callTriggersPeriod()
{
    uint8_t week = 1 << timeDate.weekDay;
    // calls the period trigger checker and return next trigger time
    int32_t minTime = countSecondDay;
    for (uint16_t indexTrigger = 0; indexTrigger != events.lenTrigPeriod; indexTrigger++)
    {
        if (events.trigPeriodList[indexTrigger].week & week)
        {
            int32_t diffTime = eventTime - events.trigPeriodList[indexTrigger].begin;
            if (diffTime >= 0)
            {

                diffTime /= events.trigPeriodList[indexTrigger].period; // period > 0
                uint32_t beginTime = events.trigPeriodList[indexTrigger].begin + events.trigPeriodList[indexTrigger].period * diffTime;
                uint32_t endTime = beginTime + events.trigPeriodList[indexTrigger].active;
                bool flag = events.trigPeriodList[indexTrigger].config & MASK_INVERSION;
                if ((beginTime <= eventTime) && (endTime > eventTime))
                {
                    flag = !flag;
                    minTime = endTime < minTime ? endTime : minTime;
                }
                else
                {
                    beginTime += events.trigPeriodList[indexTrigger].period;
                    minTime = beginTime < minTime ? beginTime : minTime;
                }
                events.trigPeriodList[indexTrigger].config = flag ? events.trigPeriodList[indexTrigger].config | MASK_TRUE : events.trigPeriodList[indexTrigger].config & MASK_FALSE;
            }
            else
            {
                minTime = events.trigPeriodList[indexTrigger].begin < minTime ? events.trigPeriodList[indexTrigger].begin : minTime;
            }
        }
    }
    if (minTime >= countSecondDay)
        return 0;
    return minTime;
}
void events_callTriggersSignal()
{
    for (uint16_t indexTrigger = 0; indexTrigger != events.lenTrigSignal; indexTrigger++)
    {
        // calls the signal trigger checker
        bool flag = (events.trigSignalList[indexTrigger].config & MASK_COMPARE) == MASK_COMPARE;
        uint32_t signal[2] = {events.trigSignalList[indexTrigger].index, events.trigSignalList[indexTrigger].data};
        for (uint8_t index = 0; index <= flag; index++)
        {
            signal[index] = events.senAnalogList[signal[index]].data;
        }
        signEvents_t sign = events.trigSignalList[indexTrigger].config >> 5;
        switch (sign)
        {
        case SIGN_MORE: //>
            flag = signal[0] > signal[1];
            break;
        case SIGN_LESS: //<
            flag = signal[0] < signal[1];
            break;
        case SIGN_EQUALMORE: //>=
            flag = signal[0] >= signal[1];
            break;
        case SIGN_EQUALLESS: //<=
            flag = signal[0] <= signal[1];
            break;
        case SIGN_EQUAL: //=
            flag = signal[0] <= signal[1];
            break;
        default:
            break;
        }
        events.trigSignalList[indexTrigger].config = flag ? events.trigSignalList[indexTrigger].config | MASK_TRUE : events.trigSignalList[indexTrigger].config & MASK_FALSE;
    }
}

// CALL SCRIPT LOGIC:
void checkTriggers(uint16_t indexScript)
{
    // checks the activity of all triggers, if all are active, then activates the script according to the specified script
    uint16_t i = 0;
    uint16_t ie = 0;
    uint8_t config = 0;
    for (uint16_t type = TYPE_TRIGGER_TIME; type <= TYPE_TRIGGER_SIGNAL; type++)
    {
        ie += events.scriptsList[indexScript].lenIndexList[type];
        while (i < ie)
        {
            switch (type)
            {
            case TYPE_TRIGGER_TIME:
                config = events.trigTimeList[events.scriptsList[indexScript].indexList[i]].config;
                break;
            case TYPE_TRIGGER_PERIOD:
                config = events.trigPeriodList[events.scriptsList[indexScript].indexList[i]].config;
                break;
            case TYPE_TRIGGER_SIGNAL:
                config = events.trigSignalList[events.scriptsList[indexScript].indexList[i]].config;
                break;
            default:
                break;
            }
            if (!(config & MASK_TRUE))
            {
                events.scriptsList[indexScript].config &= MASK_FALSE;
                return;
            }
            ++i;
        }
    }
    events.scriptsList[indexScript].config |= MASK_TRUE;
}
void allowActions(uint16_t indexScript)
{
    // activates all activators according to the specified script
    uint16_t i = events_getScriptTypeIndex(indexScript, TYPE_ACTION_KEY);
    uint16_t ie = i;
    ie += events.scriptsList[indexScript].lenIndexList[TYPE_ACTION_KEY];
    while (i < ie)
    {
        events.actKeyList[events.scriptsList[indexScript].indexList[i]].config |= MASK_TRUE;
        i++;
    }
}
void events_callScripts()
{
    // calls the script checker
    // delete position switch:
    for (uint16_t indexAction = 0; indexAction < events.lenActKey; indexAction++)
    {
        events.actKeyList[indexAction].config &= MASK_FALSE;
    }

    // check scripts and activate actions:
    for (uint16_t indexScript = 0; indexScript < events.lenScripts; indexScript++)
    {
        checkTriggers(indexScript);
        if (events.scriptsList[indexScript].config & 1)
            allowActions(indexScript);
    }
}
