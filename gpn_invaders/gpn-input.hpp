#define GPIO_LCD_DC    0
#define GPIO_TX        1
#define GPIO_WS2813    4
#define GPIO_RX        3
#define GPIO_DN        2
#define GPIO_DP        5

#define GPIO_BOOT      16
#define GPIO_MOSI      13
#define GPIO_CLK       14
#define GPIO_LCD_CS    15
#define GPIO_BNO       12

#define MUX_JOY        0
#define MUX_BAT        1
#define MUX_LDR        2
#define MUX_ALK        4
#define MUX_IN1        5

#define VIBRATOR       3
#define MQ3_EN         4
#define LCD_LED        5
#define IR_EN          6
#define OUT1           7

#define UP             790
#define DOWN           630
#define RIGHT          530
#define LEFT           1024
#define OFFSET         30

#define I2C_PCA        0x25

#define NUM_LEDS       4

#define BLACK          0x0000
#define BLUE           0x001F
#define RED            0xF800
#define GREEN          0x07E0
#define CYAN           0x07FF
#define MAGENTA        0xF81F
#define YELLOW         0xFFE0
#define WHITE          0xFFFF

#define BTN_NONE       0
#define BTN_UP         (1 << 0)
#define BTN_DOWN       (1 << 1)
#define BTN_LEFT       (1 << 2)
#define BTN_RIGHT      (1 << 3)
#define BTN_ENTER      (1 << 4)

int getInputRaw();
void setGPIO(byte channel, boolean level);
void setAnalogMUX(byte channel);
void getInputWaitRelease();
int getInputWaitTimeout(uint16_t timeout);
void delayms_queue(uint32_t ms);
uint8_t getInputWait(void);
