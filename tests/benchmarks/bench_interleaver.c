#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "lora_interleaver.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

int main(void) {
  const uint8_t sf = 7, sf_app = 7, cw_len = 8;
  const size_t blocks = 4096;
  uint8_t *in = (uint8_t *)malloc(sf_app * blocks);
  uint32_t *out = (uint32_t *)malloc(sizeof(uint32_t) * cw_len * blocks);
  uint8_t *back = (uint8_t *)malloc(sf_app * blocks);
  if (!in || !out || !back) return 2;
  for (size_t b = 0; b < sf_app * blocks; ++b) in[b] = (uint8_t)(b * 17u + 3u);

  struct timespec t0, t1;
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (size_t r = 0; r < blocks; ++r) {
    lora_interleave(&in[r * sf_app], &out[r * cw_len], sf, sf_app, cw_len, true);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  double sec = elapsed(t0, t1);
  double blocks_per_sec = blocks / sec;

  /* Quick correctness sanity: inverse */
  for (size_t r = 0; r < blocks; ++r)
    lora_deinterleave(&out[r * cw_len], &back[r * sf_app], sf, sf_app, cw_len);
  if (memcmp(in, back, sf_app * blocks) != 0) {
    fprintf(stderr, "interleave/deinterleave mismatch!\n");
    return 3;
  }

  puts("metric,value");
  printf("interleave_blocks_per_sec_sf%u_cw%u,%.1f\n", sf, cw_len, blocks_per_sec);
  free(in); free(out); free(back);
  return 0;
}

