#include <stdio.h>
#include <stdint.h>
#include "driver/spi_master.h"
#include "driver/timer.h"
#include "driver/gpio.h"
#include "esp_spiffs.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"

#include "header.h"
#include "GUI.h"
#include "analogExpand.h"
#include "events.h"
#include "matrix.h"       //
#include "matrixGUI.h"    //
#include "st7789.h"       //
#include "st7789Matrix.h" //
#include "ds3231.h"
#include "encoder.h"

timeDate_t timeDate;
extern uint32_t eventTime;

extern events_t events;
matrix_t matrix;
st7789_t st7789;
st7789Matrix_t st7789Matrix = {
    .matrix = &matrix,
    .st7789 = &st7789,
    .areaH = ST7789_H,
    .areaW = ST7789_W,
    .matrixAreaX = 0,
    .matrixAreaY = 0,
    .st7789AreaX = 0,
    .st7789AreaY = 0,
    .shiftX = 0,
    .shiftY = 0};
ds3231_t ds3231;
encoder_t encoder;
analogExpand_t analogExpand;

gpio_num_t relePins[] = {33, 32, 12};
gpio_num_t digitalPins[4] = {
    GPIO_NUM_25,
    GPIO_NUM_26,
    GPIO_NUM_27,
    GPIO_NUM_14};
analogPins_t analogPins[] = {
    {ADC_UNIT_1, ADC_CHANNEL_3},  // 39  |
    {ADC_UNIT_1, ADC_CHANNEL_6},  // 34  |
    {ADC_UNIT_1, ADC_CHANNEL_7}}; // 35  |

static const uint8_t daysMonths[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
void IRAM_ATTR isr_timer0()
{
    ++timeDate.second;
    ++eventTime;
    callEvents();
    if (timeDate.second == 60)
    {
        timeDate.second = 0;
        ++timeDate.minute;
        if (timeDate.minute == 60)
        {
            timeDate.minute = 0;
            ++timeDate.hour;
            if (timeDate.hour == 24)
            {
                eventTime = 0;
                timeDate.hour = 0;
                ++timeDate.day;
                timeDate.weekDay = timeDate.weekDay == 6 ? 0 : timeDate.weekDay + 1;
                if (timeDate.day == (daysMonths[timeDate.mouth] + ((timeDate.mouth == 1) && (timeDate.year % 4) == 0)))
                {
                    ++timeDate.mouth;
                    timeDate.day = 0;
                    if (timeDate.mouth == 12)
                    {
                        ++timeDate.year;
                        timeDate.mouth = 0;
                    }
                }
            }
        }
    }
}

static void initDevice()
{
    // I2C:
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = I2C_SPEED};
    i2c_param_config(I2C_PORT, &i2c_config);
    i2c_driver_install(I2C_PORT, i2c_config.mode, 128, 128, 0);

    // SPI:
    spi_bus_config_t spi_bus_config = {
        .sclk_io_num = VSPI_SCLK, // GPIO pin for Spi CLocK signal, or -1 if not used.
        .mosi_io_num = VSPI_MOSI, // GPIO pin for Master Out Slave In (=spi_d) signal, or -1 if not used.
        .miso_io_num = VSPI_MISO, // GPIO pin for Master In Slave Out (=spi_q) signal, or -1 if not used.O
        .quadwp_io_num = -1,      // GPIO pin for WP (Write Protect) signal which is used as D2 in 4-bit communication modes, or -1 if not used.
        .quadhd_io_num = -1,      // GPIO pin for HD (HolD) signal which is used as D3 in 4-bit communication modes, or -1 if not used.
        .max_transfer_sz = 0,
        .flags = 0,
        .intr_flags = 0};
    spi_bus_initialize(HOST, &spi_bus_config, SPI_DMA_CH_AUTO);

    // FileSystems:
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/f",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};
    esp_vfs_spiffs_register(&conf);

    // rele:
    gpio_config_t config_rele = {
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = 0,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 1};
    for (uint16_t i = 0; i < 3; i++)
    {
        gpio_pad_select_gpio(relePins[i]);
        config_rele.pin_bit_mask |= 1ULL << relePins[i];
    }
    gpio_config(&config_rele);

    // analogExpand:
    analogExpand_init(&analogExpand, analogPins, 3, digitalPins, 4, 10000);

    // encoder:
    setupEncoder(&encoder, ENCODER_KEY, ENCODER_S1, ENCODER_S2);

    // events:
    events_init();

    // st7789Matrix:
    st7789Matrix_select(&st7789Matrix);

    // matrix:
    matrix_select(&matrix);
    matrix_init(MATRIX_W, MATRIX_H, 2);

    // st7789:
    st7789_select(&st7789);
    st7789_config_t st7789_config = {
        .hostSPI = HOST,
        .speedSPI = ST7789_SPEED,
        .pinCS = ST7789_CS,
        .pinRST = ST7789_RST,
        .pinDC = ST7789_DC,
        .pinBL = ST7789_BL,
        .width = ST7789_W,
        .height = ST7789_H,
        .offsetX = ST7789_OFFSETX,
        .offsetY = ST7789_OFFSETY};
    st7789_init(&st7789_config);

    // GUI:
    GUI_init();

    // ds3231:
    ds3231_config_t ds3231_config = {
        .port = I2C_PORT,
        .frequencyHz = kHZ8192,
        .interruptAlarm = INTERRUPT_ALARM1,
        .shiftYear = DS3231_YEAR};
    ds3231_init(&ds3231, &ds3231_config);
    ds3231_getTime(&ds3231, &timeDate);
    // timeDate.second = 0;
    // timeDate.minute = 6;
    // timeDate.hour = 2;
    // timeDate.day = 9;
    // timeDate.weekDay = 1;
    // timeDate.mouth = 4;
    // timeDate.year = 0;
    // ds3231_setTime(&ds3231, &timeDate, 0);

    // timer:
    eventTime = (timeDate.hour * 60 + timeDate.minute) * 60 + timeDate.second;
    timer_config_t timer_config = {
        .divider = 40000,              // 4khz
        .counter_dir = TIMER_COUNT_UP, //+1 tick
        .counter_en = TIMER_PAUSE,     // pause
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = TIMER_AUTORELOAD_EN,
        .intr_type = TIMER_INTR_LEVEL};
    timer_init(TIMER_GROUP_0, TIMER_0, &timer_config);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 2000);
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, (void *)isr_timer0, NULL, ESP_INTR_FLAG_IRAM);
    timer_start(TIMER_GROUP_0, TIMER_0);
}

extern int64_t Itime1, Itime2, Itime3, Itime4;
void app_main(void)
{
    initDevice();

    // xTaskCreate();

    int64_t time1 = esp_timer_get_time();
    int64_t time2 = esp_timer_get_time();
    while (1)
    {
        if (time1 < esp_timer_get_time())
        {
            time1 += 100000;
            GUI_update();
            if (time2 < esp_timer_get_time())
            {
                time2 += 1000000;
                // ESP_LOGI("", "%u:%u:%u day:%u week:%u mouth:%u year:%u", timeDate.hour, timeDate.minute, timeDate.second, timeDate.day, timeDate.weekDay, timeDate.mouth, timeDate.year);
                // ESP_LOGI("delayTrigTime", "%lli", Itime1);
                // ESP_LOGI("delayTrigPeriod", "%lli", Itime2);
                // ESP_LOGI("delayTrigSignal", "%lli", Itime3);
                // ESP_LOGI("delayScripts", "%lli", Itime4);
                // uint32_t resistance[12];
                // analogExpand_getResistancePorts(&analogExpand, resistance);
                // for (uint16_t i = 0; i < 12; i++)
                // {
                //     ESP_LOGI("", "(%u)%u", i, resistance[i]);
                // }
                ESP_LOGI("", "=================");
                for (uint16_t i = 0; i < events.lenTrigTime; i++)
                    ESP_LOGI("trigTime", "%u", events.trigTimeList[i].config & 1);
                for (uint16_t i = 0; i < events.lenTrigPeriod; i++)
                    ESP_LOGI("trigPeriod", "%u", events.trigPeriodList[i].config & 1);
                for (uint16_t i = 0; i < events.lenTrigSignal; i++)
                    ESP_LOGI("trigSignal", "%u", events.trigSignalList[i].config & 1);
                ESP_LOGI("", "=================");
            }
        }
    }
}
