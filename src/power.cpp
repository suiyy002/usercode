#include "power.h"
#include <math.h>
#include <memory.h>
#include "util.h"

float calc_power_single_p(td_int16_t* ch_volt, td_int16_t* ch_curr,
                          td_int32_t ch_len, td_float_t scale) {
    td_int64_t sum = 0;
    for (td_int32_t i = 0; i < ch_len; i++) {
        sum += (td_int32_t)ch_volt[i] * (td_int32_t)ch_curr[i];
    }

    return (float)((double)sum / ch_len * scale);
}

void calc_power_phase_all(td_int16_t* ch[8], td_int32_t ch_len,
                          td_float_t pt_ct[8], td_float_t time,
                          calc_result& result) {
    // ABC
    for (int i = 0; i < 3; ++i) {
        float p, q, s;

        // 视在功率
        s = result.volt_curr_rms[i].volt_phase_rms *
            result.volt_curr_rms[i].curr_rms;

        float scale = pt_ct[i] * pt_ct[4 + i];
        // 有功功率
        p = calc_power_single_p(ch[i], ch[4 + i], ch_len, scale);

        // 无功功率
        float Q_dir;  // Q方向
        float angle = ang_2_rad(result.harm[i].volt_harm_angle[0] -
                                result.harm[i].curr_harm_angle[0]);
        if (sinf(angle) > 0) {
            Q_dir = 1.0f;
        } else {
            Q_dir = -1.0f;
        }
        q = s * s - p * p;
        if (q < 0) {
            p = s;
            q = 0;
        } else {
            q = sqrtf(q) * Q_dir;
        }

        // DPF
        result.power.power_phase[i].power_dpf = cosf(angle);

        // 赋值
        result.power.power_phase[i].power_P = p;
        result.power.power_phase[i].power_Q = q;
        result.power.power_phase[i].power_S = s;
        result.power.power_phase[i].power_pf = p / s;

        // 计算电能
        result.power.power_phase[i].energy_P = p * time;
        result.power.power_phase[i].energy_Q = q * time;
        result.power.power_phase[i].energy_S = s * time;
    }

    // 计算总功率及电能
    result.power.power_P_tol = 0.0f;
    result.power.power_Q_tol = 0.0f;
    result.power.power_S_tol = 0.0f;
    result.power.energy_P_tol = 0.0f;
    result.power.energy_Q_tol = 0.0f;
    result.power.energy_S_tol = 0.0f;
    for (int i = 0; i < 3; ++i) {
        result.power.power_P_tol += result.power.power_phase[i].power_P;
        result.power.power_Q_tol += result.power.power_phase[i].power_Q;
        result.power.power_S_tol += result.power.power_phase[i].power_S;
        result.power.energy_P_tol += result.power.power_phase[i].energy_P;
        result.power.energy_Q_tol += result.power.power_phase[i].energy_Q;
        result.power.energy_S_tol += result.power.power_phase[i].energy_S;
    }
    result.power.power_pf_tol =
        result.power.power_P_tol / result.power.power_S_tol;
}

// 两表法
void calc_power_line_all(td_int16_t* ch[8], td_int16_t* ch_line[3],
                         td_int32_t ch_len, td_float_t pt_ct[8],
                         td_float_t time, calc_result& result) {
    memset(&result.power, 0, sizeof(result.power));

    // 此处必须ABC变比一致，仅使用A相变比
    float scale = pt_ct[0] * pt_ct[4];

    // 准备Ucb
    static td_int16_t* u_cb = new td_int16_t[ch_len];
    static td_int32_t pre_ch_len = ch_len;
    if (pre_ch_len < ch_len) {
        // 通道数据长度有变化，且缓冲区长度不够（工频切换）
        delete [] u_cb;
        u_cb = new td_int16_t[ch_len];
        pre_ch_len = ch_len;
    }
    for (int i = 0; i < ch_len; ++i) {
        u_cb[i] = -ch_line[1][i];
    }

    // 有功功率
    float p1, p2, p_tol;
    p1 = calc_power_single_p(ch_line[0], ch[4], ch_len, scale);  // Uab*Ia
    p2 = calc_power_single_p(u_cb, ch[6], ch_len, scale);        // Ucb*Ic
    p_tol = p1 + p2;

    // 视在功率
    float s1, s2, s_tol;
    s1 = result.volt_curr_rms[0].volt_line_rms *
         result.volt_curr_rms[0].curr_rms;  // Uab * Ia
    s2 = result.volt_curr_rms[1].volt_line_rms *
         result.volt_curr_rms[2].curr_rms;  // Ubc * Ic
    s_tol = (float)(sqrt(3.0) * (s1 + s2) / 2);

    // 无功功率，注：无法判断方向，只能为正
    float q;
    q = sqrtf(s_tol * s_tol - p_tol * p_tol);
    if (q < 0) {
        q = 0;
        p_tol = s_tol;
    }

    // 赋值
    result.power.power_P_tol = p_tol;
    result.power.power_Q_tol = q;
    result.power.power_S_tol = s_tol;
    result.power.power_pf_tol = p_tol / s_tol;
    result.power.energy_P_tol = p_tol * time;
    result.power.energy_Q_tol = q * time;
    result.power.energy_S_tol = s_tol * time;
}

//////////////////////////////////////////////////////////////////////////

void calc_power(td_int16_t* ch[8], td_int16_t* ch_line[3], td_int32_t ch_len,
                td_float_t pt_ct[8], calc_result& result,
                td_bool_t with_neutral) {
    // 毫秒
    td_uint32_t time_ms = (td_uint32_t)((result.end_time - result.begin_time) /
                                        (TIME_RESOLUTION / 1000ULL));
    float time = (time_ms / 1000.0f) / 3600.0f;  // time_ms/1h 用于计算电能
    if (with_neutral) {
        calc_power_line_all(ch, ch_line, ch_len, pt_ct, time, result);
    } else {  // 3p4w
        calc_power_phase_all(ch, ch_len, pt_ct, time, result);
    }
}
