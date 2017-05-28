
#include <TFT_ILI9163C.h>

extern TFT_ILI9163C tft; 
#define RGB(r,g,b) (tft.Color565(r,g,b))

void lcdSetPixel(uint16_t x, uint16_t y, uint16_t color)
{
    tft.drawPixel(x, y, color);
}

void lcdDisplay()
{
  tft.writeFramebuffer();
}
