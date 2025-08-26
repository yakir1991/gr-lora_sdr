#include <stdio.h>
#include <stdint.h>
#include <complex.h>
#include "lora_log.h"
#include "lora_mod.h"
#include "lora_fft_demod.h"
#include "lora_config.h"
#include "lora_fixed.h"

int main(int argc, char **argv) {
    const char *out_path = argc > 1 ? argv[1] : "out.bin";
    const uint8_t sf = 7;
    const uint32_t bw = 125000;
    const uint32_t samp_rate = 125000;
    const size_t nsym = 4;
    const uint32_t symbols[4] = {0, 1, 2, 3};

    float complex chips_f[nsym * (1u << sf)];
    lora_modulate(symbols, chips_f, sf, samp_rate, bw, nsym);
    lora_q15_complex chips[nsym * (1u << sf)];
    for (size_t i = 0; i < nsym * (1u << sf); ++i)
        chips[i] = lora_float_to_q15(chips_f[i]);

    uint32_t rec[4] = {0};
    lora_fft_demod(chips, rec, sf, samp_rate, bw, 0.0f, nsym);
    for (size_t i = 0; i < nsym; ++i) {
        if (rec[i] != symbols[i]) {
            LORA_LOG_ERR("Mismatch at %zu: %u != %u", i, rec[i], symbols[i]);
            return 1;
        }
    }

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        LORA_LOG_ERR("fopen");
        return 1;
    }
    size_t written = fwrite(rec, sizeof(uint32_t), nsym, f);
    fclose(f);
    if (written != nsym) {
        LORA_LOG_ERR("Short write");
        return 1;
    }
    return 0;
}
