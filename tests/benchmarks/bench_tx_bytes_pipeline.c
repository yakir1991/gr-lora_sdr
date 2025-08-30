// Micro-benchmark: TX bytes pipeline (CRC add + whiten)
// Compares in-place path (optimized) vs copy-heavy path (baseline)
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "lora_whitening.h"
#include "lora_add_crc.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

static void gen_payload(uint8_t *p, size_t n) {
  uint32_t s = 0x89abcdefu;
  for (size_t i = 0; i < n; ++i) {
    s = s * 1103515245u + 12345u;
    p[i] = (uint8_t)(s >> 24);
  }
}

int main(void) {
  const size_t payload_len = 512; // typical small packet
  const size_t nsym = payload_len + 2; // payload + 2 CRC bytes

  uint8_t *payload = (uint8_t *)malloc(payload_len);
  uint8_t *tx_inplace  = (uint8_t *)malloc(nsym);
  uint8_t *tx_copybase = (uint8_t *)malloc(nsym);
  uint8_t *whitened    = (uint8_t *)malloc(nsym);
  if (!payload || !tx_inplace || !tx_copybase || !whitened) return 2;
  gen_payload(payload, payload_len);

  const int iters = 10000;
  struct timespec t0, t1; double sec;
  puts("metric,value");

  // In-place: payload -> add CRC in-place -> whiten in-place
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int r = 0; r < iters; ++r) {
    memcpy(tx_inplace, payload, payload_len);
    tx_inplace[payload_len] = 0; tx_inplace[payload_len+1] = 0;
    uint8_t nibbles[4];
    lora_add_crc(tx_inplace, nsym, nibbles);
    tx_inplace[payload_len]   = (uint8_t)((nibbles[1] << 4) | nibbles[0]);
    tx_inplace[payload_len+1] = (uint8_t)((nibbles[3] << 4) | nibbles[2]);
    lora_whiten(tx_inplace, tx_inplace, nsym);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  printf("tx_pipeline_inplace_MBps,%.2f\n", ((double)nsym * iters) / (sec * 1e6));

  // Copy-heavy: payload -> copy -> CRC -> copy -> whiten to separate buffer
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int r = 0; r < iters; ++r) {
    memcpy(tx_copybase, payload, payload_len);
    tx_copybase[payload_len] = 0; tx_copybase[payload_len+1] = 0;
    uint8_t nibbles[4];
    lora_add_crc(tx_copybase, nsym, nibbles);
    tx_copybase[payload_len]   = (uint8_t)((nibbles[1] << 4) | nibbles[0]);
    tx_copybase[payload_len+1] = (uint8_t)((nibbles[3] << 4) | nibbles[2]);
    memcpy(whitened, tx_copybase, nsym); // extra staging copy
    lora_whiten(whitened, whitened, nsym);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  printf("tx_pipeline_copy_MBps,%.2f\n", ((double)nsym * iters) / (sec * 1e6));

  free(whitened); free(tx_copybase); free(tx_inplace); free(payload);
  return 0;
}

