#include "lora_chain.h"
#include "lora_config.h"
#include "lora_io.h"
#include "lora_whitening.h"
#include "lora_add_crc.h"
#include <string.h>
#include <stdlib.h>

#include "lora_fft_demod.h"
#ifdef LORA_LITE_FIXED_POINT
#include "lora_fixed.h"
#if defined(__ARM_NEON)
#include <arm_neon.h>
#endif
#endif

lora_status lora_rx_chain(const float complex *restrict chips, size_t nchips,
                          uint8_t *restrict payload, size_t payload_buf_len,
                          size_t *restrict payload_len_out,
                          const lora_chain_cfg *cfg,
                          lora_rx_workspace *ws)
{
    if (!chips || !payload || !payload_len_out || payload_buf_len == 0 || !cfg || !ws)
        return LORA_ERR_INVALID_ARG;

    const uint8_t sf = cfg->sf;
    const uint32_t bw = cfg->bw;
    const uint32_t samp_rate = cfg->samp_rate;
    uint32_t sps = (1u << sf) * (samp_rate / bw);
    size_t nsym = nchips / sps;
    if (nsym > LORA_MAX_NSYM)
        return LORA_ERR_TOO_MANY_SYMBOLS;

    uint32_t *symbols = ws->symbols;

#ifdef LORA_LITE_FIXED_POINT
    lora_q15_complex *qchips = ws->qchips;
    size_t nlim = nchips < LORA_MAX_CHIPS ? nchips : LORA_MAX_CHIPS;

    /* Convert float complex [-1,1) to Q15 interleaved pairs. NEON path when available. */
    const float *fin = (const float *)chips; /* interleaved re,im */
#if defined(__ARM_NEON)
    const float32x4_t vmax = vdupq_n_f32(0.999969f);
    const float32x4_t vmin = vdupq_n_f32(-1.0f);
    const float32x4_t vscale = vdupq_n_f32(32767.0f);
    size_t i = 0;
    for (; i + 4 <= nlim; i += 4) {
        /* load 4 complex = 8 floats */
        float32x4x2_t deint = vld2q_f32(fin + i * 2);
        float32x4_t re = vmaxq_f32(vminq_f32(deint.val[0], vmax), vmin);
        float32x4_t im = vmaxq_f32(vminq_f32(deint.val[1], vmax), vmin);
        int32x4_t re_i32 = vcvtq_s32_f32(vmulq_f32(re, vscale));
        int32x4_t im_i32 = vcvtq_s32_f32(vmulq_f32(im, vscale));
        int16x4_t re_i16 = vqmovn_s32(re_i32);
        int16x4_t im_i16 = vqmovn_s32(im_i32);
        int16x4x2_t inter;
        inter.val[0] = re_i16;
        inter.val[1] = im_i16;
        vst2_s16((int16_t *)&qchips[i], inter);
    }
    for (; i < nlim; ++i) {
        float re = fin[2*i+0];
        float im = fin[2*i+1];
        if (re > 0.999969f) re = 0.999969f; if (re < -1.0f) re = -1.0f;
        if (im > 0.999969f) im = 0.999969f; if (im < -1.0f) im = -1.0f;
        qchips[i].r = (int16_t)(re * 32767.0f + (re >= 0 ? 0.5f : -0.5f));
        qchips[i].i = (int16_t)(im * 32767.0f + (im >= 0 ? 0.5f : -0.5f));
    }
#else
    const float q15_scale = 32767.0f;
    for (size_t i = 0; i < nlim; ++i) {
        float re = fin[2*i+0];
        if (re > 0.999969f) re = 0.999969f; if (re < -1.0f) re = -1.0f;
        float im = fin[2*i+1];
        if (im > 0.999969f) im = 0.999969f; if (im < -1.0f) im = -1.0f;
        qchips[i].r = (int16_t)(re * q15_scale + (re >= 0 ? 0.5f : -0.5f));
        qchips[i].i = (int16_t)(im * q15_scale + (im >= 0 ? 0.5f : -0.5f));
    }
#endif
    const lora_q15_complex *chip_ptr = qchips;
#else
    const float complex *chip_ptr = chips;
#endif

    size_t ws_bytes = lora_fft_workspace_bytes(sf, samp_rate, bw);
    if (ws_bytes == 0)
        return LORA_ERR_UNSUPPORTED;
    /* Reuse persistent FFT demod workspace/context in ws */
    int need_reinit = 0;
    if (!ws->fft_ws || ws->fft_ws_size != ws_bytes || ws->fft_sf != sf ||
        ws->fft_fs != samp_rate || ws->fft_bw != bw) {
        if (ws->fft_ws) free(ws->fft_ws);
        ws->fft_ws = aligned_alloc(32, ws_bytes);
        if (!ws->fft_ws)
            return LORA_ERR_OOM;
        ws->fft_ws_size = ws_bytes;
        ws->fft_sf = sf;
        ws->fft_fs = samp_rate;
        ws->fft_bw = bw;
        need_reinit = 1;
    }
    if (need_reinit) {
        if (ws->fft_ctx) {
            lora_fft_demod_free(ws->fft_ctx);
            free(ws->fft_ctx);
        }
        ws->fft_ctx = (void *)malloc(sizeof(*ws->fft_ctx));
        if (!ws->fft_ctx)
            return LORA_ERR_OOM;
        if (lora_fft_demod_init(ws->fft_ctx, sf, samp_rate, bw, ws->fft_ws, ws_bytes) != 0)
            return LORA_ERR_IO;
        ws->fft_ctx->cfo = 0.0f;
        ws->fft_ctx->cfo_phase = 0.0;
    }
    lora_fft_demod(ws->fft_ctx, chip_ptr, nsym, symbols);

    uint8_t *whitened = ws->whitened;
    for (size_t i = 0; i < nsym; ++i)
        whitened[i] = (uint8_t)(symbols[i] & 0xFF);

    uint8_t *payload_crc = ws->payload_crc;
    lora_dewhiten(whitened, payload_crc, nsym);

    if (nsym < 2)
        return LORA_ERR_INVALID_ARG;
    size_t payload_len = nsym - 2;
    if (payload_len > payload_buf_len)
        return LORA_ERR_BUFFER_TOO_SMALL;
    uint8_t crc1 = payload_crc[payload_len];
    uint8_t crc2 = payload_crc[payload_len + 1];

    uint8_t *tmp = ws->tmp;
    memcpy(tmp, payload_crc, nsym);
    tmp[payload_len] = 0;
    tmp[payload_len + 1] = 0;
    uint8_t crc_n[4];
    lora_add_crc(tmp, nsym, crc_n);
    uint8_t calc_crc1 = (uint8_t)((crc_n[1] << 4) | crc_n[0]);
    uint8_t calc_crc2 = (uint8_t)((crc_n[3] << 4) | crc_n[2]);
    if (crc1 != calc_crc1 || crc2 != calc_crc2)
        return LORA_ERR_CRC_MISMATCH;

    memcpy(payload, payload_crc, payload_len);
    *payload_len_out = payload_len;
    return LORA_OK;
}

lora_status lora_rx_run(lora_io_t *in, lora_io_t *out, const lora_chain_cfg *cfg)
{
    if (!in || !out || !cfg)
        return LORA_ERR_INVALID_ARG;

    float complex *chips = malloc(sizeof(float complex) * LORA_MAX_CHIPS);
    if (!chips)
        return LORA_ERR_OOM;
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

    uint8_t *payload = malloc(LORA_MAX_PAYLOAD_LEN);
    if (!payload) {
        free(chips);
        return LORA_ERR_OOM;
    }
    size_t payload_len;
    static lora_rx_workspace ws;
    lora_status st = lora_rx_chain(chips, nchips, payload, LORA_MAX_PAYLOAD_LEN, &payload_len, cfg, &ws);
    if (st != LORA_OK) {
        free(chips);
        free(payload);
        return st;
    }

    size_t wr = out->write(out->ctx, payload, payload_len);
    free(chips);
    free(payload);
    return (wr == payload_len) ? LORA_OK : LORA_ERR_IO;
}
