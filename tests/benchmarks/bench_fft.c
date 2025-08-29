#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <complex.h>
#include "lora_fft.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

static void *aligned_alloc32(size_t nbytes) {
  void *p = NULL;
  if (posix_memalign(&p, 32, nbytes) != 0) return NULL;
  return p;
}

int main(int argc, char **argv) {
  (void)argc; (void)argv;
  puts("metric,value");
#ifdef LORA_LITE_USE_LIQUID_FFT
  puts("fft_backend,liquid");
#else
  puts("fft_backend,kiss");
#endif

  for (unsigned sf = 7; sf <= 12; ++sf) {
    unsigned n = 1u << sf;
    size_t bytes = n * sizeof(float complex);
    float complex *in  = (float complex *)aligned_alloc32(bytes);
    float complex *out = (float complex *)aligned_alloc32(bytes);
    float complex *work= (float complex *)aligned_alloc32(bytes);
    float complex *tw  = (float complex *)aligned_alloc32(bytes);
    if (!in || !out || !work || !tw) return 2;

    for (unsigned i = 0; i < n; ++i)
      in[i] = (float)i / (float)n + I * (float)((i*7) % n) / (float)n;

    lora_fft_ctx_t ctx;
    int use_liquid_fft = 0;
#ifdef LORA_LITE_USE_LIQUID_FFT
    use_liquid_fft = 1;
#endif
    if (lora_fft_init(&ctx, n, work, tw, use_liquid_fft) != 0) return 3;

    // Calibrate iterations for ~30–50ms per size
    unsigned iters = (unsigned)(200000u / n);
    if (iters < 100) iters = 100;

    /* Warm-up */
    for (unsigned r = 0; r < 5; ++r)
      lora_fft_exec_fwd(&ctx, in, out);
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (unsigned r = 0; r < iters; ++r)
      lora_fft_exec_fwd(&ctx, in, out);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double sec = elapsed(t0, t1);
    double us_per_exec = (sec * 1e6) / (double)iters;
    printf("fft_us_per_exec_sf%u,%.3f\n", sf, us_per_exec);

    lora_fft_dispose(&ctx);
    free(in); free(out); free(work); free(tw);
  }

  return 0;
}
