#include "st7789Matrix.h"
#include <string.h>

static st7789Matrix_t *dev;

void st7789Matrix_select(st7789Matrix_t *st7789Matrix)
{
    dev = st7789Matrix;
}
void st7789Matrix_init(st7789_t *st7789, matrix_t *matrix)
{
    dev->matrix = matrix;
    dev->st7789 = st7789;
}

void st7789Matrix_setShiftX(uint16_t x)
{
    dev->shiftX = x;
}
void st7789Matrix_setShiftY(uint16_t y)
{
    dev->shiftY = y;
}
void st7789Matrix_setShiftXY(uint16_t x, uint16_t y)
{
    dev->shiftX = x;
    dev->shiftY = y;
}

void st7789Matrix_setAreaW(uint16_t w)
{
    dev->areaW = w;
}
void st7789Matrix_setAreaH(uint16_t h)
{
    dev->areaH = h;
}
void st7789Matrix_setAreaWH(uint16_t w, uint16_t h)
{
    dev->areaW = w;
    dev->areaH = h;
}

void st7789Matrix_setAreaX(uint16_t st7789AreaX, uint16_t matrixAreaX)
{
    dev->st7789AreaX = st7789AreaX;
    dev->matrixAreaX = matrixAreaX;
}
void st7789Matrix_setAreaY(uint16_t st7789AreaY, uint16_t matrixAreaY)
{
    dev->st7789AreaY = st7789AreaY;
    dev->matrixAreaY = matrixAreaY;
}
void st7789Matrix_setAreaXY(uint16_t st7789AreaX, uint16_t matrixAreaX, uint16_t st7789AreaY, uint16_t matrixAreaY)
{
    dev->st7789AreaX = st7789AreaX;
    dev->matrixAreaX = matrixAreaX;
    dev->st7789AreaY = st7789AreaY;
    dev->matrixAreaY = matrixAreaY;
}

void st7789Matrix_write()
{
    st7789_select(dev->st7789);
    matrix_select(dev->matrix);
    st7789_setActiveArea(dev->st7789AreaX, dev->st7789AreaY, dev->st7789AreaX + dev->areaW, dev->st7789AreaY + dev->areaH);
    st7789_memoryWrite();
    uint16_t dataLengthX1 = (dev->areaW - dev->shiftX) * dev->matrix->colorDepthByte;
    uint16_t dataLengthX2 = dev->shiftX * dev->matrix->colorDepthByte;
    uint16_t x1 = dev->matrix->colorDepthByte * dev->shiftX;
    for (uint16_t y = dev->shiftY; y < dev->areaH; y++)
    {
        st7789_spiWriteData(&dev->matrix->matrix[y][x1], dataLengthX1);
        if (dataLengthX2)
        {
            st7789_spiWriteData(&dev->matrix->matrix[y][0], dataLengthX2);
        }
    }
    for (uint16_t y = 0; y < dev->shiftY; y++)
    {
        st7789_spiWriteData(&dev->matrix->matrix[y][x1], dataLengthX1);
        if (dataLengthX2)
        {
            st7789_spiWriteData(&dev->matrix->matrix[y][0], dataLengthX2);
        }
    }
}
