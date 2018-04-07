#ifndef UTIL_H_
#define UTIL_H_

#include "type_def.h"

// 1秒时间分辨率 - 百纳秒
#define TIME_RESOLUTION (10000000ULL)

td_float_t ang_2_rad(td_float_t ang);
td_float_t rad_2_ang(td_float_t rad);
float get_percent(float div, float base);

#endif