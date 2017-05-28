#include <GPNBadge.hpp>
#include <FS.h>

#include "rboot.h"
#include "rboot-api.h"
#include <stdint.h>
#include <stdlib.h>

#include "gpn-tft.hpp"
#include "gpn-random.hpp"

Badge badge;

void setup()
{
  Serial.begin(115200);

  badge.init();
  badge.setBacklight(true);
  delay(300);
  badge.setGPIO(MQ3_EN,1);
  rboot_config rboot_config = rboot_get_config();
  SPIFFS.begin();
  File f = SPIFFS.open("/rom"+String(rboot_config.current_rom),"w");
  f.println("Fire!\n");
  tft.begin();
  tft.setTextSize(3);
  tft.setRotation(2);
  tft.scroll(32);
  initPalette();

}

void loop(void)
{
 ram();
}

#undef RGB
#define RGB(r,g,b) (tft.Color565(r,g,b))

#define WIDTH 128
#define HEIGHT 30

static uint16_t fire[WIDTH * HEIGHT];
static uint16_t palette[256];

static void initPalette()
{
    int i;

    for (i = 0; i < 0x20; i++) 
    {
      unsigned char double_i = i << 1;
      unsigned char quad_i = i << 2;
      unsigned char octo_i = i << 3;

      palette[i] = RGB(0, 0, double_i);
      palette[i + 32] = RGB(octo_i, 0, 64 - double_i);
      palette[i + 64] = RGB(255, octo_i, 0);
      palette[i + 96] = RGB(0xff, 0xff, quad_i);
      palette[i + 128] = RGB(0xff, 0xff, 64 + quad_i);
      palette[i + 160] = RGB(0xff, 0xff, 128 + quad_i);
      palette[i + 192] = RGB(0xff, 0xff, 192 + i);
      palette[i + 224] = RGB(0xff, 0xff, 224 + i);
    }
}

static void fireInit()
{
    int i, j;

    initPalette();

    for (i = WIDTH; i >= 0; i--) {
  for (j = 130 - HEIGHT; j >= 0; j--)
      lcdSetPixel(i, j, 0);
    }
}

static void fireUpdate()
{
    int i;
    int j = WIDTH * (HEIGHT - 1);
    int k;

    for (i = 0; i < WIDTH - 1; i++)
  fire[j + i] = 0xff * (1 + (getRandom() & 0xf) > 10);

    for (k = 0; k < HEIGHT - 1; k++) {
  fire[j - WIDTH] = (fire[j] + fire[j + 1] + fire[j - WIDTH]) / 3;

  for (i = 1; i < WIDTH - 1; i++) {
      short temp =
    (fire[j + i] + fire[j + i + 1] + fire[j + i - 1] +
     fire[j - WIDTH + i]) >> 2;
      fire[j - WIDTH + i] = temp - (temp > 1);
  }
  j -= WIDTH;
    }
}

void ram(void)
{
    fireInit();

    do {
  int i, j;

  lcdDisplay();

  fireUpdate();
  for (i = HEIGHT - 1; i >= 0; i--) {
      for (j = WIDTH - 1; j >= 0; j--) {
    lcdSetPixel(j, i + 98, palette[fire[i * WIDTH + j]]);
      }
      ESP.wdtFeed();
  }
    } while (true);
}

