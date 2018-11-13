#include "calc.h"
#include <windows.h>
#include <memory.h>
#include "flicker_wrapper.h"
#include "harm.h"
#include "power.h"
#include "rms.h"
#include "half_rms.h"
#include "sts.h"
#include "unbalance.h"
#include "zal_chn_fluct.h"

#define SAMPLE_PER_CYCLE (512)  // 每周波采样点数

struct impl {
    calc_config config_;
    calc_result result_;
    td_int16_t* sample_data_[8];       // 原始数据
    td_int16_t* sample_line_data_[3];  // 线电压波形
    statistic* p_sts_;
    flicker_wrapper* flicker_[3];
    ZalChnFluctInfo* fluct_;
};

calc::calc(void) {
    pImpl = new impl;
    pImpl->p_sts_ = new statistic;
    for (int i = 0; i != 3; ++i) {
        pImpl->flicker_[i] = new flicker_wrapper(25600);
    }
    zalChnFluctInit(pImpl->fluct_, 3);
}

calc::~calc(void) {
    delete pImpl->p_sts_;
    for (int i = 0; i != 3; ++i) {
        delete pImpl->flicker_[i];
    }
    zalChnFluctDeinit(pImpl->fluct_);
    delete pImpl;
}

void get_samprate_freq(freq_type freq_type_, // [in]
                       float* samp_rate,    // [out]
                       float* freq)         // [out]
{
    switch(freq_type_) {
    case freqtype_50hz:
        *freq = 50.0f;
        break;
    case freqtype_60hz:
        *freq = 60.0f;
        break;
    default:
        *freq = 0.0f;
        break;
    }
    *samp_rate = *freq * SAMPLE_PER_CYCLE;
}

void calc::init(calc_config& cfg) {
    pImpl->config_ = cfg;
    for (int i = 0; i < 8; ++i) {
        pImpl->sample_data_[i] = new td_int16_t[pImpl->config_.ch_data_len];
        if (i < 3) {
            pImpl->sample_line_data_[i] = new td_int16_t[pImpl->config_.ch_data_len];
        }
    }
    pImpl->p_sts_->set_interval_sec(pImpl->config_.sts_interval_sec);
    for (int i = 0; i != 3; ++i) {
        pImpl->flicker_[i]->set_params(cfg.scale[i], 230, 50);
    }
    reset();
}

namespace {
void fetch_flicker_result(impl* pImpl, real_sts_result& rs_result,
                                td_uint64_t end_time) {
    rs_result.flicker.time = end_time;
    const td_uint64_t _10min = 10llu * 60 * 1000 * 1000 * 10;
    const td_uint64_t _100ms = 100llu * 1000 * 10;
    td_uint64_t remain = end_time % _10min;
    float dummy;
    if ((remain < _100ms) || ((_10min - remain) < _100ms)) {
        rs_result.flicker.pst_available = true;
        for (int i = 0; i != 3; ++i) {
            rs_result.flicker.pst[i] = pImpl->flicker_[i]->get_pst();
        }
        float fluctChange[3];
        zalFluctFromFlicker(pImpl->fluct_, rs_result.flicker.pst,
            rs_result.flicker.fluct, fluctChange);
        zalChnFluctReset(pImpl->fluct_);
    } else {
        rs_result.flicker.pst_available = false;
    }
    const td_uint64_t _2h = 2llu * 60 * 60 * 1000 * 1000 * 10;
    remain = end_time % _2h;
    if ((remain < _100ms) || ((_2h - remain) < _100ms)) {
        rs_result.flicker.plt_available = true;
        for (int i = 0; i != 3; ++i) {
            rs_result.flicker.plt[i] = pImpl->flicker_[i]->get_plt();
        }
    } else {
        rs_result.flicker.plt_available = false;
    }
}
}

real_sts_result calc::push_data(td_int16_t* data[8], td_uint64_t begin_time,
                                td_uint64_t end_time) {
    real_sts_result rs_result;
    memset(&rs_result, 0, sizeof(rs_result));

    // 将波形数据拷贝到本地缓存
    for (int i = 0; i < 8; ++i) {
        memcpy(pImpl->sample_data_[i], data[i],
               sizeof(td_int16_t) * pImpl->config_.ch_data_len);
    }

    // 预处理波形数据
    for (int i = 0; i < pImpl->config_.ch_data_len; ++i) {
        // 调整相电压电压 A、B、C - N
        pImpl->sample_data_[0][i] = pImpl->sample_data_[0][i] - pImpl->sample_data_[3][i];
        pImpl->sample_data_[1][i] = pImpl->sample_data_[1][i] - pImpl->sample_data_[3][i];
        pImpl->sample_data_[2][i] = pImpl->sample_data_[2][i] - pImpl->sample_data_[3][i];
        // 调整线电压波形
        pImpl->sample_line_data_[0][i] = pImpl->sample_data_[0][i] - pImpl->sample_data_[1][i];
        pImpl->sample_line_data_[1][i] = pImpl->sample_data_[1][i] - pImpl->sample_data_[2][i];
        pImpl->sample_line_data_[2][i] = pImpl->sample_data_[2][i] - pImpl->sample_data_[1][i];
        // 如果是VV接线方式，调整B相电流波形ib = -（ia+ic）
        if (conntype_3p3w_vv == pImpl->config_.conn_type) {
            pImpl->sample_data_[5][i] = -(pImpl->sample_data_[4][i] + pImpl->sample_data_[6][i]);
        }
    }

    td_bool_t with_neutral = true;
    if (conntype_3p4w == pImpl->config_.conn_type) {
        with_neutral = false;
    }
    pImpl->result_.begin_time = begin_time;
    pImpl->result_.end_time = end_time;
    // 计算有效值
    calc_rms(pImpl->sample_data_, pImpl->sample_line_data_, pImpl->config_.ch_data_len,
             pImpl->config_.scale, pImpl->config_.nominal_volt, with_neutral,
             pImpl->result_.volt_curr_rms);

    // 计算半波有效值
    calc_half_rms(pImpl->sample_data_, pImpl->sample_line_data_, pImpl->config_.ch_data_len,
                  pImpl->config_.scale, pImpl->config_.freq_type, rs_result.half_rms);

    // 计算谐波
    float samp_rate, freq;
    get_samprate_freq(pImpl->config_.freq_type, &samp_rate, &freq);
    calc_harm(pImpl->sample_data_, pImpl->sample_line_data_, pImpl->config_.ch_data_len,
              pImpl->config_.scale, pImpl->result_.harm, with_neutral,
              pImpl->config_.thd_calc_order, freq, samp_rate);
    // 计算不平衡
    calc_unbal(pImpl->result_);
    // 计算功率及电能
    calc_power(pImpl->sample_data_, pImpl->sample_line_data_, pImpl->config_.ch_data_len,
               pImpl->config_.scale, pImpl->result_, with_neutral);

    // 计算闪变
    for (int i = 0; i != 3; ++i) {
        pImpl->flicker_[i]->input(pImpl->sample_data_[i], pImpl->config_.ch_data_len);
        if (pImpl->config_.conn_type == conntype_3p4w) {
            zalChnFluctCalc(i, rs_result.half_rms.volt_phase[i],
                            pImpl->fluct_, rs_result.half_rms.number);
        } else {
            zalChnFluctCalc(i, rs_result.half_rms.volt_line[i],
                            pImpl->fluct_, rs_result.half_rms.number);
        }
    }
    rs_result.real = pImpl->result_;
    rs_result.sts_available =
        pImpl->p_sts_->push_real_data(&rs_result.real, &rs_result.sts);

    fetch_flicker_result(pImpl, rs_result, end_time);

    return rs_result;
}

void calc::reset() {
    pImpl->p_sts_->reset();
    for (int i = 0; i != 3; ++i) {
        pImpl->flicker_[i]->reset();
    }
    zalChnFluctReset(pImpl->fluct_);
}

void calc::set_time_interval_sec(td_int32_t interval) {
    pImpl->config_.sts_interval_sec = interval;
    pImpl->p_sts_->set_interval_sec(interval);
    reset();
}

void calc::set_nominal_volt(td_float_t nominal_volt) {
    pImpl->config_.nominal_volt = nominal_volt;
    reset();
}

void calc::set_conn_type(conn_type ct) {
    pImpl->config_.conn_type = ct;
    reset();
}
