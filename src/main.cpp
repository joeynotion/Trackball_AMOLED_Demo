#include "input.h"
#include "qspi_display.h"
#include "trackball.h"
#include "ui.h"
#include <Arduino.h>
#include <Wire.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

// I2C Pins for trackball
#define I2C_SDA 40
#define I2C_SCL 39

// Buffer height for partial rendering (60 lines each)
#define BUF_HEIGHT 60

// Trackball instance (global, used by ui.cpp and input.cpp)
Trackball trackball;

/* Display flushing callback for LVGL 9 */
void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);

  lcd.setWindow(area->x1, area->y1, w, h);
  lcd.pushPixels((uint16_t *)px_map, w * h);

  lv_display_flush_ready(disp);
}

void setup() {
  Serial.begin(115200);
  delay(2000); // Wait for Serial Monitor to connect
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

  // CRITICAL FIX: Register tick callback BEFORE creating display
  // This replaces LV_TICK_CUSTOM_SYS_TIME_EXPR which is deprecated in LVGL 9
  lv_tick_set_cb([]() -> uint32_t { return millis(); });

  // Create display with LVGL 9 API
  lv_display_t *disp = lv_display_create(LCD_WIDTH, LCD_HEIGHT);

  // Allocate double buffers in PSRAM for smooth rendering
  size_t buf_size = LCD_WIDTH * BUF_HEIGHT * sizeof(lv_color_t);
  uint8_t *buf1 = (uint8_t *)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);
  uint8_t *buf2 = (uint8_t *)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);

  if (!buf1 || !buf2) {
    Serial.println("PSRAM buffer alloc failed, using internal RAM");
    // Fallback to internal RAM with smaller buffer
    static uint8_t fallback_buf[LCD_WIDTH * 20 * sizeof(lv_color_t)];
    lv_display_set_buffers(disp, fallback_buf, NULL, sizeof(fallback_buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
  } else {
    lv_display_set_buffers(disp, buf1, buf2, buf_size,
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    Serial.printf("PSRAM buffers allocated: %d bytes each\n", buf_size);
  }

  lv_display_set_flush_cb(disp, disp_flush);

  // Create keypad input device with LVGL 9 API
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
  lv_indev_set_read_cb(indev, keypad_read);
  lv_indev_set_display(indev, disp); // Critical: associate input with display!

  // ADDED FIX: Set faster polling rate for better responsiveness
  lv_timer_set_period(lv_indev_get_read_timer(indev), 20);

  // Build UI
  ui_init();

  Serial.println("Setup complete");
}

void loop() {
  lv_timer_handler();

  trackball.update();

  static uint32_t last_print = 0;
  if (millis() - last_print > 2000) {
    Serial.printf("Loop running... Tick: %u\n", (unsigned int)lv_tick_get());
    last_print = millis();
  }

  delay(5);
}
