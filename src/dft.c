#include "dft.h"
#include "dft_com.h"
#include "fftw3.h"

#define REAL_PART(p, index) ((p)[index][0])   // Êµ²¿
#define IMAGE_PART(p, index) ((p)[index][1])  // Ðé²¿

void dft_init(dft_context *dft, float samp_rate, uint32_t points,
              float sys_freq) {
    bzero(dft, sizeof(*dft));

    dft->points = points;
    dft->samp_rate = samp_rate;
    dft->spec_reso = dft->samp_rate / dft->points;
    dft->sys_freq = sys_freq;
    dft->spec_step = (uint32_t)(dft->sys_freq / dft->spec_reso);

    dft->in = FFTW_MANGLE_FLOAT(malloc)(sizeof(float) * dft->points);
    dft->out = FFTW_MANGLE_FLOAT(malloc)(sizeof(fftwf_complex) * dft->points);

    dft->plan = FFTW_MANGLE_FLOAT(plan_dft_r2c_1d)(dft->points, dft->in,
                                                   dft->out, FFTW_ESTIMATE);
    FFTW_MANGLE_FLOAT(execute)(dft->plan);
}

void dft_set_sacle(dft_context *dft, float scale) { dft->scale = scale; }

void dft_exec(dft_context *dft, int16_t *data, uint32_t len) {
    float *begin = dft->in;
    float *end = begin + len;

    while (begin < end) {
        *begin++ = (float)*data++;
    }
    FFTW_MANGLE_FLOAT(execute)(dft->plan);
}

void dft_get_val(dft_context *dft, uint32_t index, float *out_value) {
    float Re, Im;
    fftwf_complex *out = dft->out;
    uint32_t pps = dft->points * dft->points;
    if (index >= dft->points) {
        *out_value = 0;
        return;
    }
    if (index) {
        Re = REAL_PART(out, index);
        Im = IMAGE_PART(out, index);
        *out_value = (Re * Re + Im * Im) / pps;
    } else {  // dc
        Re = REAL_PART(out, index);
        Im = 0;
        *out_value = (Re * Re) / pps;
    }
}

void dft_get_val_ang(dft_context *dft, int index, float *out_val,
                     float *out_ang) {
    float Re, Im, ang;
    fftwf_complex *out = dft->out;
    int pps = dft->points * dft->points;

    Re = REAL_PART(out, index);
    Im = IMAGE_PART(out, index);
    ang = atan2f(Im, Re);
    if (ang < 0) ang += (float)(2 * PI);  // 0 - 2¦Ð
    *out_ang = ang;
    *out_val = (Re * Re + Im * Im) / pps;
}

void dft_destroy(dft_context *dft) {
    FFTW_MANGLE_FLOAT(free)(dft->in);
    FFTW_MANGLE_FLOAT(free)(dft->out);
    FFTW_MANGLE_FLOAT(destroy_plan)(dft->plan);
    dft->in = NULL;
    dft->out = NULL;
    dft->plan = NULL;
}
