
#ifndef DFT_COM_H_
#define DFT_COM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <stdlib.h>

/******************************************************************************/
#define DFT_MALLOC(type, len) ((type *)dft_malloc_z(sizeof(type) * (len)))
//#define DFT_FREE(p)     do { if (p) { free(p); p = NULL; } } while (0)

/******************************************************************************/
#ifndef bzero
#include <memory.h>
#define bzero(buf, size) (void)memset(buf, 0, size)
#endif

#ifndef PI
#define PI 3.141592653589793
#endif
#ifndef SQRT2
#define SQRT2 1.414213562373095
#endif

#define HI_HARM_BEGIN_FREQ (2005)
#define HI_HARM_STEP (200)

/******************************************************************************/
static void *dft_malloc_z(size_t size) {
    void *p = malloc(size);
    if (p) {
        bzero(p, size);
    }
    return p;
}

#ifdef __cplusplus
}
#endif

#endif
