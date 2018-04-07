// CALC.cpp : Defines the entry point for the console application.
//
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include "calc.h"

#define TEST_50HZ 0
#define TEST_60HZ 1
#define DATA_LEN_50HZ (5120)
#define DATA_LEN_60HZ (6144)

#define SCALE_VOLT_TMP (1000.0f / 65535)
#define SCALE_CURR_TMP (100.0f / 65535)
//#define SAMPLE_LEN 5120
td_int16_t *pTmpDataBufVol[4];
td_int16_t *pTmpDataBufCurr[4];

void GetBaseSamp(td_int16_t *pdata, td_int32_t len, td_float_t value,
                 td_float_t angle, td_float_t scale) {
    td_double_t t;
    td_double_t u;

    u = value * SQRT2;
    for (int i = 0; i < len; i++) {
        t = u * sin(PI * i / 256.0 + PI * angle / 180.0f);
        pdata[i] = (td_int16_t)(t / scale);
    }
}

// 为波形添加谐波
// pdata - 波形缓冲区，len - 缓冲区长度，index - 谐波次数（0为直流）
// value - 谐波幅值， angle - 谐波角度，scale一次比率
void add_harm(td_int16_t *pdata, td_int32_t len, td_int32_t index,
              td_float_t value, td_float_t angle, td_float_t scale) {
    if (index < 0 || index > 50) {
        return;
    }

    if (0 == index) {  // 添加直流分量
        for (int i = 0; i < len; i++) {
            pdata[i] += (td_int16_t)(value * SQRT2 / scale);
        }
    } else {  // 添加各次谐波
        for (int i = 0; i < len; i++) {
            pdata[i] += (td_int16_t)(
                (value * SQRT2 *
                 sin(PI * i * index / 256.0f + PI * angle / 180.0f)) /
                scale);
        }
    }
}

int main(int argc, char *argv[]) {

#if TEST_50HZ
    td_int32_t data_len = DATA_LEN_50HZ;
#endif
#if TEST_60HZ
    td_int32_t data_len = DATA_LEN_60HZ;
#endif

    pTmpDataBufVol[0] = new td_int16_t[data_len];
    pTmpDataBufVol[1] = new td_int16_t[data_len];
    pTmpDataBufVol[2] = new td_int16_t[data_len];
    pTmpDataBufVol[3] = new td_int16_t[data_len];
    pTmpDataBufCurr[0] = new td_int16_t[data_len];
    pTmpDataBufCurr[1] = new td_int16_t[data_len];
    pTmpDataBufCurr[2] = new td_int16_t[data_len];
    pTmpDataBufCurr[3] = new td_int16_t[data_len];

    GetBaseSamp(pTmpDataBufVol[0], data_len, 200, 0, SCALE_VOLT_TMP);
    GetBaseSamp(pTmpDataBufVol[1], data_len, 200, 240, SCALE_VOLT_TMP);
    GetBaseSamp(pTmpDataBufVol[2], data_len, 200, 120, SCALE_VOLT_TMP);
    for (int i = 0; i < data_len; ++i) {
        pTmpDataBufVol[3][i] = 0;
    }

    GetBaseSamp(pTmpDataBufCurr[0], data_len, 5, 0, SCALE_CURR_TMP);
    GetBaseSamp(pTmpDataBufCurr[1], data_len, 5, 240, SCALE_CURR_TMP);
    GetBaseSamp(pTmpDataBufCurr[2], data_len, 5, 120, SCALE_CURR_TMP);
    /*for (int i = 0; i < HIGH_HARM_DATA_LEN; ++i)
    {
        pTmpDataBufCurr[2][i] = -(pTmpDataBufCurr[0][i] +
    pTmpDataBufCurr[1][i]);
    }*/
    memset(pTmpDataBufCurr[3], 0, sizeof(td_int16_t) * data_len);

    // 添加谐波测试
    add_harm(pTmpDataBufVol[0], data_len, 0, 2, 0, SCALE_VOLT_TMP);
    add_harm(pTmpDataBufVol[0], data_len, 2, 4, 10, SCALE_VOLT_TMP);
    add_harm(pTmpDataBufVol[0], data_len, 3, 6, 20, SCALE_VOLT_TMP);
    add_harm(pTmpDataBufVol[0], data_len, 50, 8, 50, SCALE_VOLT_TMP);

    td_int16_t *p[8];
    for (int i = 0; i < 4; ++i) {
        p[i] = pTmpDataBufVol[i];
        p[4 + i] = pTmpDataBufCurr[i];
    }

    calc calc_;
    // 初始化
    calc_config cc;
    cc.scale[0] = SCALE_VOLT_TMP;
    cc.scale[1] = cc.scale[0];
    cc.scale[2] = cc.scale[0];
    cc.scale[3] = cc.scale[0];
    cc.scale[4] = SCALE_CURR_TMP;
    cc.scale[5] = cc.scale[4];
    cc.scale[6] = cc.scale[4];
    cc.scale[7] = cc.scale[4];
    cc.nominal_volt = 230;
    cc.conn_type = conntype_3p4w;
    cc.sts_interval_sec = 1;
    cc.ch_data_len = data_len;
    cc.thd_calc_order = 50;
#if TEST_50HZ
    cc.freq_type = freqtype_50hz;
#endif
#if TEST_60HZ
    cc.freq_type = freqtype_60hz;
#endif
    calc_.init(cc);

    int loop = 0;
    td_uint64_t begin_time = 0;
    td_uint64_t end_time = 0;
    while (loop++ < 20) {
        end_time = begin_time + 2000000ULL;
        real_sts_result ret = calc_.push_data(p, begin_time, end_time);
        begin_time += 2000000ULL;
        if (ret.sts_available) {
            int k = 0;
        }
    }

    printf("123\r\n");
    getchar();
    return 0;
}
