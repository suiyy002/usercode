#include "harm.h"
#include "dft.h"
#include "dft_com.h"
#include "util.h"

//////////////////////////////////////////////////////////////////////////

dft_context* volt_dft_arr = NULL;
dft_context* curr_dft_arr = NULL;
#define PHASE_COUNT (4)

//////////////////////////////////////////////////////////////////////////

void harm_50(dft_context* context, td_float_t* rms, td_float_t* percent,
             td_float_t* angle) {
    td_uint32_t step = context->spec_step;
    td_float_t scale = (float)(context->scale * SQRT2);
    td_uint32_t index = 0;
    td_float_t base_val = 0.0f;

    for (int i = 0; i < 63; i++) {
        float o1, o2, o3, ang;
        index = step * i + step;

        // 谐波子组，如,当i=0时是基波子组：45Hz,50Hz,55Hz
        dft_get_val(context, index - 1, &o1);
        dft_get_val_ang(context, index, &o2, &ang);
        dft_get_val(context, index + 1, &o3);

        // 弧度转角度
        angle[i] = rad_2_ang(ang);
        rms[i] = (float)sqrt(o1 + o2 + o3) * scale;
        if (0 == i) {  // 基波
            base_val = rms[i];
            percent[i] = 100.0f;
        } else {
            percent[i] = get_percent(rms[i], base_val);
        }
    }
}

// 获取直流分量,必须在get_harm之后，否则百分比计算错误！！！
void harm_dc(dft_context* context, td_float_t* rms, td_float_t* percent,
             td_float_t base_val) {
    float tmp = 0;
    dft_get_val(context, 0, &tmp);
    *rms = sqrt(tmp) * context->scale;
    *percent = get_percent(*rms, base_val);
}

void harm_thd(harm_single_phase value[4], td_uint32_t thd_calc_order) {
    for (int i = 0; i < 4; ++i) {
        float* p_vhr = value[i].volt_harm_rms;
        float* p_chr = value[i].curr_harm_rms;

        double sum_tol_v = 0.0f, sum_tol_c = 0.0f;    // 总
        double sum_odd_v = 0.0f, sum_odd_c = 0.0f;    // 奇次
        double sum_even_v = 0.0f, sum_even_c = 0.0f;  // 偶次
        // 基波不算入
        for (int k = 1; k < thd_calc_order; ++k) {
            float v2 = p_vhr[k] * p_vhr[k];
            float c2 = p_chr[k] * p_chr[k];
            sum_tol_v += v2;
            sum_tol_c += c2;
            if (k % 2 != 0) {  // 注意：是偶次
                sum_even_v += v2;
                sum_even_c += c2;
            } else {
                sum_odd_v += v2;
                sum_odd_c += c2;
            }
        }
        value[i].volt_harm_thd_rms = (float)sqrt(sum_tol_v);
        value[i].volt_harm_thd_percent =
            get_percent(value[i].volt_harm_thd_rms, value[i].volt_harm_rms[0]);
        value[i].volt_harm_thd_odd_rms = (float)sqrt(sum_odd_v);
        value[i].volt_harm_thd_odd_percent = get_percent(
            value[i].volt_harm_thd_odd_rms, value[i].volt_harm_rms[0]);
        value[i].volt_harm_thd_even_rms = (float)sqrt(sum_even_v);
        value[i].volt_harm_thd_even_percent = get_percent(
            value[i].volt_harm_thd_even_rms, value[i].volt_harm_rms[0]);

        value[i].curr_harm_thd_rms = (float)sqrt(sum_tol_c);
        value[i].curr_harm_thd_percent =
            get_percent(value[i].curr_harm_thd_rms, value[i].curr_harm_rms[0]);
        value[i].curr_harm_thd_odd_rms = (float)sqrt(sum_odd_c);
        value[i].curr_harm_thd_odd_percent = get_percent(
            value[i].curr_harm_thd_odd_rms, value[i].curr_harm_rms[0]);
        value[i].curr_harm_thd_even_rms = (float)sqrt(sum_even_c);
        value[i].curr_harm_thd_even_percent = get_percent(
            value[i].curr_harm_thd_even_rms, value[i].curr_harm_rms[0]);
    }
}

// 获取谐波相关数据
void get_harm(harm_single_phase value[4], td_uint32_t thd_calc_order) {
    for (int i = 0; i < 4; ++i) {
        // 电压 - 谐波
        harm_50(&volt_dft_arr[i], value[i].volt_harm_rms,
                value[i].volt_harm_percent, value[i].volt_harm_angle);
        // 电流 - 谐波
        harm_50(&curr_dft_arr[i], value[i].curr_harm_rms,
                value[i].curr_harm_percent, value[i].curr_harm_angle);
        // 电压dc
        harm_dc(&volt_dft_arr[i], &value[i].volt_harm_dc_rms,
                &value[i].volt_harm_dc_percent, value[i].volt_harm_rms[0]);
        // 电流dc
        harm_dc(&curr_dft_arr[i], &value[i].curr_harm_dc_rms,
                &value[i].curr_harm_dc_percent, value[i].curr_harm_rms[0]);
    }

    // thd
    harm_thd(value, thd_calc_order);
}

//////////////////////////////////////////////////////////////////////////

void inharm_50(dft_context* context, td_float_t* rms, td_float_t* percent,
               td_float_t base_val) {
    td_uint32_t step = context->spec_step;
    td_float_t scale = (float)(context->scale * SQRT2);
    td_uint32_t index_begin = 0, index_count = 7;

    for (int i = 0; i < 63; i++) {
        float o1, sum = 0.0f;
        index_begin = step * i + 2;
        for (td_uint32_t k = 0; k < index_count; ++k) {
            dft_get_val(context, index_begin + k, &o1);
            sum += o1;
        }
        rms[i] = (float)(sqrt(sum) * scale);
        percent[i] = get_percent(rms[i], base_val);
    }
}

// 间谐波thd
void inharm_thd(harm_single_phase value[4]) {
    for (int i = 0; i < 4; ++i) {
        float* p_vhr = value[i].volt_inharm_rms;
        float* p_chr = value[i].curr_inharm_rms;

        double sum_v = 0.0f, sum_c = 0.0f;  // 总
        // 基波不算入
        for (int k = 1; k < 63; ++k) {
            float v2 = p_vhr[k] * p_vhr[k];
            float c2 = p_chr[k] * p_chr[k];
            sum_v += v2;
            sum_c += c2;
        }
        value[i].volt_inharm_thd_rms = (float)sqrt(sum_v);
        value[i].volt_inharm_thd_percent = get_percent(
            value[i].volt_inharm_thd_rms, value[i].volt_harm_rms[0]);
        value[i].curr_inharm_thd_rms = (float)sqrt(sum_c);
        value[i].curr_inharm_thd_percent = get_percent(
            value[i].curr_inharm_thd_rms, value[i].curr_harm_rms[0]);
    }
}

// 获取间谐波相关值
void get_inharm(harm_single_phase value[4]) {
    for (int i = 0; i < 4; ++i) {
        // 电压
        inharm_50(&volt_dft_arr[i], value[i].volt_inharm_rms,
                  value[i].volt_inharm_percent, value[i].volt_harm_rms[0]);
        // 电流
        inharm_50(&curr_dft_arr[i], value[i].curr_inharm_rms,
                  value[i].curr_inharm_percent, value[i].curr_harm_rms[0]);
    }
    // thd
    inharm_thd(value);
}

//////////////////////////////////////////////////////////////////////////

void hiharm_35(dft_context* context, td_float_t* rms, td_float_t* percent,
               td_float_t base_val) {
    td_uint32_t sr = (td_uint32_t)context->spec_reso;
    td_float_t scale = (float)(context->scale * SQRT2);
    td_uint32_t index_begin = 0, index_count = HI_HARM_STEP / sr;

    for (int i = 0; i < 35; i++) {
        float o1;
        double sum = 0.0f;
        index_begin = (HI_HARM_BEGIN_FREQ + i * HI_HARM_STEP) / sr;
        for (td_uint32_t k = 0; k < index_count; ++k) {
            dft_get_val(context, index_begin + k, &o1);
            sum += o1;
        }
        rms[i] = (float)(sqrt(sum) * scale);
        percent[i] = get_percent(rms[i], base_val);
    }
}

// 获取高次谐波相关值
void get_hiharm(harm_single_phase value[4]) {
    for (int i = 0; i < 4; ++i) {
        // 电压
        hiharm_35(&volt_dft_arr[i], value[i].volt_hiharm_rms,
                  value[i].volt_hiharm_percent, value[i].volt_harm_rms[0]);
        // 电流
        hiharm_35(&curr_dft_arr[i], value[i].curr_hiharm_rms,
                  value[i].curr_hiharm_percent, value[i].curr_harm_rms[0]);
    }
}

//////////////////////////////////////////////////////////////////////////

void harm_power_50(harm_single_phase* hsp) {
    float angle;
    for (int i = 0; i < 63; ++i) {
        angle = hsp->volt_harm_angle[i] - hsp->curr_harm_angle[i];
        if (angle < 0.0f) {
            angle += 360;
        }
        hsp->power_harm_angle[i] = angle;
        hsp->power_harm_rms[i] = hsp->volt_harm_rms[i] * hsp->curr_harm_rms[i] *
                                 cosf((float)(angle / 180 * PI));
        hsp->power_harm_va_rms[i] = hsp->volt_harm_rms[i] * hsp->curr_harm_rms[i];
        hsp->power_harm_var_rms[i] = hsp->volt_harm_rms[i] * hsp->curr_harm_rms[i] * sinf((float)(angle / 180 * PI));
        if (0 == i) {
            hsp->power_harm_percent[i] = 100.0f;
            hsp->power_harm_va_percent[i] = 100.0f;
            hsp->power_harm_var_percent[i] = 100.0f;
            
            // 基波不计入总
            hsp->power_harm_total_rms = 0.0f;
            hsp->power_harm_va_total_rms = 0.0f;
            hsp->power_harm_var_total_rms = 0.0f;
        }
        else {
            hsp->power_harm_percent[i] =
                get_percent(hsp->power_harm_rms[i], hsp->power_harm_rms[0]);
            hsp->power_harm_va_percent[i] =
                get_percent(hsp->power_harm_va_rms[i], hsp->power_harm_va_rms[0]);
            hsp->power_harm_var_percent[i] =
                get_percent(hsp->power_harm_var_rms[i], hsp->power_harm_var_rms[0]);

            // 累加总
            hsp->power_harm_total_rms += hsp->power_harm_rms[i];
            hsp->power_harm_va_total_rms += hsp->power_harm_va_rms[i];
            hsp->power_harm_var_total_rms += hsp->power_harm_var_rms[i];
        }
    }// end for

    // 计算总的含有率
    hsp->power_harm_total_percent =
        get_percent(hsp->power_harm_total_rms, hsp->power_harm_rms[0]);
    hsp->power_harm_va_total_percent =
        get_percent(hsp->power_harm_va_total_rms, hsp->power_harm_va_rms[0]);
    hsp->power_harm_var_total_percent =
        get_percent(hsp->power_harm_var_total_rms, hsp->power_harm_var_rms[0]);


}

void get_harm_power(harm_single_phase value[4], td_bool_t with_neutral) {
    // 线电压不计算谐波功率 - 置零
    if (with_neutral) {
        for (int i = 0; i < 4; ++i) {
            for (int k = 0; k < 63; ++k) {
                value[i].power_harm_rms[k] = 0.0f;
                value[i].power_harm_percent[k] = 0.0f;
                value[i].power_harm_angle[k] = 0.0f;
            }
        }
        return;
    }

    for (int i = 0; i < 4; ++i) {
        harm_power_50(&value[i]);
    }
}

//////////////////////////////////////////////////////////////////////////
// 获取k系数
void get_kfactor(harm_single_phase value[4]) {
    for (int i = 0; i < 4; ++i) {
        double sum_kf = 0.0;
        float* p = &value[i].curr_harm_percent[0];
        for (int k = 1; k <= 50; ++k) {  // 1th - 50th curr harm percent
            float tmp = (*p) / 100 * (*p) / 100 * k * k;
            sum_kf += tmp;
            p++;
        }
        value[i].kf = (float)sum_kf;
    }
}

//////////////////////////////////////////////////////////////////////////

// 调整相位
void adjust_base_angle(harm_single_phase value[4]) {
    // 以A相基波相位作为参考相位
    float ref_ang = value[0].volt_harm_angle[0];
    float* pAng;
    for (int i = 0; i < 4; ++i) {
        for (int k = 0; k < 63; ++k) {
            pAng = &(value[i].volt_harm_angle[k]);
            *pAng = *pAng - ref_ang;
            *pAng = *pAng < 0.0f ? *pAng + 360 : *pAng;
            pAng = &(value[i].curr_harm_angle[k]);
            *pAng = *pAng - ref_ang;
            *pAng = *pAng < 0.0f ? *pAng + 360 : *pAng;
        }
    }
    // 固定A相电压基波相位为0°
    value[0].volt_harm_angle[0] = 0.0f;
}

//////////////////////////////////////////////////////////////////////////

void calc_harm(td_int16_t* ch[8], td_int16_t* ch_line[3], td_int32_t ch_len,
               td_float_t pt_ct[8], harm_single_phase value[4],
               td_bool_t with_neutral, td_uint32_t thd_calc_order,
                float freq, float samp_rate) {
    if (thd_calc_order > 63) thd_calc_order = 50;
    if (NULL == volt_dft_arr || NULL == curr_dft_arr) {  // 初始化
        volt_dft_arr = DFT_MALLOC(struct dft_context, PHASE_COUNT);
        curr_dft_arr = DFT_MALLOC(struct dft_context, PHASE_COUNT);
        for (int i = 0; i < PHASE_COUNT; ++i) {
            dft_init(&volt_dft_arr[i], samp_rate, ch_len, freq);
            dft_init(&curr_dft_arr[i], samp_rate, ch_len, freq);
        }
    }

    // 每次都设置sacle
    for (int i = 0; i < PHASE_COUNT; ++i) {
        volt_dft_arr[i].scale = pt_ct[i];
        curr_dft_arr[i].scale = pt_ct[PHASE_COUNT + i];
    }

    // 执行fft
    if (with_neutral) {  //线电压
        for (int i = 0; i < 3; ++i) {
            dft_exec(&volt_dft_arr[i], ch_line[i], ch_len);
        }
    } else {  // 相电压
        for (int i = 0; i < 4; ++i) {
            dft_exec(&volt_dft_arr[i], ch[i], ch_len);
        }
    }
    for (int i = 0; i < 4; ++i) {
        dft_exec(&curr_dft_arr[i], ch[4 + i], ch_len);
    }

    // 开始获取值
    get_harm(value, thd_calc_order);
    get_inharm(value);
    get_hiharm(value);
    get_harm_power(value, with_neutral);
    get_kfactor(value);

    adjust_base_angle(value);
}
