// Regression test ensuring FFT demodulator matches expected symbols and
// reports workspace reduction.

#include <assert.h>
#include <complex.h>
#include <math.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lora_fft_demod.h"
#include "lora_fixed.h"
#include "lora_log.h"
#include "lora_mod.h"

static inline size_t align_up(size_t v, size_t a) {
    return (v + a - 1) & ~(a - 1);
}

/* Workspace size calculation from the legacy implementation which used buffers
 * sized for sps samples. */
static size_t legacy_workspace_bytes(uint8_t sf, uint32_t fs, uint32_t bw)
{
    uint32_t n_bins = 1u << sf;
    uint32_t os_factor = fs / bw;
    uint32_t sps = n_bins * os_factor;
    return sps * sizeof(float complex) * 3;
}

int main(void)
{
    const uint8_t sf = 7;
    const uint32_t bw = 125000;
    const uint32_t samp_rate = 500000; // os_factor = 4
    const size_t nsym = 4;
    const uint32_t symbols[4] = {0, 1, 2, 3};

    uint32_t sps = (1u << sf) * (samp_rate / bw);
    float complex chips_f[nsym * sps];
    lora_modulate(symbols, chips_f, sf, samp_rate, bw, nsym);
#ifdef LORA_LITE_FIXED_POINT
    lora_q15_complex chips[nsym * sps];
    for (uint32_t i = 0; i < nsym * sps; ++i)
        chips[i] = lora_float_to_q15(chips_f[i]);
    const lora_q15_complex *chips_in = chips;
#else
    const float complex *chips_in = chips_f;
#endif

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
    clock_t t0 = clock();
    lora_fft_demod(&ctx, chips_in, nsym, rec);
    clock_t t1 = clock();
    lora_fft_demod_free(&ctx);
    free(ws);

    for (size_t i = 0; i < nsym; ++i) {
        int diff = (int)rec[i] - (int)symbols[i];
        if (diff < 0)
            diff = -diff;
        assert(diff <= 1);
    }

    size_t ws_old = legacy_workspace_bytes(sf, samp_rate, bw);
    size_t ws_new = lora_fft_workspace_bytes(sf, samp_rate, bw);
    LORA_LOG_INFO("legacy workspace %zu bytes, new workspace %zu bytes", ws_old, ws_new);
    assert(ws_new < ws_old);

    double new_us = (double)(t1 - t0) * 1e6 / CLOCKS_PER_SEC;
    LORA_LOG_INFO("new demod %0.2f us", new_us);
    LORA_LOG_INFO("FFT demod regression test passed");
    return 0;
}

