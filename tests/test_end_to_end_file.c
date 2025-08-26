#include <stdio.h>
#include <stdlib.h>
#include "lora_log.h"
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
        LORA_LOG_ERR("fopen input");
        return EXIT_FAILURE;
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
    const lora_chain_cfg cfg = {.sf = 8, .bw = 125000, .samp_rate = 125000};
    int tx_ret = lora_tx_chain(payload, rd, chips, LORA_MAX_CHIPS, &nchips, &cfg);
    if (tx_ret) {
        fprintf(stderr,
                "Iteration 0: lora_tx_chain failed (ret=%d, nchips=%zu, out_len=0)\n",
                tx_ret, nchips);
        return EXIT_FAILURE;
    }

    const char *bin_path = "tx_capture.bin";
    FILE *fb = fopen(bin_path, "wb");
    if (!fb) {
        LORA_LOG_ERR("fopen bin");
        return EXIT_FAILURE;
    }
    lora_io_t bin_io;
    lora_io_init_file(&bin_io, fb);
    if (bin_io.write(bin_io.ctx, (const uint8_t *)chips,
                     nchips * sizeof(float complex)) != nchips * sizeof(float complex)) {
        fclose(fb);
        return EXIT_FAILURE;
    }
    fclose(fb);

    fb = fopen(bin_path, "rb");
    if (!fb) {
        LORA_LOG_ERR("fopen bin read");
        return EXIT_FAILURE;
    }
    lora_io_init_file(&bin_io, fb);
    static float complex rx_chips[LORA_MAX_CHIPS];
    size_t rdbytes = bin_io.read(bin_io.ctx, (uint8_t *)rx_chips,
                                 nchips * sizeof(float complex));
    fclose(fb);
    size_t rdchips = rdbytes / sizeof(float complex);
    if (rdchips != nchips)
        return EXIT_FAILURE;

    static uint8_t out[LORA_MAX_PAYLOAD_LEN];
    size_t out_len = 0;
    int rx_ret = lora_rx_chain(rx_chips, nchips, out, sizeof(out), &out_len, &cfg);
    if (rx_ret) {
        fprintf(stderr,
                "Iteration 0: lora_rx_chain failed (ret=%d, nchips=%zu, out_len=%zu)\n",
                rx_ret, nchips, out_len);
        return EXIT_FAILURE;
    }

    int ok = (out_len == rd) && (memcmp(out, payload, rd) == 0);
    remove(bin_path);

    if (ok) {
        LORA_LOG_INFO("End-to-end file test passed");
        return 0;
    } else {
        LORA_LOG_INFO("End-to-end file test FAILED");
        return EXIT_FAILURE;
    }
}
