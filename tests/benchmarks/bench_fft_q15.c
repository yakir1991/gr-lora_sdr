#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <complex.h>
#include "lora_fft.h"
#include "lora_fft_q15.h"
#include "lora_fixed.h"
#include <math.h>

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

static void *aligned_alloc32(size_t nbytes) {
  void *p = NULL;
  if (posix_memalign(&p, 32, nbytes) != 0) return NULL;
  return p;
}

int main(void) {
  puts("metric,value");
#ifdef LORA_LITE_USE_CMSIS
  puts("q15_backend,cmsis_or_fallback");
#else
  puts("q15_backend,fallback_float");
#endif
  puts("float_backend,kiss");

  for (unsigned sf = 7; sf <= 12; ++sf) {
    unsigned n = 1u << sf;
    size_t fbytes = n * sizeof(float complex);
    size_t qbytes = n * sizeof(lora_q15_complex);
    float complex *in_f  = (float complex *)aligned_alloc32(fbytes);
    float complex *out_f = (float complex *)aligned_alloc32(fbytes);
    float complex *work  = (float complex *)aligned_alloc32(fbytes);
    float complex *tw    = (float complex *)aligned_alloc32(fbytes);
    lora_q15_complex *in_q  = (lora_q15_complex *)aligned_alloc32(qbytes);
    lora_q15_complex *out_q = (lora_q15_complex *)aligned_alloc32(qbytes);
    if (!in_f || !out_f || !work || !tw || !in_q || !out_q) return 2;

    for (unsigned i = 0; i < n; ++i) {
      float complex v = (float)i / (float)n + I * (float)((i*7) % n) / (float)n;
      in_f[i] = v;
      in_q[i] = lora_float_to_q15(v);
    }

    // Init float FFT
    lora_fft_ctx_t fctx;
    if (lora_fft_init(&fctx, n, work, tw, 0) != 0) return 3;
    // Init Q15 FFT
    lora_fft_q15_ctx_t qctx;
    if (lora_fft_q15_init(&qctx, n) != 0) return 4;

    // Calibrate iterations for ~30–50ms per size
    unsigned iters = (unsigned)(200000u / n);
    if (iters < 100) iters = 100;

    // Warm-up
    for (unsigned r = 0; r < 5; ++r) {
      lora_fft_exec_fwd(&fctx, in_f, out_f);
      lora_fft_q15_exec_fwd(&qctx, in_q, out_q);
    }

    struct timespec t0, t1;
    // Time Q15
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (unsigned r = 0; r < iters; ++r)
      lora_fft_q15_exec_fwd(&qctx, in_q, out_q);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double q_sec = elapsed(t0, t1);
    double q_us_per = (q_sec * 1e6) / (double)iters;

    // Time float
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (unsigned r = 0; r < iters; ++r)
      lora_fft_exec_fwd(&fctx, in_f, out_f);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double f_sec = elapsed(t0, t1);
    double f_us_per = (f_sec * 1e6) / (double)iters;

    // Compare accuracy: convert Q15 out to float and diff
    double max_abs = 0.0, rms = 0.0;
    for (unsigned i = 0; i < n; ++i) {
      float complex qf = lora_q15_to_float(out_q[i]);
      float complex df = out_f[i] - qf;
      double mag = cabsf(df);
      if (mag > max_abs) max_abs = mag;
      rms += mag * mag;
    }
    rms = (n > 0) ? sqrt(rms / (double)n) : 0.0;

    printf("q15_us_per_exec_sf%u,%.3f\n", sf, q_us_per);
    printf("float_us_per_exec_sf%u,%.3f\n", sf, f_us_per);
    printf("q15_vs_float_max_abs_sf%u,%.6f\n", sf, max_abs);
    printf("q15_vs_float_rms_sf%u,%.6f\n", sf, rms);

    lora_fft_dispose(&fctx);
    lora_fft_q15_dispose(&qctx);
    free(in_f); free(out_f); free(work); free(tw);
    free(in_q); free(out_q);
  }

  return 0;
}
