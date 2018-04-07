#ifndef _FLICKER_H_
#define _FLICKER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void* flicker_handle;

flicker_handle flicker_create(float fs);
void flicker_destroy(flicker_handle flicker);

void flicker_reset(flicker_handle flicker);
void flicker_set_scale(flicker_handle flicker, float scale);
void flicker_set_lamp(flicker_handle flicker, int lamp, int freq);

void flicker_input_raw(flicker_handle flicker, short* raw, int raw_len);
void flicker_input_rms(flicker_handle flicker, float rms);
float flicker_output_pst(flicker_handle flicker);
float flicker_output_plt(flicker_handle flicker, int restart);

#ifdef __cplusplus
}
#endif

#endif  // _FLICKER_H_