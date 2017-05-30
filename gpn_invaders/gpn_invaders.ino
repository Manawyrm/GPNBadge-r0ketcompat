#include <GPNBadge.hpp>
#include <FS.h>

#include "rboot.h"
#include "rboot-api.h"
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "gpn-tft.hpp"
#include "gpn-random.hpp"
#include "gpn-input.hpp"
#include "itoa.hpp"

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
  f.println("Invaders\n");
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
const uint8_t playerBitmap[] = {
 /* Char 80 is 7px wide @ 31 */
  0xc0,  /* **       */ 
  0xec,  /* *** **   */ 
  0x7e,  /*  ******  */ 
  0x3f,  /*   ****** */ 
  0x7e,  /*  ******  */ 
  0xec,  /* *** **   */ 
  0xc0,  /* **       */ 
};
const uint8_t invaderABitmap[] = {
 /* Char 65 is 11px wide @ 0 */
  0x70,  /*  ***     */ 
  0x18,  /*    **    */ 
  0x7d,  /*  ***** * */ 
  0xb6,  /* * ** **  */ 
  0xbc,  /* * ****   */ 
  0x3c,  /*   ****   */ 
  0xbc,  /* * ****   */ 
  0xb6,  /* * ** **  */ 
  0x7d,  /*  ***** * */ 
  0x18,  /*    **    */ 
  0x70,  /*  ***     */ 
};
const uint8_t invaderBBitmap[] = {
 /* Char 66 is 12px wide @ 11 */
  0x9c,  /* *  ***   */ 
  0x9e,  /* *  ****  */ 
  0x5e,  /*  * ****  */ 
  0x76,  /*  *** **  */ 
  0x37,  /*   ** *** */ 
  0x5f,  /*  * ***** */ 
  0x5f,  /*  * ***** */ 
  0x37,  /*   ** *** */ 
  0x76,  /*  *** **  */ 
  0x5e,  /*  * ****  */ 
  0x9e,  /* *  ****  */ 
  0x9c,  /* *  ***   */ 
};
const uint8_t invaderCBitmap[] = {
 /* Char 67 is 8px wide @ 23 */
  0x58,  /*  * **    */ 
  0xbc,  /* * ****   */ 
  0x16,  /*    * **  */ 
  0x3f,  /*   ****** */ 
  0x3f,  /*   ****** */ 
  0x16,  /*    * **  */ 
  0xbc,  /* * ****   */ 
  0x58,  /*  * **    */ 
};
const uint8_t invader2ABitmap[] = {
 /* Char 97 is 11px wide @ 54 */
  0x9e,  /* *  ****  */ 
  0x38,  /*   ***    */ 
  0x7d,  /*  ***** * */ 
  0x36,  /*   ** **  */ 
  0x3c,  /*   ****   */ 
  0x3c,  /*   ****   */ 
  0x3c,  /*   ****   */ 
  0x36,  /*   ** **  */ 
  0x7d,  /*  ***** * */ 
  0x38,  /*   ***    */ 
  0x9e,  /* *  ****  */ 
};
const uint8_t invader2BBitmap[] = {
 /* Char 98 is 12px wide @ 65 */
  0x1c,  /*    ***   */ 
  0x5e,  /*  * ****  */ 
  0xfe,  /* *******  */ 
  0xb6,  /* * ** **  */ 
  0x37,  /*   ** *** */ 
  0x5f,  /*  * ***** */ 
  0x5f,  /*  * ***** */ 
  0x37,  /*   ** *** */ 
  0xb6,  /* * ** **  */ 
  0xfe,  /* *******  */ 
  0x5e,  /*  * ****  */ 
  0x1c,  /*    ***   */ 
};
const uint8_t invader2CBitmap[] = {
 /* Char 99 is 8px wide @ 77 */
  0x98,  /* *  **    */ 
  0x5c,  /*  * ***   */ 
  0xb6,  /* * ** **  */ 
  0x5f,  /*  * ***** */ 
  0x5f,  /*  * ***** */ 
  0xb6,  /* * ** **  */ 
  0x5c,  /*  * ***   */ 
  0x98,  /* *  **    */ 
};
const uint8_t ufoBitmap[] = {
 /* Char 85 is 16px wide @ 38 */
  0x20,  /*   *     */ 
  0x30,  /*   **    */ 
  0x78,  /*  ****   */ 
  0xec,  /* *** **  */ 
  0x7c,  /*  *****  */ 
  0x3c,  /*   ****  */ 
  0x2e,  /*   * *** */ 
  0x7e,  /*  ****** */ 
  0x7e,  /*  ****** */ 
  0x2e,  /*   * *** */ 
  0x3c,  /*   ****  */ 
  0x7c,  /*  *****  */ 
  0xec,  /* *** **  */ 
  0x78,  /*  ****   */ 
  0x30,  /*   **    */ 
  0x20,  /*   *     */ 
};

#define RESX 128
#define RESY 128

#define POS_PLAYER_Y RESY-8
#define POS_PLAYER_X RESX/2-3
#define POS_UFO_Y 0
#define ENEMY_ROWS 3
#define ENEMY_COLUMNS 6
#define DISABLED 255

#define UFO_PROB 1024

#define TYPE_PLAYER  1
#define TYPE_ENEMY_A 3
#define TYPE_ENEMY_B 2
#define TYPE_ENEMY_C 4
#define TYPE_UFO     5

#define BUNKERS 4
#define BUNKER_WIDTH  10
#define BUNKER_X(b) ( RESX / (BUNKERS+1) * (b+1) - BUNKER_WIDTH/2 )
static const uint8_t ENEMY_WIDTHS[] = {8,10,12};


struct gamestate {
    char player;
	char ufo;
    char shot_x, shot_y;
	char shots_x[ENEMY_COLUMNS];
	char shots_y[ENEMY_COLUMNS];
    char alive;
    int16_t move;
	char direction, lastcol;
    bool killed;
	bool step;
	uint32_t score;
	uint16_t level;
	int8_t rokets;
    char enemy_x[ENEMY_ROWS][ENEMY_COLUMNS];
    char enemy_row_y[ENEMY_ROWS];
    uint8_t bunker[BUNKERS][BUNKER_WIDTH];
} game;
char key;

void ram(void) {
	while(1) {
		if (!screen_intro()){
      setTextColor(0xff,0x00);
      return;
    }
		game.rokets = 3;
		game.level = 1;
		game.score = 0;
		init_game();
		screen_level();
		while (game.rokets>=0) {
			////checkISP();
			lcdFill(0x00);
			check_end();
			move_ufo();
			move_shot();
			move_shots();
			move_player();
			move_enemy();
			draw_score();
			draw_ufo();
			draw_bunker();
			draw_player();
			draw_enemy();
			draw_shots();
			//        draw_status();
			lcdDisplay();
			delayms(12);
		}
		if (!screen_gameover())
      setTextColor(0xff,0x00);
      return;
	}
}

static bool screen_intro() {
	uint32_t highscore;
	char highnick[20];
	char key=0;
	bool step = false;
	//getInputWaitRelease();
	while(key==0) {
		getInputWaitRelease();
		lcdFill(0x00);
		//setIntFont(&Font_Invaders);
    setTextColor(0x00,0b11100000);
    //lcdSetCrsr(28+18,25+15);lcdPrint(step?"ABC":"abc");
    	if (step)
		{
			drawInvBitmap(28+18, 25+15, invaderABitmap, 11, 8, 0b11100000);
			drawInvBitmap(28+18+12, 25+15, invaderBBitmap, 12, 8, 0b11100000);
			drawInvBitmap(28+18+12+13, 25+15, invaderCBitmap, 8, 8, 0b11100000);
		}
		else
		{
			drawInvBitmap(28+18, 25+15, invader2ABitmap, 11, 8, 0b11100000);
			drawInvBitmap(28+18+12, 25+15, invader2BBitmap, 12, 8, 0b11100000);
			drawInvBitmap(28+18+12+13, 25+15, invader2CBitmap, 8, 8, 0b11100000);
		}
		//setIntFont(&Font_7x8);
    setTextColor(0x00,0b00011000);
		lcdSetCrsr(28+18,40+15);lcdPrint("SPACE");
    setTextColor(0x00,0b11111000);
		lcdSetCrsr(18+18,50+15);lcdPrint("INVADERS");

		highscore = highscore_get("Player");
    setTextColor(0x00,0xff);
		lcdSetCrsr(0,0);lcdPrint(IntToStr(highscore,6,F_LONG));
//		lcdSetCrsr(0,9);lcdPrint(highnick);
		lcdDisplay();
		ESP.wdtFeed();
		step = !step;
		key=getInputWaitTimeout(1000);
		ESP.wdtFeed();
	}
	//getInputWaitRelease();
	return !(key==BTN_LEFT);
}

static bool screen_gameover() {
	char key =0;
	while(key==0) {
		lcdFill(0x00);
		//setIntFont(&Font_7x8);
    setTextColor(0x00,0b11100000);
		lcdSetCrsr(14+15,32+15);lcdPrint("GAME OVER");
    setTextColor(0x00,0xff);
		lcdSetCrsr(0,0);lcdPrint(IntToStr(game.score,6,F_LONG));
		if (highscore_set(game.score, "Player")){
      setTextColor(0x00,0b00011000);
      lcdSetCrsr(0,9);lcdPrint("HIGHSCORE!");
		};
		lcdDisplay();
		key=getInputWaitTimeout(5000);
	}
	//getInputWaitRelease();
	return !(key==BTN_LEFT);
}

static void screen_level() {
	lcdFill(0x00);
	draw_score();
	//setIntFont(&Font_7x8);
	lcdSetCrsr(20,32);
  setTextColor(0x00,0xff);
	lcdPrint("Level ");
	lcdPrint(IntToStr(game.level,3,0));
	lcdDisplay();
	delayms_queue(500);
}

static bool highscore_set(uint32_t score, char nick[]) {
#if 0
    MPKT * mpkt= meshGetMessage('i');
    if(MO_TIME(mpkt->pkt)>score)
        return false;

    MO_TIME_set(mpkt->pkt,score);
    strcpy((char*)MO_BODY(mpkt->pkt),nick);
    if(GLOBAL(privacy)==0){
        uint32touint8p(GetUUID32(),mpkt->pkt+26);
        mpkt->pkt[25]=0;
    };
#endif
	return true;
}

static uint32_t highscore_get(char nick[]){
#if 0
    MPKT * mpkt= meshGetMessage('i');
    char * packet_nick = (char*)MO_BODY(mpkt->pkt);
    // the packet crc end is already zeroed
    if(MAXNICK<MESHPKTSIZE-2-6-1)
        packet_nick[MAXNICK-1] = 0;
    strcpy(nick, packet_nick);
	return MO_TIME(mpkt->pkt);
#else
	return 0;
#endif
}

static void init_game(void) {
	game.player = POS_PLAYER_X;
	game.shot_x = DISABLED;
	game.shot_y = 0;
	game.alive = ENEMY_ROWS*ENEMY_COLUMNS;
	game.move = 0;
	if (getRandom()%2 == 0) {
		game.direction = -1;
		game.lastcol = ENEMY_COLUMNS-1;
	} else {
		game.direction = 1;
		game.lastcol = 0;
	}
	game.killed = 0;
	game.step = false;
	game.ufo = DISABLED;
	init_enemy();

	for (int col=0; col<ENEMY_COLUMNS; col++){
	   game.shots_x[col] = DISABLED;
   	}

	for (int b=0; b<BUNKERS; b++){
		//for (int slice=0; slice<BUNKER_WIDTH; slice++){
		//	game.bunker[b][slice] = 255<<2;
		//}
		game.bunker[b][0] = 0b00111100;
		game.bunker[b][1] = 0b01111100;
		game.bunker[b][2] = 0b11111100;
		game.bunker[b][3] = 0b11100000;
		game.bunker[b][4] = 0b11100000;
		game.bunker[b][5] = 0b11100000;
		game.bunker[b][6] = 0b11100000;
		game.bunker[b][7] = 0b11111100;
		game.bunker[b][8] = 0b01111100;
		game.bunker[b][9] = 0b00111100;
	}
}

static void init_enemy() {
    for (int row = 0; row<ENEMY_ROWS; row++) {
        game.enemy_row_y[row] = 10 + (40/ENEMY_ROWS)*row;
        for (int col = 0; col<ENEMY_COLUMNS; col++) {
            game.enemy_x[row][col] = 5+(86/ENEMY_COLUMNS)*col+(2-row);
        }
    }
}

static bool check_bunker(char xpos, char ypos, int8_t shift){
	for (int b=0; b<BUNKERS; b++) {
		if (xpos>BUNKER_X(b) &&
				xpos<BUNKER_X(b)+BUNKER_WIDTH &&
				ypos<RESY-8 &&
				ypos>RESY-16) {
			int offset = xpos-BUNKER_X(b);
			if (game.bunker[b][offset]!=0) {
				if (shift>0)
					game.bunker[b][offset]&=game.bunker[b][offset]<<shift;
				else
					game.bunker[b][offset]&=game.bunker[b][offset]>>-shift;
				return true;
			}
		}
	}
	return false;
}

static void move_shot() {
    //No shot, do nothing
    if(game.shot_x == DISABLED) {
        return;
    }

    //moving out of top, end shot
    if (game.shot_y <= 0) {
            game.shot_x = DISABLED;
            return;
    }

   if (check_bunker(game.shot_x,game.shot_y-5,1 ))
		game.shot_x=DISABLED;

    //check for collision with enemy, kill enemy if
    for (int row=0; row<ENEMY_ROWS; row++) {
        if (game.enemy_row_y[row]+6 >= game.shot_y && game.enemy_row_y[row]+6 < game.shot_y+7) {
            for(int col = 0; col<ENEMY_COLUMNS; col++) {
                if(game.shot_x >= game.enemy_x[row][col] && game.shot_x < game.enemy_x[row][col]+ENEMY_WIDTHS[row]) {
                    game.enemy_x[row][col]=DISABLED;
                    game.shot_x = DISABLED;
                    game.alive--;
					game.score+=(3-row)*10;
                    return;
                }
            }
        }
    }

    //check for collision with ufo
   	if (game.ufo != DISABLED &&
		game.shot_x>game.ufo &&
		game.shot_x<game.ufo + 16 &&
		game.shot_y<8) {

		game.ufo = DISABLED;
		game.score += 50;
	}

    game.shot_y -= 2;
}




static void move_shots() {
    for (int col = 0; col<ENEMY_COLUMNS; col++){
		//No shot, maybe generate
		if (game.shots_x[col] == DISABLED) {
			for (int row = 0; row<ENEMY_ROWS; row++) {
				if (game.enemy_x[row][col] != DISABLED) {
					if(getRandom()%(game.alive*20/((game.level/3)+1))==0) {
						game.shots_x[col] = game.enemy_x[row][col]+5;
						game.shots_y[col] = game.enemy_row_y[row]+0;
					}
				}
			}
			continue;
		}

		//moving out of bottm, end shot
		if (game.shots_y[col] >= RESY) {
			game.shots_x[col] = DISABLED;
			return;
		}
		//check for collision with bunker
        if (check_bunker(game.shots_x[col],game.shots_y[col],-1))
			game.shots_x[col]=DISABLED;

		//check for collision with player
		if (game.shots_y[col] >= RESY-13 &&
			game.shots_x[col] > game.player+1 &&
			game.shots_x[col] < game.player+6) {

			game.killed = true;
		}

		//move shots down
		game.shots_y[col] += 1;
	}
}

static void move_ufo() {
	if (game.ufo == DISABLED) {
		if ((getRandom()%UFO_PROB)==0) {
			game.ufo = 0;
		}
		return;
	}
	if (game.ufo >= RESX){
		game.ufo = DISABLED;
		return;
	}
	game.ufo++;
}

static void move_player() {
	switch(getInputRaw()){
		case BTN_LEFT:
			if(game.player>0)
				game.player-=1;
			break;
		case BTN_RIGHT:
			if(game.player <RESX-8)
				game.player+=1;
			break;
		case BTN_ENTER:
			if(game.shot_x == 255){
				game.shot_x = game.player+4;
				game.shot_y = POS_PLAYER_Y;
			}
			break;
	};
}

static void move_enemy() {
    if(game.move > 0){
        game.move-=game.level/5+1;
        return;
    }

	game.step = !game.step;
    for (int col = 0; col < ENEMY_COLUMNS; col++) {
        for (int row = 0; row < ENEMY_ROWS; row++) {
            char pos = game.enemy_x[row][(game.direction==1)?(ENEMY_COLUMNS-(col+1)):col];
            if (pos != DISABLED) {
                //Check collision with player
                if((game.enemy_row_y[row]+8 >= POS_PLAYER_Y && pos+8 >= game.player && pos < game.player+8) ||
						game.enemy_row_y[row]+8 >= POS_PLAYER_Y+8) {
					for(int row=0; row<ENEMY_ROWS; row++) {
						game.enemy_row_y[row] = 10 + (40/ENEMY_ROWS)*row;
					}
					game.killed = true;
                }
                check_bunker(pos,game.enemy_row_y[row]+8,-2);

			    //Are we at the beginning or end? Direction change
                if((pos <=0 && game.direction != 1) ||
                   (pos >=RESX-10 && game.direction == 1)){
                    game.direction = (game.direction==1)?-1:1;
                    for (int r = 0; r<ENEMY_ROWS; r++) {
                        game.enemy_row_y[r]+=game.level>=23?4:2;
                    }
                    return;
                }
                game.enemy_x[row][(game.direction==1)?(ENEMY_COLUMNS-(col+1)):col] += game.direction;
            }
        }
    }

    game.move = game.alive*2-1;
}

static void draw_player() {
    draw_sprite(TYPE_PLAYER, game.player, POS_PLAYER_Y);
}

static void draw_ufo() {
	if (game.ufo!=DISABLED)
		draw_sprite(TYPE_UFO, game.ufo, POS_UFO_Y);
}

static void draw_enemy() {
    for (int row = 0; row<ENEMY_ROWS; row++) {
        for (int col = 0; col<ENEMY_COLUMNS; col++) {
            if (game.enemy_x[row][col] != DISABLED) {
                draw_sprite(TYPE_ENEMY_C-row,game.enemy_x[row][col],game.enemy_row_y[row]);
            }
        }
    }
}

static void draw_bunker() {
	for (int b=0; b<BUNKERS; b++) {
		for (int x=0;x<8;x++){
			for (int y=0;y<BUNKER_WIDTH;y++){
				if(game.bunker[b][y] & (1<<x)){
					lcdSetPixel(BUNKER_X(b)+y,RESY-x-8,0b00011000);
				}else{
					lcdSetPixel(BUNKER_X(b)+y,RESY-x-8,0x00);
				};
			}
		}
	};
};

static void draw_shots() {
    if (game.shot_x != 255) {
        for (int length=0; length<=5; length++) {
            lcdSetPixel(game.shot_x, game.shot_y+length, 0xff);
        }
    }

	for (int col = 0; col < ENEMY_COLUMNS; col++) {
		if (game.shots_x[col] != DISABLED) {
			for (int length=0; length<=5; length++) {
				lcdSetPixel(game.shots_x[col], game.shots_y[col]+length,0xff);
			}
		}
	}

}

static void draw_status() {
    for (int p = 0; p<game.alive; p++){
        lcdSetPixel(p+1,1,0xff);
    }
}

void drawInvBitmap(uint8_t x,uint8_t y, const uint8_t* bitmap, uint8_t w,uint8_t h, uint16_t color)
{
	uint8_t byte = 0;
	for (int wc = 0; wc < w; ++wc)
	{
		byte = bitmap[wc];
		for (int hc = 0; hc < h; ++hc)
		{
			int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte

 			byte >>= 1;

            if(byte & 0x01) 
            {
            	lcdSetPixel(x + wc, y + hc, color);
            }
            else
        	{
        		lcdSetPixel(x + wc, y + hc, 0x0000);
        	}
    		
		}
	}
}

static void draw_sprite(char type, char x, char y) {
	//setIntFont(&Font_Invaders);
	switch(type) {
		case TYPE_PLAYER:
			drawInvBitmap(x, y-1, playerBitmap, 7, 8, 0b11111000);
			break;
		case TYPE_ENEMY_A:
			if (game.step)
			{
				drawInvBitmap(x, y-1, invaderABitmap, 11, 8, 0b11100000);
			}
			else
			{
				drawInvBitmap(x, y-1, invader2ABitmap, 11, 8, 0b11100000);
			}
			break;
		case TYPE_ENEMY_B:
      		if (game.step)
			{
				drawInvBitmap(x, y-1, invaderBBitmap, 12, 8, 0b11100000);
			}
			else
			{
				drawInvBitmap(x, y-1, invader2BBitmap, 12, 8, 0b11100000);
			}
			break;
		case TYPE_ENEMY_C:
	     	if (game.step)
			{
				drawInvBitmap(x, y-1, invaderCBitmap, 8, 8, 0b11100000);
			}
			else
			{
				drawInvBitmap(x, y-1, invader2CBitmap, 8, 8, 0b11100000);
			}
			break;
		case TYPE_UFO:
			if (game.step)
			{
				drawInvBitmap(x, y-1, ufoBitmap, 16, 8, 0b11100111);
			}
			else
			{
				drawInvBitmap(x, y-1, ufoBitmap, 16, 8, 0b11100111);
			}
			break;
	}
}

static void draw_score() {
	//setIntFont(&Font_7x8);
  setTextColor(0x00,0xff);
	lcdSetCrsr(0,0);lcdPrint(IntToStr(game.score,6,F_LONG));
	lcdSetCrsr(RESX-8,0);lcdPrint(IntToStr(game.rokets,1,0));
	//setIntFont(&Font_Invaders);
	drawInvBitmap(RESX-16, 0, playerBitmap, 7, 8, 0xff);
}


static void check_end() {
    if (game.killed) {
		game.rokets--;
		delayms_queue(500);
        game.player = POS_PLAYER_X;

		for(int col=0; col<ENEMY_COLUMNS; col++) {
			game.shots_x[col] = DISABLED;
		}
        game.killed = false;
    }
    if (game.alive == 0) {
		delayms(500);
		game.level++;
		init_game();
		screen_level();
    }
}