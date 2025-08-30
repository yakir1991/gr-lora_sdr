#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdint.h>
#include <stdlib.h>
#include "signal_utils.h"

static long double rms_ld(const long double *x, size_t n) {
    if (!x || n == 0) return 0.0L;
    long double s = 0.0L;
    for (size_t i = 0; i < n; ++i) s += x[i] * x[i];
    return sqrtl(s / (long double)n);
}

int main(void) {
    /* Construct a sequence with wide dynamic range to stress summation */
    const size_t N = 4096;
    double *xd = (double*)malloc(N * sizeof(double));
    long double *xl = (long double*)malloc(N * sizeof(long double));
    float *xf = (float*)malloc(N * sizeof(float));
    if (!xd || !xl || !xf) return 1;

    for (size_t i = 0; i < N; ++i) {
        double v = (i % 2 == 0) ? 1e-3 : 1e3; /* alternate small/large */
        xd[i] = v;
        xl[i] = (long double)v;
        xf[i] = (float)v;
    }

    long double ref = rms_ld(xl, N);
    double rd = rms(xd, N);
    float rf = rmsf(xf, N);

    double rel_err_d = (ref > 0) ? fabs((double)ref - rd) / (double)ref : 0.0;
    double rel_err_f = (ref > 0) ? fabs((double)ref - (double)rf) / (double)ref : 0.0;

    printf("metric,value\n");
    printf("rms_ref,%.10Lf\n", ref);
    printf("rms_double,%.10f\n", rd);
    printf("rms_float,%.10f\n", rf);
    printf("rel_err_double,%.3e\n", rel_err_d);
    printf("rel_err_float,%.3e\n", rel_err_f);

    free(xd); free(xl); free(xf);

    /* Guardrails: keep errors small */
    if (rel_err_d > 1e-12) {
        fprintf(stderr, "rel_err_double too large: %.3e\n", rel_err_d);
        return 2;
    }
    if (rel_err_f > 1e-5f) {
        fprintf(stderr, "rel_err_float too large: %.3e\n", rel_err_f);
        return 3;
    }

    puts("Signal utils block tests passed");
    return 0;
}

