#ifndef MAIN_MATRIXGUI_H_
#define MAIN_MATRIXGUI_H_

#include <stdint.h>
#include <string.h>
#include "matrix.h"

#define max(a, b) a > b ? a : b
#define min(a, b) a > b ? b : a

typedef enum
{
    GUI_NULL,
    GUI_TEXT,
    GUI_TEXTEDIT,
    GUI_SLIDER,
    GUI_CHOICE,
    GUI_CHOICELINE,
    GUI_CHOICESTATUS,
    GUI_SCROLLER,
    GUI_VALUEEDIT,
    GUI_TIMEREDIT,
    GUI_SWITCHER,
    GUI_GROUP,
    GUI_LIST,
    GUI_CONNECTION
} typeLineGUI_t;

// FUNCTIONS===================================================================
void matrixGUI_replaceString(matrixFont_t *matrixFont, uint16_t x, uint16_t y, char *text, char *preText, uint8_t *colorBackground);
void matrixGUI_drawSign(uint16_t xc, uint16_t yc, int16_t rh, uint8_t *colorSign);
uint16_t matrixGUI_buildSimbolList(char **list, char *form);
void matrixGUI_strToVal(uint64_t *value, const char *str, uint8_t count);
void matrixGUI_valToStr(uint64_t value, char *str, uint8_t count);

// TEXT========================================================================
typedef struct
{
    typeLineGUI_t _type;
    // visual:
    uint8_t *colorBackground;
    matrixFont_t *matrixFont;
} patternTextGUI_t;
typedef struct
{
    patternTextGUI_t *patternTextGUI;
    uint16_t x;
    uint16_t y;
    char *text;
    char *_preText;
} textGUI_t;
patternTextGUI_t *matrixGUI_newPatternText(uint8_t *colorBackground, matrixFont_t *matrixFont);
textGUI_t *matrixGUI_newText(patternTextGUI_t *patternTextGUI, char *text, uint16_t x, uint16_t y);
void matrixGUI_buildText(textGUI_t *line);
void matrixGUI_updateText(textGUI_t *line);
void matrixGUI_deleteText(textGUI_t *line);

// TEXT_EDIT===================================================================
typedef struct
{
    typeLineGUI_t _type;
    // configs:
    char *simbolList;
    uint16_t sizeSimbolList;
    // visual:
    // uint8_t fillRadius;
    uint8_t *colorBackground;
    uint8_t *colorSign;
    matrixFont_t *matrixFont;
} patternTextEditGUI_t;
typedef struct
{
    patternTextEditGUI_t *patternTextEditGUI;
    uint16_t x;
    uint16_t y;
    char *text;
    uint8_t sign;
    int16_t index;
    int16_t indexSegment;
    int16_t _preIndexSegment;
    uint8_t _preSign;
    uint8_t *_code;
} textEditGUI_t;
patternTextEditGUI_t *matrixGUI_newPatternTextEdit(char *simbolList, uint16_t sizeSimbolList, uint8_t *colorBackground, uint8_t *colorSign, matrixFont_t *matrixFont);
textEditGUI_t *matrixGUI_newTextEdit(patternTextEditGUI_t *patternTextEditGUI, char *text, uint8_t sign, int16_t indexSegment, uint16_t x, uint16_t y);
void matrixGUI_buildTextEdit(textEditGUI_t *line);
void matrixGUI_updateTextEdit(textEditGUI_t *line);
void matrixGUI_deleteTextEdit(textEditGUI_t *line);

// VALUE_EDIT===================================================================
typedef struct
{
    typeLineGUI_t _type;
    // visual:
    uint8_t *colorBackground;
    uint8_t *colorSign;
    matrixFont_t *matrixFont;
} patternValueEditGUI_t;
typedef struct
{
    patternValueEditGUI_t *patternValueEditGUI;
    uint16_t x;
    uint16_t y;
    char *text;
    char *maxValue;
    char *minValue;
    int8_t index;
    int8_t indexSegment;
    uint8_t sign;
    uint8_t _preSign;
    int8_t _preIndexSegment;
} valueEditGUI_t;
patternValueEditGUI_t *matrixGUI_newPatternValueEdit(uint8_t *colorBackground, uint8_t *colorSign, matrixFont_t *matrixFont);
valueEditGUI_t *matrixGUI_newValueEdit(patternValueEditGUI_t *patternValueEditGUI, char *text, char *maxValue, char *minValue, uint8_t sign, int8_t indexSegment, uint16_t x, uint16_t y);
void matrixGUI_buildValueEdit(valueEditGUI_t *line);
void matrixGUI_updateValueEdit(valueEditGUI_t *line);
void matrixGUI_deleteValueEdit(valueEditGUI_t *line);

// TIMER_EDIT===================================================================
typedef struct
{
    typeLineGUI_t _type;
    uint16_t _countSeparators;
    char *formTimer; // example: 23:59:59
    uint8_t *colorBackground;
    uint8_t *colorSign;
    matrixFont_t *matrixFont;
} patternTimerEditGUI_t;
typedef struct
{
    patternTimerEditGUI_t *patternTimerEditGUI;
    uint16_t x;
    uint16_t y;
    char *text;
    int8_t index;
    int8_t indexSegment;
    uint8_t sign;
    uint8_t _preSign;
    int8_t _preIndexSegment;
    int8_t _preIndexChar;
} timerEditGUI_t;
patternTimerEditGUI_t *matrixGUI_newPatternTimerEdit(char *formTimer, uint8_t *colorBackground, uint8_t *colorSign, matrixFont_t *matrixFont);
timerEditGUI_t *matrixGUI_newTimerEdit(patternTimerEditGUI_t *patternTimerEditGUI, char *text, uint8_t sign, int8_t indexSegment, uint16_t x, uint16_t y);
void matrixGUI_buildTimerEdit(timerEditGUI_t *line);
void matrixGUI_updateTimerEdit(timerEditGUI_t *line);
void matrixGUI_deleteTimerEdit(timerEditGUI_t *line);

// SLIDER======================================================================
typedef struct
{
    typeLineGUI_t _type;
    // visual:
    uint8_t *colorBackground;
    uint8_t *colorFrame;
    uint8_t *colorTrue;
    uint8_t *colorFalse;
} patternSliderGUI_t;
typedef struct
{
    patternSliderGUI_t *patternSliderGUI;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t maxCountPart;
    int16_t index;
    int16_t _preIndex;
} sliderGUI_t;
patternSliderGUI_t *matrixGUI_newPatternSlider(uint8_t *colorBackground, uint8_t *colorFrame, uint8_t *colorTrue, uint8_t *colorFalse);
sliderGUI_t *matrixGUI_newSlider(patternSliderGUI_t *patternSliderGUI, uint16_t width, uint16_t height, uint16_t maxCountParts, int16_t index, uint16_t x, uint16_t y);
void matrixGUI_buildSlider(sliderGUI_t *line);
void matrixGUI_updateSlider(sliderGUI_t *line);
void matrixGUI_deleteSlider(sliderGUI_t *line);

// CHOICE=======================================================================
typedef struct
{
    typeLineGUI_t _type;
    // visual:
    uint8_t *colorBackground;
    uint8_t *colorSign;
    matrixFont_t *matrixFont;
} patternChoiceGUI_t;
typedef struct
{
    patternChoiceGUI_t *patternChoiceGUI;
    uint16_t x;
    uint16_t y;
    char **variants;
    uint16_t countVariants;
    uint16_t widthField;
    uint16_t sizeSign;
    int16_t index;
    int16_t _preIndex;
} choiceGUI_t;
patternChoiceGUI_t *matrixGUI_newPatternChoice(uint8_t *colorBackground, uint8_t *colorSign, matrixFont_t *matrixFont);
choiceGUI_t *matrixGUI_newChoice(patternChoiceGUI_t *patternChoiceGUI, char **variants, uint16_t countVariants, uint16_t widthField, uint16_t sizeSign, int16_t index, uint16_t x, uint16_t y);
void matrixGUI_buildChoice(choiceGUI_t *line);
void matrixGUI_updateChoice(choiceGUI_t *line);
void matrixGUI_deleteChoice(choiceGUI_t *line);

// CHOICE_LINE==================================================================
typedef struct
{
    typeLineGUI_t _type;
    uint16_t height;
    uint16_t widthSegment;
    uint16_t sizeSign;
    uint8_t **colorSegments;
    uint8_t *colorFrame;
    uint8_t *colorSign;
    uint8_t *colorBackground;
    matrixFont_t *matrixFont;
} patternChoiceLineGUI_t;
typedef struct
{
    patternChoiceLineGUI_t *patternChoiceLineGUI;
    uint16_t x;
    uint16_t y;
    uint8_t *positions;
    uint8_t countPositions;
    uint8_t sign;
    char **variants;
    uint16_t countVariants;
    int16_t indexVariant;
    int16_t index;
    uint8_t _preSign;
    int16_t _preIndexVariant;
} choiceLineGUI_t;
patternChoiceLineGUI_t *matrixGUI_newPatternChoiceLine(uint16_t height, uint16_t widthSegment, uint16_t sizeSign, uint8_t **colorSegments, uint8_t *colorBackground, uint8_t *colorFrame, uint8_t *colorSign, matrixFont_t *matrixFont);
choiceLineGUI_t *matrixGUI_newChoiceLine(patternChoiceLineGUI_t *patternChoiceLineGUI, char **variants, uint16_t countVariants, uint8_t *positions, uint8_t countPositions, uint8_t sign, int16_t indexVariant, uint16_t x, uint16_t y);
void matrixGUI_buildChoiceLine(choiceLineGUI_t *line);
void matrixGUI_updateChoiceLine(choiceLineGUI_t *line);
void matrixGUI_deleteChoiceLine(choiceLineGUI_t *line);

// CHOICE_STATUS================================================================
typedef struct
{
    typeLineGUI_t _type;
    uint8_t radius;
    uint8_t **colorStatuses;
    uint8_t countStatuses;
    uint8_t *colorBackground;
    matrixFont_t *matrixFont;
} patternChoiceStatusGUI_t;
typedef struct
{
    patternChoiceStatusGUI_t *patternChoiceStatusGUI;
    uint16_t x;
    uint16_t y;
    char *text;
    int16_t index;
    int16_t _preIndex;
} choiceStatusGUI_t;
patternChoiceStatusGUI_t *matrixGUI_newPatternChoiceStatus(uint8_t raduis, uint8_t **colorStatuses, uint8_t countStatuses, uint8_t *colorBackground, matrixFont_t *matrixFont);
choiceStatusGUI_t *matrixGUI_newChoiceStatus(patternChoiceStatusGUI_t *patternChoiceStatusGUI, char *text, int8_t index, uint16_t x, uint16_t y);
void matrixGUI_buildChoiceStatus(choiceStatusGUI_t *line);
void matrixGUI_updateChoiceStatus(choiceStatusGUI_t *line);
void matrixGUI_deleteChoiceStatus(choiceStatusGUI_t *line);

// SCROLLER=====================================================================
typedef struct
{
    typeLineGUI_t _type;
    // visual:
    uint8_t *colorBackground;
    uint8_t *colorFrame;
    uint8_t *colorTrue;
    uint8_t *colorFalse;
} patternScrollerGUI_t;
typedef struct
{
    patternScrollerGUI_t *patternScrollerGUI;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t sizeGeneralPart;
    uint16_t countParts;
    int16_t index;
    int16_t _preIndex;
} scrollerGUI_t;
patternScrollerGUI_t *matrixGUI_newPatternScroller(uint8_t *colorBackground, uint8_t *colorFrame, uint8_t *colorTrue, uint8_t *colorFalse);
scrollerGUI_t *matrixGUI_newScroller(patternScrollerGUI_t *patternScrollerGUI, uint16_t width, uint16_t height, uint16_t countParts, uint16_t sizeGeneralPart, int16_t index, uint16_t x, uint16_t y);
void matrixGUI_buildScroller(scrollerGUI_t *line);
void matrixGUI_updateScroller(scrollerGUI_t *line);
void matrixGUI_deleteScroller(scrollerGUI_t *line);

// SWITCHER=====================================================================
typedef struct
{
    typeLineGUI_t _type;
} patternSwitcherGUI_t;
typedef struct
{
    patternSwitcherGUI_t *patternSwitcherGUI;
    uint16_t x;
    uint16_t y;
    void **elements;
    uint16_t countElements;
    int16_t index;
    int16_t _preIndex;
} switcherGUI_t;
patternSwitcherGUI_t *matrixGUI_newPatternSwitcher();
switcherGUI_t *matrixGUI_newSwitcher(patternSwitcherGUI_t *patternSwitcherGUI, void **elements, uint16_t countElements, int16_t index, uint16_t x, uint16_t y);
void matrixGUI_buildSwitcher(switcherGUI_t *line);
void matrixGUI_updateSwitcher(switcherGUI_t *line);
void matrixGUI_deleteSwitcher(switcherGUI_t *line);

// GROUP========================================================================
typedef struct
{
    typeLineGUI_t _type;
} patternGroupGUI_t;
typedef struct
{
    patternGroupGUI_t *patternGroupGUI;
    void **elements;
    uint16_t countElements;
} groupGUI_t;
patternGroupGUI_t *matrixGUI_newPatternGroup();
groupGUI_t *matrixGUI_newGroup(patternGroupGUI_t *patternGroupGUI, void *elements, uint16_t countElements);
void matrixGUI_buildGroup(groupGUI_t *line);
void matrixGUI_updateGroup(groupGUI_t *line);
void matrixGUI_deleteGroup(groupGUI_t *line);

// LIST=========================================================================
typedef struct
{
    typeLineGUI_t _type;
    // visual:
    uint8_t *colorBackground;
    uint8_t *colorSign;
} patternListGUI_t;
typedef struct
{
    patternListGUI_t *patternListGUI;
    uint16_t x;
    uint16_t y;
    void **elements;
    uint16_t countElements;
    uint16_t heightElement;
    uint16_t sizeScreen;
    uint16_t setCursor;
    uint16_t sizeCursor;
    int16_t index;
    int16_t _preIndex;
    groupGUI_t _groupGUI;
} listGUI_t;
patternListGUI_t *matrixGUI_newPatternList(uint8_t *colorBackground, uint8_t *colorSign);
listGUI_t *matrixGUI_newList(patternListGUI_t *patternListGUI, void **elements, uint16_t countElements, uint16_t heightElement, uint16_t sizeScreen, uint16_t sizeCursor, uint16_t setCursor, int16_t index, uint16_t x, uint16_t y);
void matrixGUI_buildList(listGUI_t *line);
void matrixGUI_updateList(listGUI_t *line);
void matrixGUI_deleteList(listGUI_t *line);

// CONNECTIONS==================================================================
typedef struct
{
    typeLineGUI_t _type;
} patternConnectionGUI_t;
typedef struct
{
    patternConnectionGUI_t *patternConnectionsGUI;
    uint16_t x;
    uint16_t y;
    void **elements;
    uint16_t countElements;
    uint16_t *shiftsX;
    uint16_t *shiftsY;
} connectionGUI_t;
patternConnectionGUI_t *matrixGUI_newPatternConnection();
connectionGUI_t *matrixGUI_newConnection(patternConnectionGUI_t *patternConnectionsGUI, void **elements, uint16_t countElements, uint16_t *shiftX, uint16_t *shiftY, uint16_t x, uint16_t y);
void matrixGUI_buildConnection(connectionGUI_t *line);
void matrixGUI_updateConnection(connectionGUI_t *line);
void matrixGUI_deleteConnection(connectionGUI_t *line);

// UNIVERSAL====================================================================
void matrixGUI_build(void *element);
void matrixGUI_update(void *element);
void matrixGUI_delete(void *element);
uint16_t matrixGUI_getWidth(void *element);
uint16_t matrixGUI_getHeight(void *element);
typeLineGUI_t matrixGUI_getType(void *element);
//==============================================================================

#endif