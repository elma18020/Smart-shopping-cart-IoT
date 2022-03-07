#ifndef __DISPLAY_H__
#define __DISPLAY_H__


#include <string.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_system.h"
#include "freertos/task.h"

#include "ssd1306.h"
#include "font8x8_basic.h"
#include "debug.h"

#define SDA_PIN 2  // SDA -> D4
#define SCL_PIN 5  // SCL -> D1

#define tag "SSD1306"

static void i2c_master_init();


static void ssd1306_init();
static void ssd1306_display_pattern();
static void ssd1306_display_clear();
static void ssd1306_contrast();
static void ssd1306_scroll();
static void ssd1306_display_text(char* ime);


#endif /* __DISPLAY_H__ */
