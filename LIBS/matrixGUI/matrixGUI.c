#include "matrixGUI.h"
#include "esp_log.h"
#include <math.h>
#include <stdio.h>

typedef struct
{
    typeLineGUI_t type;
} patternMap_t;
typedef struct
{
    patternMap_t *patternMap;
    uint16_t x;
    uint16_t y;
} elementMap_t;

// FUNCTIONS===================================================================
void matrixGUI_replaceString(matrixFont_t *matrixFont, uint16_t x, uint16_t y, char *text, char *preText, uint8_t *colorBackground)
{
    matrix_fontSelect(matrixFont);
    uint16_t lengthText = strlen(text);
    uint16_t lengthPreText = strlen(preText);
    // uint16_t textY = y + matrixFont->font->h - 1;
    uint16_t index = 0;
    for (; index < (min(lengthText, lengthPreText)); index++)
    {
        if (text[index] != preText[index])
        {
            matrix_drawFillRectWH(x, y, matrixFont->font->w, matrixFont->font->h, colorBackground);
            matrix_drawChar(x, y, (uint8_t)text[index]);
        }
        x += matrixFont->font->w;
    }
    if (lengthPreText > lengthText)
    {
        matrix_drawFillRectWH(x, y, (lengthPreText - lengthText) * matrixFont->font->w, y, colorBackground);
        // matrix_drawString(matrixGUI->matrixFont->font, cordX, cordY, (uint8_t *)&line->preText[lengthText], config->colorBackground);
    }
    else if (lengthPreText < lengthText)
    {
        matrix_drawString(x, y, (uint8_t *)&text[lengthPreText]);
    }
}
void matrixGUI_drawSign(uint16_t xc, uint16_t yc, int16_t rh, uint8_t *colorSign)
{
    int16_t x = xc - rh;
    int16_t y1 = yc - rh;
    int16_t y2 = yc + rh;
    matrix_drawLine(xc, yc, x, y1, colorSign);
    matrix_drawLine(xc, yc, x, y2, colorSign);
}
uint16_t matrixGUI_buildSimbolList(char **list, char *form)
{
    uint16_t length = strlen(form);
    uint16_t index = 0;
    // get count allow simbols:
    uint16_t count = 0;
    while (index < length)
    {
        if (form[index + 1] == '-')
        {
            count += (form[index + 2] - form[index]) + 1;
            index += 3;
        }
        else
        {
            count++;
            index++;
        }
    }
    *list = malloc(count);
    uint16_t indexChar = 0;
    index = 0;
    while (index < length)
    {
        if (form[index + 1] == '-')
        {
            count = (form[index + 2] - form[index]) + form[index];
            for (uint16_t ch = form[index]; ch <= count; ch++)
            {
                (*list)[indexChar] = ch;
                indexChar++;
            }
            index += 3;
        }
        else
        {
            (*list)[indexChar] = form[index];
            indexChar++;
            index++;
        }
    }
    return indexChar;
}
uint16_t matrixGUI_pow(uint16_t val, uint8_t degree)
{
    uint16_t out = degree ? val : 1;
    for (uint16_t i = 1; i < degree; i++)
    {
        out *= val;
    }
    return out;
}
void matrixGUI_strToVal(uint64_t *value, const char *str, uint8_t count)
{
    uint64_t i = 1;
    while (count > 0)
    {
        *value += (str[--count] - '0') * i;
        i *= 10;
    }
}
void matrixGUI_valToStr(uint64_t value, char *str, uint8_t count)
{
    while (count > 0)
    {
        str[--count] = value % 10 + '0';
        value /= 10;
    }
}
// TEXT========================================================================
patternTextGUI_t *matrixGUI_newPatternText(uint8_t *colorBackground, matrixFont_t *matrixFont)
{
    patternTextGUI_t *pattern = malloc(sizeof(patternTextGUI_t));
    pattern->_type = GUI_TEXT;
    pattern->colorBackground = colorBackground;
    pattern->matrixFont = matrixFont;
    return pattern;
}
textGUI_t *matrixGUI_newText(patternTextGUI_t *patternTextGUI, char *text, uint16_t x, uint16_t y)
{
    textGUI_t *line = malloc(sizeof(textGUI_t));
    line->patternTextGUI = patternTextGUI;
    line->text = text;
    line->x = x;
    line->y = y;
    return line;
}
void matrixGUI_buildText(textGUI_t *line)
{
    matrix_fontSelect(line->patternTextGUI->matrixFont);
    matrix_drawString(line->x, line->y, (uint8_t *)line->text);
    line->_preText = malloc(sizeof(line->text));
    strcpy(line->_preText, line->text);
}
void matrixGUI_updateText(textGUI_t *line)
{
    if (strcmp(line->text, line->_preText))
    {
        matrixGUI_replaceString(line->patternTextGUI->matrixFont, line->x, line->y, line->text, line->_preText, line->patternTextGUI->colorBackground);
        free(line->_preText);
        line->_preText = malloc(sizeof(line->text));
        strcpy(line->_preText, line->text);
    }
}
void matrixGUI_deleteText(textGUI_t *line)
{
    uint16_t lengthText = strlen(line->_preText);
    matrix_drawFillRectWH(line->x, line->y, (lengthText * line->patternTextGUI->matrixFont->font->w), line->patternTextGUI->matrixFont->font->h, line->patternTextGUI->colorBackground);
    // ESP_LOGI(__FUNCTION__, "x = %u, y = %u, lw = %u, h = %u", line->x, line->y, (lengthText * line->patternTextGUI->matrixFont->font->w), line->patternTextGUI->matrixFont->font->h);
    free(line->_preText);
}
// TEXT_EDIT===================================================================
patternTextEditGUI_t *matrixGUI_newPatternTextEdit(char *simbolList, uint16_t sizeSimbolList, uint8_t *colorBackground, uint8_t *colorSign, matrixFont_t *matrixFont)
{
    patternTextEditGUI_t *pattern = malloc(sizeof(patternTextEditGUI_t));
    pattern->_type = GUI_TEXTEDIT;
    pattern->simbolList = simbolList;
    pattern->sizeSimbolList = sizeSimbolList;
    pattern->colorBackground = colorBackground;
    pattern->colorSign = colorSign;
    pattern->matrixFont = matrixFont;
    return pattern;
}
textEditGUI_t *matrixGUI_newTextEdit(patternTextEditGUI_t *patternTextEditGUI, char *text, uint8_t sign, int16_t indexSegment, uint16_t x, uint16_t y)
{
    textEditGUI_t *line = malloc(sizeof(textEditGUI_t));
    line->patternTextEditGUI = patternTextEditGUI;
    line->text = text;
    line->indexSegment = indexSegment;
    line->sign = sign;
    line->x = x;
    line->y = y;
    return line;
}
void matrixGUI_buildTextEdit(textEditGUI_t *line)
{
    uint16_t lt = strlen(line->text);

    line->indexSegment %= lt;
    line->indexSegment = line->indexSegment < 0 ? lt + line->indexSegment : line->indexSegment;

    matrixFont_t *matrixFont = line->patternTextEditGUI->matrixFont;
    matrix_fontSelect(matrixFont);

    line->_code = malloc(lt);
    memset(line->_code, 0, lt);
    for (uint16_t indexSegment = 0; indexSegment < lt; indexSegment++)
    {
        for (uint16_t i = 0; i < line->patternTextEditGUI->sizeSimbolList; i++)
        {
            if (line->patternTextEditGUI->simbolList[i] == line->text[indexSegment])
            {
                line->_code[indexSegment] = i;
            }
        }
        line->text[indexSegment] = line->patternTextEditGUI->simbolList[line->_code[indexSegment]];
    }
    line->index = line->_code[line->indexSegment];

    uint16_t x = line->x;
    uint16_t i = 0;
    while (i < line->indexSegment)
    {
        matrix_drawChar(x, line->y, (uint8_t)line->text[i]);
        x += matrixFont->font->w;
        ++i;
    }
    if (line->sign)
    {
        uint8_t fill = matrixFont->fill;
        uint8_t *fillColor = matrixFont->fillColor;
        matrixFont->fill = 1;
        matrixFont->fillColor = line->patternTextEditGUI->colorSign;
        matrix_drawChar(x, line->y, (uint8_t)line->text[line->indexSegment]);
        matrixFont->fill = fill;
        matrixFont->fillColor = fillColor;
    }
    else
    {
        matrix_drawChar(x, line->y, (uint8_t)line->text[line->indexSegment]);
    }
    x += matrixFont->font->w;
    ++i;
    while (i < lt)
    {
        matrix_drawChar(x, line->y, (uint8_t)line->text[i]);
        x += matrixFont->font->w;
        ++i;
    }
    line->_preIndexSegment = line->indexSegment;
    line->_preSign = line->sign;
}
void matrixGUI_updateTextEdit(textEditGUI_t *line)
{
    uint16_t lt = strlen(line->text);

    line->indexSegment %= lt;
    line->indexSegment = line->indexSegment < 0 ? lt + line->indexSegment : line->indexSegment;

    line->index %= line->patternTextEditGUI->sizeSimbolList;
    line->index = line->index < 0 ? line->patternTextEditGUI->sizeSimbolList + line->index : line->index;

    uint8_t f1 = line->_code[line->_preIndexSegment] != line->index;
    uint8_t f2 = line->sign != line->_preSign;
    uint8_t f3 = line->indexSegment != line->_preIndexSegment;
    if (f1 || f2 || f3)
    {
        matrixFont_t *matrixFont = line->patternTextEditGUI->matrixFont;
        matrix_fontSelect(matrixFont);
        uint8_t fill = matrixFont->fill;
        uint8_t *fillColor = matrixFont->fillColor;
        matrixFont->fill = 1;

        uint8_t *colorSign = line->sign ? line->patternTextEditGUI->colorSign : line->patternTextEditGUI->colorBackground;

        if ((!f3 && f2) || f1 || (f3 && line->_preSign))
        {
            line->_code[line->_preIndexSegment] = line->index;
            line->text[line->_preIndexSegment] = line->patternTextEditGUI->simbolList[line->index];
            matrixFont->fillColor = f3 ? line->patternTextEditGUI->colorBackground : colorSign;
            matrix_drawChar(line->x + line->_preIndexSegment * matrixFont->font->w, line->y, (uint8_t)line->text[line->_preIndexSegment]);
        }

        if (f3)
        {
            if (line->sign)
            {
                matrixFont->fillColor = line->patternTextEditGUI->colorSign;
                matrix_drawChar(line->x + line->indexSegment * matrixFont->font->w, line->y, (uint8_t)line->text[line->indexSegment]);
            }
            line->index = line->_code[line->indexSegment];
            line->_preIndexSegment = line->indexSegment;
        }
        line->_preSign = line->sign;
        matrixFont->fill = fill;
        matrixFont->fillColor = fillColor;
    }
}
void matrixGUI_deleteTextEdit(textEditGUI_t *line)
{
    uint16_t lt = strlen(line->text);
    matrix_drawFillRectWH(line->x, line->y, lt * line->patternTextEditGUI->matrixFont->font->w, line->patternTextEditGUI->matrixFont->font->h, line->patternTextEditGUI->colorBackground);
    free(line->_code);
}
// VALUE_EDIT===================================================================
patternValueEditGUI_t *matrixGUI_newPatternValueEdit(uint8_t *colorBackground, uint8_t *colorSign, matrixFont_t *matrixFont)
{
    patternValueEditGUI_t *pattern = malloc(sizeof(patternValueEditGUI_t));
    pattern->_type = GUI_VALUEEDIT;
    pattern->colorBackground = colorBackground;
    pattern->colorSign = colorSign;
    pattern->matrixFont = matrixFont;
    return pattern;
}
valueEditGUI_t *matrixGUI_newValueEdit(patternValueEditGUI_t *patternValueEditGUI, char *text, char *maxValue, char *minValue, uint8_t sign, int8_t indexSegment, uint16_t x, uint16_t y)
{
    valueEditGUI_t *line = malloc(sizeof(valueEditGUI_t));
    line->patternValueEditGUI = patternValueEditGUI;
    line->x = x;
    line->y = y;
    line->text = text;
    line->sign = sign;
    line->indexSegment = indexSegment;
    line->maxValue = maxValue;
    line->minValue = minValue;
    return line;
}
void matrixGUI_buildValueEdit(valueEditGUI_t *line)
{
    // ___________________________________DATA:___________________________________
    uint16_t lt = strlen(line->maxValue);

    line->indexSegment %= lt;
    line->indexSegment = line->indexSegment < 0 ? line->indexSegment + lt : line->indexSegment;

    line->_preIndexSegment = line->indexSegment;

    uint16_t i;
    // uint8_t f1 = line->_preIndexSegment < lt;
    //  ___________________________________MAX:___________________________________
    i = 0;
    while (i < lt)
    {
        if (line->maxValue[i] < line->text[i])
        {
            line->text[i] = line->maxValue[i];
        }
        else if (line->maxValue[i] != line->text[i])
        {
            break;
        }
        ++i;
    }
    // ___________________________________MIN:___________________________________
    i = 0;
    while (i < lt)
    {
        if (line->minValue[i] > line->text[i])
        {
            line->text[i] = line->minValue[i];
        }
        else if (line->maxValue[i] != line->text[i])
        {
            break;
        }
        ++i;
    }
    // ___________________________________PROCESS:___________________________________
    line->index = line->text[line->_preIndexSegment] - '0';
    if (line->sign)
    {
        matrix_drawFillRectWH(line->x + line->patternValueEditGUI->matrixFont->font->w * line->_preIndexSegment, line->y, line->patternValueEditGUI->matrixFont->font->w, line->patternValueEditGUI->matrixFont->font->h, line->patternValueEditGUI->colorSign);
    }
    matrix_fontSelect(line->patternValueEditGUI->matrixFont);
    matrix_drawString(line->x, line->y, (uint8_t *)line->text);
    line->_preSign = line->sign;
}
void matrixGUI_updateValueEdit(valueEditGUI_t *line)
{
    // ___________________________________DATA:___________________________________
    uint16_t lt = strlen(line->maxValue);

    line->indexSegment %= lt;
    line->indexSegment = line->indexSegment < 0 ? line->indexSegment + lt : line->indexSegment;

    uint16_t i;
    uint8_t f1 = line->indexSegment != line->_preIndexSegment;
    uint8_t f2 = line->sign != line->_preSign;
    // ___________________________________MAX:___________________________________
    char max = line->maxValue[line->_preIndexSegment];
    i = 0;
    while (i < line->_preIndexSegment)
    {
        if (line->maxValue[i] < line->text[i])
        {
            line->text[i] = line->maxValue[i];
        }
        else if (line->maxValue[i] != line->text[i])
        {
            max = '9';
            break;
        }
        ++i;
    }
    if (i == line->_preIndexSegment)
    {
        ++i;
        while (i < lt)
        {
            if (line->text[i] > line->maxValue[i])
            {
                --max;
                break;
            }
            else if (line->text[i] < line->maxValue[i])
            {
                break;
            }
            ++i;
        }
    }

    // ___________________________________MIN:___________________________________
    char min = line->minValue[line->_preIndexSegment];
    i = 0;
    while (i < line->_preIndexSegment)
    {
        if (line->minValue[i] > line->text[i])
        {
            line->text[i] = line->minValue[i];
        }
        else if (line->minValue[i] != line->text[i])
        {
            min = '0';
            break;
        }
        ++i;
    }
    if (i == line->_preIndexSegment)
    {
        ++i;
        while (i < lt)
        {
            if (line->text[i] < line->minValue[i])
            {
                ++min;
                break;
            }
            else if (line->text[i] > line->minValue[i])
            {
                break;
            }
            ++i;
        }
    }

    // ___________________________________PROCESS:___________________________________
    uint8_t mni = min - '0';
    uint8_t mxi = max - '0';
    line->index -= mni;
    i = (mxi - mni) + 1;
    line->index %= i;
    line->index = line->index < 0 ? line->index + i : line->index;
    line->index += mni;

    i = line->index + '0';

    ESP_LOGI(__FUNCTION__, "index(%i), char(%c), max(%c), min(%c)", line->index, i, max, min);

    uint8_t f3 = i != line->text[line->_preIndexSegment];

    if (f3 || f1 || f2)
    {
        matrixFont_t *matrixFont = line->patternValueEditGUI->matrixFont;
        matrix_fontSelect(matrixFont);

        uint8_t fill = matrixFont->fill;
        uint8_t *fillColor = matrixFont->fillColor;
        matrixFont->fill = 1;
        if ((!f1 && f2) || (f1 && line->_preSign) || f3)
        {
            uint8_t *colorSign = line->sign ? line->patternValueEditGUI->colorSign : line->patternValueEditGUI->colorBackground;
            matrixFont->fillColor = f1 ? line->patternValueEditGUI->colorBackground : colorSign;
            line->text[line->_preIndexSegment] = i;
            matrix_drawChar(line->x + line->_preIndexSegment * matrixFont->font->w, line->y, (uint8_t)line->text[line->_preIndexSegment]);
        }
        if (f1)
        {
            if (line->sign)
            {
                matrixFont->fillColor = line->patternValueEditGUI->colorSign;
                matrix_drawChar(line->x + line->indexSegment * matrixFont->font->w, line->y, (uint8_t)line->text[line->indexSegment]);
            }
            line->index = line->text[line->indexSegment] - '0';
            line->_preIndexSegment = line->indexSegment;
        }
        matrixFont->fill = fill;
        matrixFont->fillColor = fillColor;
        line->_preSign = line->sign;
    }
}
void matrixGUI_deleteValueEdit(valueEditGUI_t *line)
{
    uint16_t lt = strlen(line->maxValue);
    matrix_drawFillRectWH(line->x, line->y, line->patternValueEditGUI->matrixFont->font->w * lt, line->patternValueEditGUI->matrixFont->font->h, line->patternValueEditGUI->colorBackground);
}
// TIMER_EDIT===================================================================
patternTimerEditGUI_t *matrixGUI_newPatternTimerEdit(char *formTimer, uint8_t *colorBackground, uint8_t *colorSign, matrixFont_t *matrixFont)
{
    patternTimerEditGUI_t *pattern = malloc(sizeof(patternTimerEditGUI_t));
    pattern->_type = GUI_TIMEREDIT;
    pattern->colorBackground = colorBackground;
    pattern->colorSign = colorSign;
    pattern->matrixFont = matrixFont;
    pattern->formTimer = formTimer;
    pattern->_countSeparators = 0;
    uint16_t lt = strlen(formTimer);
    for (uint16_t i = 0; i < lt; i++)
    {
        pattern->_countSeparators += (formTimer[i] > '9') || (formTimer[i] < '0');
    }
    return pattern;
}
timerEditGUI_t *matrixGUI_newTimerEdit(patternTimerEditGUI_t *patternTimerEditGUI, char *text, uint8_t sign, int8_t indexSegment, uint16_t x, uint16_t y)
{
    timerEditGUI_t *line = malloc(sizeof(timerEditGUI_t));
    line->patternTimerEditGUI = patternTimerEditGUI;
    line->text = text;
    line->sign = sign;
    line->indexSegment = indexSegment;
    line->x = x;
    line->x = y;
    return line;
}
void matrixGUI_buildTimerEdit(timerEditGUI_t *line)
{
    matrixFont_t *matrixFont = line->patternTimerEditGUI->matrixFont;
    matrix_fontSelect(matrixFont);

    uint16_t lt = strlen(line->patternTimerEditGUI->formTimer);
    line->indexSegment %= lt - line->patternTimerEditGUI->_countSeparators;
    line->indexSegment = line->indexSegment < 0 ? lt - line->patternTimerEditGUI->_countSeparators + line->indexSegment : line->indexSegment;
    line->_preIndexSegment = line->indexSegment;
    line->_preSign = line->sign;

    uint16_t l = 0, i = 0;
    line->_preIndexChar = lt;
    while (l < lt)
    {
        while ((line->patternTimerEditGUI->formTimer[l] <= '9') && (line->patternTimerEditGUI->formTimer[l] >= '0'))
        {
            if (line->text[l] > line->patternTimerEditGUI->formTimer[l])
            {
                line->text[l] = ((line->text[l] - '0') % (line->patternTimerEditGUI->formTimer[l] - '0' + 1)) + '0';
                // line->text[l] = line->patternTimerEditGUI->formTimer[l];
            }
            if (line->text[l] != line->patternTimerEditGUI->formTimer[l])
            {
                break;
            }
            ++l;
        }
        while ((line->patternTimerEditGUI->formTimer[l] <= '9') && (line->patternTimerEditGUI->formTimer[l] >= '0'))
        {
            ++l;
        }
        line->text[l] = line->patternTimerEditGUI->formTimer[l];
        if (line->indexSegment < (l - i) && line->_preIndexChar == lt)
        {
            line->_preIndexChar = line->indexSegment + i;
            uint8_t *colorSign = line->sign ? line->patternTimerEditGUI->colorSign : line->patternTimerEditGUI->colorBackground;
            matrix_drawFillRectWH(line->x + line->_preIndexChar * matrixFont->font->w, line->y, matrixFont->font->w, matrixFont->font->h, colorSign);
            line->index = line->text[line->_preIndexChar] - '0';
        }
        ++l;
        ++i;
    }
    matrix_drawString(line->x, line->y, (uint8_t *)line->text);
}
void matrixGUI_updateTimerEdit(timerEditGUI_t *line)
{
    uint16_t lt = strlen(line->patternTimerEditGUI->formTimer);
    line->indexSegment %= lt - line->patternTimerEditGUI->_countSeparators;
    line->indexSegment = line->indexSegment < 0 ? lt - line->patternTimerEditGUI->_countSeparators + line->indexSegment : line->indexSegment;

    char *timer = line->patternTimerEditGUI->formTimer;
    uint8_t f1 = line->sign != line->_preSign;
    uint8_t f2 = line->indexSegment != line->_preIndexSegment;

    char ch = line->index + '0';
    if (ch != line->text[line->_preIndexChar])
    {
        char max = timer[line->_preIndexChar];
        uint16_t l = line->_preIndexChar;
        while (l != 0 && (timer[--l] <= '9') && (timer[l] >= '0'))
        {
            if (line->text[l] < timer[l])
            {
                max = '9';
                l = line->_preIndexChar;
                break;
            }
        }
        if (l == line->_preIndexChar)
        {
            l = line->_preIndexChar + 1;
            while ((timer[l] <= '9') && (timer[l] >= '0'))
            {
                if (line->text[l] > timer[l])
                {
                    max = timer[line->_preIndexChar] - 1;
                    break;
                }
                else if (line->text[l] != timer[l])
                {
                    break;
                }
                ++l;
            }
        }
        l = max - '0' + 1;
        line->index %= l;
        line->index = line->index < 0 ? line->index + l : line->index;
        ch = line->index + '0';
    }

    uint8_t f3 = ch != line->text[line->_preIndexChar];

    if (f3 || f1 || f2)
    {
        matrixFont_t *matrixFont = line->patternTimerEditGUI->matrixFont;
        matrix_fontSelect(matrixFont);

        uint8_t fill = matrixFont->fill;
        uint8_t *fillColor = matrixFont->fillColor;
        uint8_t fillRadius = matrixFont->fillRadius;
        matrixFont->fill = 1;
        matrixFont->fillRadius = 0;
        if ((!f2 && f1) || (f2 && line->_preSign) || f3)
        {
            uint8_t *colorSign = line->sign ? line->patternTimerEditGUI->colorSign : line->patternTimerEditGUI->colorBackground;
            matrixFont->fillColor = f2 ? line->patternTimerEditGUI->colorBackground : colorSign;
            line->text[line->_preIndexChar] = ch;
            matrix_drawChar(line->x + line->_preIndexChar * matrixFont->font->w, line->y, (uint8_t)line->text[line->_preIndexChar]);
        }
        if (f2)
        {
            uint16_t l = 0, i = 0;
            while (1)
            {
                if ((line->text[l] >= '0') && (line->text[l] <= '9'))
                {
                    if (i == line->indexSegment)
                    {
                        line->_preIndexChar = l;
                        break;
                    }
                    ++i;
                }
                ++l;
            }
            if (line->sign)
            {
                matrixFont->fillColor = line->patternTimerEditGUI->colorSign;
                matrix_drawChar(line->x + line->_preIndexChar * matrixFont->font->w, line->y, (uint8_t)line->text[line->_preIndexChar]);
            }
            line->index = line->text[line->_preIndexChar] - '0';
            line->_preIndexSegment = line->indexSegment;
        }
        matrixFont->fill = fill;
        matrixFont->fillColor = fillColor;
        matrixFont->fillRadius = fillRadius;
        line->_preSign = line->sign;
    }
}
void matrixGUI_deleteTimerEdit(timerEditGUI_t *line)
{
    uint16_t lt = strlen(line->patternTimerEditGUI->formTimer);
    matrix_drawFillRectWH(line->x, line->y, line->patternTimerEditGUI->matrixFont->font->w * lt, line->patternTimerEditGUI->matrixFont->font->h, line->patternTimerEditGUI->colorBackground);
}
// SLIDER======================================================================
patternSliderGUI_t *matrixGUI_newPatternSlider(uint8_t *colorBackground, uint8_t *colorFrame, uint8_t *colorTrue, uint8_t *colorFalse)
{
    patternSliderGUI_t *pattern = malloc(sizeof(patternSliderGUI_t));
    pattern->_type = GUI_SLIDER;
    pattern->colorBackground = colorBackground;
    pattern->colorFrame = colorFrame;
    pattern->colorTrue = colorTrue;
    pattern->colorFalse = colorFalse;
    return pattern;
}
sliderGUI_t *matrixGUI_newSlider(patternSliderGUI_t *patternSliderGUI, uint16_t width, uint16_t height, uint16_t maxCountParts, int16_t index, uint16_t x, uint16_t y)
{
    sliderGUI_t *line = malloc(sizeof(sliderGUI_t));
    line->patternSliderGUI = patternSliderGUI;
    line->height = height;
    line->width = width;
    line->maxCountPart = maxCountParts;
    line->index = index;
    line->x = x;
    line->y = y;
    return line;
}
void matrixGUI_buildSlider(sliderGUI_t *line)
{
    line->index %= (line->maxCountPart + 1);
    line->index = line->index < 0 ? (line->maxCountPart + 1) + line->index : line->index;

    matrix_drawRectWH(line->x, line->y, line->width, line->height, line->patternSliderGUI->colorFrame);
    uint16_t insideHeight = line->height - 2;
    uint16_t insideWidth = line->width - 2;
    float widthPart = (float)insideWidth / line->maxCountPart;
    uint16_t y = line->y + 1;
    uint16_t x = line->x + 1;
    uint16_t w = ceil(line->index * widthPart);
    matrix_drawFillRectWH(x, y, w, insideHeight, line->patternSliderGUI->colorTrue);
    x += w;
    w = insideWidth - w;
    matrix_drawFillRectWH(x, y, w, insideHeight, line->patternSliderGUI->colorFalse);
    line->_preIndex = line->index;
}
void matrixGUI_updateSlider(sliderGUI_t *line)
{
    line->index %= (line->maxCountPart + 1);
    line->index = line->index < 0 ? (line->maxCountPart + 1) + line->index : line->index;

    if (line->index != line->_preIndex)
    {
        float pixelPart = (line->width - 2) / line->maxCountPart;
        uint16_t preWidthPart = pixelPart * line->_preIndex;
        uint16_t widthPart = pixelPart * line->index;
        if (widthPart < preWidthPart)
        {
            matrix_drawFillRectWH(line->x + widthPart + 2, line->y + 1, preWidthPart, line->height - 2, line->patternSliderGUI->colorFalse);
        }
        else if (widthPart > preWidthPart)
        {
            matrix_drawFillRectWH(line->x + preWidthPart + 2, line->y + 1, widthPart, line->height - 2, line->patternSliderGUI->colorTrue);
        }
        line->_preIndex = line->index;
    }
}
void matrixGUI_deleteSlider(sliderGUI_t *line)
{
    matrix_drawFillRectWH(line->x, line->y, line->width, line->height, line->patternSliderGUI->colorBackground);
}
// CHOICE=======================================================================
patternChoiceGUI_t *matrixGUI_newPatternChoice(uint8_t *colorBackground, uint8_t *colorSign, matrixFont_t *matrixFont)
{
    patternChoiceGUI_t *pattern = malloc(sizeof(patternChoiceGUI_t));
    pattern->_type = GUI_CHOICE;
    pattern->colorBackground = colorBackground;
    pattern->colorSign = colorSign;
    pattern->matrixFont = matrixFont;
    return pattern;
}
choiceGUI_t *matrixGUI_newChoice(patternChoiceGUI_t *patternChoiceGUI, char **variants, uint16_t countVariants, uint16_t widthField, uint16_t sizeSign, int16_t index, uint16_t x, uint16_t y)
{
    choiceGUI_t *line = malloc(sizeof(choiceGUI_t));
    line->patternChoiceGUI = patternChoiceGUI;
    line->index = index;
    line->variants = variants;
    line->countVariants = countVariants;
    line->widthField = widthField;
    line->sizeSign = sizeSign;
    line->x = x;
    line->y = y;
    return line;
}
void matrixGUI_buildChoice(choiceGUI_t *line)
{
    line->index %= line->countVariants;
    line->index = line->index < 0 ? line->countVariants + line->index : line->index;

    matrixFont_t *matrixFont = line->patternChoiceGUI->matrixFont;
    matrix_fontSelect(matrixFont);

    char *variant = line->variants[line->index];
    uint16_t indent = (line->widthField - (strlen(variant) * matrixFont->font->w)) / 2;
    uint16_t x = line->x;
    matrixGUI_drawSign(x, line->y + (matrixFont->font->h / 2), -line->sizeSign, line->patternChoiceGUI->colorSign);
    x += line->sizeSign + 1;
    matrix_drawString(x + indent, line->y, (uint8_t *)variant);
    x += line->widthField + line->sizeSign;
    matrixGUI_drawSign(x, line->y + (matrixFont->font->h / 2), line->sizeSign, line->patternChoiceGUI->colorSign);
    line->_preIndex = line->index;
}
void matrixGUI_updateChoice(choiceGUI_t *line)
{
    line->index %= line->countVariants;
    line->index = line->index < 0 ? line->countVariants + line->index : line->index;

    if (line->index != line->_preIndex)
    {
        matrixFont_t *matrixFont = line->patternChoiceGUI->matrixFont;
        matrix_fontSelect(matrixFont);

        char *variant = line->variants[line->_preIndex];
        uint16_t lt = strlen(variant);
        uint16_t indent = (line->widthField - (lt * matrixFont->font->w)) / 2;
        uint16_t x = line->x + line->sizeSign + 1;

        matrix_drawFillRectWH(x + indent, line->y, lt * matrixFont->font->w, matrixFont->font->h, line->patternChoiceGUI->colorBackground);

        variant = line->variants[line->index];
        lt = strlen(variant);
        indent = (line->widthField - (lt * matrixFont->font->w)) / 2;
        matrix_drawString(x + indent, line->y, (uint8_t *)variant);

        line->_preIndex = line->index;
    }
}
void matrixGUI_deleteChoice(choiceGUI_t *line)
{
    matrixFont_t *matrixFont = line->patternChoiceGUI->matrixFont;

    char *variant = line->variants[line->_preIndex];
    uint16_t lt = strlen(variant);
    uint16_t indent = (line->widthField - (lt * line->patternChoiceGUI->matrixFont->font->w)) / 2;

    uint16_t x = line->x;
    matrixGUI_drawSign(x, line->y + (matrixFont->font->h / 2), -line->sizeSign, line->patternChoiceGUI->colorBackground);
    x += line->sizeSign + 1;
    matrix_drawFillRectWH(x + indent, line->y, matrixFont->font->w * lt, matrixFont->font->h, line->patternChoiceGUI->colorBackground);
    x += line->widthField + line->sizeSign;
    matrixGUI_drawSign(x, line->y + (matrixFont->font->h / 2), line->sizeSign, line->patternChoiceGUI->colorBackground);
    line->_preIndex = line->index;
}
// CHOICE_LINE==================================================================
patternChoiceLineGUI_t *matrixGUI_newPatternChoiceLine(uint16_t height, uint16_t widthSegment, uint16_t sizeSign, uint8_t **colorSegments, uint8_t *colorBackground, uint8_t *colorFrame, uint8_t *colorSign, matrixFont_t *matrixFont)
{
    patternChoiceLineGUI_t *pattern = malloc(sizeof(patternChoiceLineGUI_t));
    pattern->_type = GUI_CHOICELINE;
    pattern->colorSegments = colorSegments;
    pattern->colorFrame = colorFrame;
    pattern->colorSign = colorSign;
    pattern->matrixFont = matrixFont;
    pattern->widthSegment = widthSegment;
    pattern->height = height;
    pattern->sizeSign = sizeSign;
    pattern->colorBackground = colorBackground;
    return pattern;
}
choiceLineGUI_t *matrixGUI_newChoiceLine(patternChoiceLineGUI_t *patternChoiceLineGUI, char **variants, uint16_t countVariants, uint8_t *positions, uint8_t countPositions, uint8_t sign, int16_t indexVariant, uint16_t x, uint16_t y)
{
    choiceLineGUI_t *line = malloc(sizeof(choiceLineGUI_t));
    line->patternChoiceLineGUI = patternChoiceLineGUI;
    line->x = x;
    line->y = y;
    line->variants = variants;
    line->countVariants = countVariants;
    line->positions = positions;
    line->countPositions = countPositions;
    line->sign = sign;
    line->indexVariant = indexVariant;
    return line;
}
void matrixGUI_buildChoiceLine(choiceLineGUI_t *line)
{
    line->indexVariant %= line->countVariants;
    line->indexVariant = line->indexVariant < 0 ? line->indexVariant + line->countVariants : line->indexVariant;

    line->_preSign = line->sign;
    matrix_fontSelect(line->patternChoiceLineGUI->matrixFont);

    uint16_t w = line->patternChoiceLineGUI->widthSegment * line->countVariants + line->countVariants + 1;
    uint16_t x = line->x;
    uint16_t y = line->y + 1;
    uint16_t wc = line->patternChoiceLineGUI->widthSegment / 2;

    matrix_drawLineW(x, line->y, w, line->patternChoiceLineGUI->colorFrame);
    matrix_drawLineW(x, line->y + line->patternChoiceLineGUI->height + 1, w, line->patternChoiceLineGUI->colorFrame);
    matrix_drawLineH(x, y, line->patternChoiceLineGUI->height, line->patternChoiceLineGUI->colorFrame);

    uint16_t i = 0;
    while (i < line->indexVariant)
    {
        ++x;
        line->positions[i] %= line->countPositions;
        matrix_drawFillRectWH(x, y, line->patternChoiceLineGUI->widthSegment, line->patternChoiceLineGUI->height, line->patternChoiceLineGUI->colorSegments[line->positions[i]]);
        uint16_t twc = strlen(line->variants[i]) * line->patternChoiceLineGUI->matrixFont->font->w / 2;
        matrix_drawString(x + (wc - twc), y, (uint8_t *)line->variants[i]);
        x += line->patternChoiceLineGUI->widthSegment;
        matrix_drawLineH(x, y, line->patternChoiceLineGUI->height, line->patternChoiceLineGUI->colorFrame);
        ++i;
    }
    ++x;
    line->positions[i] %= line->countPositions;
    line->index = line->positions[i];
    if (line->sign)
    {
        matrix_drawFillRectWH(x, y, line->patternChoiceLineGUI->widthSegment, line->patternChoiceLineGUI->height - line->patternChoiceLineGUI->sizeSign, line->patternChoiceLineGUI->colorSegments[line->positions[i]]);
        matrix_drawFillRectWH(x, y + line->patternChoiceLineGUI->height - line->patternChoiceLineGUI->sizeSign, line->patternChoiceLineGUI->widthSegment, line->patternChoiceLineGUI->sizeSign, line->patternChoiceLineGUI->colorSign);
    }
    else
    {
        matrix_drawFillRectWH(x, y, line->patternChoiceLineGUI->widthSegment, line->patternChoiceLineGUI->height, line->patternChoiceLineGUI->colorSegments[line->positions[i]]);
    }
    uint16_t twc = strlen(line->variants[i]) * line->patternChoiceLineGUI->matrixFont->font->w / 2;
    matrix_drawString(x + (wc - twc), y, (uint8_t *)line->variants[i]);
    x += line->patternChoiceLineGUI->widthSegment;
    matrix_drawLineH(x, y, line->patternChoiceLineGUI->height, line->patternChoiceLineGUI->colorFrame);
    ++i;
    while (i < line->countVariants)
    {
        ++x;
        line->positions[i] %= line->countPositions;
        matrix_drawFillRectWH(x, y, line->patternChoiceLineGUI->widthSegment, line->patternChoiceLineGUI->height, line->patternChoiceLineGUI->colorSegments[line->positions[i]]);
        uint16_t twc = strlen(line->variants[i]) * line->patternChoiceLineGUI->matrixFont->font->w / 2;
        matrix_drawString(x + (wc - twc), y, (uint8_t *)line->variants[i]);
        x += line->patternChoiceLineGUI->widthSegment;
        matrix_drawLineH(x, y, line->patternChoiceLineGUI->height, line->patternChoiceLineGUI->colorFrame);
        ++i;
    }
    line->_preIndexVariant = line->indexVariant;
}
void matrixGUI_updateChoiceLine(choiceLineGUI_t *line)
{
    line->indexVariant %= line->countVariants;
    line->indexVariant = line->indexVariant < 0 ? line->indexVariant + line->countVariants : line->indexVariant;
    line->index %= line->countPositions;
    line->index = line->index < 0 ? line->index + line->countPositions : line->index;

    uint8_t f1 = line->sign != line->_preSign;
    uint8_t f2 = line->indexVariant != line->_preIndexVariant;
    uint8_t f3 = line->index != line->positions[line->_preIndexVariant];

    if (f1 || f2 || f3)
    {
        matrixFont_t *matrixFont = line->patternChoiceLineGUI->matrixFont;
        matrix_fontSelect(matrixFont);
        uint16_t x = line->x + line->patternChoiceLineGUI->widthSegment * line->_preIndexVariant + line->_preIndexVariant + 1;
        uint16_t y = line->y + 1;
        uint16_t wc = line->patternChoiceLineGUI->widthSegment / 2;
        line->positions[line->_preIndexVariant] = line->index;

        if (line->_preSign && f2)
        {
            matrix_drawFillRectWH(x, y + line->patternChoiceLineGUI->height - line->patternChoiceLineGUI->sizeSign, line->patternChoiceLineGUI->widthSegment, line->patternChoiceLineGUI->sizeSign, line->patternChoiceLineGUI->colorSegments[line->positions[line->_preIndexVariant]]);
        }
        else if (f1)
        {
            uint8_t *color = line->sign ? line->patternChoiceLineGUI->colorSign : line->patternChoiceLineGUI->colorSegments[line->positions[line->_preIndexVariant]];
            matrix_drawFillRectWH(x, y + line->patternChoiceLineGUI->height - line->patternChoiceLineGUI->sizeSign, line->patternChoiceLineGUI->widthSegment, line->patternChoiceLineGUI->sizeSign, color);
        }
        if (f3)
        {
            matrix_drawFillRectWH(x, y, line->patternChoiceLineGUI->widthSegment, line->patternChoiceLineGUI->height - line->patternChoiceLineGUI->sizeSign, line->patternChoiceLineGUI->colorSegments[line->positions[line->_preIndexVariant]]);
        }
        if ((line->_preSign && f2) || f1 || f3)
        {
            uint16_t twc = strlen(line->variants[line->_preIndexVariant]) * line->patternChoiceLineGUI->matrixFont->font->w / 2;
            matrix_drawString(x + wc - twc, y, (uint8_t *)line->variants[line->_preIndexVariant]);
        }

        if (f2)
        {
            if (line->sign)
            {
                x = line->x + line->patternChoiceLineGUI->widthSegment * line->indexVariant + line->indexVariant + 1;
                matrix_drawFillRectWH(x, y + line->patternChoiceLineGUI->height - line->patternChoiceLineGUI->sizeSign, line->patternChoiceLineGUI->widthSegment, line->patternChoiceLineGUI->sizeSign, line->patternChoiceLineGUI->colorSign);
                uint16_t twc = strlen(line->variants[line->indexVariant]) * line->patternChoiceLineGUI->matrixFont->font->w / 2;
                matrix_drawString(x + (wc - twc), y, (uint8_t *)line->variants[line->indexVariant]);
            }
            line->index = line->positions[line->indexVariant];
            line->_preIndexVariant = line->indexVariant;
        }
        line->_preSign = line->sign;
    }
}
void matrixGUI_deleteChoiceLine(choiceLineGUI_t *line)
{
    matrix_drawFillRectWH(line->x, line->y, line->countVariants * line->patternChoiceLineGUI->widthSegment + line->countVariants + 1, line->patternChoiceLineGUI->height + 2, line->patternChoiceLineGUI->colorBackground);
}
// CHOICE_STATUS================================================================
patternChoiceStatusGUI_t *matrixGUI_newPatternChoiceStatus(uint8_t raduis, uint8_t **colorStatuses, uint8_t countStatuses, uint8_t *colorBackground, matrixFont_t *matrixFont)
{
    patternChoiceStatusGUI_t *pattern = malloc(sizeof(patternChoiceStatusGUI_t));
    pattern->_type = GUI_CHOICESTATUS;
    pattern->radius = raduis;
    pattern->colorBackground = colorBackground;
    pattern->colorStatuses = colorStatuses;
    pattern->countStatuses = countStatuses;
    pattern->matrixFont = matrixFont;
    return pattern;
}
choiceStatusGUI_t *matrixGUI_newChoiceStatus(patternChoiceStatusGUI_t *patternChoiceStatusGUI, char *text, int8_t index, uint16_t x, uint16_t y)
{
    choiceStatusGUI_t *line = malloc(sizeof(choiceStatusGUI_t));
    line->patternChoiceStatusGUI = patternChoiceStatusGUI;
    line->x = x;
    line->y = y;
    line->index = index;
    line->text = text;
    return line;
}
void matrixGUI_buildChoiceStatus(choiceStatusGUI_t *line)
{
    line->index %= line->patternChoiceStatusGUI->countStatuses;
    line->index = line->index < 0 ? line->index + line->patternChoiceStatusGUI->countStatuses : line->index;

    matrixFont_t *matrixFont = line->patternChoiceStatusGUI->matrixFont;
    uint8_t *preFillColor = matrixFont->fillColor;
    uint8_t preRadius = matrixFont->fillRadius;
    uint8_t preFill = matrixFont->fill;
    matrixFont->fill = 1;
    matrixFont->fillColor = line->patternChoiceStatusGUI->colorStatuses[line->index];
    matrixFont->fillRadius = line->patternChoiceStatusGUI->radius;
    matrix_fontSelect(line->patternChoiceStatusGUI->matrixFont);
    matrix_drawString(line->x, line->y, (uint8_t *)line->text);
    matrixFont->fill = preFill;
    matrixFont->fillColor = preFillColor;
    matrixFont->fillRadius = preRadius;
    line->_preIndex = line->index;
}
void matrixGUI_updateChoiceStatus(choiceStatusGUI_t *line)
{
    line->index %= line->patternChoiceStatusGUI->countStatuses;
    line->index = line->index < 0 ? line->index + line->patternChoiceStatusGUI->countStatuses : line->index;

    if (line->index != line->_preIndex)
    {
        matrixFont_t *matrixFont = line->patternChoiceStatusGUI->matrixFont;
        uint8_t *preFillColor = matrixFont->fillColor;
        uint8_t preRadius = matrixFont->fillRadius;
        uint8_t preFill = matrixFont->fill;
        matrixFont->fill = 1;
        matrixFont->fillColor = line->patternChoiceStatusGUI->colorStatuses[line->index];
        matrixFont->fillRadius = line->patternChoiceStatusGUI->radius;
        matrix_fontSelect(line->patternChoiceStatusGUI->matrixFont);
        matrix_drawString(line->x, line->y, (uint8_t *)line->text);
        matrixFont->fill = preFill;
        matrixFont->fillColor = preFillColor;
        matrixFont->fillRadius = preRadius;
        line->_preIndex = line->index;
    }
}
void matrixGUI_deleteChoiceStatus(choiceStatusGUI_t *line)
{
    uint16_t lt = strlen(line->text);
    matrix_drawFillRectWH(line->x, line->y, lt * line->patternChoiceStatusGUI->matrixFont->font->w, line->patternChoiceStatusGUI->matrixFont->font->h, line->patternChoiceStatusGUI->colorBackground);
}
// SCROLLER=====================================================================
patternScrollerGUI_t *matrixGUI_newPatternScroller(uint8_t *colorBackground, uint8_t *colorFrame, uint8_t *colorTrue, uint8_t *colorFalse)
{
    patternScrollerGUI_t *pattern = malloc(sizeof(patternScrollerGUI_t));
    pattern->_type = GUI_SCROLLER;
    pattern->colorBackground = colorBackground;
    pattern->colorFrame = colorFrame;
    pattern->colorTrue = colorTrue;
    pattern->colorFalse = colorFalse;
    return pattern;
}
scrollerGUI_t *matrixGUI_newScroller(patternScrollerGUI_t *patternScrollerGUI, uint16_t width, uint16_t height, uint16_t countParts, uint16_t sizeGeneralPart, int16_t index, uint16_t x, uint16_t y)
{
    scrollerGUI_t *line = malloc(sizeof(scrollerGUI_t));
    line->patternScrollerGUI = patternScrollerGUI;
    line->countParts = countParts;
    line->sizeGeneralPart = sizeGeneralPart;
    line->index = index;
    line->x = x;
    line->y = y;
    line->width = width;
    line->height = height;
    return line;
}
void matrixGUI_buildScroller(scrollerGUI_t *line)
{
    line->index %= line->countParts;
    line->index = line->index < 0 ? line->index + line->countParts : line->index;

    matrix_drawRectWH(line->x, line->y, line->width, line->height, line->patternScrollerGUI->colorFrame);
    uint16_t insideHeight = line->height - 2;
    uint16_t insideWidth = line->width - 2;
    uint16_t countParts = line->countParts + line->sizeGeneralPart - 1;
    float sizePart = insideHeight / (float)countParts;
    uint16_t x = line->x + 1;
    uint16_t y = line->y + 1;
    uint16_t h;
    if (line->index)
    {
        h = sizePart * line->index;
        matrix_drawFillRectWH(x, y, insideWidth, h, line->patternScrollerGUI->colorFalse);
        y += h;
    }
    h = ceil(line->sizeGeneralPart * sizePart);
    matrix_drawFillRectWH(x, y, insideWidth, h, line->patternScrollerGUI->colorTrue);
    y += h;
    uint16_t diff = line->countParts - line->index - 1;
    if (diff)
    {
        h = diff * sizePart;
        matrix_drawFillRectWH(x, y, insideWidth, h, line->patternScrollerGUI->colorFalse);
    }
    line->_preIndex = line->index;
}
void matrixGUI_updateScroller(scrollerGUI_t *line)
{
    line->index %= line->countParts;
    line->index = line->index < 0 ? line->index + line->countParts : line->index;
    if (line->index != line->_preIndex)
    {
        uint16_t insideHeight = line->height - 2;
        uint16_t insideWidth = line->width - 2;
        uint16_t countParts = line->countParts + line->sizeGeneralPart - 1;
        float sizePart = insideHeight / (float)countParts;
        uint16_t x = line->x + 1;
        uint16_t y = line->y + 1;
        uint16_t h = ceil(sizePart * line->sizeGeneralPart);
        uint16_t y1 = y + (sizePart * line->index);
        uint16_t yh1 = y1 + h;
        uint16_t y2 = y + (sizePart * line->_preIndex);
        uint16_t yh2 = y2 + h;
        if (line->index > line->_preIndex)
        {
            if (y1 < yh2)
            {
                h = y1 - y2;
                y1 = yh2;
            }
            matrix_drawFillRectWH(x, y2, insideWidth, h, line->patternScrollerGUI->colorFalse);
            h = yh1 - y1;
            matrix_drawFillRectWH(x, y1, insideWidth, h, line->patternScrollerGUI->colorTrue);
        }
        else
        {
            if (yh1 > y2)
            {
                h = y2 - y1;
                y2 = yh1;
            }
            matrix_drawFillRectWH(x, y1, insideWidth, h, line->patternScrollerGUI->colorTrue);
            h = yh1 - y1;
            matrix_drawFillRectWH(x, y2, insideWidth, h, line->patternScrollerGUI->colorFalse);
        }
        line->_preIndex = line->index;
    }
}
void matrixGUI_deleteScroller(scrollerGUI_t *line)
{
    matrix_drawFillRectWH(line->x, line->y, line->width, line->height, line->patternScrollerGUI->colorBackground);
}
// SWITCHER=====================================================================
patternSwitcherGUI_t *matrixGUI_newPatternSwitcher()
{
    patternSwitcherGUI_t *pattern = malloc(sizeof(patternSwitcherGUI_t));
    pattern->_type = GUI_SWITCHER;
    return pattern;
}
switcherGUI_t *matrixGUI_newSwitcher(patternSwitcherGUI_t *patternSwitcherGUI, void **elements, uint16_t countElements, int16_t index, uint16_t x, uint16_t y)
{
    switcherGUI_t *line = malloc(sizeof(switcherGUI_t));
    line->elements = elements;
    line->countElements = countElements;
    line->patternSwitcherGUI = patternSwitcherGUI;
    line->index = index;
    line->x = x;
    line->y = y;
    return line;
}
void matrixGUI_buildSwitcher(switcherGUI_t *line)
{
    line->index %= line->countElements;
    line->index = line->index < 0 ? line->index + line->countElements : line->index;

    void *element = line->elements[line->index];
    ((elementMap_t *)element)->x = line->x;
    ((elementMap_t *)element)->y = line->y;
    matrixGUI_build(element);
    line->_preIndex = line->index;
}
void matrixGUI_updateSwitcher(switcherGUI_t *line)
{
    line->index %= line->countElements;
    line->index = line->index < 0 ? line->index + line->countElements : line->index;

    void *element = line->elements[line->index];
    if (line->index != line->_preIndex)
    {
        void *preElement = line->elements[line->_preIndex];
        ((elementMap_t *)element)->x = line->x;
        ((elementMap_t *)element)->y = line->y;
        matrixGUI_delete(preElement);
        matrixGUI_build(element);
        line->_preIndex = line->index;
    }
    else
    {
        matrixGUI_update(element);
    }
}
void matrixGUI_deleteSwitcher(switcherGUI_t *line)
{
    void *preElement = line->elements[line->_preIndex];
    matrixGUI_delete(preElement);
}
// GROUP========================================================================
patternGroupGUI_t *matrixGUI_newPatternGroup()
{
    patternGroupGUI_t *pattern = malloc(sizeof(patternGroupGUI_t));
    pattern->_type = GUI_GROUP;
    return pattern;
}
groupGUI_t *matrixGUI_newGroup(patternGroupGUI_t *patternGroupGUI, void *elements, uint16_t countElements)
{
    groupGUI_t *line = malloc(sizeof(groupGUI_t));
    line->patternGroupGUI = patternGroupGUI;
    line->elements = elements;
    line->countElements = countElements;
    return line;
}
void matrixGUI_buildGroup(groupGUI_t *line)
{
    for (uint16_t index = 0; index < line->countElements; index++)
    {
        ESP_LOGI(__FUNCTION__, "index(%u)", index);
        matrixGUI_build(line->elements[index]);
    }
}
void matrixGUI_updateGroup(groupGUI_t *line)
{
    for (uint16_t index = 0; index < line->countElements; index++)
    {
        matrixGUI_update(line->elements[index]);
    }
}
void matrixGUI_deleteGroup(groupGUI_t *line)
{
    for (uint16_t index = 0; index < line->countElements; index++)
    {
        matrixGUI_delete(line->elements[index]);
    }
}
// LIST=========================================================================
patternListGUI_t *matrixGUI_newPatternList(uint8_t *colorBackground, uint8_t *colorSign)
{
    patternListGUI_t *pattern = malloc(sizeof(patternListGUI_t));
    pattern->_type = GUI_LIST;
    pattern->colorBackground = colorBackground;
    pattern->colorSign = colorSign;
    return pattern;
}
listGUI_t *matrixGUI_newList(patternListGUI_t *patternListGUI, void **elements, uint16_t countElements, uint16_t heightElement, uint16_t sizeScreen, uint16_t sizeCursor, uint16_t setCursor, int16_t index, uint16_t x, uint16_t y)
{
    listGUI_t *line = malloc(sizeof(listGUI_t));
    line->patternListGUI = patternListGUI;
    line->elements = elements;
    line->countElements = countElements;
    line->heightElement = heightElement;
    line->sizeScreen = sizeScreen;
    line->sizeCursor = sizeCursor;
    line->setCursor = setCursor;
    line->index = index;
    line->x = x;
    line->y = y;
    return line;
}
void matrixGUI_buildList(listGUI_t *line)
{
    line->index %= line->countElements;
    line->index = line->index < 0 ? line->index + line->countElements : line->index;

    groupGUI_t *groupGUI = &line->_groupGUI;
    for (uint16_t index = 0; index < line->countElements; index++)
    {
        ((elementMap_t *)line->elements[index])->x = line->x;
    }

    uint16_t xc = line->x - 2;
    uint16_t yc = line->y + (line->heightElement * line->setCursor) + (line->heightElement / 2);
    matrixGUI_drawSign(xc, yc, line->sizeCursor, line->patternListGUI->colorSign);

    uint16_t maxCountUp = line->index;
    uint16_t maxCountDown = line->countElements - maxCountUp - 1;
    uint16_t countUp = maxCountUp < line->setCursor ? maxCountUp : line->setCursor;
    uint16_t countDown = maxCountDown < (line->sizeScreen - line->setCursor) ? maxCountDown : (line->sizeScreen - line->setCursor);
    groupGUI->elements = &line->elements[line->index - countUp];
    groupGUI->countElements = countUp + countDown + 1;
    uint16_t y = line->y + ((line->setCursor - countUp) * line->heightElement);
    for (uint16_t index = 0; index < groupGUI->countElements; index++)
    {
        ((elementMap_t *)groupGUI->elements[index])->y = y;
        y += line->heightElement;
    }
    matrixGUI_buildGroup(groupGUI);
    line->_preIndex = line->index;
}
void matrixGUI_updateList(listGUI_t *line)
{
    line->index %= line->countElements;
    line->index = line->index < 0 ? line->index + line->countElements : line->index;

    groupGUI_t *groupGUI = &line->_groupGUI;
    if (line->_preIndex != line->index)
    {
        matrixGUI_deleteGroup(groupGUI);
        uint16_t maxCountUp = line->index;
        uint16_t maxCountDown = line->countElements - maxCountUp - 1;
        uint16_t countUp = maxCountUp < line->setCursor ? maxCountUp : line->setCursor;
        uint16_t countDown = maxCountDown < (line->sizeScreen - line->setCursor) ? maxCountDown : (line->sizeScreen - line->setCursor);
        groupGUI->countElements = countUp + countDown + 1;
        uint16_t y = line->y + ((line->setCursor - countUp) * line->heightElement);
        groupGUI->elements = &line->elements[line->index - countUp];
        for (uint16_t index = 0; index < groupGUI->countElements; index++)
        {
            ((elementMap_t *)groupGUI->elements[index])->y = y;
            y += line->heightElement;
        }
        matrixGUI_buildGroup(groupGUI);
        line->_preIndex = line->index;
    }
    else
    {
        matrixGUI_updateGroup(groupGUI);
    }
}
void matrixGUI_deleteList(listGUI_t *line)
{
    matrixGUI_deleteGroup(&line->_groupGUI);
    matrix_drawFillRectWH(line->x - line->sizeCursor - 2, (line->y + (line->heightElement * line->setCursor) + (line->heightElement / 2)) - line->sizeCursor, line->sizeCursor + 1, line->sizeCursor + line->sizeCursor + 1, line->patternListGUI->colorBackground);
}
// CONNECTIONS==================================================================
patternConnectionGUI_t *matrixGUI_newPatternConnection()
{
    patternConnectionGUI_t *pattern = malloc(sizeof(patternConnectionGUI_t));
    pattern->_type = GUI_CONNECTION;
    return pattern;
}
connectionGUI_t *matrixGUI_newConnection(patternConnectionGUI_t *patternConnectionsGUI, void **elements, uint16_t countElements, uint16_t *shiftX, uint16_t *shiftY, uint16_t x, uint16_t y)
{
    connectionGUI_t *line = malloc(sizeof(connectionGUI_t));
    line->patternConnectionsGUI = patternConnectionsGUI;
    line->elements = elements;
    line->countElements = countElements;
    line->shiftsX = shiftX;
    line->shiftsY = shiftY;
    line->x = x;
    line->y = y;
    return line;
}
void matrixGUI_buildConnection(connectionGUI_t *line)
{
    for (uint16_t i = 0; i < line->countElements; i++)
    {
        ((elementMap_t *)line->elements[i])->x = line->x + line->shiftsX[i];
        ((elementMap_t *)line->elements[i])->y = line->y + line->shiftsY[i];
        matrixGUI_build(line->elements[i]);
    }
}
void matrixGUI_updateConnection(connectionGUI_t *line)
{
    for (uint16_t i = 0; i < line->countElements; i++)
    {
        ((elementMap_t *)line->elements[i])->x = line->x + line->shiftsX[i];
        ((elementMap_t *)line->elements[i])->y = line->y + line->shiftsY[i];
        matrixGUI_update(line->elements[i]);
    }
}
void matrixGUI_deleteConnection(connectionGUI_t *line)
{
    for (uint16_t i = 0; i < line->countElements; i++)
    {
        ((elementMap_t *)line->elements[i])->x = line->x + line->shiftsX[i];
        ((elementMap_t *)line->elements[i])->y = line->y + line->shiftsY[i];
        matrixGUI_delete(line->elements[i]);
    }
}
// UNIVERSAL====================================================================
void matrixGUI_build(void *element)
{
    typeLineGUI_t type = **((typeLineGUI_t **)element);
    switch (type)
    {
    case GUI_TEXT:
        matrixGUI_buildText(element);
        break;
    case GUI_TEXTEDIT:
        matrixGUI_buildTextEdit(element);
        break;
    case GUI_SLIDER:
        matrixGUI_buildSlider(element);
        break;
    case GUI_CHOICE:
        matrixGUI_buildChoice(element);
        break;
    case GUI_CHOICELINE:
        matrixGUI_buildChoiceLine(element);
        break;
    case GUI_CHOICESTATUS:
        matrixGUI_buildChoiceStatus(element);
        break;
    case GUI_SCROLLER:
        matrixGUI_buildScroller(element);
        break;
    case GUI_VALUEEDIT:
        matrixGUI_buildValueEdit(element);
        break;
    case GUI_TIMEREDIT:
        matrixGUI_buildTimerEdit(element);
        break;
    case GUI_GROUP:
        matrixGUI_buildGroup(element);
        break;
    case GUI_LIST:
        matrixGUI_buildList(element);
        break;
    case GUI_SWITCHER:
        matrixGUI_buildSwitcher(element);
        break;
    case GUI_CONNECTION:
        matrixGUI_buildConnection(element);
        break;
    default:
        break;
    }
}
void matrixGUI_update(void *element)
{
    typeLineGUI_t type = **((typeLineGUI_t **)element);
    switch (type)
    {
    case GUI_TEXT:
        matrixGUI_updateText(element);
        break;
    case GUI_TEXTEDIT:
        matrixGUI_updateTextEdit(element);
        break;
    case GUI_SLIDER:
        matrixGUI_updateSlider(element);
        break;
    case GUI_CHOICE:
        matrixGUI_updateChoice(element);
        break;
    case GUI_CHOICELINE:
        matrixGUI_updateChoiceLine(element);
        break;
    case GUI_CHOICESTATUS:
        matrixGUI_updateChoiceStatus(element);
        break;
    case GUI_SCROLLER:
        matrixGUI_updateScroller(element);
        break;
    case GUI_VALUEEDIT:
        matrixGUI_updateValueEdit(element);
        break;
    case GUI_TIMEREDIT:
        matrixGUI_updateTimerEdit(element);
        break;
    case GUI_GROUP:
        matrixGUI_updateGroup(element);
        break;
    case GUI_LIST:
        matrixGUI_updateList(element);
        break;
    case GUI_SWITCHER:
        matrixGUI_updateSwitcher(element);
        break;
    case GUI_CONNECTION:
        matrixGUI_updateConnection(element);
        break;
    default:
        break;
    }
}
void matrixGUI_delete(void *element)
{
    typeLineGUI_t type = **((typeLineGUI_t **)element);
    switch (type)
    {
    case GUI_TEXT:
        matrixGUI_deleteText(element);
        break;
    case GUI_TEXTEDIT:
        matrixGUI_deleteTextEdit(element);
        break;
    case GUI_SLIDER:
        matrixGUI_deleteSlider(element);
        break;
    case GUI_CHOICE:
        matrixGUI_deleteChoice(element);
        break;
    case GUI_CHOICELINE:
        matrixGUI_deleteChoiceLine(element);
        break;
    case GUI_CHOICESTATUS:
        matrixGUI_deleteChoiceStatus(element);
        break;
    case GUI_SCROLLER:
        matrixGUI_deleteScroller(element);
        break;
    case GUI_VALUEEDIT:
        matrixGUI_deleteValueEdit(element);
        break;
    case GUI_TIMEREDIT:
        matrixGUI_deleteTimerEdit(element);
        break;
    case GUI_GROUP:
        matrixGUI_deleteGroup(element);
        break;
    case GUI_LIST:
        matrixGUI_deleteList(element);
        break;
    case GUI_SWITCHER:
        matrixGUI_deleteSwitcher(element);
        break;
    case GUI_CONNECTION:
        matrixGUI_deleteConnection(element);
        break;
    default:
        break;
    }
}
uint16_t matrixGUI_getWidth(void *element)
{
    uint16_t width = 0;
    typeLineGUI_t type = **((typeLineGUI_t **)element);
    switch (type)
    {
    case GUI_TEXT:
        width = ((textGUI_t *)element)->patternTextGUI->matrixFont->font->w * strlen(((textGUI_t *)element)->text);
        break;
    case GUI_TEXTEDIT:
        width = ((textEditGUI_t *)element)->patternTextEditGUI->matrixFont->font->w * strlen(((textEditGUI_t *)element)->text);
        break;
    case GUI_SLIDER:
        width = ((sliderGUI_t *)element)->width; // ERROR?
        break;
    case GUI_CHOICE:
        width = ((choiceGUI_t *)element)->widthField; // ERROR?
        break;
    case GUI_SCROLLER:
        width = ((scrollerGUI_t *)element)->width; // ERROR?
        break;
    case GUI_GROUP:
        break;
    case GUI_LIST:
        width = ((listGUI_t *)element)->heightElement * (((listGUI_t *)element)->sizeScreen + 1); // ERROR?
        break;
    default:
        break;
    }
    return width;
}
uint16_t matrixGUI_getHeight(void *element)
{
    uint16_t height = 0;
    typeLineGUI_t type = **((typeLineGUI_t **)element);
    switch (type)
    {
    case GUI_TEXT:
        height = ((textGUI_t *)element)->patternTextGUI->matrixFont->font->h;
        break;
    case GUI_TEXTEDIT:
        height = ((textEditGUI_t *)element)->patternTextEditGUI->matrixFont->font->h;
        break;
    case GUI_SLIDER:
        height = ((sliderGUI_t *)element)->height;
        break;
    case GUI_CHOICE:
        height = ((choiceGUI_t *)element)->patternChoiceGUI->matrixFont->font->h;
        break;
    case GUI_SCROLLER:
        height = ((scrollerGUI_t *)element)->height;
        break;
    case GUI_GROUP:
        break;
    case GUI_LIST:
        height = ((listGUI_t *)element)->heightElement * (((listGUI_t *)element)->sizeScreen + 1);
        break;
    default:
        break;
    }
    return height;
}
typeLineGUI_t matrixGUI_getType(void *element)
{
    return **((typeLineGUI_t **)element);
}
//==============================================================================