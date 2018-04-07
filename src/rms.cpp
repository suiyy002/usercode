#include "rms.h"
#include <math.h>
#include <memory.h>

typedef struct base_rms {
    td_float_t rms;     // 有效值
    td_float_t pk_pos;  // 正峰值
    td_float_t pk_neg;  // 负峰值
    td_float_t cf;      // 峰值因数
} base_rms;

void calc_base_(td_int16_t* sample_chl, td_int32_t point_count,
                td_float_t pt_ct, base_rms& val) {
    memset(&val, 0, sizeof(val));
    td_uint64_t sum = 0;
    for (int s = 0; s < point_count; ++s) {
        // 寻找正峰值
        if (val.pk_pos < sample_chl[s]) val.pk_pos = sample_chl[s];

        // 寻找负峰值
        if (val.pk_neg > sample_chl[s]) val.pk_neg = sample_chl[s];

        // 平方和
        sum += sample_chl[s] * sample_chl[s];
    }
    // 平均
    td_float_t r = sum / (td_float_t)point_count;
    //开根
    r = sqrt(r);

    val.rms = r * pt_ct;
    val.pk_pos = val.pk_pos * pt_ct;
    val.pk_neg = val.pk_neg * pt_ct;
    if (val.rms < 0.000001) {
        val.cf = 0.0f;
    } else {
        val.cf = val.pk_pos / val.rms;
    }
}

void calc_rms(td_int16_t* ch[8], td_int16_t* ch_line[3], td_int32_t ch_len,
              td_float_t pt_ct[8], td_float_t nominal_volt,
              td_bool_t with_neutral, volt_curr_value_single value[4]) {
    // 相电压
    for (int i = 0; i < 4; ++i) {
        base_rms val;
        calc_base_(ch[i], ch_len, pt_ct[i], val);
        value[i].volt_phase_rms = val.rms;
        value[i].volt_phase_pk_pos = val.pk_pos;
        value[i].volt_phase_pk_neg = val.pk_neg;
        value[i].volt_phase_cf = val.cf;
    }

    // 线电压
    for (int i = 0; i < 3; ++i) {
        base_rms val;
        calc_base_(ch_line[i], ch_len, pt_ct[i], val);
        value[i].volt_line_rms = val.rms;
        value[i].volt_line_pk_pos = val.pk_pos;
        value[i].volt_line_pk_neg = val.pk_neg;
        value[i].volt_line_cf = val.cf;
    }

    // 计算电压偏差
    for (int i = 0; i < 3; ++i) {
        float* p;
        if (with_neutral)
            p = &value[i].volt_line_rms;
        else
            p = &value[i].volt_phase_rms;
        value[i].volt_deviation = (*p - nominal_volt) / nominal_volt * 100.0f;
    }

    // 电流
    for (int i = 0; i < 4; ++i) {
        base_rms val;
        calc_base_(ch[4 + i], ch_len, pt_ct[4 + i], val);
        value[i].curr_rms = val.rms;
        value[i].curr_pk_pos = val.pk_pos;
        value[i].curr_pk_neg = val.pk_neg;
        value[i].curr_cf = val.cf;
    }
}
