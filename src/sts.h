#ifndef STS_H_
#define STS_H_

#include "com_type.h"

class statistic {
   public:
    statistic(void);
    ~statistic(void);
    td_bool_t push_real_data(const calc_result* real_result,
                             sts_result* sts_result);
    void reset();
    // 设置统计间隔
    void set_interval_sec(td_uint32_t interval);

   private:
    calc_result m_tr;
    sts_result m_sr;
    int m_count;
    td_time_t m_time_plot;  // 统计间隔
};

#endif