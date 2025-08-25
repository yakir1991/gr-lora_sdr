#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include "lora_chain.h"

int main(void)
{
    const char *in_path = "../../../data/GRC_default/example_tx_source.txt";
    FILE *fi = fopen(in_path, "rb");
    if (!fi) {
        perror("fopen input");
        return 1;
    }
    if (fseek(fi, 0, SEEK_END) != 0) {
        fclose(fi);
        return 1;
    }
    long flen = ftell(fi);
    if (flen < 0) {
        fclose(fi);
        return 1;
    }
    rewind(fi);
    uint8_t *payload = (uint8_t *)malloc((size_t)flen);
    if (!payload) {
        fclose(fi);
        return 1;
    }
    size_t rd = fread(payload, 1, (size_t)flen, fi);
    fclose(fi);
    if (rd != (size_t)flen) {
        free(payload);
        return 1;
    }

    float complex *chips = NULL;
    size_t nchips = 0;
    if (lora_tx_chain(payload, rd, &chips, &nchips) != 0) {
        free(payload);
        fprintf(stderr, "lora_tx_chain failed\n");
        return 1;
    }

    const char *bin_path = "tx_capture.bin";
    FILE *fb = fopen(bin_path, "wb");
    if (!fb) {
        perror("fopen bin");
        free(payload);
        free(chips);
        return 1;
    }
    fwrite(chips, sizeof(float complex), nchips, fb);
    fclose(fb);
    free(chips);

    fb = fopen(bin_path, "rb");
    if (!fb) {
        perror("fopen bin read");
        free(payload);
        return 1;
    }
    float complex *rx_chips = (float complex *)malloc(nchips * sizeof(float complex));
    if (!rx_chips) {
        fclose(fb);
        free(payload);
        return 1;
    }
    size_t rdchips = fread(rx_chips, sizeof(float complex), nchips, fb);
    fclose(fb);
    if (rdchips != nchips) {
        free(rx_chips);
        free(payload);
        return 1;
    }

    uint8_t *out = NULL;
    size_t out_len = 0;
    if (lora_rx_chain(rx_chips, nchips, &out, &out_len) != 0) {
        free(rx_chips);
        free(payload);
        fprintf(stderr, "lora_rx_chain failed\n");
        return 1;
    }
    free(rx_chips);

    int ok = (out_len == rd) && (memcmp(out, payload, rd) == 0);
    free(out);
    free(payload);
    remove(bin_path);

    if (ok) {
        printf("End-to-end file test passed\n");
        return 0;
    } else {
        printf("End-to-end file test FAILED\n");
        return 1;
    }
}
