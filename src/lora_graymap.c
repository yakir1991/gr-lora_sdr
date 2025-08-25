#include "lora_graymap.h"

void lora_gray_map(const uint32_t *in, uint32_t *out,
                   uint8_t sf, size_t len)
{
    uint32_t mask = (1u << sf) - 1u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t v = in[i] & mask;
        out[i] = (v ^ (v >> 1)) & mask;
    }
}

void lora_gray_demap(const uint32_t *in, uint32_t *out,
                     uint8_t sf, size_t len)
{
    uint32_t mask = (1u << sf) - 1u;
    for (size_t i = 0; i < len; ++i) {
        uint32_t v = in[i] & mask;
        uint32_t res = v;
        for (uint8_t j = 1; j < sf; ++j) {
            res ^= (v >> j);
        }
        out[i] = (res + 1) & mask;
    }
}
