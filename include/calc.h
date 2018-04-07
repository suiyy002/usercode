#ifndef CALC_H_
#define CALC_H_

#include "com_type.h"

struct impl;

typedef struct real_sts_result {
    calc_result real;         // 实时结果
    sts_result sts;           // 统计结果
    td_bool_t sts_available;  // 统计结果是否有效
    flicker_result flicker;   // 波动闪变结果
} real_sts_result;

class __declspec(dllimport) calc {
   public:
    calc(void);
    ~calc(void);

    // 初始化
    void init(calc_config& cfg);
    // data排序为UaUbUcUnIaIbIcIn
    real_sts_result push_data(td_int16_t* data[8], td_uint64_t begin_time,
                              td_uint64_t end_time);
    void reset();
    void set_time_interval_sec(td_int32_t interval);
    void set_nominal_volt(td_float_t nominal_volt);
    void set_conn_type(conn_type ct);

   private:
    impl* pImpl;
};

#endif