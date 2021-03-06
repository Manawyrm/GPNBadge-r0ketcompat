#include "gpn-tft.hpp"
#include <TFT_ILI9163C.h>

extern TFT_ILI9163C tft;
#define RGB(r, g, b)    (tft.Color565(r, g, b))
#define MAX_LINE_THICKNESS    16

#define SWAP(p1, p2)    do { int SWAP = p1; p1 = p2; p2 = SWAP; } while (0)
#define ABS(p)          (((p) < 0) ? -(p) : (p))

void setTextColor(uint8_t bg, uint8_t fg)
{
	tft.setTextSize(1);
	tft.setTextColor(colorConvert(fg), colorConvert(bg));
}

uint16_t colorConvert(uint8_t color)
{
	uint8_t r = (color & 0b11100000) >> 5;

	r = (r * 31) / 7;

	uint8_t g = (color & 0b00011100) >> 2;
	g *= 9;

	uint8_t b = (color & 0b00000011);
	b = (b * 31) / 3;

	uint16_t colorLCD = 0;
	colorLCD  = b;
	colorLCD |= (g << 5);
	colorLCD |= (r << 11);
	return(colorLCD);
}

void lcdSetCrsr(uint8_t x, uint8_t y)
{
	tft.setCursor(x, y);
	return;
}

void lcdPrint(const char *text)
{
	tft.print(text);
	return;
}

void setIntFont(char *font)
{
	return;
}

void lcdFill(uint8_t color)
{
	tft.fillScreen(colorConvert(color));
}

void lcdSetPixel(unsigned short x, unsigned short y, unsigned short color)
{
	tft.drawPixel(x, y, colorConvert(color));
}

void lcdDisplay()
{
	tft.writeFramebuffer();
}

void DoString(uint16_t x, uint16_t y, const char *data)
{
	tft.setTextSize(1);
	tft.setCursor(x, y);
	tft.print(data);
	return;
}

void DoChar(uint16_t x, uint16_t y, const char data)
{
	tft.setTextSize(1);
	tft.setCursor(x, y);
	tft.print(data);
	return;
}

void drawHLine(int y, int x1, int x2, uint8_t color)
{
	if (x1 > x2)
	{
		SWAP(x1, x2);
	}
	for (int i = x1; i <= x2; ++i)
	{
		lcdSetPixel(i, y, color);
	}
}

void drawVLine(int x, int y1, int y2, uint8_t color)
{
	if (y1 > y2)
	{
		SWAP(y1, y2);
	}
	for (int i = y1; i <= y2; ++i)
	{
		lcdSetPixel(x, i, color);
	}
}

void drawRectFill(int x, int y, int width, int heigth, uint8_t color)
{
	for (int i = y; i < y + heigth; ++i)
	{
		drawHLine(i, x, x + width - 1, color);
	}
}

void drawLine(int x1, int y1, int x2, int y2, uint8_t color, int thickness)
{
	if (thickness < 1)
	{
		thickness = 1;
	}
	if (thickness > MAX_LINE_THICKNESS)
	{
		thickness = MAX_LINE_THICKNESS;
	}
	bool xSwap = x1 > x2;
	bool ySwap = y1 > y2;
	if (x1 == x2)
	{
		if (ySwap)
		{
			SWAP(y1, y2);
		}
		drawRectFill(x1 - thickness / 2, y1, thickness, y2 - y1, color);
		return;
	}
	if (y1 == y2)
	{
		if (xSwap)
		{
			SWAP(x1, x2);
		}
		drawRectFill(x1, y1 - thickness / 2, x2 - x1, thickness, color);
		return;
	}
	if (xSwap)
	{
		x1 = -x1;
		x2 = -x2;
	}
	if (ySwap)
	{
		y1 = -y1;
		y2 = -y2;
	}
	bool mSwap = ABS(x2 - x1) < ABS(y2 - y1);
	if (mSwap)
	{
		SWAP(x1, y1);
		SWAP(x2, y2);
	}
	int dx = x2 - x1;
	int dy = y2 - y1;
	int D  = 2 * dy - dx;

	// lcdSetPixel(x1, y1, color);
	int y = y1;
	for (int x = x1; x <= x2; x++)
	{
		int px = mSwap ? y : x;
		if (xSwap)
		{
			px = -px;
		}
		int py = mSwap ? x : y;
		if (ySwap)
		{
			py = -py;
		}
		lcdSetPixel(px, py, color);
		if (D > 0)
		{
			y++;
			D += 2 * dy - 2 * dx;
		}
		else
		{
			D += 2 * dy;
		}

		for (int t = 1; t < thickness; t++)
		{
			int offset = ((t - 1) / 2 + 1) * (t % 2 * 2 - 1);
			int tx     = px;
			int ty     = py;
			if (mSwap)
			{
				tx += offset;
			}
			else
			{
				ty += offset;
			}
			lcdSetPixel(tx, ty, color);
		}
	}
}
