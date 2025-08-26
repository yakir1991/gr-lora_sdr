#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "lora_log.h"
#include "lora_whitening.h"

static uint32_t seed = 0;
static uint32_t lcg_rand(void)
{
    seed = 1664525u * seed + 1013904223u;
    return seed;
}

int main(void)
{
    const size_t len = 32;
    uint8_t input[32];
    uint8_t output[32];

    for (size_t i = 0; i < len; ++i) {
        input[i] = (uint8_t)(lcg_rand() & 0xFFu);
    }

    lora_whiten(input, output, len);

    static const uint8_t golden[32] = {
        160, 204, 21, 204, 243, 103, 239, 45,
        236, 13, 158, 2, 183, 150, 132, 179,
        169, 143, 99, 176, 123, 134, 29, 184,
        119, 235, 67, 168, 19, 175, 38, 231
    };

    if (memcmp(output, golden, len) != 0) {
        LORA_LOG_INFO("Whitening golden test failed");
        return 1;
    }

    LORA_LOG_INFO("Whitening golden test passed");
    return 0;
}
