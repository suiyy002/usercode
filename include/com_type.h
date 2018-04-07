/*
    detail: 本文件用于定义参数配置结构体、计算结果结构体等公用结构体
*/

#ifndef COM_TYPE_H_
#define COM_TYPE_H_

#include "type_def.h"

#define CHANNAL_COUNT (8)  // 4U4I

// 接线方式枚举定义
typedef enum conn_type_t {
    conntype_3p4w = 1,        // 三相四线
    conntype_3p3w_star = 2,   // 三相三线星形
    conntype_3p3w_angle = 3,  // 三相三线三角形
    conntype_3p3w_vv = 4      // 三相三线VV
} conn_type;

// 工频枚举定义
typedef enum freq_type {
    freqtype_50hz   = 1,    // 50Hz
    freqtype_60hz   = 2,    // 60hz
    freqtype_solid  = 3     // 定频采样（预留）
}freq_type;

 typedef struct calibration_param {
    td_float_t  k;
    td_float_t  b;
}calibration_param;

// 参数配置结构体
typedef struct calc_config {
    enum conn_type_t conn_type;    // 接线方式
    td_uint32_t sts_interval_sec;  // 统计时间间隔，单位为s
    td_int32_t ch_data_len;        // 一次接收单通道数据长度
    td_float_t
        scale[CHANNAL_COUNT];  // 实际比率，包括一次和二次,UaUbUcUnIaIbIcIn
     calibration_param cali_param[CHANNAL_COUNT];  //
    // 八通道校准参数，UaUbUcUnIaIbIcIn
    td_float_t nominal_volt;  // 标称电压，用于计算电压偏差
    td_uint32_t thd_calc_order; // THD计算次数
    freq_type   freq_type;      // 工频频率，要与ch_data_len同步使用
} calc_config;

// 单相谐波数据
typedef struct harm_single_phase {
    // 谐波 - 电压
    td_float_t volt_harm_rms[63];  // 1 - 63次谐波电压有效值
    td_float_t volt_harm_percent[63];  // 1 - 63次谐波电压百分比（含有率）
    td_float_t volt_harm_angle[63];         // 1 - 63次谐波电压相位角
    td_float_t volt_harm_thd_rms;           // 电压总谐波有效值
    td_float_t volt_harm_thd_percent;       // 电压总谐波百分比
    td_float_t volt_harm_thd_odd_rms;       // 奇次电压总谐波有效值
    td_float_t volt_harm_thd_odd_percent;   // 奇次电压总谐波百分比
    td_float_t volt_harm_thd_even_rms;      // 偶次电压总谐波有效值
    td_float_t volt_harm_thd_even_percent;  // 偶次电压总谐波百分比
    td_float_t volt_harm_dc_rms;            // 电压直流分量有效值
    td_float_t volt_harm_dc_percent;        // 电压直流分量含有率

    // 谐波 - 电流
    td_float_t curr_harm_rms[63];  // 1 - 63次谐波电流有效值
    td_float_t curr_harm_percent[63];  // 1 - 63次谐波电流百分比（含有率）
    td_float_t curr_harm_angle[63];         // 1 - 63次谐波电流相位角
    td_float_t curr_harm_thd_rms;           // 电流总谐波有效值
    td_float_t curr_harm_thd_percent;       // 电流总谐波百分比
    td_float_t curr_harm_thd_odd_rms;       // 奇次电流总谐波有效值
    td_float_t curr_harm_thd_odd_percent;   // 奇次电流总谐波百分比
    td_float_t curr_harm_thd_even_rms;      // 偶次电流总谐波有效值
    td_float_t curr_harm_thd_even_percent;  // 偶次电流总谐波百分比
    td_float_t curr_harm_dc_rms;            // 电流直流分量有效值
    td_float_t curr_harm_dc_percent;        // 电流直流分量含有率

    // 间谐波
    td_float_t volt_inharm_rms[63];      // 1 - 63次电压间谐波有效值
    td_float_t volt_inharm_percent[63];  // 1 - 63次电压间谐波含有率
    td_float_t volt_inharm_thd_rms;      // 电压间谐波总畸变有效值
    td_float_t volt_inharm_thd_percent;  // 电压间谐波总畸变含有率
    td_float_t curr_inharm_rms[63];      // 1 - 63次电流间谐波有效值
    td_float_t curr_inharm_percent[63];  // 1 - 63次电流间谐波含有率
    td_float_t curr_inharm_thd_rms;      // 电流间谐波总畸变有效值
    td_float_t curr_inharm_thd_percent;  // 电流间谐波总畸变含有率

    // 高次谐波
    td_float_t volt_hiharm_rms[35];      // 1 - 35次电压高次谐波有效值
    td_float_t volt_hiharm_percent[35];  // 1 - 35次电压高次谐波含有率
    td_float_t curr_hiharm_rms[35];      // 1 - 35次电流高次谐波有效值
    td_float_t curr_hiharm_percent[35];  // 1 - 35次电流高次谐波含有率

    // 谐波功率 - 有功
    td_float_t power_harm_rms[63];      // 1 - 63次谐波功率有效值
    td_float_t power_harm_percent[63];  // 1 - 63次谐波功率含有率
    td_float_t power_harm_angle[63];  // 1 - 63次谐波功率相位角（u-i）
    td_float_t power_harm_total_rms;    // 总谐波有功有效值
    td_float_t power_harm_total_percent;    // 总谐波有功含有率

    // 谐波功率 - 无功
    td_float_t power_harm_var_rms[63];      // 1 - 63次谐波功率有效值
    td_float_t power_harm_var_percent[63];  // 1 - 63次谐波功率含有率
    td_float_t power_harm_var_total_rms;    // 总谐波无功有效值
    td_float_t power_harm_var_total_percent;    // 总谐波无功含有率


    // 谐波功率 - 视在
    td_float_t power_harm_va_rms[63];      // 1 - 63次谐波功率有效值
    td_float_t power_harm_va_percent[63];  // 1 - 63次谐波功率含有率
    td_float_t power_harm_va_total_rms;    // 总谐波视在有效值
    td_float_t power_harm_va_total_percent;    // 总谐波视在含有率

    td_float_t kf;  // k factor
} harm_single_phase;

// 单相电压电流基本值
typedef struct volt_curr_value_single {
    // 线电压值
    td_float_t volt_line_rms;     // 线电压有效值
    td_float_t volt_line_pk_pos;  // 线电压正峰值
    td_float_t volt_line_pk_neg;  // 线电压负峰值
    td_float_t volt_line_cf;      // 线电压峰值因数 pk/rms
    // 相电压值
    td_float_t volt_phase_rms;     // 相电压有效值
    td_float_t volt_phase_pk_pos;  // 相电压正峰值
    td_float_t volt_phase_pk_neg;  // 相电压负峰值
    td_float_t volt_phase_cf;      // 相电压峰值因数 pk/rms
    // 电压偏差 - 3P4W时用相电压，3P3W时用线电压
    td_float_t volt_deviation;  // N无效
    // 电流值
    td_float_t curr_rms;     // 电流有效值
    td_float_t curr_pk_pos;  // 电流正峰值
    td_float_t curr_pk_neg;  // 电流负峰值
    td_float_t curr_cf;      // 电流峰值因数 pk/rms
} volt_curr_value_single;

// 不平衡
typedef struct unbalance {
    // 不平衡 - 序分量及不平衡度
    td_float_t volt_component_zero;  // 电压零序分量
    td_float_t curr_component_zero;  // 电流零序分量
    td_float_t volt_component_pos;   // 电压正序分量
    td_float_t curr_component_pos;   // 电流正序分量
    td_float_t volt_component_neg;   // 电压负序分量
    td_float_t curr_component_neg;   // 电流负序分量
    td_float_t volt_unbal_neg;       // 电压负序不平衡度
    td_float_t curr_unbal_neg;       // 电流负序不平衡度
    td_float_t volt_unbal_zero;      // 电压零序不平衡度
    td_float_t curr_unbal_zero;      // 电流零序不平衡度
} unbalance;

// 单项功率电能
typedef struct power_energy_phase_single {
    td_float_t power_P;    // ABC有功功率
    td_float_t power_Q;    // ABC无功功率
    td_float_t power_S;    // ABC视在功率
    td_float_t power_pf;   // ABC功率因数
    td_float_t power_dpf;  // ABC基波功率因数
    td_float_t energy_P;   // ABC有功电能
    td_float_t energy_Q;   // ABC无功电能
    td_float_t energy_S;   // ABC视在功率电能
} power_energy_phase_single;

typedef struct power_energy {
    power_energy_phase_single power_phase[3];
    td_float_t power_P_tol;   // 总有功功率
    td_float_t power_Q_tol;   // 总无功功率
    td_float_t power_S_tol;   // 总视在功率
    td_float_t power_pf_tol;  // 总功率因数
    td_float_t energy_P_tol;  // 总有功电能
    td_float_t energy_Q_tol;  // 总无功电能
    td_float_t energy_S_tol;  // 总视在功率电能
} power_energy;

// 计算结果结构体
typedef struct calc_result {
    td_uint64_t begin_time;                   // 开始时间
    td_uint64_t end_time;                     // 结束时间
    td_float_t freq;                          // 频率
    volt_curr_value_single volt_curr_rms[4];  // ABCN 有效值、峰值等
    harm_single_phase harm[4];                // ABCN 谐波
    unbalance unbal;                          // 不平衡
    power_energy power;                       // 功率电能
} calc_result;

// 统计结果结构体
typedef struct sts_result {
    td_uint64_t begin_time;  // 开始时间
    td_uint64_t end_time;    // 结束时间
    calc_result max;         // 最大值
    calc_result min;         // 最小值
    calc_result avg;         // 平均值
} sts_result;

// 闪变计算结果结构体
typedef struct flicker_result {
    td_time_t time;           // 结果时间
    td_float_t fluct[3];      // 波动,通过pst反推，与pst同时有效
    td_float_t pst[3];        // 短闪变
    td_bool_t pst_available;  // 短闪变有效标识
    td_float_t plt[3];        // 长闪变
    td_bool_t plt_available;  // 长闪变有效标识
} flicker_result;

#endif
