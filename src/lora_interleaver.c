#include "lora_interleaver.h"
#include "lora_interleaver_tables.h"

void lora_interleave(const uint8_t *restrict in, uint32_t *restrict out,
                     uint8_t sf, uint8_t sf_app, uint8_t cw_len,
                     bool add_parity)
{
    const uint8_t (*perm)[12] = LORA_INTERLEAVE_PERM[sf - 7][sf_app - 1];
    const uint8_t shift_data = (uint8_t)(sf - sf_app);
    const uint8_t parity_pos = (uint8_t)(sf - 1u - sf_app);
    for (uint8_t i = 0; i < cw_len; ++i) {
        /* Build data bits (top sf_app bits, MSB-first) and parity */
        uint32_t data = 0u;
        uint8_t parity = 0u;
        for (uint8_t j = 0; j < sf_app; ++j) {
            const uint8_t idx = perm[i][j];
            const uint8_t b = (uint8_t)((in[idx] >> (cw_len - 1u - i)) & 1u);
            parity ^= b;
            data = (data << 1) | (uint32_t)b;
        }
        /* Place data at top bits and (optionally) insert parity column */
        uint32_t sym = (data << shift_data);
        if (add_parity) {
            sym |= ((uint32_t)parity << parity_pos);
        }
        out[i] = sym;
    }
}

void lora_deinterleave(const uint32_t *restrict in, uint8_t *restrict out,
                       uint8_t sf, uint8_t sf_app, uint8_t cw_len)
{
    const uint8_t (*perm)[12] = LORA_INTERLEAVE_PERM[sf - 7][sf_app - 1];
    /* Accumulate codewords directly, MSB-first across i */
    for (uint8_t k = 0; k < sf_app; ++k) out[k] = 0u;
    for (uint8_t i = 0; i < cw_len; ++i) {
        const uint32_t sym = in[i];
        for (uint8_t j = 0; j < sf_app; ++j) {
            const uint8_t b = (uint8_t)((sym >> (sf - 1u - j)) & 1u);
            const uint8_t idx = perm[i][j];
            out[idx] = (uint8_t)((out[idx] << 1) | b);
        }
    }
}
