#ifndef MAIN_FONTX_H_
#define MAIN_FONTX_H_
#define FontxGlyphBufSize (32 * 32 / 8)

#include <stdint.h>
#include "esp_spiffs.h"

#define bool _Bool

typedef struct
{
	FILE *file;
	char *path;
	uint8_t opened;
	uint16_t size;

	char signature[6];
	char name[8];
	uint8_t w;
	uint8_t h;
	uint8_t flag;
} fontx_t;

void fontx_init(fontx_t *fx, char *path);
uint8_t fontx_open(fontx_t *fx);
void fontx_close(fontx_t *fx);
uint8_t fontx_getFont(fontx_t *fx, uint16_t code, uint8_t *font);
uint8_t fontx_getW(fontx_t *fx);
uint8_t fontx_getH(fontx_t *fx);

// UTF8 to SJIS table
// https://www.mgo-tec.com/blog-entry-utf8sjis01.html
//#define Utf8Sjis "Utf8Sjis.tbl"
// uint16_t UTF2SJIS(spiffs_file fd, uint8_t *utf8);
// int String2SJIS(spiffs_file fd, unsigned char *str_in, size_t stlen, uint16_t *sjis, size_t ssize);
#endif /* MAIN_FONTX_H_ */
