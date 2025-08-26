#include <stdio.h>
#include <complex.h>
#include "lora_chain.h"
#include "lora_config.h"

int main(void)
{
    int ret;
    size_t nchips;
    static float complex chips[(LORA_MAX_NSYM + 1) * LORA_MAX_SPS];
    static uint8_t payload[LORA_MAX_PAYLOAD_LEN + 1];
    const lora_chain_cfg cfg = {.sf = 8, .bw = 125000, .samp_rate = 125000};

    ret = lora_tx_chain(payload, LORA_MAX_PAYLOAD_LEN + 1, chips, LORA_MAX_CHIPS, &nchips, &cfg);
    if (ret >= 0) {
        printf("Expected failure for oversized payload\n");
        return 1;
    }

    ret = lora_tx_chain(NULL, 1, chips, LORA_MAX_CHIPS, &nchips, &cfg);
    if (ret >= 0) {
        printf("Expected failure for NULL payload\n");
        return 1;
    }

    ret = lora_tx_chain(payload, 1, NULL, LORA_MAX_CHIPS, &nchips, &cfg);
    if (ret >= 0) {
        printf("Expected failure for NULL chips\n");
        return 1;
    }

    ret = lora_tx_chain(payload, 1, chips, 0, &nchips, &cfg);
    if (ret >= 0) {
        printf("Expected failure for zero chip buffer\n");
        return 1;
    }

    ret = lora_tx_chain(payload, 1, chips, LORA_MAX_CHIPS, NULL, &cfg);
    if (ret >= 0) {
        printf("Expected failure for NULL nchips_out\n");
        return 1;
    }

    const uint8_t small_payload[1] = {0x42};
    ret = lora_tx_chain(small_payload, sizeof(small_payload), chips, LORA_MAX_CHIPS, &nchips, &cfg);
    if (ret != 0) {
        printf("TX setup failed\n");
        return 1;
    }

    static uint8_t out[LORA_MAX_PAYLOAD_LEN];
    size_t out_len;

    ret = lora_rx_chain(chips, (LORA_MAX_NSYM + 1) * LORA_MAX_SPS, out, sizeof(out), &out_len, &cfg);
    if (ret >= 0) {
        printf("Expected failure for oversized nchips\n");
        return 1;
    }

    ret = lora_rx_chain(NULL, 0, out, sizeof(out), &out_len, &cfg);
    if (ret >= 0) {
        printf("Expected failure for NULL chips\n");
        return 1;
    }

    ret = lora_rx_chain(chips, 0, NULL, sizeof(out), &out_len, &cfg);
    if (ret >= 0) {
        printf("Expected failure for NULL payload\n");
        return 1;
    }

    ret = lora_rx_chain(chips, 0, out, 0, &out_len, &cfg);
    if (ret >= 0) {
        printf("Expected failure for zero payload buffer\n");
        return 1;
    }

    ret = lora_rx_chain(chips, 0, out, sizeof(out), NULL, &cfg);
    if (ret >= 0) {
        printf("Expected failure for NULL payload_len_out\n");
        return 1;
    }

    ret = lora_tx_chain(payload, 1, chips, LORA_MAX_CHIPS, &nchips, NULL);
    if (ret >= 0) {
        printf("Expected failure for NULL cfg in tx\n");
        return 1;
    }

    ret = lora_rx_chain(chips, 0, out, sizeof(out), &out_len, NULL);
    if (ret >= 0) {
        printf("Expected failure for NULL cfg in rx\n");
        return 1;
    }

    printf("Edge case tests passed\n");
    return 0;
}
