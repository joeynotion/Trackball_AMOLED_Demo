#include "input.h"
#include "qspi_display.h"
#include "trackball.h"
#include "ui.h"
#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>

// I2C Pins for trackball
#define I2C_SDA 40
#define I2C_SCL 39

// LVGL Buffers
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[LCD_WIDTH * 20];

// Trackball instance (global, used by ui.cpp and input.cpp)
Trackball trackball;

/* Display flushing callback for LVGL */
void disp_flush(lv_disp_drv_t *disp, const lv_area_t *area,
                lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  lcd.setWindow(area->x1, area->y1, w, h);
  lcd.pushPixels((uint16_t *)color_p, w * h);

  lv_disp_flush_ready(disp);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  // Init I2C for trackball
  Wire.begin(I2C_SDA, I2C_SCL);

  // Init Display
  if (!lcd.begin()) {
    Serial.println("Display init failed!");
    while (1)
      delay(100);
  }

  // Init Trackball
  if (!trackball.begin(Wire)) {
    Serial.println("Trackball not found!");
  } else {
    trackball.setRGBW(0, 0, 64, 0); // Start with dim blue
    Serial.println("Trackball ready");
  }

  // Init LVGL
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, LCD_WIDTH * 20);

  // Register display driver
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = LCD_WIDTH;
  disp_drv.ver_res = LCD_HEIGHT;
  disp_drv.flush_cb = disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Register keypad input driver (trackball)
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_KEYPAD;
  indev_drv.read_cb = keypad_read;
  lv_indev_drv_register(&indev_drv);

  // Build UI
  ui_init();

  Serial.println("Setup complete");
}

void loop() {
  lv_timer_handler();
  delay(5);
}
