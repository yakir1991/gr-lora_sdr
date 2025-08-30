// Benchmark: Payload ID decimal formatting (u8_to_dec vs snprintf)
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

// Local copy of the optimized writer (0..255)
static inline size_t u8_to_dec(uint8_t v, char *dst, size_t cap)
{
  if (cap == 0) return 0;
  if (v < 10) {
    dst[0] = (char)('0' + v);
    return 1;
  } else if (v < 100) {
    if (cap < 2) return 0;
    uint8_t d10 = (uint8_t)(v / 10);
    uint8_t d1  = (uint8_t)(v - d10 * 10);
    dst[0] = (char)('0' + d10);
    dst[1] = (char)('0' + d1);
    return 2;
  } else {
    if (cap < 3) return 0;
    uint8_t r = (uint8_t)(v - (v >= 200 ? 200 : 100));
    dst[0] = (char)('1' + (v >= 200));
    dst[1] = (char)('0' + (r / 10));
    dst[2] = (char)('0' + (r % 10));
    return 3;
  }
}

int main(void) {
  const size_t iters = 2000000; // 2M iterations
  char buf[8];

  puts("metric,value");

  // u8_to_dec bench
  struct timespec t0, t1; double sec;
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (size_t i = 0; i < iters; ++i) {
    (void)u8_to_dec((uint8_t)(i & 0xFF), buf, sizeof buf);
    buf[0] ^= (char)(i & 1); // prevent optimizing away
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  printf("payload_id_u8_to_dec_Mops,%.2f\n", (double)iters / 1e6 / sec);

  // snprintf bench
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (size_t i = 0; i < iters; ++i) {
    (void)snprintf(buf, sizeof buf, "%u", (unsigned)(i & 0xFF));
    buf[0] ^= (char)(i & 1);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  printf("payload_id_snprintf_Mops,%.2f\n", (double)iters / 1e6 / sec);

  // Append-after micro-bench: append decimal after a constant prefix
  // Simulates typical pattern: "MSG:" + id
  char msg_after[32];
  memcpy(msg_after, "MSG:", 4);
  // u8_to_dec append-after
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (size_t i = 0; i < iters; ++i) {
    size_t n = u8_to_dec((uint8_t)(i & 0xFF), msg_after + 4, sizeof(msg_after) - 4);
    msg_after[4 + (n ? n : 0)] ^= (char)(i & 1);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  printf("payload_id_append_after_u8_to_dec_Mops,%.2f\n", (double)iters / 1e6 / sec);

  // snprintf append-after
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (size_t i = 0; i < iters; ++i) {
    (void)snprintf(msg_after + 4, sizeof(msg_after) - 4, "%u", (unsigned)(i & 0xFF));
    msg_after[4] ^= (char)(i & 1);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  printf("payload_id_append_after_snprintf_Mops,%.2f\n", (double)iters / 1e6 / sec);

  // Append-before micro-bench: write decimal first, then append message
  // Simulates pattern: id + ":" + base
  const char *base = ":HELLO";
  char msg_before[32];
  // u8_to_dec append-before
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (size_t i = 0; i < iters; ++i) {
    size_t n = u8_to_dec((uint8_t)(i & 0xFF), msg_before, sizeof(msg_before));
    size_t cap = sizeof(msg_before) - (n ? n : 0);
    if (cap > 0) {
      size_t to_copy = strlen(base);
      if (to_copy > cap) to_copy = cap;
      memcpy(msg_before + (n ? n : 0), base, to_copy);
      msg_before[0] ^= (char)(i & 1);
    }
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  printf("payload_id_append_before_u8_to_dec_Mops,%.2f\n", (double)iters / 1e6 / sec);

  // snprintf append-before
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (size_t i = 0; i < iters; ++i) {
    int n2 = snprintf(msg_before, sizeof(msg_before), "%u", (unsigned)(i & 0xFF));
    size_t off = (n2 > 0) ? (size_t)n2 : 0;
    size_t cap = (off < sizeof(msg_before)) ? (sizeof(msg_before) - off) : 0;
    if (cap > 0) {
      size_t to_copy = strlen(base);
      if (to_copy > cap) to_copy = cap;
      memcpy(msg_before + off, base, to_copy);
      msg_before[0] ^= (char)(i & 1);
    }
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  printf("payload_id_append_before_snprintf_Mops,%.2f\n", (double)iters / 1e6 / sec);

  return 0;
}
