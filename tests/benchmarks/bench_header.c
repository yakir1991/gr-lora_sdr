// Micro-bench for LoRa header encode/decode
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include "lora_header.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

int main(void) {
  const size_t iters = 1000000; // 1M
  uint8_t hdr[5];
  struct timespec t0, t1;

  // Encode bench
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (size_t i = 0; i < iters; ++i) {
    lora_build_header((uint8_t)(i & 0xFF), (uint8_t)((i>>2) & 0xF), (i & 1) != 0, hdr);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  double sec_enc = elapsed(t0, t1);
  double Mops_enc = (double)iters / 1e6 / sec_enc;

  // Decode bench (ignore failures)
  uint8_t len, cr; bool has_crc;
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (size_t i = 0; i < iters; ++i) {
    (void)lora_parse_header(hdr, &len, &cr, &has_crc);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  double sec_dec = elapsed(t0, t1);
  double Mops_dec = (double)iters / 1e6 / sec_dec;

  puts("metric,value");
  printf("header_encode_Mops,%.2f\n", Mops_enc);
  printf("header_decode_Mops,%.2f\n", Mops_dec);
  return 0;
}

