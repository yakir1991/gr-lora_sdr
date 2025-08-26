#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "lora_interleaver.h"

#define ITERATIONS 1000000

static double elapsed(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(void)
{
    const uint8_t cw_len = 8;
    for (uint8_t sf = 7; sf <= 12; ++sf) {
        uint8_t sf_app = sf;
        uint8_t input[16];
        uint32_t inter[cw_len];
        uint8_t output[16];

        for (uint8_t i = 0; i < sf_app; ++i)
            input[i] = i * 3 + 1;

        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        for (uint32_t k = 0; k < ITERATIONS; ++k)
            lora_interleave(input, inter, sf, sf_app, cw_len, false);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        double inter_time = elapsed(t0, t1);

        clock_gettime(CLOCK_MONOTONIC, &t0);
        for (uint32_t k = 0; k < ITERATIONS; ++k)
            lora_deinterleave(inter, output, sf, sf_app, cw_len);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        double deinter_time = elapsed(t0, t1);

        printf("sf %u: interleave %f s, deinterleave %f s\n", sf, inter_time, deinter_time);
    }
    return 0;
}
