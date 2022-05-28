#ifndef MAIN_ST7789MATRIX_H_
#define MAIN_ST7789MATRIX_H_

#include "st7789.h"
#include "matrix.h"
#include <stdint.h>

typedef struct
{
    matrix_t *matrix;
    st7789_t *st7789;
    uint16_t matrixAreaX;
    uint16_t matrixAreaY;
    uint16_t st7789AreaX;
    uint16_t st7789AreaY;
    uint16_t areaW;
    uint16_t areaH;
    uint16_t shiftX;
    uint16_t shiftY;
} st7789Matrix_t;

void st7789Matrix_select(st7789Matrix_t *st7789Matrix);
void st7789Matrix_init(st7789_t *st7789, matrix_t *matrix);

void st7789Matrix_setShiftX(uint16_t x);
void st7789Matrix_setShiftY(uint16_t y);
void st7789Matrix_setShiftXY(uint16_t x, uint16_t y);

void st7789Matrix_setAreaW(uint16_t w);
void st7789Matrix_setAreaH(uint16_t h);
void st7789Matrix_setAreaWH(uint16_t w, uint16_t h);

void st7789Matrix_setAreaX(uint16_t st7789AreaX, uint16_t matrixAreaX);
void st7789Matrix_setAreaY(uint16_t st7789AreaY, uint16_t matrixAreaY);
void st7789Matrix_setAreaXY(uint16_t st7789AreaX, uint16_t matrixAreaX, uint16_t st7789AreaY, uint16_t matrixAreaY);

void st7789Matrix_write();

#endif