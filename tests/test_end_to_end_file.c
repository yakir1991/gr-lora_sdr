#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <complex.h>
#include "lora_chain.h"
#include "lora_config.h"

int main(void)
{
    const char *in_path = "../../legacy_gr_lora_sdr/data/GRC_default/example_tx_source.txt";
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
    if ((size_t)flen > LORA_MAX_PAYLOAD_LEN) {
        fclose(fi);
        return 1;
    }
    static uint8_t payload[LORA_MAX_PAYLOAD_LEN];
    size_t rd = fread(payload, 1, (size_t)flen, fi);
    fclose(fi);
    if (rd != (size_t)flen)
        return 1;

    static float complex chips[LORA_MAX_CHIPS];
    size_t nchips = 0;
    if (lora_tx_chain(payload, rd, chips, LORA_MAX_CHIPS, &nchips) != 0) {
        fprintf(stderr, "lora_tx_chain failed\n");
        return 1;
    }

    const char *bin_path = "tx_capture.bin";
    FILE *fb = fopen(bin_path, "wb");
    if (!fb) {
        perror("fopen bin");
        return 1;
    }
    fwrite(chips, sizeof(float complex), nchips, fb);
    fclose(fb);

    fb = fopen(bin_path, "rb");
    if (!fb) {
        perror("fopen bin read");
        return 1;
    }
    static float complex rx_chips[LORA_MAX_CHIPS];
    size_t rdchips = fread(rx_chips, sizeof(float complex), nchips, fb);
    fclose(fb);
    if (rdchips != nchips)
        return 1;

    static uint8_t out[LORA_MAX_PAYLOAD_LEN];
    size_t out_len = 0;
    if (lora_rx_chain(rx_chips, nchips, out, sizeof(out), &out_len) != 0) {
        fprintf(stderr, "lora_rx_chain failed\n");
        return 1;
    }

    int ok = (out_len == rd) && (memcmp(out, payload, rd) == 0);
    remove(bin_path);

    if (ok) {
        printf("End-to-end file test passed\n");
        return 0;
    } else {
        printf("End-to-end file test FAILED\n");
        return 1;
    }
}
