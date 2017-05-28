#include <GPNBadge.hpp>
#include <FS.h>

#include "rboot.h"
#include "rboot-api.h"
#include <stdint.h>
#include <stdlib.h>
#include <math.h>


#include "gpn-tft.hpp"
#include "gpn-random.hpp"
#include "itoa.hpp"
byte portExpanderConfig = 0; //stores the 74HC595 config

Badge badge;

#define GPIO_LCD_DC 0
#define GPIO_TX     1
#define GPIO_WS2813 4
#define GPIO_RX     3
#define GPIO_DN     2
#define GPIO_DP     5

#define GPIO_BOOT   16
#define GPIO_MOSI   13
#define GPIO_CLK    14
#define GPIO_LCD_CS 15
#define GPIO_BNO    12

#define MUX_JOY 0
#define MUX_BAT 1
#define MUX_LDR 2
#define MUX_ALK 4
#define MUX_IN1 5

#define VIBRATOR 3
#define MQ3_EN   4
#define LCD_LED  5
#define IR_EN    6
#define OUT1     7

#define UP      790
#define DOWN    630
#define RIGHT   530
#define LEFT    1024
#define OFFSET  30

#define I2C_PCA 0x25

#define NUM_LEDS    4

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define BTN_NONE 0
#define BTN_UP   (1<<0)
#define BTN_DOWN (1<<1)
#define BTN_LEFT (1<<2)
#define BTN_RIGHT (1<<3)
#define BTN_ENTER (1<<4)

int getInputRaw() {
  uint16_t adc = analogRead(A0);

  if (adc < UP + OFFSET && adc > UP - OFFSET)             return BTN_UP;
  else if (adc < DOWN + OFFSET && adc > DOWN - OFFSET)    return BTN_DOWN;
  else if (adc < RIGHT + OFFSET && adc > RIGHT - OFFSET)  return BTN_RIGHT;
  else if (adc < LEFT + OFFSET && adc > LEFT - OFFSET)    return BTN_LEFT;
  if (digitalRead(GPIO_BOOT) == 1) return BTN_ENTER;
  return 0;
}
void setGPIO(byte channel, boolean level) {
  bitWrite(portExpanderConfig, channel, level);
  Wire.beginTransmission(I2C_PCA);
  Wire.write(portExpanderConfig);
  Wire.endTransmission();
}

void setAnalogMUX(byte channel) {
  portExpanderConfig = portExpanderConfig & 0b11111000;
  portExpanderConfig = portExpanderConfig | channel;
  Wire.beginTransmission(I2C_PCA);
  Wire.write(portExpanderConfig);
  Wire.endTransmission();
}


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
  f.println("Tetris\n");
  tft.begin();
  tft.setTextSize(3);
  tft.setRotation(2);
  tft.scroll(32);
  tft.fillScreen(0x0000);
	setAnalogMUX(MUX_JOY);
    delay(20);
  setGPIO(LCD_LED, HIGH);

}

void loop(void)
{
	ram();
}

void delayms(uint16_t delayt)
{
	delay(delayt);
}

#define MAX_X (130)
#define MAX_Y (130)
#define BLOCK_SIZE (6)
#define ROW_OFFSET (10)
#define COL_OFFSET (10)
#define BLOCKS_PER_COL (20)
#define BLOCKS_PER_ROW (10)
#define BLOCKS_CNT (4)

#define RIGHT 0
#define LEFT 2
#define UP 3
#define DOWN 1

struct pos {
  int x,y;
};

struct brc {
    int row, col;
};

static int BLOCKS[7][4][8];
static int colors[7];

int bl[BLOCKS_PER_COL][BLOCKS_PER_ROW];
int lastBlocks[8];
struct Tile {
    int type;
    int rotation;
    struct pos pos;
};

struct Tile currentTile;
struct Tile nextTile;

int score;
int highscore;
int c;
int speed;
int lastKey;
int stop;


void ram(void)
{
    highscore = 0;

    initTetris();
    // initially reset everything
    newGame();

    while (1)
    {
        if(!(++c % speed) && !stop) {
            if( checkPos( currentTile.pos.x, currentTile.pos.y+1, currentTile.rotation ) ) {
                currentTile.pos.y++;
            } else {
                if( placeTile() ) {
                    stop = 1;
                    drawTile();
                    if( theEnd() ) return;
                } else {
                    speed--;
                    getNewTile();
                }
            }

            drawTile();
        }
        
        if( handleInput() ) {
            drawTile();
        }
        delayms(3);
    }
}

static bool handleInput()
{
    struct Tile ct = currentTile;
    int key = getInputRaw();
    bool ret = false;

    if (lastKey != key) {
        if(key&BTN_ENTER || key&BTN_UP) {
            int rotation = (ct.rotation+1)%4; 
            if( checkPos( ct.pos.x, ct.pos.y, rotation ) ) {
                currentTile.rotation = rotation;
                ret = true;
            } else {
                if( checkPos( ct.pos.x-1, ct.pos.y, rotation ) ) {
                    currentTile.rotation = rotation;
                    currentTile.pos.x--;
                    ret = true;
                } else if(checkPos( ct.pos.x+1, ct.pos.y, rotation ) ) {
                    currentTile.rotation = rotation;
                    currentTile.pos.x++;
                    ret = true;
                } 
            }
        } else if( key&BTN_LEFT ) {
            if( checkPos(ct.pos.x-1,ct.pos.y,ct.rotation) ) {
                currentTile.pos.x--;
                ret = true;
            }
        } else if( key&BTN_RIGHT ) {
            if( checkPos(ct.pos.x+1,ct.pos.y,ct.rotation) ) {
                currentTile.pos.x++;
                ret = true;
            } 
        } else if( key&BTN_DOWN ) {
            int i;
            for( i=ct.pos.y+1 ; i<BLOCKS_PER_COL && checkPos(ct.pos.x, i,ct.rotation) ; i++ );

            currentTile.pos.y = i-1;
            ret = true;
        }
    }
    lastKey = key;
	
    return ret;
};

static bool checkPos(int x, int y, int rotation) {
    struct Tile ct = currentTile;

    if( ct.type < 0 || ct.type >= 7 || rotation < 0 || rotation >= 4 ) return true;

    int *tile = BLOCKS[ct.type][rotation],
        i;

    for( i=0 ; i<BLOCKS_CNT ; i++ ) {
        int x1 = x+tile[i*2],
            y1 = y+tile[i*2+1];

        if(x1 < 0 || x1 >= BLOCKS_PER_ROW) {
            return false;
        }
        if(y1 >= BLOCKS_PER_COL ) {
            return false;
        }
        if( y1 >= 0 && bl[y1][x1] != 0 ) {
            return false;
        }
    }

    return true;
};

static void getNewTile() {
    bool first = nextTile.type == -1? true:false;
    //bool init = true;

    currentTile.type     = first? getRandom()%7 : nextTile.type;
    currentTile.rotation = first? getRandom()%4 : nextTile.rotation;
    currentTile.pos.x    = 5;
    currentTile.pos.y    = -2;

    nextTile.type = getRandom()%7;
    nextTile.rotation = getRandom()%4;

    drawNextTile();
}; 

static bool placeTile() {
    int i,j,i1,j1,
        minY = BLOCKS_PER_COL-1,
        maxY = 0;
    struct Tile ct = currentTile;
    
    if( ct.type < 0 || ct.type >= 7 || ct.rotation < 0 || ct.rotation >= 4 ) return false;
    
    int *tile = BLOCKS[ct.type][ct.rotation];
    
    lastBlocks[0] = -1;
    
    for( i=0 ; i<BLOCKS_CNT ; i++ ) {
        int x = ct.pos.x + tile[i*2],
            y = ct.pos.y + tile[i*2+1];
       
        if( x >= 0 && x < BLOCKS_PER_ROW && y >= 0 && y < BLOCKS_PER_COL ) bl[y][x] = colors[ct.type]; 
        if( minY > y ) minY = y;
        if( maxY < y ) maxY = y;
    }

    if( maxY <= 1 ) return true;

    int redraw = false;
    for( i=minY ; i<=maxY ; i++ ) {
        int complete = true,
            cnt = 0;
        for( j=0 ; j<BLOCKS_PER_ROW ; j++ ) {
            if( bl[i][j] == 0 ) {
                complete = false;
                cnt++;
            }
        }
        if( complete ) {
            for( i1=i ; i1>=1 ; i1-- ) for( j1=0 ; j1<BLOCKS_PER_ROW ; j1++ ) bl[i1][j1] = bl[i1-1][j1];
            score += BLOCKS_PER_COL-i;
            redraw = true;
        }
    }

    if( redraw == true ) {
        for( i=maxY ; i>=1 ; i-- ) {
            for( j=0 ; j<BLOCKS_PER_ROW ; j++ ) {
                int color = bl[i][j];
                if( color == 0 ) color = 0xff;
                drawBlock(j,i,color); 
            }
        }

        DoString(85,50,IntToStr(score,6,0));
        if( score > highscore ) {
            highscore = score;
            DoString(85,80,IntToStr(highscore,6,0));
        }   
    }

    lcdDisplay();
    return false;
};

static void initTetris() {

    int i,j,k;
    static int blocks[7][4][8] = {{
            { -2, 0,  -1, 0,   0, 0,   1, 0 },
            {  0,-1,   0, 0,   0, 1,   0, 2 },
            { -2, 0,  -1, 0,   0, 0,   1, 0 },
            {  0,-1,   0, 0,   0, 1,   0, 2 }
        },{
            { -1,-1,  -1, 0,   0, 0,   0,-1 }, 
            { -1,-1,  -1, 0,   0, 0,   0,-1 }, 
            { -1,-1,  -1, 0,   0, 0,   0,-1 }, 
            { -1,-1,  -1, 0,   0, 0,   0,-1 } 
        },{
            { -1, 0,   0, 0,   1, 0,   1, 1 },
            {  0,-1,   0, 0,   0, 1,  -1, 1 },
            { -1,-1,  -1, 0,   0, 0,   1, 0 },
            {  0,-1,   1,-1,   0, 0,   0, 1 }
        },{
            { -1, 0,   0, 0,   1, 0,   1,-1 },
            {  0,-1,   0, 0,   0, 1,   1, 1 },
            { -1, 1,  -1, 0,   0, 0,   1, 0 },
            {  0,-1,  -1,-1,   0, 0,   0, 1 }
        },{
            { -1, 0,   0, 0,   1, 0,   0,-1 },
            {  0,-1,   0, 0,   0, 1,   1, 0 },
            { -1, 0,   0, 0,   1, 0,   0, 1 },
            {  0,-1,   0, 0,   0, 1,  -1, 0 }
        },{
            { -1, 0,   0, 0,   0, 1,   1, 1 },
            {  0,-1,   0, 0,  -1, 0,  -1, 1 },
            { -1, 0,   0, 0,   0, 1,   1, 1 },
            {  0,-1,   0, 0,  -1, 0,  -1, 1 },
        },{
            { -1, 0,   0, 0,   0,-1,   1,-1 },
            {  0,-1,   0, 0,   1, 0,   1, 1 },
            { -1, 0,   0, 0,   0,-1,   1,-1 },
            {  0,-1,   0, 0,   1, 0,   1, 1 }
        }};

    for( i=0 ; i<7 ; i++ ) for( j=0 ; j<4 ; j++ ) for( k=0 ; k<8 ; k++ ) BLOCKS[i][j][k] = blocks[i][j][k];

    static int c[7] = { 0b0000000111, 0b111000000, 0b000111000, 0b111111000, 0b000111111, 0b111000111, 0b010101010 }; 
    for( i=0 ; i<7 ; i++ ) colors[i] = c[i];

    lastBlocks[0] = -1;
};

static void newGame() {
    int i,j;

    struct Tile nt = {.type = -1, .rotation = 0, .pos = { .x=0, .y=0 }};
    struct Tile ct = {.type = -1, .rotation = 0, .pos = { .x=0, .y=0 }};

    nextTile = nt;
    currentTile = ct;

    score = 0;
    stop = 0;
    speed = 333;
    lastKey = 0;
    stop = 0;

    // setup the screen
    lcdClear();
    for (i=0 ; i<MAX_Y; i++) {
        lcdSetPixel(COL_OFFSET + BLOCKS_PER_ROW * BLOCK_SIZE,i,0b000101011);
        lcdSetPixel(COL_OFFSET + BLOCKS_PER_ROW * BLOCK_SIZE + 2,i,0b000101011);
    }
    for (i=0 ; i<MAX_Y; i++) {
        lcdSetPixel(COL_OFFSET-1,i,0b000101011);
        lcdSetPixel(COL_OFFSET-3,i,0b000101011);
    } 

    for(i=0 ; i<BLOCKS_PER_COL ; i++ ) {
        for( j=0 ; j<BLOCKS_PER_ROW ; j++ ) {
            bl[i][j] = 0;
        }
    }

    DoString(85,40,"Score");
    DoString(85,50,IntToStr(score,6,0));
    DoString(85,70,"High");
    DoString(85,80,IntToStr(highscore,6,0));
    getNewTile();

    lcdDisplay();
}

static void drawTile() {
    int i;
    struct Tile ct = currentTile;
    
    if( ct.type < 0 || ct.type >= 7 || ct.rotation < 0 || ct.rotation >= 4 ) return;
    
    int *tile = BLOCKS[ct.type][ct.rotation];
    struct pos pos = currentTile.pos;

    if( lastBlocks[0] != -1 ) {
        for( i=0 ; i<BLOCKS_CNT ; i++) {
            drawBlock( lastBlocks[i*2], lastBlocks[i*2+1], 0xff );
        }
    }

    for( i=0 ; i<BLOCKS_CNT ; i++ ) {
        int x = pos.x + tile[i*2],
            y = pos.y + tile[i*2+1];
        
        drawBlock( x, y, colors[ct.type] );
        lastBlocks[i*2] = x;
        lastBlocks[i*2+1] = y;
    }
    
    lcdDisplay();
};

static void drawNextTile() {

    int i,j;
    struct Tile ct = nextTile;
    
    int *tile = BLOCKS[ct.type][ct.rotation];

    for( i=0 ; i<4 ; i++ ) for( j=0 ; j<4 ; j ++ ) drawBlock( 13+i , j , 0xff);
    for( i=0 ; i<BLOCKS_CNT ; i++ ) {
        int x = 15 + tile[i*2],
            y = 1 + tile[i*2+1];
        
        drawBlock( x, y, colors[ct.type] );
    }
    
    lcdDisplay();
};

static void drawBlock(int x, int y, int f) {
  x = x * BLOCK_SIZE + COL_OFFSET;
  y = y * BLOCK_SIZE + ROW_OFFSET;

  lcdSetPixel(x  , y,   f);
  lcdSetPixel(x+1, y,   f);
  lcdSetPixel(x+2, y,   f);
  lcdSetPixel(x+3, y,   f);
  lcdSetPixel(x+4, y,   f);
  lcdSetPixel(x+5, y,   f);
  lcdSetPixel(x,   y+1, f);
  lcdSetPixel(x+1, y+1, f);
  lcdSetPixel(x+2, y+1, f);
  lcdSetPixel(x+3, y+1, f);
  lcdSetPixel(x+4, y+1, f);
  lcdSetPixel(x+5, y+1, f);
  lcdSetPixel(x,   y+2, f);
  lcdSetPixel(x+1, y+2, f);
  lcdSetPixel(x+2, y+2, f);
  lcdSetPixel(x+3, y+2, f);
  lcdSetPixel(x+4, y+2, f);
  lcdSetPixel(x+5, y+2, f);
  lcdSetPixel(x,   y+3, f);
  lcdSetPixel(x+1, y+3, f);
  lcdSetPixel(x+2, y+3, f);
  lcdSetPixel(x+3, y+3, f);
  lcdSetPixel(x+4, y+3, f);
  lcdSetPixel(x+5, y+3, f);
  lcdSetPixel(x,   y+4, f);
  lcdSetPixel(x+1, y+4, f);
  lcdSetPixel(x+2, y+4, f);
  lcdSetPixel(x+3, y+4, f);
  lcdSetPixel(x+4, y+4, f);
  lcdSetPixel(x+5, y+4, f);
  lcdSetPixel(x,   y+5, f);
  lcdSetPixel(x+1, y+5, f);
  lcdSetPixel(x+2, y+5, f);
  lcdSetPixel(x+3, y+5, f);
  lcdSetPixel(x+4, y+5, f);
  lcdSetPixel(x+5, y+5, f);
}

static bool theEnd() {
    DoString(32,95,"GAME OVER");
    DoString(15,115,"Press button...");
    lcdDisplay();
    
    //setHighscore(highscore);
    
    while(1) {
       int key = getInputRaw();
       if( key == 0 ) break;
    }
    while(1) {
        int key = getInputRaw();
        if( key&BTN_LEFT ) {
            return true;
        } else if( key != 0 ) {
            newGame();
            stop = 0;
            return false;
        }
        delayms(3);
    }
}

void DoString (uint16_t x, uint16_t y, const char* data)
{
	  tft.setTextSize(1);

	tft.setTextColor(0x0000, 0xFFFF);
  	tft.setCursor(x,y);
  	tft.print(data);
	return;
}