#include <stdio.h>
#include <stdint.h>
#include <complex.h>
#include <stdlib.h>

#include "lora_log.h"
#include "lora_mod.h"
#include "lora_utils.h"
#include "lora_fft_demod.h"
#include "lora_config.h"
#include "lora_fixed.h"

#ifdef LORA_LITE_FIXED_POINT
typedef lora_q15_complex chip_t;
#define TO_CHIP(x) lora_float_to_q15((x))
#else
typedef float complex chip_t;
#define TO_CHIP(x) (x)
#endif

int main(int argc, char **argv) {
    const char *out_path = argc > 1 ? argv[1] : "out.bin";
    const uint8_t sf = 7;
    const uint32_t bw = 125000;
    const uint32_t samp_rate = 125000;
    const size_t nsym = 4;
    const uint32_t symbols[4] = {0, 1, 2, 3};

    float complex chips_f[nsym * (1u << sf)];
    lora_modulate(symbols, chips_f, sf, samp_rate, bw, nsym);
    chip_t chips[nsym * (1u << sf)];
    for (size_t i = 0; i < nsym * (1u << sf); ++i)
        chips[i] = TO_CHIP(chips_f[i]);

    uint32_t rec[4] = {0};
    size_t ws_bytes = lora_fft_workspace_bytes(sf, samp_rate, bw);
    void *ws = aligned_alloc(32, ws_bytes);
    if (!ws)
        return 1;
    lora_fft_demod_ctx_t ctx;
    if (lora_fft_demod_init(&ctx, sf, samp_rate, bw, ws, ws_bytes) != 0) {
        free(ws);
        return 1;
    }
    ctx.cfo = 0.0f;
    ctx.cfo_phase = 0.0;
    lora_fft_demod(&ctx, (const chip_t *)chips, nsym, rec);
    lora_fft_demod_free(&ctx);
    free(ws);
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
