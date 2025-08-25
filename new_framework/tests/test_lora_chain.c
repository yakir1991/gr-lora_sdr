#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <complex.h>
#include "lora_chain.h"

int main(void)
{
    const uint8_t payload[] = { 'A', 'B', 'C' };
    float complex *chips = NULL;
    size_t nchips = 0;
    if (lora_tx_chain(payload, sizeof(payload), &chips, &nchips) != 0) {
        fprintf(stderr, "TX chain failed\n");
        return 1;
    }

    uint8_t *out = NULL;
    size_t out_len = 0;
    if (lora_rx_chain(chips, nchips, &out, &out_len) != 0) {
        free(chips);
        fprintf(stderr, "RX chain failed\n");
        return 1;
    }
    free(chips);

    int ok = (out_len == sizeof(payload)) && (memcmp(out, payload, sizeof(payload)) == 0);
    free(out);

    if (ok) {
        printf("Payload recovered successfully\n");
        return 0;
    } else {
        printf("Payload mismatch\n");
        return 1;
    }
}

