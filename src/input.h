#pragma once

#include <lvgl.h>

/**
 * Trackball keypad read callback for LVGL
 * Handles 4-way navigation with 90-degree counter-clockwise rotation
 */
void keypad_read(lv_indev_drv_t *drv, lv_indev_data_t *data);
