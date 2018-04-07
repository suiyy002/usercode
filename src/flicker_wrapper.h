#ifndef _FLICKER_WRAPPER_H_
#define _FLICKER_WRAPPER_H_

#include "flicker.h"

class flicker_wrapper {
   public:
    flicker_wrapper(float sample_rate);
    ~flicker_wrapper();

    void set_params(float scale, int lamp, int freq);
    void reset();
    void input(short* raw, int raw_len);

    float get_pst();
    float get_plt();

   private:
    flicker_handle self;
    float scale;
};

#endif  // _FLICKER_WRAPPER_H_