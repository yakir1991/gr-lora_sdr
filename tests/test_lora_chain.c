#include <stdio.h>
#include <string.h>
#include <complex.h>
#include "lora_log.h"
#include "lora_chain.h"
#include "lora_config.h"

int main(void)
{
    const uint8_t payload[] = { 'A', 'B', 'C' };
    static float complex chips[LORA_MAX_CHIPS];
    size_t nchips = 0;
    if (lora_tx_chain(payload, sizeof(payload), chips, LORA_MAX_CHIPS, &nchips) != 0) {
        LORA_LOG_ERR("TX chain failed");
        return 1;
    }

    static uint8_t out[LORA_MAX_PAYLOAD_LEN];
    size_t out_len = 0;
    if (lora_rx_chain(chips, nchips, out, sizeof(out), &out_len) != 0) {
        LORA_LOG_ERR("RX chain failed");
        return 1;
    }

    int ok = (out_len == sizeof(payload)) && (memcmp(out, payload, sizeof(payload)) == 0);

    if (ok) {
        LORA_LOG_INFO("Payload recovered successfully");
        return 0;
    } else {
        LORA_LOG_INFO("Payload mismatch");
        return 1;
    }
}

