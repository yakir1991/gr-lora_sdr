#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include "lora_log.h"
#include "lora_chain.h"
#include "lora_config.h"

int main(void)
{
    const uint8_t payload[] = { 'A', 'B', 'C' };
    static float complex chips[LORA_MAX_CHIPS];
    static lora_tx_workspace tx_ws;
    size_t nchips = 0;
    const lora_chain_cfg cfg = {.sf = 8, .bw = 125000, .samp_rate = 125000};
    lora_status tx_ret = lora_tx_chain(payload, sizeof(payload), chips, LORA_MAX_CHIPS, &nchips, &cfg, &tx_ws);
    if (tx_ret != LORA_OK) {
        fprintf(stderr,
                "Iteration 0: lora_tx_chain failed (ret=%d, nchips=%zu, out_len=0)\n",
                (int)tx_ret, nchips);
        return EXIT_FAILURE;
    }

    static uint8_t out[LORA_MAX_PAYLOAD_LEN];
    static lora_rx_workspace rx_ws;
    if (!rx_ws.fft_ws) {
        size_t need = lora_rx_fft_workspace_bytes(&cfg);
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
    size_t out_len = 0;
    lora_status rx_ret = lora_rx_chain(chips, nchips, out, sizeof(out), &out_len, &cfg, &rx_ws);
    if (rx_ret != LORA_OK) {
        fprintf(stderr,
                "Iteration 0: lora_rx_chain failed (ret=%d, nchips=%zu, out_len=%zu)\n",
                (int)rx_ret, nchips, out_len);
        return EXIT_FAILURE;
    }

    int ok = (out_len == sizeof(payload)) && (memcmp(out, payload, sizeof(payload)) == 0);

    if (ok) {
        LORA_LOG_INFO("Payload recovered successfully");
        return 0;
    } else {
        LORA_LOG_INFO("Payload mismatch");
        return EXIT_FAILURE;
    }
}
