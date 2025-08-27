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
    size_t out_len = 0;
    if (lora_rx_chain_init(&rx_ws, cfg.sf, cfg.samp_rate, cfg.bw) != LORA_OK)
        return EXIT_FAILURE;
    lora_status rx_ret = lora_rx_chain(chips, nchips, out, sizeof(out), &out_len, &cfg, &rx_ws);
    lora_rx_chain_free(&rx_ws);
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

