#include "sts.h"
#include <memory.h>
#include "util.h"

statistic::statistic(void) { m_count = 0; }

statistic::~statistic(void) {}

bool is_power(calc_result* r, td_float_t* p) {
    if (p == &r->power.energy_P_tol || p == &r->power.energy_Q_tol ||
        p == &r->power.energy_S_tol) {
        return true;
    }
    for (int i = 0; i < 3; ++i) {
        if (p == &r->power.power_phase[i].energy_P ||
            p == &r->power.power_phase[i].energy_Q ||
            p == &r->power.power_phase[i].energy_S) {
            return true;
        }
    }
    return false;
}

td_bool_t statistic::push_real_data(const calc_result* real_result,
                                    sts_result* sts_result) {
    memcpy(&m_tr, real_result, sizeof(m_tr));
    m_count++;
    bool end_sts = false;

    if (1 == m_count) {
        m_sr.begin_time = m_tr.begin_time;
        memcpy(&m_sr.max, &m_tr, sizeof(m_tr));
        memcpy(&m_sr.min, &m_tr, sizeof(m_tr));
        memcpy(&m_sr.avg, &m_tr, sizeof(m_tr));
    } else {
        if (m_tr.end_time / TIME_RESOLUTION -
                m_sr.begin_time / TIME_RESOLUTION >=
            m_time_plot) {  // 统计结束
            m_sr.end_time = m_tr.end_time;
            end_sts = true;
        }

        float* pe = ((float*)&m_tr.power);
        float* p_src = &m_tr.freq;
        float* p_des_max = &m_sr.max.freq;
        float* p_des_min = &m_sr.min.freq;
        float* p_des_avg = &m_sr.avg.freq;
        while (p_src != pe) {
            *p_des_max = *p_des_max > *p_src ? *p_des_max : *p_src;
            *p_des_min = *p_des_min < *p_src ? *p_des_min : *p_src;
            *p_des_avg += *p_src;
            if (end_sts) {
                *p_des_avg /= m_count;
            }
            p_src++;
            p_des_max++;
            p_des_min++;
            p_des_avg++;
        }

        // 功率统计 - 只计算平均值，最大最小最后复制平均值
        pe = (float*)(&m_tr + 1);
        while (p_src != pe) {
            *p_des_avg += *p_src;
            if (end_sts) {
                *p_des_avg /= is_power(&m_tr, p_src) ? 1 : m_count;
            }
            p_des_avg++;
            p_src++;
        }
    }

    if (end_sts) {
        // 复制功率平均值到最大最小值
        memcpy(&m_sr.max.power, &m_sr.avg.power, sizeof(m_sr.avg.power));
        memcpy(&m_sr.min.power, &m_sr.avg.power, sizeof(m_sr.avg.power));

        memcpy(sts_result, &m_sr, sizeof(m_sr));
        m_count = 0;
        return true;
    }
    return false;
}

void statistic::reset() { m_count = 0; }

void statistic::set_interval_sec(td_uint32_t interval) {
    m_time_plot = (td_time_t)interval;
    reset();
}
