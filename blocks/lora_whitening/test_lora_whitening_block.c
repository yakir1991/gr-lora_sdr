#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "lora_whitening.h"

static int check_invert(const uint8_t *in, size_t len) {
  uint8_t *w = (uint8_t*)malloc(len);
  uint8_t *d = (uint8_t*)malloc(len);
  if (!w || !d) return -1;
  lora_whiten(in, w, len);
  lora_dewhiten(w, d, len);
  int ok = (memcmp(in, d, len) == 0);
  free(w); free(d);
  return ok ? 0 : 1;
}

int main(void) {
  int fails = 0;
  /* Patterns and lengths */
  const size_t lens[] = { 1, 7, 16, 64, 255, 256, 511, 512, 1024 };
  for (size_t t = 0; t < sizeof(lens)/sizeof(lens[0]); ++t) {
    size_t L = lens[t];
    uint8_t *buf = (uint8_t*)malloc(L);
    if (!buf) return 1;
    /* zero */
    memset(buf, 0x00, L); fails += check_invert(buf, L);
    /* ones */
    memset(buf, 0xFF, L); fails += check_invert(buf, L);
    /* counter */
    for (size_t i = 0; i < L; ++i) buf[i] = (uint8_t)i; fails += check_invert(buf, L);
    /* pseudo-random (LCG) */
    uint32_t s = 1234567u; for (size_t i = 0; i < L; ++i) { s = s*1664525u + 1013904223u; buf[i] = (uint8_t)(s >> 24); }
    fails += check_invert(buf, L);
    free(buf);
  }
  printf("metric,value\n");
  printf("tests,%zu\n", (size_t)(4 * (sizeof(lens)/sizeof(lens[0]))));
  printf("mismatches,%d\n", fails);
  if (fails == 0) {
    puts("Whitening block test passed");
    return 0;
  }
  return 2;
}

