#ifndef POWER_H_
#define POWER_H_

#include "com_type.h"

// 计算功率
void calc_power(td_int16_t* ch[8], td_int16_t* ch_line[3], td_int32_t ch_len,
                td_float_t pt_ct[8], calc_result& result,
                td_bool_t with_neutral);

#endif