#include "input.h"
#include "trackball.h"
#include <Arduino.h>

// External trackball instance (defined in main.cpp)
extern Trackball trackball;

// Navigation threshold - lower = more responsive
static const int16_t NAVIGATION_THRESHOLD = 4;

// Minimum key press duration in milliseconds
static const uint32_t KEY_PRESS_DURATION_MS = 50;

void keypad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  static uint32_t poll_count = 0;
  if (poll_count++ % 50 == 0) {
    Serial.println("Indev polling...");
  }
  
  trackball.update();

  static int16_t acc_x = 0;
  static int16_t acc_y = 0;
  static uint32_t key_press_time = 0;
  static uint32_t last_key = 0;
  static bool key_is_pressed = false;

  uint32_t now = millis();

  // If a key is currently pressed, maintain PRESSED state for minimum duration
  if (key_is_pressed && (now - key_press_time) < KEY_PRESS_DURATION_MS) {
    data->state = LV_INDEV_STATE_PRESSED;
    data->key = last_key;
    return;
  }

  // After minimum duration, release the key
  if (key_is_pressed) {
    data->state = LV_INDEV_STATE_RELEASED;
    data->key = last_key;
    key_is_pressed = false;
    last_key = 0;
    return;
  }

  // Read raw trackball movement
  int16_t raw_dx = trackball.right() - trackball.left();
  int16_t raw_dy = trackball.down() - trackball.up();

  if (raw_dx != 0 || raw_dy != 0) {
    Serial.printf("Raw: dx=%d dy=%d\n", raw_dx, raw_dy);
  }

  // Rotate 90 degrees counter-clockwise: new_x = -old_y, new_y = old_x
  int16_t dx = -raw_dy;
  int16_t dy = raw_dx;

  // Accumulate movement
  acc_x += dx;
  acc_y += dy;

  // Check for button press (highest priority)
  if (trackball.isPressed()) {
    Serial.println("Trackball: PRESSED -> LV_KEY_ENTER");
    data->state = LV_INDEV_STATE_PRESSED;
    data->key = LV_KEY_ENTER;
    last_key = LV_KEY_ENTER;
    key_press_time = now;
    key_is_pressed = true;
  }
  // Check for horizontal navigation
  else if (abs(acc_x) >= NAVIGATION_THRESHOLD) {
    Serial.printf("Nav X: %d -> %s\n", acc_x, (acc_x > 0) ? "RIGHT" : "LEFT");
    data->state = LV_INDEV_STATE_PRESSED;
    data->key = (acc_x > 0) ? LV_KEY_RIGHT : LV_KEY_LEFT;
    last_key = data->key;
    key_press_time = now;
    key_is_pressed = true;
    acc_x = 0;
    acc_y = 0;
  }
  // Check for vertical navigation
  else if (abs(acc_y) >= NAVIGATION_THRESHOLD) {
    Serial.printf("Nav Y: %d -> %s\n", acc_y, (acc_y > 0) ? "DOWN" : "UP");
    data->state = LV_INDEV_STATE_PRESSED;
    data->key = (acc_y > 0) ? LV_KEY_DOWN : LV_KEY_UP;
    last_key = data->key;
    key_press_time = now;
    key_is_pressed = true;
    acc_x = 0;
    acc_y = 0;
  }
  // No input detected
  else {
    data->state = LV_INDEV_STATE_RELEASED;
    data->key = 0;
  }
}
