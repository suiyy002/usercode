
#ifndef _HALF_RMS_H_
#define _HALF_RMS_H_

#include "com_type.h"

void calc_half_rms(td_int16_t* ch[8], td_int16_t* ch_line[3], td_int32_t ch_len,
    td_float_t pt_ct[8], freq_type freq, half_rms_result& value);

#endif // _HALF_RMS_H_
