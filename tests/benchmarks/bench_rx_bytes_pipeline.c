// Micro-benchmark: RX bytes pipeline (dewhiten + CRC check)
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
  uint32_t s = 0x12345678u;
  for (size_t i = 0; i < n; ++i) {
    s = s * 1664525u + 1013904223u;
    p[i] = (uint8_t)(s >> 24);
  }
}

int main(void) {
  const size_t payload_len = 512; // realistic small payload
  const size_t nsym = payload_len + 2; // payload + 2 CRC bytes (nibbles)
  uint8_t *payload = (uint8_t *)malloc(payload_len);
  uint8_t *tx_buf  = (uint8_t *)malloc(nsym);
  uint8_t *tx_whiten = (uint8_t *)malloc(nsym);
  if (!payload || !tx_buf || !tx_whiten) return 2;
  gen_payload(payload, payload_len);

  // Build TX: payload + CRC (as two bytes, packed from nibbles), then whiten
  memcpy(tx_buf, payload, payload_len);
  tx_buf[payload_len] = 0;
  tx_buf[payload_len+1] = 0;
  uint8_t nibbles[4];
  lora_add_crc(tx_buf, nsym, nibbles);
  tx_buf[payload_len]   = (uint8_t)((nibbles[1] << 4) | nibbles[0]);
  tx_buf[payload_len+1] = (uint8_t)((nibbles[3] << 4) | nibbles[2]);
  lora_whiten(tx_buf, tx_whiten, nsym);

  // Bench setup
  const int iters = 10000;
  uint8_t *rx_bytes = (uint8_t *)malloc(nsym);
  uint8_t *tmp1 = (uint8_t *)malloc(nsym);
  uint8_t *tmp2 = (uint8_t *)malloc(nsym);
  if (!rx_bytes || !tmp1 || !tmp2) return 3;
  size_t ok_inplace = 0, ok_copy = 0;

  puts("metric,value");

  // In-place path: dewhiten in-place and CRC over same buffer
  struct timespec t0, t1; double sec;
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int r = 0; r < iters; ++r) {
    memcpy(rx_bytes, tx_whiten, nsym);
    lora_dewhiten(rx_bytes, rx_bytes, nsym);
    uint8_t save1 = rx_bytes[payload_len];
    uint8_t save2 = rx_bytes[payload_len+1];
    rx_bytes[payload_len] = 0;
    rx_bytes[payload_len+1] = 0;
    uint8_t out4[4];
    lora_add_crc(rx_bytes, nsym, out4);
    rx_bytes[payload_len] = save1;
    rx_bytes[payload_len+1] = save2;
    uint8_t c1 = (uint8_t)((out4[1] << 4) | out4[0]);
    uint8_t c2 = (uint8_t)((out4[3] << 4) | out4[2]);
    ok_inplace += (c1 == save1 && c2 == save2);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  printf("rx_pipeline_inplace_MBps,%.2f\n", ((double)nsym * iters) / (sec * 1e6));
  printf("rx_pipeline_inplace_ok,%zu\n", ok_inplace);

  // Copy-heavy baseline: dewhiten into tmp1, then copy payload and compute CRC
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int r = 0; r < iters; ++r) {
    lora_dewhiten(tx_whiten, tmp1, nsym);
    memcpy(tmp2, tmp1, nsym); // extra copy to simulate staging
    uint8_t out4[4];
    tmp2[payload_len] = 0; tmp2[payload_len+1] = 0;
    lora_add_crc(tmp2, nsym, out4);
    uint8_t c1 = (uint8_t)((out4[1] << 4) | out4[0]);
    uint8_t c2 = (uint8_t)((out4[3] << 4) | out4[2]);
    ok_copy += (c1 == tmp1[payload_len] && c2 == tmp1[payload_len+1]);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  printf("rx_pipeline_copy_MBps,%.2f\n", ((double)nsym * iters) / (sec * 1e6));
  printf("rx_pipeline_copy_ok,%zu\n", ok_copy);

  free(tmp2); free(tmp1); free(rx_bytes);
  free(tx_whiten); free(tx_buf); free(payload);
  return 0;
}

