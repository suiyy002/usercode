#include "flicker.h"
#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include "system_response.h"

#define FLUCT_DEF_CPF_LEVEL_COUNT (16384)
#define FLUCT_DEF_CPF_MIN_LEVEL (1E-4f)
#define FLUCT_DEF_CPF_MAX_LEVEL (6400.0f)
#define FLUCT_DEF_PST_TIME (60 * 10)
#define FLUCT_DEF_PLT_COUNT (2)
#define FLUCT_DEF_STEADY_TIME (1)
#define __FS50 (800 * 2)
#define __FS60 (960 * 2)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
    int cpf_level_count;
    double cpf_min_level;
    double cpf_max_level;
    int ifs_discard_count;
    int pst_time;
    int plt_count;
} flicker_attribute;

#if __FS50 == 400
/*
 * filter parameters for 50Hz system @ fs=400bps
 */
static const double b_hp0p05[] = {0.99984662544737024081,
                                  -0.99984662544737024081};
static const double a_hp0p05[] = {1.00000000000000000000,
                                  -0.99969325089474025958};

static const double b_bw[] = {
    0.00017098737347332396, 0.00102592424083994380, 0.00256481060209985939,
    0.00341974746946647904, 0.00256481060209985939, 0.00102592424083994380,
    0.00017098737347332396,
};
static const double a_bw[] = {
    1.00000000000000000000,  -3.88066713921202532944, 6.53545082689976730705,
    -6.04948397973253904070, 3.22758099335941173180,  -0.93738908713244517568,
    0.11545157772012287767,
};
static const double b_bw50[] = {
    0.00105164679630761104f, 0.00630988077784566670f, 0.01577470194461416675f,
    0.02103293592615222002f, 0.01577470194461416675f, 0.00630988077784566670f,
    0.00105164679630761104f,
};
static const double a_bw50[] = {
    1.00000000000000000000f, -2.97852992612412803553f,
    4.13608099825747377309f, -3.25976427975097005074f,
    1.51727884474046748053f, -0.39111723059391217960f,
    0.04335698843475589703f,
};

/**
 *  weight
 */
static const double b_weight[] = {
    0.0093509686948439023346235643,  0.0003289757561777009275787165,
    -0.0183729616335101021695752621, -0.0003289757561776988133844801,
    0.0090219929386662033043986497,
};
static const double a_weight[] = {
    1.0000000000000000000000000000, -3.5487540776957531285518143704,
    4.7145479073186100649195395818, -2.7760103547932795109431935998,
    0.6103247813925570808279985613,
};

/*! 1, 0.53Hz, 300ms  */
static const double b_lp[] = {0.0041494015064909468837317519,
                              0.0041494015064909468837317519};
static const double a_lp[] = {1.000000, -0.9917011969870181964381572470};

/*
 * filter parameters for 60Hz system
 */
static const double b_hp0p05_60Hz[] = {
    0.99967285781060410965,
    -0.99967285781060410965,
};

static const double a_hp0p05_60Hz[] = {
    1.00000000000000000000,
    -0.99934571562120821930,
};

static const double b_lp42Hz_60Hz[] = {
    0.00017098737347332396, 0.00102592424083994380, 0.00256481060209985939,
    0.00341974746946647904, 0.00256481060209985939, 0.00102592424083994380,
    0.00017098737347332396,
};

static const double a_lp42Hz_60Hz[] = {
    1.00000000000000000000,  -3.88066713921202532944, 6.53545082689976730705,
    -6.04948397973253904070, 3.22758099335941173180,  -0.93738908713244517568,
    0.11545157772012287767,
};

static const double b_lp60Hz_60Hz[] = {
    0.00105164679630761104, 0.00630988077784566670, 0.01577470194461416675,
    0.02103293592615222002, 0.01577470194461416675, 0.00630988077784566670,
    0.00105164679630761104,
};
static const double a_lp60Hz_60Hz[] = {
    1.00000000000000000000,  -2.97852992612412803553, 4.13608099825747377309,
    -3.25976427975097005074, 1.51727884474046748053,  -0.39111723059391217960,
    0.04335698843475589703,
};

static const double b_weight_60Hz[] = {
    0.0044788085817488616779957766,  0.0001691051521942102387179724,
    -0.0087885120113035137406898301, -0.0001691051521942102387179724,
    0.0043097034295546511953323154,
};

static const double a_weight_60Hz[] = {
    1.0000000000000000000000000000, -3.6618726568410071564585450687,
    5.0284586097398884518838713120, -3.0679125981257135968860438879,
    0.7013757815018113461746906978,
};

static const double b_cost300ms_60Hz[] = {
    0.0034602214702047742918289419,
    0.0034602214702047742918289419,
};

static const double a_cost300ms_60Hz[] = {
    1.0000000000000000000000000000,
    -0.9930795570595905008559611815,
};
#else
/*__FS1600*/
/*
 * filter parameters for 50Hz system @ fs=1600bps
 */
static const double b_hp0p05[] = {
    0.99990183486659944823,
    -0.99990183486659944823,
};
static const double a_hp0p05[] = {
    1.00000000000000000000,
    -0.99980366973319878543,
};

static const double b_bw[] = {
    0.00000008151996000289, 0.00000048911976001735, 0.00000122279940004337,
    0.00000163039920005783, 0.00000122279940004337, 0.00000048911976001735,
    0.00000008151996000289,
};
static const double a_bw[] = {
    1.00000000000000000000,   -5.46903611713411219597, 12.48430992888281920727,
    -15.22410730016634161643, 10.45926407132676949630, -3.83810328840772152503,
    0.58767792277602715068,
};
static const double b_bw50[] = {
    0.00000062419117981099, 0.00000374514707886595, 0.00000936286769716487,
    0.00001248382359621982, 0.00000936286769716487, 0.00000374514707886595,
    0.00000062419117981099,
};
static const double a_bw50[] = {
    1.00000000000000000000,   -5.24160021213447713251, 11.49025716279382081098,
    -13.47981979048537759525, 8.92371177658399439281,  -3.16008836607898091486,
    0.46757937755652789757,
};

/**
 *  weight
 */
static const double b_weight[] = {
    0.0006865548875845530829922780,  0.0000061191383756923947690419,
    -0.0013669906367934137915443049, -0.0000061191383756923947690419,
    0.0006804357492088607085520269,
};
static const double a_weight[] = {
    1.0000000000000000000000000000, -3.8800970061779676001378902583,
    5.6448073041240700575826849672, -3.6491996942121436298123171582,
    0.8844898996730486961936890111,
};

/*! 1, 0.53Hz, 300ms  */
static const double b_lp[] = {
    0.0010405831023034372954938487,
    0.0010405831023034372954938487,
};
static const double a_lp[] = {
    1.0000000000000000000000000000,
    -0.9979188337953932563806347389,
};

/*
 * filter parameters for 60Hz system
 */
static const double b_hp0p05_60Hz[] = {
    0.99991819438383666707,
    -0.99991819438383666707,
};

static const double a_hp0p05_60Hz[] = {
    1.00000000000000000000,
    -0.99983638876767322312,
};

static const double b_lp42Hz_60Hz[] = {
    8.151996000289137861596319755e-008, 4.89119760017348298165558787e-007,
    1.222799400043370745413896968e-006, 1.630399200057827519379704748e-006,
    1.222799400043370745413896968e-006, 4.89119760017348298165558787e-007,
    8.151996000289137861596319755e-008,

};

static const double a_lp42Hz_60Hz[] = {
    1,
    -5.469036117134112195969919412,
    12.48430992888281743091738463,
    -15.22410730016633984007512481,
    10.45926407132676949629512819,
    -3.838103288407721525032911813,
    0.5876779227760269286306993308,
};

static const double b_lp60Hz_60Hz[] = {
    6.241911798109911343484146502e-007, 3.745147078865947017848724715e-006,
    9.362867697164866274072390906e-006, 1.248382359621982226345181938e-005,
    9.362867697164866274072390906e-006, 3.745147078865947017848724715e-006,
    6.241911798109911343484146502e-007,

};
static const double a_lp60Hz_60Hz[] = {
    1,
    -5.241600212134477132508436625,
    11.49025716279382258733221533,
    -13.47981979048537937160290312,
    8.923711776583997945522241935,
    -3.160088366078982691220744528,
    0.4675793775565282306416747815,
};

static const double b_weight_60Hz[] = {
    0.0003139325636082858578604359856,  3.005825309592868316971563808e-006,
    -0.0006248593019069788702737899833, -3.005825309592868316971563808e-006,
    0.0003109267382986930124133539977,

};

static const double a_weight_60Hz[] = {
    1,
    -3.912571651035466668844264859,
    5.740492963189209874030893843,
    -3.743212086017166040363690627,
    0.9152909922113815488486920913,
};

static const double b_cost300ms_60Hz[] = {
    0.0008673029062929008860260982,
    0.0008673029062929008860260982,
};

static const double a_cost300ms_60Hz[] = {
    1.0000000000000000000000000000,
    -0.9982653941874143344037406678,
};
#endif

typedef struct {
    system_response* sr_lp35Hz;
    system_response* sr_lp50Hz;
    system_response* sr_weight;
    system_response* sr_hp0p05Hz;
    system_response* sr_hp0p05Hz2;
    system_response* sr_const300ms;
} flicker_filter;

typedef struct {
    float x;
    float y;
} flicker_cpf_point;

typedef struct {
    flicker_filter filter;
    int fs;
    int step;
    int inst_count;
    float* p_inst;
    int inited;
    int system_freq; /* system frequency: 50/60Hz */
    int lamp;        /* lamp */

    int flicker_status;

    int ifs_discard_count;

    double cpf_base;
    double cpf_mantissa;
    double log_cpf_mantissa;
    int cpf_level_count;
    flicker_cpf_point* p_cpf;
    int cpf_buf_is_static;

    int real_time_pst_enable;
    int* cpf_buf_temp;
    int* cpf_buf;

    double pst3_sum;
    int pst3_count;
    double one_unit;

    flicker_attribute attrib;

    float rms_processing;
    float cpf_scale;
    float rms;
    float rms_sum;
    float rms_scale;
    unsigned int rms_count;
} flicker_context;

static void flicker_restart(flicker_context* self);
static void flicker_pst_restart(flicker_context* self);
static void flicker_plt_restart(flicker_context* self);
static void quadratic(double* pwave, int n);
static void flicker_state_machine_rebuild(flicker_context* self);
static void flicker_rms_rebuild(flicker_context* flicker);
static void flicker_rms_preview(flicker_context* flicker);
static void flicker_get_pst(flicker_context* self, double* pst, int* pst_count,
                            void* cpf, int cpf_count);

flicker_handle flicker_create(float fs) {
    flicker_context* self;
    self = (flicker_context*)malloc(sizeof(flicker_context));
    if (!self) return self;
    memset(self, 0x00, sizeof(flicker_context));
    self->fs = (int)fs;
    self->inited = 0;
    {
        flicker_filter* p = &self->filter;
        p->sr_lp35Hz = system_response_create();
        p->sr_lp50Hz = system_response_create();
        p->sr_weight = system_response_create();
        p->sr_hp0p05Hz = system_response_create();
        p->sr_hp0p05Hz2 = system_response_create();
        p->sr_const300ms = system_response_create();
    }

    flicker_set_lamp(self, 230, 50);

    flicker_attribute attrib;
    attrib.cpf_level_count = FLUCT_DEF_CPF_LEVEL_COUNT;
    attrib.cpf_min_level = FLUCT_DEF_CPF_MIN_LEVEL;
    attrib.cpf_max_level = FLUCT_DEF_CPF_MAX_LEVEL;
    attrib.pst_time = FLUCT_DEF_PST_TIME;
    attrib.plt_count = 12;
    attrib.ifs_discard_count = __FS50 * 60;

    self->ifs_discard_count = 0;
    self->pst3_sum = 0;
    self->cpf_base = attrib.cpf_min_level;
    self->cpf_mantissa = pow(attrib.cpf_max_level / attrib.cpf_min_level,
                             1.0 / (attrib.cpf_level_count - 1));
    self->log_cpf_mantissa = 1.0f / log(self->cpf_mantissa);
    self->cpf_level_count = attrib.cpf_level_count;
    self->p_cpf = (flicker_cpf_point*)malloc(sizeof(flicker_cpf_point) *
                                             self->cpf_level_count);

    if (self->cpf_level_count > 0) {
        int cpfSize = self->cpf_level_count * sizeof(int);
        self->cpf_buf_temp = (int*)malloc(cpfSize);
        self->cpf_buf = (int*)malloc(cpfSize);
        memset(self->cpf_buf, 0, cpfSize);
    }

    /*
     *  Make a backup copy for the attributes, however,
     *  it's better not to use the values in the attribute object.
     */
    self->attrib = attrib;
    flicker_state_machine_rebuild(self);
    return self;
}

void flicker_destroy(flicker_handle flicker) {
    flicker_context* self = (flicker_context*)flicker;
    flicker_filter* p = &self->filter;
    system_response_delete(p->sr_lp35Hz);
    system_response_delete(p->sr_lp50Hz);
    system_response_delete(p->sr_weight);
    system_response_delete(p->sr_hp0p05Hz);
    system_response_delete(p->sr_hp0p05Hz2);
    system_response_delete(p->sr_const300ms);

    if (!self->cpf_buf_is_static) {
        free(self->cpf_buf);
        self->cpf_buf_is_static = 1;
    }
    if (self->cpf_buf_temp) {
        free(self->cpf_buf_temp);
        self->cpf_buf_temp = 0;
    }
    if (self->p_inst) {
        free(self->p_inst);
        self->p_inst = 0;
    }
    if (self->p_cpf) {
        free(self->p_cpf);
        self->p_cpf = 0;
    }
}

void flicker_reset(flicker_handle flicker) {
    flicker_context* self = (flicker_context*)flicker;
    flicker_state_machine_rebuild(self);
    flicker_restart(self);
}

void flicker_set_scale(flicker_handle flicker, float scale) {
    flicker_context* self = (flicker_context*)flicker;
    self->rms_scale = scale;
}

void flicker_set_lamp(flicker_handle flicker, int lamp, int freq) {
    flicker_context* self = (flicker_context*)flicker;
    double fix_scale[4] = {
        1.0 / 1.0161, /* 0: 230V, 50Hz */
        1.0,          /* 1: 120V, 50Hz */
        1.0,          /* 2: 230V, 60Hz */
        1.0 / 1.0696, /* 3: 120V, 60Hz */
    };
    int select = 0;
    flicker_filter* p = &self->filter;
    self->system_freq = freq;
    self->lamp = lamp;

    switch (lamp) {
        default:
        case 230: /* 230V lamp */
            /* new filter  */
            system_response_set_order(p->sr_lp35Hz, 6, a_bw, b_bw);
            system_response_set_order(p->sr_lp50Hz, 6, a_bw50, b_bw50);
            system_response_set_order(p->sr_weight, 4, a_weight, b_weight);
            system_response_set_order(p->sr_hp0p05Hz, 1, a_hp0p05, b_hp0p05);
            system_response_set_order(p->sr_hp0p05Hz2, 1, a_hp0p05, b_hp0p05);
            system_response_set_order(p->sr_const300ms, 1, a_lp, b_lp);

            self->one_unit = 1.2838e+6;
            break;
        case 120: /* 120V lamp */
            system_response_set_order(p->sr_lp35Hz, 6, a_lp42Hz_60Hz,
                                      b_lp42Hz_60Hz);
            system_response_set_order(p->sr_lp50Hz, 6, a_lp60Hz_60Hz,
                                      b_lp60Hz_60Hz);
            system_response_set_order(p->sr_weight, 4, a_weight_60Hz,
                                      b_weight_60Hz);
            system_response_set_order(p->sr_hp0p05Hz, 1, a_hp0p05_60Hz,
                                      b_hp0p05_60Hz);
            system_response_set_order(p->sr_hp0p05Hz2, 1, a_hp0p05_60Hz,
                                      b_hp0p05_60Hz);
            system_response_set_order(p->sr_const300ms, 1, a_cost300ms_60Hz,
                                      b_cost300ms_60Hz);

            self->one_unit = 1.2838e+6;
            select |= 1;
            break;
    }

    switch (freq) {
        default:
        case 50:
            self->step = self->fs / __FS50;
            self->inst_count = (__FS50 * 200) / 1000;
            break;
        case 60:
            self->step = self->fs / __FS60;
            self->inst_count = (__FS60 * 200) / 1000;
            select |= 2;
            break;
    }
    if (self->p_inst) free(self->p_inst);
    self->p_inst = (float*)malloc(self->inst_count * sizeof(float));

    self->one_unit *= fix_scale[select] * fix_scale[select];

    flicker_state_machine_rebuild(self);
}

void flicker_input_raw(flicker_handle flicker, short* raw, int raw_len) {
    flicker_context* self = (flicker_context*)flicker;
    flicker_filter* filter = &self->filter;
    int inst_len = self->inst_count;
    float* p_inst = self->p_inst;
    double fbuffer[384];
    int buffer_np = sizeof(fbuffer) / sizeof(fbuffer[0]);
    int i;
    int n = raw_len / (buffer_np * self->step);
    int index = 0;
    int step = self->step;
    double cpf_scale = self->cpf_scale;
    short* end = raw + raw_len;

    n++;
    for (index = 0; index < n; index++) {
        int np = 0;
        for (i = 0; i < buffer_np && raw < end; ++i) {
            fbuffer[i] = *raw;
            raw += step;
        }
        np = i;
        if (np == 0) {
            break;
        }
        system_response_input(filter->sr_hp0p05Hz2, fbuffer, fbuffer, np);
        quadratic(fbuffer, np);
        system_response_input(filter->sr_lp35Hz, fbuffer, fbuffer, np);
        system_response_input(filter->sr_hp0p05Hz, fbuffer, fbuffer, np);
        system_response_input(filter->sr_lp50Hz, fbuffer, fbuffer, np);
        system_response_input(filter->sr_weight, fbuffer, fbuffer, np);
        quadratic(fbuffer, np);
        system_response_input(filter->sr_const300ms, fbuffer, fbuffer, np);

        int i = 0;
        // double baseCoef = 1.0f / self->cpf_base * 1.2838e+6 *
        // 0.71386*0.71386/0.75/0.75;
        double baseCoef;
        double mantissaCoef = self->log_cpf_mantissa;

        if (self->ifs_discard_count < self->attrib.ifs_discard_count) {
            self->ifs_discard_count += np;
            if (self->ifs_discard_count >= self->attrib.ifs_discard_count) {
                flicker_rms_rebuild(self);
                flicker_rms_preview(self);
            }
            return;
        }
        self->inited = 1;

        baseCoef = self->cpf_scale / self->cpf_base;
        for (i = 0; i < np; i++) {
            double r = fbuffer[i] * baseCoef;
            double f = log(MAX(r, 1)) * mantissaCoef;
            int n = MIN(((int)f), (self->cpf_level_count - 1));
            ++(self->cpf_buf[n]);
        }

        if (p_inst) {
            int j;
            for (j = 0; j < np && inst_len; j++, inst_len--) {
                /*              *p_inst++ = fbuffer[j] * 1.2838e+6; */
                *p_inst++ = (float)(fbuffer[j] * cpf_scale);
            }
        }
    }
}

void flicker_input_rms(flicker_handle flicker, float rms) {
    flicker_context* self = (flicker_context*)flicker;
    self->rms_sum += rms;
    self->rms_count++;
}

static void flicker_output_current_pst(flicker_handle flicker, void* cpf,
                                       int cpf_count, float* pst) {
    flicker_context* self = (flicker_context*)flicker;
    double p;
    self->real_time_pst_enable = 1;
    int dummy;
    flicker_get_pst(self, &p, &dummy, cpf, cpf_count);
    if (pst) {
        float fix = self->rms_processing / self->rms;
        *pst = (float)(fix * fix * p);
    }
}

float flicker_output_pst(flicker_handle flicker) {
    flicker_context* self = (flicker_context*)flicker;
    double p;
    double fix;
    int c;
    int* pst_count = &c;
    void* cpf = self->p_cpf;
    int cpf_count = self->cpf_level_count;

    if (!self->inited) {
        if (cpf) {
            memset(cpf, 0x00, sizeof(flicker_cpf_point) * cpf_count);
        }
        *pst_count = 0;
        return 0.0f;
    }

    flicker_rms_rebuild(self);
    fix = self->rms_processing / self->rms;
    /* printf( "rms: %f vs %f ,%f\n", self->rms, self->rms_processing, fix ); */
    flicker_rms_preview(self);
    self->real_time_pst_enable = 0;
    int dummy;
    flicker_get_pst(self, &p, &dummy, cpf, cpf_count);

    fix *= fix;
    p *= fix;
    self->pst3_sum += p * p * p;
    self->pst3_count++;
    flicker_pst_restart(self);
    *pst_count = self->pst3_count;
    return p;
}

float flicker_output_plt(flicker_handle flicker, int restart) {
    flicker_context* self = (flicker_context*)flicker;
    double p = 0.0f;

    if (self && self->pst3_count > 0) {
        p = pow(self->pst3_sum / self->pst3_count, 1 / 3.0);
        if (restart) flicker_plt_restart(self);
    }
    return (float)p;
}

static void flicker_restart(flicker_context* self) {
    flicker_pst_restart(self);
    flicker_plt_restart(self);
}

static void flicker_pst_restart(flicker_context* self) {
    memset(self->cpf_buf, 0, self->cpf_level_count * sizeof(self->cpf_buf[0]));
}

static void flicker_plt_restart(flicker_context* self) {
    self->pst3_sum = 0;
    self->pst3_count = 0;
}

static void quadratic(double* pwave, int n) {
    register int i;
    register double f;

    for (i = 0; i < n; i++) {
        f = *pwave;
        *pwave++ = f * f;
    }
}
static void flicker_rms_rebuild(flicker_context* self) {
    self->rms = self->rms_sum / self->rms_count / self->rms_scale;
    self->rms_count = 0;
    self->rms_sum = 0;
}

static void flicker_rms_preview(flicker_context* self) {
    float rms = self->rms;
    float one_unit = (float)self->one_unit;
    self->rms_processing = rms;
    rms *= rms;
    rms *= rms;  // rms^4 * SQRT(2)^4
    rms *= 4.0;
    self->cpf_scale = one_unit / rms;
}

static void flicker_get_pst(flicker_context* self, double* pst, int* pst_count,
                            void* cpf, int cpf_count) {
    static const double probNodes[] = {
        80.0, 50.0, 30.0, 17.0, 13.0, 10.0, 8.0, 6.0,
        4.0,  3.0,  2.2,  1.5,  1.0,  0.7,  0.1,
    };
    flicker_cpf_point* pCpf = cpf;
    double p = 0;
    double ifsNodes[sizeof(probNodes) / sizeof(probNodes[0])];
    double fullProbLevel = 0;
    int i = 0, j = 0;
    int* cpf_buf;

    // save if ...
    if (self->real_time_pst_enable) {
        memcpy(self->cpf_buf_temp, self->cpf_buf,
               self->cpf_level_count * sizeof(self->cpf_buf[0]));
        cpf_buf = self->cpf_buf_temp;
    } else {
        cpf_buf = self->cpf_buf;
    }

    /*
     *  Accumulate probability histogram
     */
    for (i = self->cpf_level_count - 2; i >= 0; i--) {
        cpf_buf[i] += cpf_buf[i + 1];
    }
    fullProbLevel = (double)(cpf_buf[0]);

    if (self->cpf_buf[0] == 0) {
        if (pst) {
            *pst = 0;
        }
        return;
    }

    /*
     *  Convert & Copy CPF
     */
    if (pCpf) {
        int r = MAX(self->cpf_level_count / cpf_count, 1);

        for (i = 0, j = 0; i < cpf_count && j < self->cpf_level_count;
             i += 1, j += r) {
            pCpf[i].x = (float)(self->cpf_base * pow(self->cpf_mantissa, j));
            pCpf[i].y = (float)(cpf_buf[j] / fullProbLevel);
        }

        if (i < cpf_count) {
            /*
             *  TODO: Set the X coordinates to appropriate b*m^x values.
             */
            memset(&pCpf[i], 0, (cpf_count - i) * sizeof(flicker_cpf_point));
        }
    }

    /*
     *  Get typical IFS values @[80%,50%, ..., 0.1%] on CPF graph
     */
    for (i = 0, j = 0; i < sizeof(probNodes) / sizeof(probNodes[0]); i++) {
        double neighborX[3];  // IFS
        double neighborY[3];  // Prob.

        double dstIfsIndex = 0;
        double dstProb = fullProbLevel * probNodes[i] / 100.0f;

        int dstProbLevel = (int)dstProb;

        /*
         *  Start from where the last time we stop, to make it work,
         *  we should sort the 'probNodes' in descending order.
         */
        for (/*j = 0*/; j < self->cpf_level_count; j++) {
            if (cpf_buf[j] < dstProbLevel) {
                break;
            }
        }
        if (j >= self->cpf_level_count) {
            j = self->cpf_level_count - 1;
            // status = PALIB_FAILED;
            // break;
        }
#if 0
        /*
         *  Second order Lagrange interpolation
         */
        if (j < 2)
        {
            neighborY[0] = (double)(0);
            neighborY[1] = (double)(1);
            neighborY[2] = (double)(2);
            neighborX[0] = (double)cpf_buf[0];
            neighborX[1] = (double)cpf_buf[1];
            neighborX[2] = (double)cpf_buf[2];
        }
        else
        {
            neighborY[0] = (double)(j - 2);
            neighborY[1] = (double)(j - 1);
            neighborY[2] = (double)(j - 0);
            neighborX[0] = (double)cpf_buf[j - 2];
            neighborX[1] = (double)cpf_buf[j - 1];
            neighborX[2] = (double)cpf_buf[j - 0];
        }

        if (fabs(neighborX[0] - neighborX[1]) > 1e-15 &&
            fabs(neighborX[1] - neighborX[2]) > 1e-15)
        {
            dstIfsIndex = (double)LagrangeInterp(neighborX, neighborY, 3, (PA_REAL)dstProb, &startNode);
        }

        dstIfsIndex = MIN(dstIfsIndex, (double)(self->cpf_level_count - 1));
        dstIfsIndex = MAX(dstIfsIndex, (double)0);
#else
        /*
         *  Piecewise linear interpolation
         */
        j = MAX(j, 1);

        neighborY[0] = (double)(j - 1);
        neighborY[1] = (double)(j - 0);
        neighborX[0] = (double)cpf_buf[j - 1];
        neighborX[1] = (double)cpf_buf[j - 0];

        if (fabs(neighborX[1] - neighborX[0]) > 1e-15) {
            dstIfsIndex = neighborY[0] + (dstProb - neighborX[0]) *
                                             (neighborY[1] - neighborY[0]) /
                                             (neighborX[1] - neighborX[0]);
        } else {
            dstIfsIndex = neighborX[0];
        }

#endif

        if (dstIfsIndex >= 1) {
            ifsNodes[i] = self->cpf_base * pow(self->cpf_mantissa, dstIfsIndex);
        } else {
            ifsNodes[i] = 0;
        }
    }

    /*
     *  Calculate statistical value of PST
     */
    if (1) {
        double ifsWeightSum = 0;
        double smoothIfsNodes[5];
        double ifsWeightTab[] = {
            // Pst = SQRT[0.0314*P0_1+0.0525*P1s+0.0657*P3s+0.28*P10s+0.08*P50s]
            // 0.0314f, 0.0525f, 0.0657f, 0.28f, 0.08f,
            0.08f, 0.28f, 0.0657f, 0.0525f, 0.0314f,
        };

        smoothIfsNodes[0] = (ifsNodes[0] + ifsNodes[1] + ifsNodes[2]) / 3;
        smoothIfsNodes[1] = (ifsNodes[3] + ifsNodes[4] + ifsNodes[5] +
                             ifsNodes[6] + ifsNodes[7]) /
                            5;
        smoothIfsNodes[2] = (ifsNodes[8] + ifsNodes[9] + ifsNodes[10]) / 3;
        smoothIfsNodes[3] = (ifsNodes[11] + ifsNodes[12] + ifsNodes[13]) / 3;
        smoothIfsNodes[4] = (ifsNodes[14]);

        for (i = 0; i < sizeof(ifsWeightTab) / sizeof(ifsWeightTab[0]); i++) {
            ifsWeightSum += smoothIfsNodes[i] * ifsWeightTab[i];
        }
        p = sqrt(ifsWeightSum);

        if (pst) {
            if (p > 100 || p < 0) {
                p = 0;
            }
            *pst = p;
        }
    }
}

static void flicker_state_machine_rebuild(flicker_context* self) {
    flicker_filter* p = &self->filter;
    self->ifs_discard_count = 0;
    self->inited = 0;
    self->cpf_scale = 0;
    self->rms_sum = 0;
    self->rms_count = 0;

    system_response_reset(p->sr_lp35Hz);
    system_response_reset(p->sr_lp50Hz);
    system_response_reset(p->sr_weight);
    system_response_reset(p->sr_hp0p05Hz);
    system_response_reset(p->sr_hp0p05Hz2);
    system_response_reset(p->sr_const300ms);
}
