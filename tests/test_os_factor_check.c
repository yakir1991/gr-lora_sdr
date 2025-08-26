#include "lora_fft_demod.h"
#include "lora_mod.h"
#include "lora_log.h"
#include <stdio.h>
#include <string.h>
#include <complex.h>

int main(void) {
    uint8_t sf = 7;
    uint32_t fs = 1000000; // sample rate
    uint32_t bw = 123456;  // not a divisor of fs

    size_t ws = lora_fft_workspace_bytes(sf, fs, bw);
    if (ws != 0) {
        LORA_LOG_INFO("Workspace check failed");
        return 1;
    }

    unsigned char buf[1024];
    lora_fft_demod_ctx_t ctx;
    if (lora_fft_demod_init(&ctx, sf, fs, bw, buf, sizeof(buf)) != -1) {
        LORA_LOG_INFO("Init check failed");
        return 1;
    }

    float complex chip = 1.0f + 1.0f*I;
    uint32_t sym = 0;
    lora_modulate(&sym, &chip, sf, fs, bw, 1);
    if (crealf(chip) != 1.0f || cimagf(chip) != 1.0f) {
        LORA_LOG_INFO("Modulator check failed");
        return 1;
    }

    LORA_LOG_INFO("OS factor check test passed");
    return 0;
}
