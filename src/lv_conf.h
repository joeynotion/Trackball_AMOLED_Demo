#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 1
#define LV_COLOR_SCREEN_TRANSP 0

#define LV_MEM_CUSTOM 0
#if LV_MEM_CUSTOM == 0
#define LV_MEM_SIZE (48U * 1024U)
#define LV_MEM_ADR 0
#endif

#define LV_TICK_CUSTOM 1
#if LV_TICK_CUSTOM
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
#endif

#define LV_DRAW_COMPLEX 1

#define LV_USE_BTN 1
#define LV_USE_LABEL 1
#define LV_USE_LIST 1
#define LV_USE_GROUP 1
#define LV_USE_GRIDNAV 1
#define LV_FONT_MONTSERRAT_24 1

#endif
