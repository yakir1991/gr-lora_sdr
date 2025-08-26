#include "lora_interleaver.h"

static void int_to_bits(uint32_t value, uint8_t *bits, uint8_t n_bits)
{
    for (uint8_t i = 0; i < n_bits; ++i) {
        bits[n_bits - 1 - i] = (value >> i) & 1u;
    }
}

static uint32_t bits_to_int(const uint8_t *bits, uint8_t n_bits)
{
    uint32_t val = 0;
    for (uint8_t i = 0; i < n_bits; ++i) {
        val = (val << 1) | (bits[i] & 1u);
    }
    return val;
}

void lora_interleave(const uint8_t *restrict in, uint32_t *restrict out,
                     uint8_t sf, uint8_t sf_app, uint8_t cw_len,
                     bool add_parity)
{
    uint8_t cw_bin[sf_app][cw_len];
    for (uint8_t i = 0; i < sf_app; ++i) {
        int_to_bits(in[i], cw_bin[i], cw_len);
    }

    uint8_t inter_bin[cw_len][sf];
    if (add_parity && sf_app < sf) {
        for (uint8_t i = 0; i < cw_len; ++i) {
            for (uint8_t j = 0; j < sf_app; ++j) {
                uint8_t idx = (i + sf_app - j - 1) % sf_app;
                inter_bin[i][j] = cw_bin[idx][i];
            }
            for (uint8_t j = sf_app; j < sf; ++j) {
                inter_bin[i][j] = 0;
            }
            uint8_t parity = 0;
            for (uint8_t j = 0; j < sf_app; ++j)
                parity ^= inter_bin[i][j];
            inter_bin[i][sf_app] = parity;
            out[i] = bits_to_int(inter_bin[i], sf);
        }
    } else {
        for (uint8_t i = 0; i < cw_len; ++i) {
            for (uint8_t j = 0; j < sf_app; ++j) {
                uint8_t idx = (i + sf_app - j - 1) % sf_app;
                inter_bin[i][j] = cw_bin[idx][i];
            }
            for (uint8_t j = sf_app; j < sf; ++j) {
                inter_bin[i][j] = 0;
            }
            out[i] = bits_to_int(inter_bin[i], sf);
        }
    }
}

void lora_deinterleave(const uint32_t *in, uint8_t *out,
                       uint8_t sf, uint8_t sf_app, uint8_t cw_len)
{
    uint8_t inter_bin[cw_len][sf];
    for (uint8_t i = 0; i < cw_len; ++i) {
        int_to_bits(in[i], inter_bin[i], sf);
    }

    uint8_t deinter_bin[sf_app][cw_len];
    for (uint8_t i = 0; i < sf_app; ++i)
        for (uint8_t j = 0; j < cw_len; ++j)
            deinter_bin[i][j] = 0;

    for (uint8_t i = 0; i < cw_len; ++i) {
        for (uint8_t j = 0; j < sf_app; ++j) {
            uint8_t idx = (i + sf_app - j - 1) % sf_app;
            deinter_bin[idx][i] = inter_bin[i][j];
        }
    }

    for (uint8_t i = 0; i < sf_app; ++i) {
        out[i] = (uint8_t)bits_to_int(deinter_bin[i], cw_len);
    }
}

