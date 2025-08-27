#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "lora_add_crc.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

int main(void) {
  const size_t len = 512 + 2; /* include room for CRC bytes (zeros) */
  uint8_t *buf = (uint8_t *)malloc(len);
  if (!buf) return 2;
  for (size_t i = 0; i < len - 2; ++i) buf[i] = (uint8_t)(i * 13u + 5u);
  buf[len-2] = buf[len-1] = 0;

  uint8_t out[4];
  const int iters = 200000; /* ~100MB processed */
  struct timespec t0, t1;
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int r = 0; r < iters; ++r) lora_add_crc(buf, len, out);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  double sec = elapsed(t0, t1);
  double MBps = ((double)(len - 2) * iters) / (sec * 1e6);

  puts("metric,value");
  printf("crc_MBps,%.2f\n", MBps);
  printf("last_out,%u%u%u%u\n", out[0], out[1], out[2], out[3]);
  free(buf);
  return 0;
}

