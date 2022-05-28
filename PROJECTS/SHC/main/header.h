// SPI:
#define VSPI_SCLK 18
#define VSPI_MOSI 23
#define VSPI_MISO 19
#define HOST VSPI_HOST

// I2C:
#define I2C_SDA 22
#define I2C_SCL 21
#define I2C_SPEED 400000
#define I2C_PORT 0

// ds3231:
#define DS3231_INT 5
#define DS3231_YEAR 2020

// encoder:
#define ENCODER_KEY 4
#define ENCODER_S2 36
#define ENCODER_S1 15

// st7789:
#define ST7789_SPEED 40000000
#define ST7789_CS 17
#define ST7789_DC 16
#define ST7789_RST -1
#define ST7789_BL -1
#define ST7789_OFFSETX 0
#define ST7789_OFFSETY 0
#define ST7789_H 320
#define ST7789_W 240

// GUI:
#define FILE_FONT "/f/EU_RU_32p_LATIN.FNT"
#define MATRIX_H 320
#define MATRIX_W 240
#define COLOR_BACKGROUND 0x0020
#define COLOR_TEXT 0xFFFF
#define COLOR_FRAME 0xFFFF
#define COLOR_FALSE 0x31A6
#define COLOR_TRUE 0x0C60
#define COLOR_SIGN 0xAD55

// file names:
#define TRIGGERS_TIME_FILE "/f/trigtime"
#define TRIGGERS_PERIOD_FILE "/f/trigperiod"
#define TRIGGERS_SIGNAL_FILE "/f/trigsignal"
#define SCRIPTS_FILE "/f/scripts"
#define SENSORS_ANALOG_FILE "/f/sensors"
#define ACTIONS_KEY_FILE "/f/actions"

// events configs:
#define SCRIPTS_INDEX_MAX 128
#define NAME_CHAR_MAX 16
#define TRIGGERS_TIME_MAX 128
#define TRIGGERS_PERIOD_MAX 128
#define TRIGGERS_SIGNAL_MAX 128
#define SCRIPTS_MAX 128
#define SENSORS_MAX 24
#define ACTIONS_MAX 16