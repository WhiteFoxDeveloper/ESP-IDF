#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"

#include "fontx.h"

void fontx_init(fontx_t *fx, char *path)
{
	fx->opened = 0;
	fx->path = path;
}

uint8_t fontx_open(fontx_t *fx)
{
	if (!fx->opened)
	{
		fx->file = fopen(fx->path, "r");
		if (fx->file == NULL)
		{
			fclose(fx->file);
			return 0;
		}
		fx->opened = 1;
		if (fread(fx->signature, 1, 17, fx->file) != 17) // signature(6), name(8), w(1), h(1), flag(1) = (17)
			return 0;
		fx->size = (fx->w + 7) / 8 * fx->h;
	}
	return 1;
}
void fontx_close(fontx_t *fx)
{
	if (fx->opened)
	{
		fclose(fx->file);
		fx->opened = false;
	}
}
uint8_t fontx_getFont(fontx_t *fx, uint16_t code, uint8_t *font)
{
	fontx_open(fx);
	if (fx->flag)
	{
		return 0;
	}
	else
	{
		fpos_t indexFlash = 17 + code * fx->size;
		if (fsetpos(fx->file, &indexFlash))
			return 0;
		if (fread(font, 1, fx->size, fx->file) != fx->size)
			return 0;
	}
	return 1;
}
uint8_t fontx_getW(fontx_t *fx)
{
	return fx->w;
}
uint8_t fontx_getH(fontx_t *fx)
{
	return fx->h;
}

// void Font2Bitmap(uint8_t *fonts, uint8_t *line, uint8_t w, uint8_t h, uint8_t inverse)
// {
// 	int x, y;
// 	for (y = 0; y < (h / 8); y++)
// 	{
// 		for (x = 0; x < w; x++)
// 		{
// 			line[y * 32 + x] = 0;
// 		}
// 	}

// 	int mask = 7;
// 	int fontp;
// 	fontp = 0;
// 	for (y = 0; y < h; y++)
// 	{
// 		for (x = 0; x < w; x++)
// 		{
// 			uint8_t d = fonts[fontp + x / 8];
// 			uint8_t linep = (y / 8) * 32 + x;
// 			if (d & (0x80 >> (x % 8)))
// 				line[linep] = line[linep] + (1 << mask);
// 		}
// 		mask--;
// 		if (mask < 0)
// 			mask = 7;
// 		fontp += (w + 7) / 8;
// 	}

// 	if (inverse)
// 	{
// 		for (y = 0; y < (h / 8); y++)
// 		{
// 			for (x = 0; x < w; x++)
// 			{
// 				line[y * 32 + x] = RotateByte(line[y * 32 + x]);
// 			}
// 		}
// 	}
// }

// // アンダーラインを追加
// void UnderlineBitmap(uint8_t *line, uint8_t w, uint8_t h)
// {
// 	int x, y;
// 	uint8_t wk;
// 	for (y = 0; y < (h / 8); y++)
// 	{
// 		for (x = 0; x < w; x++)
// 		{
// 			wk = line[y * 32 + x];
// 			if ((y + 1) == (h / 8))
// 				line[y * 32 + x] = wk + 0x80;
// 		}
// 	}
// }

// // ビットマップを反転
// void ReversBitmap(uint8_t *line, uint8_t w, uint8_t h)
// {
// 	int x, y;
// 	uint8_t wk;
// 	for (y = 0; y < (h / 8); y++)
// 	{
// 		for (x = 0; x < w; x++)
// 		{
// 			wk = line[y * 32 + x];
// 			line[y * 32 + x] = ~wk;
// 		}
// 	}
// }

// // フォントパターンの表示
// void ShowFont(uint8_t *fonts, uint8_t pw, uint8_t ph)
// {
// 	int x, y, fpos;
// 	printf("[ShowFont pw=%d ph=%d]\n", pw, ph);
// 	fpos = 0;
// 	for (y = 0; y < ph; y++)
// 	{
// 		printf("%02d", y);
// 		for (x = 0; x < pw; x++)
// 		{
// 			if (fonts[fpos + x / 8] & (0x80 >> (x % 8)))
// 			{
// 				printf("*");
// 			}
// 			else
// 			{
// 				printf(".");
// 			}
// 		}
// 		printf("\n");
// 		fpos = fpos + (pw + 7) / 8;
// 	}
// 	printf("\n");
// }

// // Bitmapの表示
// void ShowBitmap(uint8_t *bitmap, uint8_t pw, uint8_t ph)
// {
// 	int x, y, fpos;
// 	printf("[ShowBitmap pw=%d ph=%d]\n", pw, ph);
// #if 0
// 	for (y=0;y<(ph+7)/8;y++) {
// 		for (x=0;x<pw;x++) {
// 			printf("%02x ",bitmap[x+y*32]);
// 		}
// 		printf("\n");
// 	}
// #endif

// 	fpos = 0;
// 	for (y = 0; y < ph; y++)
// 	{
// 		printf("%02d", y);
// 		for (x = 0; x < pw; x++)
// 		{
// 			// printf("b=%x m=%x\n",bitmap[x+(y/8)*32],0x80 >> fpos);
// 			if (bitmap[x + (y / 8) * 32] & (0x80 >> fpos))
// 			{
// 				printf("*");
// 			}
// 			else
// 			{
// 				printf(".");
// 			}
// 		}
// 		printf("\n");
// 		fpos++;
// 		if (fpos > 7)
// 			fpos = 0;
// 	}
// 	printf("\n");
// }

// // 8ビットデータを反転
// uint8_t RotateByte(uint8_t ch1)
// {
// 	uint8_t ch2 = 0;
// 	int j;
// 	for (j = 0; j < 8; j++)
// 	{
// 		ch2 = (ch2 << 1) + (ch1 & 0x01);
// 		ch1 = ch1 >> 1;
// 	}
// 	return ch2;
// }

#if 0

// UTF code(3Byte) を SJIS Code(2 Byte) に変換
// https://www.mgo-tec.com/blog-entry-utf8sjis01.html
#endif
