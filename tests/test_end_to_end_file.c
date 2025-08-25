#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <complex.h>
#include "lora_chain.h"
#include "lora_config.h"
#include "lora_io.h"

int main(void)
{
    const char *in_path = "../../legacy_gr_lora_sdr/data/GRC_default/example_tx_source.txt";
    FILE *fi = fopen(in_path, "rb");
    if (!fi) {
        perror("fopen input");
        return 1;
    }
    lora_io_t in_io;
    lora_io_init_file(&in_io, fi);
    static uint8_t payload[LORA_MAX_PAYLOAD_LEN];
    size_t rd = 0;
    while (rd < LORA_MAX_PAYLOAD_LEN) {
        size_t n = in_io.read(in_io.ctx, payload + rd, LORA_MAX_PAYLOAD_LEN - rd);
        if (n == 0)
            break;
        rd += n;
    }
    fclose(fi);

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
    lora_io_t bin_io;
    lora_io_init_file(&bin_io, fb);
    if (bin_io.write(bin_io.ctx, (const uint8_t *)chips,
                     nchips * sizeof(float complex)) != nchips * sizeof(float complex)) {
        fclose(fb);
        return 1;
    }
    fclose(fb);

    fb = fopen(bin_path, "rb");
    if (!fb) {
        perror("fopen bin read");
        return 1;
    }
    lora_io_init_file(&bin_io, fb);
    static float complex rx_chips[LORA_MAX_CHIPS];
    size_t rdbytes = bin_io.read(bin_io.ctx, (uint8_t *)rx_chips,
                                 nchips * sizeof(float complex));
    fclose(fb);
    size_t rdchips = rdbytes / sizeof(float complex);
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
