#include "unbalance.h"
#include <math.h>
#include "util.h"

// 计算序分量
void unbal(td_float_t* out_pos, td_float_t* out_neg, td_float_t* out_zero,
           td_float_t rms[3], td_float_t angle[3]) {
    float angleA;
    float angleB120;
    float angleB240;
    float angleC120;
    float angleC240;

    float real;
    float image;
    float angleBx[3];
    float angleCx[3];

    angleA = angle[0];
    angleB120 = angle[1] + (float)(2 * PI / 3);
    angleB240 = angle[1] - (float)(2 * PI / 3);
    angleC120 = angle[2] + (float)(2 * PI / 3);
    angleC240 = angle[2] - (float)(2 * PI / 3);

    angleBx[0] = angle[1];
    angleBx[1] = angleB120;
    angleBx[2] = angleB240;

    angleCx[0] = angle[2];
    angleCx[1] = angleC240;
    angleCx[2] = angleC120;

    // zero, pos, neg
    float* p_arr[3] = {out_zero, out_pos, out_neg};
    for (int i = 0; i < 3; i++) {
        real = rms[0] * cosf(angleA) + rms[1] * cosf(angleBx[i]) +
               rms[2] * cosf(angleCx[i]);
        image = rms[0] * sinf(angleA) + rms[1] * sinf(angleBx[i]) +
                rms[2] * sinf(angleCx[i]);
        *p_arr[i] = sqrtf(real * real + image * image) / 3.0f;
    }
}

void calc_unbal(calc_result& result) {
    float rms[3];
    float angle[3];
    unbalance* pUnb = &result.unbal;

    // 电压 - 序分量
    for (int i = 0; i < 3; ++i) {
        rms[i] = result.harm[i].volt_harm_rms[0];
        angle[i] = ang_2_rad(result.harm[i].volt_harm_angle[0]);  // 弧度
    }
    unbal(&pUnb->volt_component_pos, &pUnb->volt_component_neg,
          &pUnb->volt_component_zero, rms, angle);

    // 电流 - 序分量
    for (int i = 0; i < 3; ++i) {
        rms[i] = result.harm[i].curr_harm_rms[0];
        angle[i] = ang_2_rad(result.harm[i].curr_harm_angle[0]);  // 弧度
    }
    unbal(&pUnb->curr_component_pos, &pUnb->curr_component_neg,
          &pUnb->curr_component_zero, rms, angle);

    // 不平衡度
    pUnb->volt_unbal_neg =
        pUnb->volt_component_neg / pUnb->volt_component_pos * 100.0f;
    pUnb->volt_unbal_zero =
        pUnb->volt_component_zero / pUnb->volt_component_pos * 100.0f;
    pUnb->curr_unbal_neg =
        pUnb->curr_component_neg / pUnb->curr_component_pos * 100.0f;
    pUnb->curr_unbal_zero =
        pUnb->curr_component_zero / pUnb->curr_component_pos * 100.0f;
}
