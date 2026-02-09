#include "input.h"
#include "trackball.h"
#include <Arduino.h>

// External trackball instance (defined in main.cpp)
extern Trackball trackball;

// Navigation threshold - lower = more responsive
static const int16_t NAVIGATION_THRESHOLD = 8;

void keypad_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  trackball.update();

  static int16_t acc_x = 0;
  static int16_t acc_y = 0;
  static uint32_t last_key = 0;
  static bool waiting_release = false;

  // If we just sent a PR state, send REL state to complete the "click"
  if (waiting_release) {
    data->state = LV_INDEV_STATE_REL;
    data->key = last_key;
    waiting_release = false;
    return;
  }

  int16_t raw_dx = trackball.right() - trackball.left();
  int16_t raw_dy = trackball.down() - trackball.up();

  // Rotate 90 degrees counter-clockwise: new_x = -old_y, new_y = old_x
  int16_t dx = -raw_dy;
  int16_t dy = raw_dx;

  acc_x += dx;
  acc_y += dy;

  if (trackball.isPressed()) {
    data->state = LV_INDEV_STATE_PR;
    data->key = LV_KEY_ENTER;
    last_key = LV_KEY_ENTER;
    waiting_release = true;
  } else if (abs(acc_x) >= NAVIGATION_THRESHOLD) {
    data->state = LV_INDEV_STATE_PR;
    data->key = (acc_x > 0) ? LV_KEY_RIGHT : LV_KEY_LEFT;
    last_key = data->key;
    acc_x = 0;
    acc_y = 0;
    waiting_release = true;
  } else if (abs(acc_y) >= NAVIGATION_THRESHOLD) {
    data->state = LV_INDEV_STATE_PR;
    data->key = (acc_y > 0) ? LV_KEY_DOWN : LV_KEY_UP;
    last_key = data->key;
    acc_x = 0;
    acc_y = 0;
    waiting_release = true;
  } else {
    data->state = LV_INDEV_STATE_REL;
    data->key = 0;
  }
}
