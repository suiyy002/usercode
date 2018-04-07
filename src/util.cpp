#include "util.h"

td_float_t ang_2_rad(td_float_t ang) {
    td_float_t ang_t = ang;
    while (ang_t < 0.0f) {
        ang_t += 360.0f;
    }
    return (float)(ang_t / 180.0f * PI);
}

td_float_t rad_2_ang(td_float_t rad) { return (float)(rad * 180.0f / PI); }

float get_percent(float div, float base) {
    if (base < 0.000001f) {
        return 0.0f;
    } else {
        return div / base * 100;
    }
}