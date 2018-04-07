#include "flicker_wrapper.h"
#include <cmath>

flicker_wrapper::flicker_wrapper(float sample_rate) {
    self = flicker_create(sample_rate);
}

flicker_wrapper::~flicker_wrapper() { flicker_destroy(self); }

void flicker_wrapper::set_params(float scale, int lamp, int freq) {
    flicker_set_lamp(self, lamp, freq);
    flicker_set_scale(self, scale);
    this->scale = scale;
}

void flicker_wrapper::reset() { flicker_reset(self); }

namespace {
float get_rms(short* in, int len) {
    float sum = 0.0f;
    short* end = in + len;
    while (in < end) {
        float n = (float)*in++;
        sum += n * n;
    }
    return std::sqrt(sum / len);
}
}  // namespace

void flicker_wrapper::input(short* raw, int len) {
    float rms = get_rms(raw, len);
    flicker_input_rms(self, rms * scale);
    flicker_input_raw(self, raw, len);
}

float flicker_wrapper::get_pst() { return flicker_output_pst(self); }

float flicker_wrapper::get_plt() { return flicker_output_plt(self, 0); }