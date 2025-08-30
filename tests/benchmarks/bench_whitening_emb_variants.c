// Embedded-profile whitening variants micro-benchmark
// Measures throughput of: sequence XOR vs LUT8 dynamic vs LUT8 static
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LORA_WHITENING_SEQ_LEN 255

static void generate_whitening_seq(uint8_t *seq) {
  uint8_t lfsr = 0xFF;
  for (size_t i = 0; i < LORA_WHITENING_SEQ_LEN; ++i) {
    seq[i] = lfsr;
    uint8_t new_bit = ((lfsr >> 7) ^ (lfsr >> 5) ^ (lfsr >> 4) ^ (lfsr >> 3)) & 0x01u;
    lfsr = (uint8_t)((lfsr << 1) | new_bit);
  }
}

static void build_lut8(uint64_t masks[256], uint8_t next8[256]) {
  for (int s = 0; s < 256; ++s) {
    uint8_t lfsr = (uint8_t)s;
    uint8_t bytes[8];
    for (int i = 0; i < 8; ++i) {
      bytes[i] = lfsr;
      uint8_t new_bit = ((lfsr >> 7) ^ (lfsr >> 5) ^ (lfsr >> 4) ^ (lfsr >> 3)) & 0x01u;
      lfsr = (uint8_t)((lfsr << 1) | new_bit);
    }
    uint64_t mask = 0;
    memcpy(&mask, bytes, 8);
    masks[s] = mask;
    next8[s] = lfsr;
  }
}

// Include static LUT8 tables from source
extern const uint64_t LORA_WHITEN_LUT8_STATIC[256];
extern const uint8_t  LORA_WHITEN_NEXT8_STATIC[256];
#include "../../src/whiten_lut8_static.c"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

static void whiten_seq(const uint8_t *in, uint8_t *out, size_t len,
                       const uint8_t *seq) {
  size_t i = 0, seq_idx = 0;
  while (i < len) {
    size_t chunk = LORA_WHITENING_SEQ_LEN - seq_idx;
    if (chunk > len - i) chunk = len - i;
    const uint8_t *s = seq + seq_idx;
    for (size_t k = 0; k < chunk; ++k) out[i + k] = (uint8_t)(in[i + k] ^ s[k]);
    i += chunk;
    seq_idx += chunk;
    if (seq_idx == LORA_WHITENING_SEQ_LEN) seq_idx = 0;
  }
}

static void whiten_lut8_dyn(const uint8_t *in, uint8_t *out, size_t len,
                            const uint64_t *masks, const uint8_t *next8) {
  size_t i = 0;
  uint8_t lfsr = 0xFFu;
  for (; i + 8 <= len; i += 8) {
    uint64_t a64;
    memcpy(&a64, in + i, 8);
    a64 ^= masks[lfsr];
    memcpy(out + i, &a64, 8);
    lfsr = next8[lfsr];
  }
  while (i < len) {
    out[i] = (uint8_t)(in[i] ^ lfsr);
    uint8_t new_bit = ((lfsr >> 7) ^ (lfsr >> 5) ^ (lfsr >> 4) ^ (lfsr >> 3)) & 0x01u;
    lfsr = (uint8_t)((lfsr << 1) | new_bit);
    ++i;
  }
}

static void whiten_lut8_static(const uint8_t *in, uint8_t *out, size_t len) {
  size_t i = 0;
  uint8_t lfsr = 0xFFu;
  for (; i + 8 <= len; i += 8) {
    uint64_t a64;
    memcpy(&a64, in + i, 8);
    a64 ^= LORA_WHITEN_LUT8_STATIC[lfsr];
    memcpy(out + i, &a64, 8);
    lfsr = LORA_WHITEN_NEXT8_STATIC[lfsr];
  }
  while (i < len) {
    out[i] = (uint8_t)(in[i] ^ lfsr);
    uint8_t new_bit = ((lfsr >> 7) ^ (lfsr >> 5) ^ (lfsr >> 4) ^ (lfsr >> 3)) & 0x01u;
    lfsr = (uint8_t)((lfsr << 1) | new_bit);
    ++i;
  }
}

int main(void) {
  const size_t len = 8192;
  uint8_t *in = (uint8_t *)malloc(len);
  uint8_t *out = (uint8_t *)malloc(len);
  if (!in || !out) return 2;
  for (size_t i = 0; i < len; ++i) in[i] = (uint8_t)(i * 31u + 7u);

  uint8_t seq[LORA_WHITENING_SEQ_LEN];
  generate_whitening_seq(seq);

  uint64_t masks[256];
  uint8_t next8[256];
  build_lut8(masks, next8);

  struct timespec t0, t1;
  const int iters = 2000;
  double sec;
  puts("metric,value");

  // Sequence XOR
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int r = 0; r < iters; ++r) whiten_seq(in, out, len, seq);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  printf("whitening_seq_MBps,%.3f\n", ((double)len * iters) / (sec * 1e6));

  // LUT8 dynamic
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int r = 0; r < iters; ++r) whiten_lut8_dyn(in, out, len, masks, next8);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  printf("whitening_lut8_dyn_MBps,%.3f\n", ((double)len * iters) / (sec * 1e6));

  // LUT8 static
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int r = 0; r < iters; ++r) whiten_lut8_static(in, out, len);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  printf("whitening_lut8_static_MBps,%.3f\n", ((double)len * iters) / (sec * 1e6));

  printf("bytes,%.0f\n", (double)len);

  free(in);
  free(out);
  return 0;
}

