#include "half_rms.h"
#include <cmath>

namespace {
td_float_t get_rms(td_int16_t* data, td_int32_t len, td_float_t scale) {
    td_float_t sum = 0.0f;
    for (td_int32_t i=0; i<len; ++i) {
        sum += (data[i]*data[i]);
    }
    return scale*std::sqrt(sum/len);
}
}

void calc_half_rms(td_int16_t* ch[8], td_int16_t* ch_line[3], td_int32_t ch_len,
        td_float_t pt_ct[8], freq_type freq, half_rms_result& value) {
    value.number = freq==freqtype_50hz ? 20 : 24;
    td_int32_t len = ch_len / value.number;
    for (int i=0; i<4; ++i) {
        for (int j=0; j<value.number; ++j) {
            value.volt_line[i][j] = get_rms(ch[i]+j*len, len, pt_ct[i]);
        }
    }
    for (int i=0; i<3; ++i) {
        for (int j=0; j<value.number; ++j) {
            value.volt_phase[i][j] = get_rms(ch_line[i]+j*len, len, pt_ct[i]);
        }
    }
    for (int i=0; i<4; ++i) {
        for (int j=0; j<value.number; ++j) {
            value.curr[i][j] = get_rms(ch[i+4]+j*len, len, pt_ct[i+4]);
        }
    }
}
