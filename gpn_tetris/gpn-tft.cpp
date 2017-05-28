
#include <TFT_ILI9163C.h>

extern TFT_ILI9163C tft; 
#define RGB(r,g,b) (tft.Color565(r,g,b))
#define MAX_LINE_THICKNESS 16

#define SWAP(p1, p2) do { int SWAP = p1; p1 = p2; p2 = SWAP; } while (0)
#define ABS(p) (((p)<0) ? -(p) : (p))


 void lcdSetPixel(unsigned short x, unsigned short y, unsigned short color)
{
	//color = color >> 1;
	uint8_t r = (color & 0b11100000) >> 5;
	r = (r * 31) / 7; 

	uint8_t g = (color & 0b00011100) >> 2;
	g *= 9; 

	uint8_t b = (color & 0b00000011);
	b = (b * 31) / 3; 

	uint16_t colorLCD = 0;
	colorLCD = b;
	colorLCD |= (g << 5);
	colorLCD |= (r << 11); 

    tft.drawPixel(x, y,  colorLCD);
}

/* Achtung! Color conversion durch lookup Table funktioniert nicht richtig, keine Ahnung warum.
Wenn du es gefixt bekommst, bitte Pull Requesten ;) 
*/ 
/*
uint16_t colorLookup[255] = {
0, 10, 20, 31, 288, 298, 308, 319, 576, 586, 596, 607, 864, 874, 884, 895, 1152, 1162,
1172, 1183, 1440, 1450, 1460, 1471, 1728, 1738, 1748, 1759, 2016, 2026, 2036, 2047, 8192,
8202, 8212, 8223, 8480, 8490, 8500, 8511, 8768, 8778, 8788, 8799, 9056, 9066, 9076, 9087,
9344, 9354, 9364, 9375, 9632, 9642, 9652, 9663, 9920, 9930, 9940, 9951, 10208, 10218, 10228,
10239, 16384, 16394, 16404, 16415, 16672, 16682, 16692, 16703, 16960, 16970, 16980, 16991,
17248, 17258, 17268, 17279, 17536, 17546, 17556, 17567, 17824, 17834, 17844, 17855, 18112,
18122, 18132, 18143, 18400, 18410, 18420, 18431, 26624, 26634, 26644, 26655, 26912, 26922,
26932, 26943, 27200, 27210, 27220, 27231, 27488, 27498, 27508, 27519, 27776, 27786, 27796, 
27807, 28064, 28074, 28084, 28095, 28352, 28362, 28372, 28383, 28640, 28650, 28660, 28671,
34816, 34826, 34836, 34847, 35104, 35114, 35124, 35135, 35392, 35402, 35412, 35423, 35680,
35690, 35700, 35711, 35968, 35978, 35988, 35999, 36256, 36266, 36276, 36287, 36544, 36554, 
36564, 36575, 36832, 36842, 36852, 36863, 45056, 45066, 45076, 45087, 45344, 45354, 45364, 
45375, 45632, 45642, 45652, 45663, 45920, 45930, 45940, 45951, 46208, 46218, 46228, 46239, 
46496, 46506, 46516, 46527, 46784, 46794, 46804, 46815, 47072, 47082, 47092, 47103, 53248, 
53258, 53268, 53279, 53536, 53546, 53556, 53567, 53824, 53834, 53844, 53855, 54112, 54122, 
54132, 54143, 54400, 54410, 54420, 54431, 54688, 54698, 54708, 54719, 54976, 54986, 54996, 
55007, 55264, 55274, 55284, 55295, 63488, 63498, 63508, 63519, 63776, 63786, 63796, 63807,
64064, 64074, 64084, 64095, 64352, 64362, 64372, 64383, 64640, 64650, 64660, 64671, 64928, 
64938, 64948, 64959, 65216, 65226, 65236, 65247, 65504, 65514, 65524};
void lcdSetPixel(unsigned short x, unsigned short y, unsigned short color)
{
    tft.drawPixel(x, y, colorLookup[(uint8_t) color]);
}
*/

void lcdDisplay()
{
  tft.writeFramebuffer();
}
void lcdClear()
{
   tft.fillScreen(0xFFFF);
}

void drawHLine(int y, int x1, int x2, uint8_t color) {
	if(x1>x2) {
		SWAP(x1, x2);
	}
    for (int i=x1; i<=x2; ++i) {
        lcdSetPixel(i, y, color);
    }
}

void drawVLine(int x, int y1, int y2, uint8_t color) {
	if(y1>y2) {
		SWAP(y1,y2);
	}
    for (int i=y1; i<=y2; ++i) {
        lcdSetPixel(x, i, color);
    }
}

void drawRectFill(int x, int y, int width, int heigth, uint8_t color) {
    for (int i=y; i<y+heigth; ++i) {
        drawHLine(i, x, x+width-1, color);
    }
}

void drawLine(int x1, int y1, int x2, int y2, uint8_t color, int thickness)
{
	if(thickness<1) {
		thickness = 1;
	}
	if(thickness>MAX_LINE_THICKNESS) {
		thickness = MAX_LINE_THICKNESS;
	}
	bool xSwap = x1 > x2;
	bool ySwap = y1 > y2;
	if(x1==x2) {
		if(ySwap) { SWAP(y1,y2); }
		drawRectFill(x1-thickness/2, y1, thickness, y2-y1, color);
		return;
	}
	if(y1==y2) {
		if(xSwap) { SWAP(x1,x2); }
		drawRectFill(x1, y1-thickness/2, x2-x1, thickness, color);
		return;
	}
	if(xSwap){
		x1 = -x1;
		x2 = -x2;
	}
	if(ySwap){
		y1 = -y1;
		y2 = -y2;
	}
	bool mSwap = ABS(x2-x1) < ABS(y2-y1);
	if(mSwap) {
		SWAP(x1,y1);
		SWAP(x2,y2);
	}
	int dx = x2-x1;
	int dy = y2-y1;
	int D = 2*dy - dx;
	
	// lcdSetPixel(x1, y1, color);
	int y = y1;
	for(int x = x1; x <= x2; x++) {
		int px = mSwap ? y : x;
		if(xSwap) {
			px = -px;
		}
		int py = mSwap ? x : y;
		if(ySwap) {
			py = -py;
		}
		lcdSetPixel(px, py, color);
		if(D > 0) {
			y++;
			D += 2 * dy - 2 * dx;
		} else {
			D += 2 * dy;
		}
		
		for(int t=1; t<thickness; t++) {
			int offset = ((t-1)/2+1)*(t%2*2-1);
			int tx = px;
			int ty = py;
			if(mSwap) {
				tx += offset;
			} else {
				ty += offset;
			}
			lcdSetPixel(tx, ty, color);
		}
	}
}
