#include "lora_chain.h"
#include "lora_config.h"
#include "lora_io.h"
#include "lora_whitening.h"
#include "lora_add_crc.h"
#include <string.h>

#ifdef LORA_LITE_FIXED_POINT
#include "lora_fft_demod.h"
#include "lora_fixed.h"
#endif

int lora_rx_chain(const float complex *restrict chips, size_t nchips,
                  uint8_t *restrict payload, size_t payload_buf_len,
                  size_t *restrict payload_len_out,
                  const lora_chain_cfg *cfg)
{
    if (!chips || !payload || !payload_len_out || payload_buf_len == 0 || !cfg)
        return -1;

    const uint8_t sf = cfg->sf;
    const uint32_t bw = cfg->bw;
    const uint32_t samp_rate = cfg->samp_rate;
    uint32_t sps = (1u << sf) * (samp_rate / bw);
    size_t nsym = nchips / sps;
    if (nsym > LORA_MAX_NSYM)
        return -1;

    uint32_t symbols[LORA_MAX_NSYM];

#ifdef LORA_LITE_FIXED_POINT
    lora_q15_complex qchips[LORA_MAX_CHIPS];
    const float q15_scale = 32767.0f;
    for (size_t i = 0; i < nchips && i < LORA_MAX_CHIPS; ++i) {
        float re = crealf(chips[i]);
        if (re > 0.999969f)
            re = 0.999969f;
        if (re < -1.0f)
            re = -1.0f;
        float im = cimagf(chips[i]);
        if (im > 0.999969f)
            im = 0.999969f;
        if (im < -1.0f)
            im = -1.0f;
        qchips[i].r = (int16_t)(re * q15_scale + (re >= 0 ? 0.5f : -0.5f));
        qchips[i].i = (int16_t)(im * q15_scale + (im >= 0 ? 0.5f : -0.5f));
    }
    lora_fft_demod(qchips, symbols, sf, samp_rate, bw, 0.0f, nsym);
#else
#error "lora_rx_chain requires LORA_LITE_FIXED_POINT"
#endif

    uint8_t whitened[LORA_MAX_NSYM];
    for (size_t i = 0; i < nsym; ++i)
        whitened[i] = (uint8_t)(symbols[i] & 0xFF);

    uint8_t payload_crc[LORA_MAX_NSYM];
    lora_dewhiten(whitened, payload_crc, nsym);

    if (nsym < 2)
        return -1;
    size_t payload_len = nsym - 2;
    if (payload_len > payload_buf_len)
        return -1;
    uint8_t crc1 = payload_crc[payload_len];
    uint8_t crc2 = payload_crc[payload_len + 1];

    uint8_t tmp[LORA_MAX_NSYM];
    memcpy(tmp, payload_crc, nsym);
    tmp[payload_len] = 0;
    tmp[payload_len + 1] = 0;
    uint8_t crc_n[4];
    lora_add_crc(tmp, nsym, crc_n);
    uint8_t calc_crc1 = (uint8_t)((crc_n[1] << 4) | crc_n[0]);
    uint8_t calc_crc2 = (uint8_t)((crc_n[3] << 4) | crc_n[2]);
    if (crc1 != calc_crc1 || crc2 != calc_crc2)
        return -1;

    memcpy(payload, payload_crc, payload_len);
    *payload_len_out = payload_len;
    return 0;
}

int lora_rx_run(lora_io_t *in, lora_io_t *out, const lora_chain_cfg *cfg)
{
    if (!in || !out || !cfg)
        return -1;

    float complex chips[LORA_MAX_CHIPS];
    size_t total = 0;
    size_t max_bytes = LORA_MAX_CHIPS * sizeof(float complex);
    uint8_t *chip_bytes = (uint8_t *)chips;
    while (total < max_bytes) {
        size_t n = in->read(in->ctx, chip_bytes + total, max_bytes - total);
        if (n == 0)
            break;
        total += n;
    }
    size_t nchips = total / sizeof(float complex);

    uint8_t payload[LORA_MAX_PAYLOAD_LEN];
    size_t payload_len;
    if (lora_rx_chain(chips, nchips, payload, sizeof(payload), &payload_len, cfg) != 0)
        return -1;

    size_t wr = out->write(out->ctx, payload, payload_len);
    return (wr == payload_len) ? 0 : -1;
}

