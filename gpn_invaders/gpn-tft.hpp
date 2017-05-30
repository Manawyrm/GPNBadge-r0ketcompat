#ifndef _GPN_TFT_H
#define _GPN_TFT_H

#include <stdint.h>

void lcdSetPixel(uint16_t x, uint16_t y, uint16_t color);
void lcdDisplay();
void drawHLine(int y, int x1, int x2, uint8_t color);
void drawVLine(int x, int y1, int y2, uint8_t color);
void setTextColor(uint8_t fg, uint8_t bg);
void drawRectFill(int x, int y, int width, int heigth, uint8_t color);
void drawLine(int x1, int y1, int x2, int y2, uint8_t color, int thickness);
void lcdFill(uint8_t color);
void lcdSetCrsr(uint8_t x, uint8_t y);
void lcdPrint(const char *text);
void setIntFont(char *font);
uint16_t colorConvert(uint8_t color);
void DoString(uint16_t x, uint16_t y, const char *data);
void DoChar(uint16_t x, uint16_t y, const char data);

#endif
