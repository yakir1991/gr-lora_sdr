#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include "lora_fft.h"

static void *aligned_alloc32(size_t nbytes) {
  void *p = NULL;
  if (posix_memalign(&p, 32, nbytes) != 0) return NULL;
  return p;
}

int main(void) {
  puts("FFT round-trip (fwd->inv) test");
  for (unsigned sf = 7; sf <= 12; ++sf) {
    unsigned n = 1u << sf;
    size_t bytes = n * sizeof(float complex);
    float complex *x = (float complex *)aligned_alloc32(bytes);
    float complex *X = (float complex *)aligned_alloc32(bytes);
    float complex *y = (float complex *)aligned_alloc32(bytes);
    float complex *work = (float complex *)aligned_alloc32(bytes);
    float complex *tw = (float complex *)aligned_alloc32(bytes);
    if (!x || !X || !y || !work || !tw) return 2;

    // deterministic pseudo-random input
    for (unsigned i = 0; i < n; ++i) {
      float a = (float)((i * 1103515245u + 12345u) & 0xFFFF) / 65536.0f;
      float b = (float)((i * 2654435761u + 33331u) & 0xFFFF) / 65536.0f;
      x[i] = (a - 0.5f) + I * (b - 0.5f);
    }

    lora_fft_ctx_t ctx;
    int use_liquid = 0;
#ifdef LORA_LITE_USE_LIQUID_FFT
    use_liquid = 1;
#endif
    if (lora_fft_init(&ctx, n, work, tw, use_liquid) != 0) return 3;
    lora_fft_exec_fwd(&ctx, x, X);
    lora_fft_exec_inv(&ctx, X, y);

    float max_err = 0.0f, rms = 0.0f;
    for (unsigned i = 0; i < n; ++i) {
      float complex d = y[i] - x[i];
      float e = hypotf(crealf(d), cimagf(d));
      if (e > max_err) max_err = e;
      rms += e * e;
    }
    rms = sqrtf(rms / (float)n);
    printf("sf=%u max_err=%.3e rms=%.3e\n", sf, max_err, rms);
    if (!(max_err < 1e-5f && rms < 1e-6f)) {
      fprintf(stderr, "Round-trip error too large for sf=%u\n", sf);
      return 4;
    }

    lora_fft_dispose(&ctx);
    free(x); free(X); free(y); free(work); free(tw);
  }
  puts("FFT round-trip test passed");
  return 0;
}

