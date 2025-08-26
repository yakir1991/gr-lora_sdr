#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <time.h>
#include <malloc.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
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
        char cwd[256];
        if (getcwd(cwd, sizeof cwd))
            fprintf(stderr, "cwd: %s\n", cwd);
        return EXIT_FAILURE;
    }
    fprintf(csv, "cycles,bytes_allocated,packets_per_sec\n");

    size_t peak_bytes = 0;
    uint64_t start = now_ns();
    for (int i = 0; i < ITERATIONS; ++i)
    {
        size_t nchips = 0, out_len = 0;
        int tx_ret = lora_tx_chain(payload, sizeof payload, chips, LORA_MAX_CHIPS, &nchips);
        if (tx_ret)
        {
            fprintf(stderr,
                    "Iteration %d: lora_tx_chain failed (ret=%d, nchips=%zu, out_len=%zu)\n",
                    i, tx_ret, nchips, out_len);
            return EXIT_FAILURE;
        }
        if (nchips == 0 || nchips > LORA_MAX_CHIPS)
        {
            fprintf(stderr, "Iteration %d: unexpected chip count %zu\n", i, nchips);
            return EXIT_FAILURE;
        }

        int rx_ret = lora_rx_chain(chips, nchips, out, sizeof out, &out_len);
        if (rx_ret)
        {
            fprintf(stderr, "Iteration %d: lora_rx_chain failed (%d, nchips=%zu, out_len=%zu)\n", i, rx_ret, nchips, out_len);
            return EXIT_FAILURE;
        }
        if (out_len != sizeof payload)
        {
            fprintf(stderr, "Iteration %d: unexpected output length %zu\n", i, out_len);
            return EXIT_FAILURE;
        }

#ifdef _WIN32
        size_t used = 0; /* _msize requires a pointer; omit for now */
        if (i == 0)
            fprintf(stderr, "Warning: memory usage metrics unavailable on this platform\n");
#else
        size_t used = 0;
#if defined(__GLIBC__) && __GLIBC_PREREQ(2, 33)
        struct mallinfo2 mi = mallinfo2();
        used = mi.uordblks;
#else
        if (i == 0)
            fprintf(stderr, "Warning: memory usage metrics unavailable on this platform\n");
#endif
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
    return EXIT_SUCCESS;
}
