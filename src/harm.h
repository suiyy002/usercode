#ifndef HARM_H_
#define HARM_H_

#include "com_type.h"

// 计算谐波
void calc_harm(td_int16_t* ch[8], td_int16_t* ch_line[3], td_int32_t ch_len,
               td_float_t pt_ct[8], harm_single_phase value[4],
               td_bool_t with_neutral, td_uint32_t thd_calc_order,
                float freq/*工频*/, float samp_rate/*采样率*/);

#endif
