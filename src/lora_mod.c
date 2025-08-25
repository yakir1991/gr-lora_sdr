#include "lora_mod.h"
#include "lora_utils.h"
#include "lora_config.h"
#include <math.h>
#include <string.h>

void lora_modulate(const uint32_t *symbols, float complex *chips,
                   uint8_t sf, uint32_t samp_rate, uint32_t bw,
                   size_t nsym)
{
    uint32_t n_bins = 1u << sf;
    uint32_t os_factor = samp_rate / bw;
    uint32_t sps = n_bins * os_factor;

    if (sps > LORA_MAX_SPS)
        return;

    float complex tmp[LORA_MAX_SPS];

    for (size_t s = 0; s < nsym; ++s) {
        uint32_t sym = symbols[s] & (n_bins - 1u);
        lora_build_upchirp(tmp, sym, sf, os_factor);
        memcpy(&chips[s * sps], tmp, sps * sizeof(float complex));
    }
}
