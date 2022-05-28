#include "matrix.h"
#include <malloc.h>
#include <string.h>
#include <math.h>
#include "esp_log.h"

//#define max(a, b) a > b ? a : b
//#define min(a, b) a > b ? b : a

static matrix_t *dev;
static matrixFont_t *devFont;

void matrix_alignX(uint16_t *x, int16_t *w)
{
    if (*w < 0)
    {
        *w = -*w;
        *x = *w > *x ? dev->areaW + (*x - *w) : *x - *w;
        *w += 1;
    }
    *w = *w > dev->areaW ? dev->areaW : *w;
}
void matrix_alignY(uint16_t *y, int16_t *h)
{
    if (*h < 0)
    {
        *h = -*h;
        *y = *h > *y ? dev->areaH + (*y - *h) : *y - *h;
        *h += 1;
    }
    *h = *h > dev->areaH ? dev->areaH : *h;
}

void matrix_select(matrix_t *matrix)
{
    dev = matrix;
}
void matrix_clone(matrix_t *matrix)
{
    memcpy(matrix, dev, sizeof(matrix_t));
    matrix->matrix = (uint8_t **)calloc(dev->matrixH, sizeof(void *));
    for (uint16_t index = 0; index < dev->matrixH; index++)
    {
        matrix->matrix[index] = dev->matrix[index];
    }
}
void matrix_init(uint16_t width, uint16_t height, uint8_t colorDepthByte)
{
    dev->matrixW = width;
    dev->matrixH = height;
    dev->colorDepthByte = colorDepthByte;
    dev->matrix = (uint8_t **)calloc(height, sizeof(void *));
    uint16_t sizeW = width * colorDepthByte;
    for (uint16_t index = 0; index < height; index++)
    {
        dev->matrix[index] = (uint8_t *)malloc(sizeW);
        memset(dev->matrix[index], 0, sizeW);
    }
    dev->areaX = 0;
    dev->areaY = 0;
    dev->areaW = width;
    dev->areaH = height;
}
void matrix_setArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if ((y + h) > dev->matrixH)
        return;
    if ((x + w) > dev->matrixW)
        return;

    int16_t diffY = y - dev->areaY;
    int16_t clearX = -1 * dev->areaX * dev->colorDepthByte;
    int16_t setX = x * dev->colorDepthByte;
    for (uint16_t index = 0; index < dev->areaH; index++)
    {
        dev->matrix[index] = &dev->matrix[index][clearX];
    }
    dev->matrix = &dev->matrix[diffY];
    for (uint16_t index = 0; index < h; index++)
    {
        dev->matrix[index] = &dev->matrix[index][setX];
    }
    dev->areaX = x;
    dev->areaY = y;
    dev->areaW = w;
    dev->areaH = h;
}

void matrix_drawPixel(uint16_t x, uint16_t y, uint8_t *color)
{
    x %= dev->areaW;
    y %= dev->areaH;

    x = x * dev->colorDepthByte;
    for (uint16_t colorDepth = 0; colorDepth < dev->colorDepthByte; colorDepth++)
    {
        dev->matrix[y][x] = color[colorDepth];
        x++;
    }
}
void matrix_drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t *color)
{
    // x1 %= dev->areaW;
    // y1 %= dev->areaH;
    // x2 %= dev->areaW;
    // y2 %= dev->areaH;

    int16_t i;
    int16_t E;
    int16_t dx = (x2 > x1) ? x2 - x1 : x1 - x2;
    int16_t dy = (y2 > y1) ? y2 - y1 : y1 - y2;
    int16_t sx = (x2 > x1) ? 1 : -1;
    int16_t sy = (y2 > y1) ? 1 : -1;
    if (dx > dy)
    {
        E = -dx;
        for (i = 0; i <= dx; i++)
        {
            matrix_drawPixel(x1, y1, color);
            x1 += sx;
            E += 2 * dy;
            if (E >= 0)
            {
                y1 += sy;
                E -= 2 * dx;
            }
        }
    }
    else
    {
        E = -dy;
        for (i = 0; i <= dy; i++)
        {
            matrix_drawPixel(x1, y1, color);
            y1 += sy;
            E += 2 * dx;
            if (E >= 0)
            {
                x1 += sx;
                E -= 2 * dy;
            }
        }
    }
}
void matrix_drawRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, float h, uint8_t *color)
{
    int16_t xw = x2 - x1;
    int16_t yh = y2 - y1;
    if (xw == 0)
    {
        matrix_drawRectWH(x1, y1, h, yh, color);
    }
    else if (yh == 0)
    {
        matrix_drawRectWH(x1, y1, xw, h, color);
    }
    else
    {
        float diff = h / sqrt((xw * xw) + (yh * yh));
        int16_t x = yh * diff;
        int16_t y = xw * diff;
        matrix_drawLine(x1, y1, x2, y2, color);
        matrix_drawLine(x1, y1, x1 - x, y1 + y, color);
        matrix_drawLine(x2, y2, x2 - x, y2 + y, color);
        matrix_drawLine(x1 - x, y1 + y, x2 - x, y2 + y, color);
    }
}
void matrix_drawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, float h, uint8_t *color)
{
    x1 %= dev->areaW;
    y1 %= dev->areaH;
    x2 %= dev->areaW;
    y2 %= dev->areaH;
    int16_t xw = x2 - x1;
    int16_t yh = y2 - y1;
    if (xw == 0)
    {
        matrix_drawTriangleW(x1, y1, h, yh, color);
    }
    else if (yh == 0)
    {
        matrix_drawTriangleH(x1, y1, xw, h, color);
    }
    else
    {
        float diff = h / sqrt((xw * xw) + (yh * yh));
        int16_t x = yh * diff;
        int16_t y = xw * diff;
        x = (x1 + (xw / 2)) - x;
        y = (y1 + (yh / 2)) + y;
        matrix_drawLine(x1, y1, x2, y2, color);
        matrix_drawLine(x1, y1, x, y, color);
        matrix_drawLine(x2, y2, x, y, color);
    }
}
void matrix_drawCircle(uint16_t x, uint16_t y, uint16_t d, uint8_t *color)
{
    x %= dev->areaW;
    y %= dev->areaH;

    uint16_t r = d / 2;
    uint16_t a = ceil(r / sqrt(2));
    uint16_t xw = x + d - 1;
    uint16_t yh = y + d - 1;

    uint16_t xi, yi = 0;
    while (yi <= a)
    {
        xi = r - sqrt(r * r - yi * yi);
        matrix_drawPixel(x + xi, y + r - yi, color);
        matrix_drawPixel(x + xi, yh - r + yi, color);
        matrix_drawPixel(xw - xi, y + r - yi, color);
        matrix_drawPixel(xw - xi, yh - r + yi, color);

        matrix_drawPixel(x + r - yi, y + xi, color);
        matrix_drawPixel(x + r - yi, yh - xi, color);
        matrix_drawPixel(xw - r + yi, y + xi, color);
        matrix_drawPixel(xw - r + yi, yh - xi, color);
        yi++;
    }
}

void matrix_drawLineW(uint16_t x, uint16_t y, int16_t w, uint8_t *color)
{
    matrix_alignX(&x, &w);
    x %= dev->areaW;
    y %= dev->areaH;

    // x = x * dev->colorDepthByte;
    uint16_t xw = x + w;
    uint16_t colorDepth = 0;
    if (xw > dev->areaW)
    {
        xw %= dev->areaW;
        xw *= dev->colorDepthByte;
        for (uint16_t indexX = 0; indexX < xw; indexX++)
        {
            dev->matrix[y][indexX] = color[colorDepth++];
            colorDepth = colorDepth == dev->colorDepthByte ? 0 : colorDepth;
        }
        xw = dev->areaW;
    }
    xw *= dev->colorDepthByte;
    x *= dev->colorDepthByte;
    for (uint16_t indexX = x; indexX < xw; indexX++)
    {
        dev->matrix[y][indexX] = color[colorDepth++];
        colorDepth = colorDepth == dev->colorDepthByte ? 0 : colorDepth;
    }
}
void matrix_drawLineH(uint16_t x, uint16_t y, int16_t h, uint8_t *color)
{
    matrix_alignY(&y, &h);
    x %= dev->areaW;
    y %= dev->areaH;

    x = x * dev->colorDepthByte;
    uint16_t yh = y + h;
    if (yh > dev->areaH)
    {
        yh %= dev->areaH;
        for (uint16_t colorDepth = 0; colorDepth < dev->colorDepthByte; colorDepth++)
        {
            uint16_t indexX = x + colorDepth;
            for (uint16_t indexY = 0; indexY < yh; indexY++)
            {
                dev->matrix[indexY][indexX] = color[colorDepth];
            }
        }
        yh = dev->areaH;
    }
    for (uint16_t colorDepth = 0; colorDepth < dev->colorDepthByte; colorDepth++)
    {
        uint16_t indexX = x + colorDepth;
        for (uint16_t indexY = y; indexY < yh; indexY++)
        {
            dev->matrix[indexY][indexX] = color[colorDepth];
        }
    }
}
void matrix_drawRectWH(uint16_t x, uint16_t y, int16_t w, int16_t h, uint8_t *color)
{
    matrix_alignX(&x, &w);
    matrix_alignY(&y, &h);

    uint16_t xw = x + w - 1;
    uint16_t yh = y + h - 1;
    matrix_drawLineW(x, y, w, color);
    matrix_drawLineH(x, y, h, color);
    matrix_drawLineW(x, yh, w, color);
    matrix_drawLineH(xw, y, h, color);
}
void matrix_drawTriangleW(uint16_t x, uint16_t y, int16_t w, int16_t h, uint8_t *color)
{
    int16_t yh = y + h;
    int16_t xw = x + w;
    yh = yh < 0 ? dev->areaH + (yh % dev->areaH) : yh;
    xw = xw < 0 ? dev->areaW + (xw % dev->areaW) : xw;
    int16_t yc = y + (h / 2);
    matrix_drawLineH(x, y, h, color);
    matrix_drawLine(x, y, xw, yc, color);
    matrix_drawLine(x, yh, xw, yc, color);
}
void matrix_drawTriangleH(uint16_t x, uint16_t y, int16_t w, int16_t h, uint8_t *color)
{
    int16_t yh = y + h;
    int16_t xw = x + w;
    yh = yh < 0 ? dev->areaH + (yh % dev->areaH) : yh;
    xw = xw < 0 ? dev->areaW + (xw % dev->areaW) : xw;
    int16_t xc = x + (w / 2);
    matrix_drawLineW(x, y, w, color);
    matrix_drawLine(x, y, xc, yh, color);
    matrix_drawLine(xw, y, xc, yh, color);
}

void matrix_drawFillRectWH(uint16_t x, uint16_t y, int16_t w, int16_t h, uint8_t *color)
{
    matrix_alignX(&x, &w);
    matrix_alignY(&y, &h);

    x %= dev->areaW;
    y %= dev->areaH;
    uint16_t xw = x + w;
    uint16_t yh = y + h;
    int8_t isOutX = xw >= dev->areaW;
    int8_t isOutY = yh >= dev->areaH;
    int16_t outX = xw - dev->areaW;
    int16_t outY = yh - dev->areaH;
    yh = isOutY ? dev->areaH : yh;
    xw = isOutX ? dev->areaW : xw;
    outX *= dev->colorDepthByte;
    x *= dev->colorDepthByte;
    xw *= dev->colorDepthByte;
    uint16_t colorDepth = 0;
    if (isOutX && isOutY)
    {
        for (uint16_t indexY = 0; indexY < outY; indexY++)
        {
            for (uint16_t indexX = 0; indexX < outX; indexX++)
            {
                dev->matrix[indexY][indexX] = color[colorDepth++];
                colorDepth = colorDepth == dev->colorDepthByte ? 0 : colorDepth;
            }
        }
    }
    if (isOutX)
    {
        for (uint16_t indexY = y; indexY < yh; indexY++)
        {
            for (uint16_t indexX = 0; indexX < outX; indexX++)
            {
                dev->matrix[indexY][indexX] = color[colorDepth++];
                colorDepth = colorDepth == dev->colorDepthByte ? 0 : colorDepth;
            }
        }
    }
    if (isOutY)
    {
        for (uint16_t indexY = 0; indexY < outY; indexY++)
        {
            for (uint16_t indexX = x; indexX < xw; indexX++)
            {
                dev->matrix[indexY][indexX] = color[colorDepth++];
                colorDepth = colorDepth == dev->colorDepthByte ? 0 : colorDepth;
            }
        }
    }
    for (uint16_t indexY = y; indexY < yh; indexY++)
    {
        for (uint16_t indexX = x; indexX < xw; indexX++)
        {
            dev->matrix[indexY][indexX] = color[colorDepth++];
            colorDepth = colorDepth == dev->colorDepthByte ? 0 : colorDepth;
        }
    }
}
void matrix_drawFillCircle(uint16_t x, uint16_t y, uint16_t d, uint8_t *color)
{
    uint16_t r = d / 2;
    uint16_t a = ceil(r / sqrt(2));
    uint16_t xw = x + d - 1;
    uint16_t yh = y + d - 1;

    uint16_t xi, yi = 0;
    while (yi <= a)
    {
        xi = r - sqrt(r * r - yi * yi);
        matrix_drawLineW(x + xi, y + r - yi, d - (xi * 2), color);
        matrix_drawLineW(x + xi, yh - r + yi, d - (xi * 2), color);

        matrix_drawLineH(x + r - yi, y + xi, a - xi, color);
        matrix_drawLineH(x + r - yi, yh - xi, -a + xi, color);
        matrix_drawLineH(xw - r + yi, y + xi, a - xi, color);
        matrix_drawLineH(xw - r + yi, yh - xi, -a + xi, color);
        yi++;
    }
}

void matrix_drawRectRoundWH(uint16_t x, uint16_t y, int16_t w, int16_t h, uint16_t r, uint8_t *color)
{
    if (r == 0)
    {
        matrix_drawRectWH(x, y, w, h, color);
    }
    else
    {
        matrix_alignX(&x, &w);
        matrix_alignY(&y, &h);
        x %= dev->areaW;
        y %= dev->areaH;

        // uint16_t yi;
        // for (int16_t xi = 0; xi <= r; xi++)
        // {
        //     yi = r - sqrt((r * r) - (xi * xi));

        //     matrix_drawLineH(xc1 + xi, y + yi, h - (2 * yi), color);
        //     matrix_drawLineH(xc2 - xi, y + yi, h - (2 * yi), color);
        // }
    }
}
void matrix_drawFillRectRoundWH(uint16_t x, uint16_t y, int16_t w, int16_t h, uint16_t r, uint8_t *color)
{
    if (r == 0)
    {
        matrix_drawFillRectWH(x, y, w, h, color);
    }
    else
    {
        matrix_alignX(&x, &w);
        matrix_alignY(&y, &h);
        x %= dev->areaW;
        y %= dev->areaH;

        // uint16_t d = 2 * r;
        //  if ((d <= w) && (d <= h))
        uint16_t a = ceil(r / sqrt(2));
        uint16_t xw = x + w - 1;
        uint16_t yh = y + h - 1;

        uint16_t xi, yi;

        yi = 0;
        while (yi <= a)
        {
            xi = r - sqrt(r * r - yi * yi);
            matrix_drawLineW(x + xi, yh - r + yi, w - xi * 2, color);
            matrix_drawLineW(x + xi, y + r - yi, w - xi * 2, color);

            matrix_drawLineH(x + r - yi, y + xi, a - xi, color);
            matrix_drawLineH(x + r - yi, yh - xi, -a + xi, color);
            matrix_drawLineH(xw - r + yi, y + xi, a - xi, color);
            matrix_drawLineH(xw - r + yi, yh - xi, -a + xi, color);
            yi++;
        }
        // xi = 0;
        // while (xi <= a)
        // {
        //     yi = r - sqrt(r * r - xi * xi);
        //     matrix_drawLineH(xw - r + xi, yh - yi, -a + yi, color);
        //     matrix_drawLineH(xw - r + xi, y + yi, a - yi, color);
        //     matrix_drawLineH(x + r - xi, yh - yi, -a + yi, color);
        //     matrix_drawLineH(x + r - xi, y + yi, a - yi, color);
        //     xi++;
        // }
        matrix_drawFillRectWH(x, y + r + 1, w, h - (r + 1) * 2, color);
        matrix_drawFillRectWH(x + r + 1, y, w - (r + 1) * 2, a, color);
        matrix_drawFillRectWH(x + r + 1, yh, w - (r + 1) * 2, -a, color);
    }
}

void matrix_drawPartRound(uint16_t x, uint16_t y, uint16_t d, float degree1, float degree2, uint8_t *color)
{
    uint16_t r = d / 2;
    degree1 = fmod(degree1, 360.0);
    degree2 = fmod(degree2, 360.0);
    float radian1 = M_PI / 180 * degree1;
    float radian2 = M_PI / 180 * degree2;
    int16_t yd1 = r * sin(radian1);
    int16_t xd1 = r * cos(radian1);
    int16_t yd2 = r * sin(radian2);
    int16_t xd2 = r * cos(radian2);
    ESP_LOGI(__FUNCTION__, "%f %f %f %f", cos(radian1), sin(radian1), cos(radian2), sin(radian2));
    ESP_LOGI(__FUNCTION__, "%i %i %i %i", xd1, yd1, xd2, yd2);

    // CODE

    // matrix_drawCircle(x, y, d, color);
    // matrix_drawLineW(x + xd1, y + yd1, 5, color);
    // matrix_drawLineW(x + xd2, y + yd2, 5, color);
}
void matrix_drawFillPartRound(uint16_t x, uint16_t y, uint16_t d, float degree1, float degree2, uint8_t *color)
{
    uint16_t r = d / 2;
    degree1 = fmod(degree1, 360.0);
    degree2 = fmod(degree2, 360.0);
    float radian1 = M_PI / 180 * degree1;
    float radian2 = M_PI / 180 * degree2;
    int16_t yd1 = r * sin(radian1);
    int16_t xd1 = r * cos(radian1);
    int16_t yd2 = r * sin(radian2);
    int16_t xd2 = r * cos(radian2);
    ESP_LOGI(__FUNCTION__, "%f %f %f %f", cos(radian1), sin(radian1), cos(radian2), sin(radian2));
    ESP_LOGI(__FUNCTION__, "%i %i %i %i", xd1, yd1, xd2, yd2);

    // CODE

    // matrix_drawCircle(x, y, d, color);
    // matrix_drawLineW(x + xd1, y + yd1, 5, color);
    // matrix_drawLineW(x + xd2, y + yd2, 5, color);
}

void matrix_drawPolyline(uint16_t *points, uint16_t count, uint8_t *color)
{
    count -= 1;
    uint16_t index = 0;
    for (uint16_t point = 0; point < count; point++)
    {
        matrix_drawLine(points[index], points[index + 1], points[index + 2], points[index + 3], color);
        index += 2;
    }
}
void matrix_drawFigure(uint16_t *points, uint16_t count, uint8_t *color)
{
    matrix_drawPolyline(points, count, color);
    count -= 1;
    uint16_t index = count * 2;
    matrix_drawLine(points[0], points[1], points[index], points[index + 1], color);
}

void matrix_fontSelect(matrixFont_t *matrixFont)
{
    devFont = matrixFont;
}
int16_t matrix_drawString(uint16_t x, uint16_t y, uint8_t *code)
{
    uint16_t length = strlen((char *)code);
    uint8_t fill = devFont->fill;
    if (fill)
    {
        devFont->fill = 0;
        if (devFont->direction == 0)
        {
            matrix_drawFillRectRoundWH(x, y, (length * devFont->font->w), devFont->font->h, devFont->fillRadius, devFont->fillColor);
        }
        else if (devFont->direction == 3)
        {
            matrix_drawFillRectRoundWH(x, y, -devFont->font->h, (length * devFont->font->w), devFont->fillRadius, devFont->fillColor);
        }
        else if (devFont->direction == 2)
        {
            matrix_drawFillRectRoundWH(x, y, -(length * devFont->font->w), -devFont->font->h, devFont->fillRadius, devFont->fillColor);
        }
        else
        {
            matrix_drawFillRectRoundWH(x, y, devFont->font->h, -(length * devFont->font->w), devFont->fillRadius, devFont->fillColor);
        }
    }
    uint16_t *buff = devFont->direction % 2 ? &y : &x;
    for (uint16_t index = 0; index < length; index++)
    {
        *buff = matrix_drawChar(x, y, code[index]);
    }
    devFont->fill = fill;
    return *buff;
}
int16_t matrix_drawChar(uint16_t x, uint16_t y, uint8_t code)
{
    uint8_t font[devFont->font->size];
    fontx_getFont(devFont->font, code, font);

    int8_t d[2];   // Directions
    uint8_t p[2];  // Pointers
    int16_t i[2];  // Start cords
    int16_t ie[2]; // End cords

    uint16_t w = (devFont->font->w + 7) / 8;

    if (devFont->direction == 0)
    {
        d[0] = 1;                     // x
        d[1] = 1;                     // y
        i[0] = x;                     // x
        i[1] = y;                     // y
        ie[0] = x + w * 8;            // x
        ie[1] = y + devFont->font->h; // y
        p[0] = 0;                     // x
        p[1] = 1;                     // y
    }
    else if (devFont->direction == 3)
    {
        d[0] = -1;                    // x
        d[1] = 1;                     // y
        i[0] = y;                     // x
        i[1] = x;                     // y
        ie[0] = y - w * 8;            // x
        ie[1] = x + devFont->font->h; // y
        p[0] = 1;                     // y
        p[1] = 0;                     // x
    }
    else if (devFont->direction == 2)
    {
        d[0] = -1;                    // x
        d[1] = -1;                    // y
        i[0] = x;                     // x
        i[1] = y;                     // y
        ie[0] = x - w * 8;            // x
        ie[1] = y - devFont->font->h; // y
        p[0] = 0;                     // x
        p[1] = 1;                     // y
    }
    else
    {
        d[0] = 1;                     // x
        d[1] = -1;                    // y
        i[0] = y;                     // x
        i[1] = x;                     // y
        ie[0] = y + w * 8;            // x
        ie[1] = x - devFont->font->h; // y
        p[0] = 1;                     // y
        p[1] = 0;                     // x
    }

    if (devFont->fill)
    {
        matrix_drawFillRectRoundWH(x, y, ie[p[0]] - i[p[0]], ie[p[1]] - i[p[1]], devFont->fillRadius, devFont->fillColor);
    }

    uint8_t mask;
    int16_t ix = i[0];
    uint16_t index = 0;
    while (i[1] != ie[1])
    {
        i[0] = ix;
        while (i[0] != ie[0])
        {
            mask = 128;
            for (uint8_t bit = 0; bit < 8; bit++)
            {
                if (font[index] & mask)
                    matrix_drawPixel(i[p[0]], i[p[1]], devFont->color);
                mask = mask >> 1;
                i[0] += d[0];
            }
            index++;
        }
        i[1] += d[1];
    }
    return ie[0] - (w * 8 - devFont->font->w) * d[0];
}