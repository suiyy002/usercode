#ifndef RMS_H_
#define RMS_H_

#include "com_type.h"

/*
    计算有效值
*/

void calc_rms(td_int16_t* ch[8], td_int16_t* ch_line[3], td_int32_t ch_len,
              td_float_t pt_ct[8], td_float_t nominal_volt,
              td_bool_t with_neutral, volt_curr_value_single value[4]);

#endif