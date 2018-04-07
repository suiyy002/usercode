#ifndef DFT_H_
#define DFT_H_

typedef signed short int16_t;
typedef unsigned int uint32_t;

#ifdef __cplusplus
extern "C" {
#endif

// DFT上下文
typedef struct dft_context {
    float scale;         // 变比
    uint32_t points;     // fft点数
    float samp_rate;     // 采样率
    float spec_reso;     // 频谱分辨率
    uint32_t spec_step;  // 频谱步进,不用于高次谐波（200Hz）
    float sys_freq;      // 系统频率

    void *plan;
    void *in;
    void *out;
} dft_context;

/******************************************************************************/
void dft_init(dft_context *dft, float samp_rate, uint32_t points,
              float sys_freq);
void dft_set_sacle(dft_context *dft, float scale);
void dft_exec(dft_context *dft, int16_t *data, uint32_t len);
void dft_get_val(dft_context *dft, uint32_t index, float *out_value);
void dft_get_val_ang(dft_context *dft, int index, float *out_value,
                     float *out_ang);
void dft_destroy(dft_context *dft);

#ifdef __cplusplus
}
#endif

#endif
