// Benchmark: full-frame symbols pipeline (preamble + SFD + payload)
// Uses symbol-domain sync (frame_sync) + dewhiten + CRC check
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "lora_frame_sync.h"
#include "lora_whitening.h"
#include "lora_add_crc.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

static void gen_payload(uint8_t *p, size_t n) {
  uint32_t s = 0x31415926u;
  for (size_t i = 0; i < n; ++i) {
    s = s * 1664525u + 1013904223u;
    p[i] = (uint8_t)(s >> 24);
  }
}

int main(void) {
  const uint16_t preamble_len = 8;
  const uint8_t sfd[2] = {5,6};
  const size_t payload_len = 128;
  const size_t nsym = preamble_len + 2 + payload_len + 2; // preamble + SFD + payload + CRC bytes

  // Build one golden frame (symbols domain)
  uint32_t *syms = (uint32_t *)malloc(sizeof(uint32_t) * nsym);
  uint8_t *payload = (uint8_t *)malloc(payload_len);
  uint8_t *bytes = (uint8_t *)malloc(payload_len + 2);
  if (!syms || !payload || !bytes) return 2;
  gen_payload(payload, payload_len);

  // bytes: payload + CRC (packed from nibbles) then whiten in-place
  memcpy(bytes, payload, payload_len);
  bytes[payload_len] = 0; bytes[payload_len+1] = 0;
  uint8_t nibbles[4];
  lora_add_crc(bytes, payload_len + 2, nibbles);
  bytes[payload_len]   = (uint8_t)((nibbles[1] << 4) | nibbles[0]);
  bytes[payload_len+1] = (uint8_t)((nibbles[3] << 4) | nibbles[2]);
  lora_whiten(bytes, bytes, payload_len + 2);

  // Compose symbols: preamble zeros, SFD, then whitened bytes as symbols
  size_t pos = 0;
  for (uint16_t i = 0; i < preamble_len; ++i) syms[pos++] = 0;
  syms[pos++] = sfd[0];
  syms[pos++] = sfd[1];
  for (size_t i = 0; i < payload_len + 2; ++i) syms[pos++] = bytes[i];

  // Quick correctness pass via frame sync + dewhiten + CRC check
  size_t pre_end = lora_frame_sync_find_preamble(syms, nsym, preamble_len);
  size_t sfd_end = lora_frame_sync_find_sfd(syms, nsym, pre_end, 2, 4);
  size_t sym_off = (sfd_end < nsym) ? sfd_end : 0;
  if (sym_off != (size_t)(preamble_len + 2)) {
    fprintf(stderr, "frame sync offset mismatch: got %zu expected %u\n",
            sym_off, (unsigned)(preamble_len + 2));
    return 3;
  }
  size_t rem = nsym - sym_off;
  uint8_t *rx = (uint8_t *)malloc(rem);
  if (!rx) return 4;
  for (size_t i = 0; i < rem; ++i) rx[i] = (uint8_t)(syms[sym_off + i] & 0xFF);
  lora_dewhiten(rx, rx, rem);
  uint8_t save1 = rx[payload_len];
  uint8_t save2 = rx[payload_len+1];
  rx[payload_len] = 0; rx[payload_len+1] = 0;
  uint8_t out4[4];
  lora_add_crc(rx, payload_len + 2, out4);
  uint8_t c1 = (uint8_t)((out4[1] << 4) | out4[0]);
  uint8_t c2 = (uint8_t)((out4[3] << 4) | out4[2]);
  if (c1 != save1 || c2 != save2) {
    fprintf(stderr, "CRC check failed in sanity\n");
    return 5;
  }

  // Benchmark: repeated full-frame decode in symbol domain
  const int iters = 20000;
  size_t ok = 0;
  struct timespec t0, t1; double sec;
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int r = 0; r < iters; ++r) {
    size_t pe = lora_frame_sync_find_preamble(syms, nsym, preamble_len);
    size_t se = lora_frame_sync_find_sfd(syms, nsym, pe, 2, 4);
    size_t off = (se < nsym) ? se : 0;
    size_t nrem = nsym - off;
    // bytes view
    for (size_t i = 0; i < nrem; ++i) rx[i] = (uint8_t)(syms[off + i] & 0xFF);
    lora_dewhiten(rx, rx, nrem);
    uint8_t s1 = rx[payload_len];
    uint8_t s2 = rx[payload_len+1];
    rx[payload_len] = 0; rx[payload_len+1] = 0;
    lora_add_crc(rx, payload_len + 2, out4);
    uint8_t cc1 = (uint8_t)((out4[1] << 4) | out4[0]);
    uint8_t cc2 = (uint8_t)((out4[3] << 4) | out4[2]);
    ok += (cc1 == s1 && cc2 == s2);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  double Mfps = (double)iters / 1e6 / sec;

  puts("metric,value");
  printf("full_frame_pipeline_Mfps,%.3f\n", Mfps);
  printf("full_frame_ok,%zu\n", ok);

  free(rx); free(bytes); free(payload); free(syms);
  return 0;
}

