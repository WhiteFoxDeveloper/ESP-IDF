#ifndef MAIN_MATRIX_H_
#define MAIN_MATRIX_H_

#include <stdint.h>
#include "fontx.h"

typedef struct
{
    uint8_t **matrix;
    uint16_t matrixW;
    uint16_t matrixH;
    uint16_t colorDepthByte;
    uint16_t areaX;
    uint16_t areaY;
    uint16_t areaW;
    uint16_t areaH;
} matrix_t;

typedef struct
{
    fontx_t *font;
    uint8_t *color;
    uint8_t direction;
    uint8_t *fillColor;
    uint8_t fillRadius;
    uint8_t fill;
    uint8_t *underlineColor;
    uint8_t underline;
} matrixFont_t;

void matrix_select(matrix_t *matrix);
void matrix_clone(matrix_t *matrix);
void matrix_init(uint16_t width, uint16_t height, uint8_t colorDepthByte);
void matrix_setArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

void matrix_drawPixel(uint16_t x, uint16_t y, uint8_t *color);
void matrix_drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t *color);
void matrix_drawRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, float h, uint8_t *color);
void matrix_drawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, float h, uint8_t *color);
void matrix_drawCircle(uint16_t x, uint16_t y, uint16_t d, uint8_t *color);

void matrix_drawLineH(uint16_t x, uint16_t y, int16_t h, uint8_t *color);
void matrix_drawLineW(uint16_t x, uint16_t y, int16_t w, uint8_t *color);
void matrix_drawRectWH(uint16_t x, uint16_t y, int16_t w, int16_t h, uint8_t *color);
void matrix_drawTriangleW(uint16_t x, uint16_t y, int16_t w, int16_t h, uint8_t *color);
void matrix_drawTriangleH(uint16_t x, uint16_t y, int16_t w, int16_t h, uint8_t *color);

void matrix_drawFillRectWH(uint16_t x, uint16_t y, int16_t w, int16_t h, uint8_t *color);
void matrix_drawFillCircle(uint16_t x, uint16_t y, uint16_t d, uint8_t *color);

void matrix_drawRectRoundWH(uint16_t x, uint16_t y, int16_t w, int16_t h, uint16_t r, uint8_t *color);
void matrix_drawFillRectRoundWH(uint16_t x, uint16_t y, int16_t w, int16_t h, uint16_t r, uint8_t *color);

void matrix_drawPartRound(uint16_t x, uint16_t y, uint16_t d, float degree1, float degree2, uint8_t *color);
void matrix_drawFillPartRound(uint16_t x, uint16_t y, uint16_t d, float degree1, float degree2, uint8_t *color);

void matrix_drawPolyline(uint16_t *points, uint16_t count, uint8_t *color);
void matrix_drawFigure(uint16_t *points, uint16_t count, uint8_t *color);

// STRINGS:
void matrix_fontSelect(matrixFont_t *matrixFont);
int16_t matrix_drawChar(uint16_t x, uint16_t y, uint8_t code);
int16_t matrix_drawString(uint16_t x, uint16_t y, uint8_t *code);

#endif