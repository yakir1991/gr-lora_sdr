#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "lora_log.h"
#include "lora_graymap.h"

static uint32_t seed = 0;
static uint32_t lcg_rand(void)
{
    seed = 1664525u * seed + 1013904223u;
    return seed;
}

int main(void)
{
    const uint8_t sf = 7;
    const size_t len = 16;
    uint32_t input[16];
    uint32_t mapped[16];
    for (size_t i = 0; i < len; ++i) {
        input[i] = lcg_rand() & 0x7Fu;
    }
    lora_gray_map(input, mapped, sf, len);
    static const uint32_t map_golden[16] = {
        112, 43, 93, 46, 2, 5, 59, 60,
        84, 23, 41, 114, 14, 89, 79, 120
    };
    if (memcmp(mapped, map_golden, sizeof(map_golden)) != 0) {
        LORA_LOG_INFO("Gray map golden test failed");
        return 1;
    }
    uint32_t demapped[16];
    lora_gray_demap(map_golden, demapped, sf, len);
    static const uint32_t demap_golden[16] = {
        96, 51, 106, 53, 4, 7, 46, 41,
        104, 27, 50, 93, 12, 111, 118, 81
    };
    if (memcmp(demapped, demap_golden, sizeof(demap_golden)) != 0) {
        LORA_LOG_INFO("Gray demap golden test failed");
        return 1;
    }
    LORA_LOG_INFO("Gray golden test passed");
    return 0;
}
