#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "signal_utils.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

int main(void) {
  const size_t N = 2048;
  const int iters = 20000;
  double *xd = (double*)malloc(N * sizeof(double));
  float *xf = (float*)malloc(N * sizeof(float));
  if (!xd || !xf) return 1;

  for (size_t i = 0; i < N; ++i) {
    double v = sin((double)i * 0.01) * 0.5 + 0.5;
    xd[i] = v;
    xf[i] = (float)v;
  }

  struct timespec t0, t1;
  double acc = 0.0; /* prevent DCE */

  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int r = 0; r < iters; ++r) acc += rms(xd, N);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  double us_d = (elapsed(t0, t1) * 1e6) / iters;

  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int r = 0; r < iters; ++r) acc += rmsf(xf, N);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  double us_f = (elapsed(t0, t1) * 1e6) / iters;

  printf("metric,value\n");
  printf("rms_us_per_call_double,%.3f\n", us_d);
  printf("rms_us_per_call_float,%.3f\n", us_f);
  (void)acc;
  free(xd); free(xf);
  return 0;
}

