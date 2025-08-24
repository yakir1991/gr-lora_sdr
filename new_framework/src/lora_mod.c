#include "lora_mod.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static void build_upchirp(float complex *chirp, uint32_t id,
                          uint8_t sf, uint32_t os_factor)
{
    double N = (double)(1u << sf);
    uint32_t sps = (uint32_t)(N * os_factor);
    int n_fold = (int)(sps - id * os_factor);
    for (uint32_t n = 0; n < sps; ++n) {
        double phase;
        if ((int)n < n_fold)
            phase = 2.0 * M_PI * ((double)n * (double)n / (2.0 * N * os_factor * os_factor) +
                                  ((double)id / N - 0.5) * (double)n / os_factor);
        else
            phase = 2.0 * M_PI * ((double)n * (double)n / (2.0 * N * os_factor * os_factor) +
                                  ((double)id / N - 1.5) * (double)n / os_factor);
        chirp[n] = cexpf(I * (float)phase);
    }
}

void lora_modulate(const uint32_t *symbols, float complex *chips,
                   uint8_t sf, uint32_t samp_rate, uint32_t bw,
                   size_t nsym)
{
    uint32_t n_bins = 1u << sf;
    uint32_t os_factor = samp_rate / bw;
    uint32_t sps = n_bins * os_factor;

    float complex *tmp = (float complex *)malloc(sps * sizeof(float complex));
    if (!tmp)
        return;

    for (size_t s = 0; s < nsym; ++s) {
        uint32_t sym = symbols[s] & (n_bins - 1u);
        build_upchirp(tmp, sym, sf, os_factor);
        memcpy(&chips[s * sps], tmp, sps * sizeof(float complex));
    }
    free(tmp);
}
