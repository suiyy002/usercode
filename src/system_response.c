#include "system_response.h"
#include <stdlib.h>

static double get_delay_unit_output(const double *d, const double *coef,
                                    int index, int n) {
    double s = 0;
    double t;
    int i;

    for (i = index; i < n; i++) {
        double coe = *coef++;
        t = d[i] * coe;
        s += t;
    }
    for (i = 0; i < index; i++) {
        double coe = *coef++;
        t = d[i] * coe;
        s += t;
    }
    return s;
}

static void input(system_response *self, const double *signal_input,
                  double *signal_output, int signal_len) {
    double *d = self->d;
    const double *a = self->a + 1;
    const double *b = self->b + 1;
    double b0 = self->b[0];
    double a0 = self->a[0];
    int n = self->n;
    int w = self->w;
    const int *next = self->map_next;
    int i;
    double t;
    double t0;
    double out;
    double t_lasta;
    double t_lastb;
    double *outp = signal_output;

    if (a0 == 1) {
        for (i = 0; i < signal_len; i++) {
            t_lasta = get_delay_unit_output(d, a, w, n);
            t_lastb = get_delay_unit_output(d, b, w, n);
            t0 = *signal_input++ - t_lasta;
            t = t0 * b0;
            out = t + t_lastb;
            *outp++ = out;
            w = next[w];
            d[w] = t0;
        }
    } else {
        for (i = 0; i < signal_len; i++) {
            t_lasta = get_delay_unit_output(d, a, w, n);
            t_lastb = get_delay_unit_output(d, b, w, n);

            t0 = *signal_input++ - t_lasta;
            t0 = t0 / a0;
            t = t0 * b0;
            out = t + t_lastb;
            *outp++ = out;
            w = next[w];
            d[w] = t0;
        }
    }

    self->w = w;
    if (self->next) {
        self->input(self->next, signal_input, signal_output, signal_len);
    }
}

system_response *system_response_create() {
    system_response *self = (system_response *)malloc(sizeof(system_response));
    if (!self) return NULL;
    memset(self, 0, sizeof(*self));
    self->input = input;
    self->qx = 0;
    return self;
}

void system_response_delete(system_response *self) {
    if (!self) return;
    if (self->next) {
        system_response_delete(self->next);
        self->next = NULL;
    }

    if (self->d) {
        free(self->d);
        self->d = NULL;
    }
    if (self->map_next) {
        free(self->map_next);
        self->map_next = NULL;
    }

    if (self->ab_dynamic) {
        self->ab_dynamic = 0;
        free(self->a);
        free(self->b);
    }

    free(self);
}

void system_response_reset(system_response *self) {
    memset(self->d, 0x00, sizeof(double) * self->n);
    self->w = 0;
}

void system_response_set_order(system_response *self, int n, const double *a,
                               const double *b) {
    double *p;
    int *map;
    int i;

    p = (double *)realloc(self->d,
                          sizeof(double) * (n)); /**<  need n-1 delay unit */
    memset(p, 0, sizeof(double) * (n));
    map = (int *)realloc(self->map_next, sizeof(int) * (n));
    for (i = 0; i < n; i++) {
        map[i] = i - 1 < 0 ? n - 1 : i - 1;
    }
    self->d = p;
    self->a = (double *)a;
    self->b = (double *)b;
    self->n = n;
    self->map_next = map;
}

void system_response_set_order_dynamic(system_response *self, int n) {
    if (self->ab_dynamic) {
        if (n == self->n) {
            return;
        }
    } else {
        self->a = NULL;
        self->b = NULL;
    }
    self->a = (double *)realloc(self->a, sizeof(double) * (n + 1));
    self->b = (double *)realloc(self->b, sizeof(double) * (n + 1));
    system_response_set_order(self, n, self->a, self->b);
    self->ab_dynamic = 1;
}

void system_response_append(system_response *self, system_response *next) {
    while (self->next) {
        self = self->next;
        if (self == next) return;
    }
    self->next = next;
}

void system_response_input(system_response *self, const double *signal_input,
                           double *signal_output, int signal_len) {
    if (self->input) {
        self->input(self, signal_input, signal_output, signal_len);
    }
}

double *system_response_get_a(system_response *self) { return self->a; }

double *system_response_get_b(system_response *self) { return self->b; }
