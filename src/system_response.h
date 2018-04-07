#ifndef _SYSTEM_RESPONSE_H_
#define _SYSTEM_RESPONSE_H_

#include <memory.h>
#include <stdint.h>

typedef struct system_response_t {
    int n;
    double *d; /**<  @brief  delay */
    double *a; /**<  @brief coef y[] */
    double *b; /**<  @brief coef x[] */
    int *map_next;
    int w;
    int ab_dynamic;
    struct system_response_t *next;
    void (*input)(struct system_response_t *self, const double *signal_input,
                  double *signal_output, int signal_len);
    int qx;
} system_response;

system_response *system_response_create(void);
void system_response_delete(system_response *self);
void system_response_reset(system_response *self);

void system_response_set_order(system_response *self, int n, const double *a,
                               const double *b);
void system_response_set_order_dynamic(system_response *self, int n);

void system_response_append(system_response *self, system_response *next);
void system_response_input(system_response *self, const double *signal_input,
                           double *signal_output, int signal_len);

double *system_response_get_a(system_response *self);
double *system_response_get_b(system_response *self);

#endif  //_SYSTEM_RESPONSE_H_
