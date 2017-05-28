#ifndef _GPN_TFT_H
#define _GPN_TFT_H
void lcdSetPixel(uint16_t x, uint16_t y, uint16_t color);
void lcdDisplay();
void drawHLine(int y, int x1, int x2, uint8_t color);
void drawVLine(int x, int y1, int y2, uint8_t color);

void drawRectFill(int x, int y, int width, int heigth, uint8_t color);
void drawLine(int x1, int y1, int x2, int y2, uint8_t color, int thickness);
#endif