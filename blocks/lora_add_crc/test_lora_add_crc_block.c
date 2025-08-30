#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "lora_add_crc.h"

static uint16_t crc16_ref(const uint8_t *payload, size_t length) {
  if (length < 2) return 0;
  size_t data_len = length - 2;
  uint16_t crc = 0x0000; /* legacy init */
  for (size_t i = 0; i < data_len; ++i) {
    crc ^= ((uint16_t)payload[i]) << 8;
    for (int b = 0; b < 8; ++b)
      crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
  }
  /* legacy post-xor with trailing two bytes */
  crc ^= (uint16_t)payload[length - 1] ^ ((uint16_t)payload[length - 2] << 8);
  return crc;
}

int main(void) {
  const size_t N = 513; /* include 2 trailing bytes */
  uint8_t *buf = (uint8_t*)malloc(N);
  if (!buf) return 1;

  int fails = 0;
  /* Deterministic payloads */
  for (int v = 0; v < 4; ++v) {
    for (size_t i = 0; i < N - 2; ++i)
      buf[i] = (uint8_t)((i * 33u + v * 7u) & 0xFF);
    buf[N-2] = (uint8_t)(v * 11);
    buf[N-1] = (uint8_t)(v * 13);

    uint16_t exp = crc16_ref(buf, N);
    uint8_t nib[4];
    lora_add_crc(buf, N, nib);
    uint16_t got = (uint16_t)((nib[3] << 12) | (nib[2] << 8) | (nib[1] << 4) | nib[0]);
    if (exp != got) {
      fprintf(stderr, "Mismatch v=%d exp=0x%04x got=0x%04x\n", v, exp, got);
      fails++;
    }
  }

  puts("metric,value");
  printf("tests,4\n");
  printf("mismatches,%d\n", fails);
  free(buf);
  if (fails == 0) {
    puts("CRC block test passed");
    return 0;
  }
  return 2;
}

