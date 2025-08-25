#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "lora_log.h"
#include <complex.h>
#include "lora_mod.h"
#include "lora_fft_demod.h"

int main(void)
{
    const uint8_t sf = 7;
    const uint32_t bw = 125000;
    const uint32_t samp_rate = 125000;
    const size_t nsym = 4;
    const uint32_t symbols[4] = {0, 1, 2, 3};

    uint32_t sps = (1u << sf);
    float complex chips[nsym * (1u << sf)];

    lora_modulate(symbols, chips, sf, samp_rate, bw, nsym);

    uint32_t rec[4] = {0};
    lora_fft_demod(chips, rec, sf, samp_rate, bw, 0.0f, nsym);

    for (size_t i = 0; i < nsym; ++i) {
        assert(rec[i] == symbols[i]);
    }
    LORA_LOG_INFO("Mod/demod round-trip passed");
    return 0;
}
