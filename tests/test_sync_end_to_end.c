#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <stdlib.h>
#include "lora_chain.h"
#include "lora_config.h"
#include "lora_mod.h"

static void apply_cfo(float complex *chips, size_t n, float fs, float cfo_hz)
{
    if (cfo_hz == 0.0f) return;
    double dphi = 2.0 * M_PI * (double)cfo_hz / (double)fs;
    float complex ph = 1.0f + I * 0.0f;
    float complex step = cexpf(I * (float)dphi);
    for (size_t i = 0; i < n; ++i) {
        chips[i] *= ph;
        ph *= step;
    }
}

static inline float randu(void) { return (float)rand() / (float)RAND_MAX; }

static float randn(void)
{
    float u1 = randu();
    float u2 = randu();
    if (u1 < 1e-9f) u1 = 1e-9f;
    float r = sqrtf(-2.0f * logf(u1));
    float t = 2.0f * (float)M_PI * u2;
    return r * cosf(t);
}

static void add_awgn(float complex *chips, size_t n, float snr_db)
{
    if (!isfinite(snr_db)) return;
    float snr_lin = powf(10.0f, snr_db / 10.0f);
    float sigma2 = 1.0f / snr_lin; /* assume unit signal power */
    float sigma = sqrtf(0.5f * sigma2); /* per component */
    for (size_t i = 0; i < n; ++i) {
        float nr = sigma * randn();
        float ni = sigma * randn();
        chips[i] += nr + I * ni;
    }
}

static int run_case(const lora_chain_cfg *cfg,
                    const uint32_t *preamble_syms, uint32_t preamble_len,
                    const uint32_t *sfd_syms, uint32_t sfd_len,
                    const uint8_t *payload, size_t payload_len,
                    float cfo_hz, float snr_db,
                    int pre_garbage_syms)
{
    const uint8_t sf = cfg->sf;
    const uint32_t fs = cfg->samp_rate;
    const uint32_t bw = cfg->bw;
    const uint32_t os = fs / bw;
    const uint32_t n_bins = 1u << sf;
    const uint32_t sps = n_bins * os;

    /* Build header: garbage + preamble + SFD */
    uint32_t total_hdr_sym = (uint32_t)pre_garbage_syms + preamble_len + sfd_len;
    uint32_t hdr_syms[64];
    uint32_t idx = 0;
    for (int g = 0; g < pre_garbage_syms; ++g) hdr_syms[idx++] = (uint32_t)(3 + (g % 5));
    for (uint32_t i = 0; i < preamble_len; ++i) hdr_syms[idx++] = preamble_syms[i];
    for (uint32_t i = 0; i < sfd_len; ++i) hdr_syms[idx++] = sfd_syms[i];

    float complex hdr_chips[64 * LORA_MAX_SPS];
    memset(hdr_chips, 0, sizeof(hdr_chips));
    lora_modulate(hdr_syms, hdr_chips, sf, fs, bw, total_hdr_sym);

    /* Payload chips via TX chain */
    static float complex payload_chips[LORA_MAX_CHIPS];
    static lora_tx_workspace tx_ws;
    size_t payload_nchips = 0;
    lora_status tx_ret = lora_tx_chain(payload, payload_len, payload_chips, LORA_MAX_CHIPS, &payload_nchips, cfg, &tx_ws);
    if (tx_ret != LORA_OK) {
        fprintf(stderr, "tx_chain failed %d\n", (int)tx_ret);
        return 1;
    }

    size_t hdr_nchips = (size_t)total_hdr_sym * sps;
    static float complex chips[LORA_MAX_CHIPS];
    size_t nchips = hdr_nchips + payload_nchips;
    if (nchips > LORA_MAX_CHIPS) {
        fprintf(stderr, "not enough buffer for chips\n");
        return 1;
    }
    memcpy(chips, hdr_chips, hdr_nchips * sizeof(float complex));
    memcpy(chips + hdr_nchips, payload_chips, payload_nchips * sizeof(float complex));

    /* Impairments */
    apply_cfo(chips, nchips, (float)fs, cfo_hz);
    add_awgn(chips, nchips, snr_db);

    /* RX chain */
    static lora_rx_workspace rx_ws;
    uint8_t out[LORA_MAX_PAYLOAD_LEN];
    size_t out_len = 0;
    if (!rx_ws.fft_ws) {
        size_t need = lora_rx_fft_workspace_bytes(cfg);
        size_t need_al = (need + 31u) & ~((size_t)31u);
        void *p = NULL;
#if defined(_POSIX_C_SOURCE)
        if (posix_memalign(&p, 32, need_al) != 0) p = NULL;
#else
        p = aligned_alloc(32, need_al);
#endif
        rx_ws.fft_ws = p;
        rx_ws.fft_ws_size = rx_ws.fft_ws ? need_al : 0;
        rx_ws.fft_inited = 0;
    }
    lora_status rx_ret = lora_rx_chain(chips, nchips, out, sizeof(out), &out_len, cfg, &rx_ws);
    if (rx_ret != LORA_OK) {
        fprintf(stderr, "rx_chain failed %d\n", (int)rx_ret);
        return 1;
    }
    if (out_len != payload_len || memcmp(out, payload, payload_len) != 0) {
        fprintf(stderr, "Recovered payload mismatch (len %zu)\n", out_len);
        return 1;
    }
    return 0;
}

int main(void)
{
    srand(1);
    const lora_chain_cfg cfg = {.sf = 8, .bw = 125000, .samp_rate = 125000};
    const uint8_t payload[] = { 'H','E','L','L','O' };

    uint32_t pre1[8]; for (int i = 0; i < 8; ++i) pre1[i] = 0;
    uint32_t sfd[2] = {5,6};

    if (run_case(&cfg, pre1, 8, sfd, 2, payload, sizeof(payload), 120.0f, INFINITY, 0)) return 1;
    if (run_case(&cfg, pre1, 8, sfd, 2, payload, sizeof(payload), -80.0f, INFINITY, 0)) return 1;
    if (run_case(&cfg, pre1, 8, sfd, 2, payload, sizeof(payload), 100.0f, 12.0f, 0)) return 1;
    uint32_t pre2[8] = {0,0,0,2,0,0,0,0};
    if (run_case(&cfg, pre2, 8, sfd, 2, payload, sizeof(payload), 60.0f, INFINITY, 3)) return 1;

    printf("Sync + CFO + Align test passed\n");
    return 0;
}
