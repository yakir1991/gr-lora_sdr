#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <time.h>
#include <malloc.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "lora_chain.h"
#include "lora_config.h"

#define ITERATIONS 100

static uint64_t now_ns(void)
{
#ifdef _WIN32
    LARGE_INTEGER freq, t;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t);
    return (uint64_t)(t.QuadPart * 1000000000ULL / freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
#endif
}

int main(void)
{
    const uint8_t payload[] = { 'A', 'B', 'C' };
    static float complex chips[LORA_MAX_CHIPS];
    static uint8_t out[LORA_MAX_PAYLOAD_LEN];

    FILE *csv = fopen("bench_results.csv", "w");
    if (!csv)
    {
        perror("bench_results.csv");
        return 1;
    }
    fprintf(csv, "cycles,bytes_allocated,packets_per_sec\n");

    size_t peak_bytes = 0;
    uint64_t start = now_ns();
    for (int i = 0; i < ITERATIONS; ++i)
    {
        size_t nchips = 0, out_len = 0;
        if (lora_tx_chain(payload, sizeof payload, chips, LORA_MAX_CHIPS, &nchips) != 0)
        {
            fprintf(stderr, "TX chain failed\n");
            return 1;
        }
        if (lora_rx_chain(chips, nchips, out, sizeof out, &out_len) != 0)
        {
            fprintf(stderr, "RX chain failed\n");
            return 1;
        }
#ifdef _WIN32
        size_t used = 0; /* _msize requires a pointer; omit for now */
#else
#if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 33))
        struct mallinfo2 mi = mallinfo2();
#else
        struct mallinfo mi = mallinfo();
#endif
        size_t used = mi.uordblks;
#endif
        if (used > peak_bytes)
            peak_bytes = used;
    }
    uint64_t end = now_ns();

    uint64_t cycles = end - start;
    double pps = (double)ITERATIONS * 1e9 / (double)cycles;
    fprintf(csv, "%llu,%zu,%.3f\n", (unsigned long long)cycles, peak_bytes, pps);
    fclose(csv);

    printf("cycles=%llu, peak_bytes=%zu, packets_per_sec=%.3f\n",
           (unsigned long long)cycles, peak_bytes, pps);
    return 0;
}
