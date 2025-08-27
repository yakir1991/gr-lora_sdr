#include "lora_interleaver.h"
#include "lora_interleaver_tables.h"

void lora_interleave(const uint8_t *restrict in, uint32_t *restrict out,
                     uint8_t sf, uint8_t sf_app, uint8_t cw_len,
                     bool add_parity)
{
    const uint8_t (*perm)[12] = LORA_INTERLEAVE_PERM[sf - 7][sf_app - 1];
    for (uint8_t i = 0; i < cw_len; ++i) {
        uint32_t sym = 0;
        uint8_t parity = 0;
        for (uint8_t j = 0; j < sf; ++j) {
            uint8_t b;
            if (j < sf_app) {
                uint8_t idx = perm[i][j];
                b = (uint8_t)((in[idx] >> (cw_len - 1u - i)) & 1u);
                parity ^= b;
            } else if (add_parity && j == sf_app) {
                b = parity;
            } else {
                b = 0u;
            }
            sym = (sym << 1) | (uint32_t)b;
        }
        out[i] = sym;
    }
}

void lora_deinterleave(const uint32_t *in, uint8_t *out,
                       uint8_t sf, uint8_t sf_app, uint8_t cw_len)
{
    const uint8_t (*perm)[12] = LORA_INTERLEAVE_PERM[sf - 7][sf_app - 1];
    /* Accumulate codewords directly, MSB-first across i */
    for (uint8_t k = 0; k < sf_app; ++k) out[k] = 0u;
    for (uint8_t i = 0; i < cw_len; ++i) {
        uint32_t sym = in[i];
        for (uint8_t j = 0; j < sf; ++j) {
            if (j >= sf_app) break; /* ignore padding/parity columns here */
            uint8_t b = (uint8_t)((sym >> (sf - 1u - j)) & 1u);
            uint8_t idx = perm[i][j];
            out[idx] = (uint8_t)((out[idx] << 1) | b);
        }
    }
}
