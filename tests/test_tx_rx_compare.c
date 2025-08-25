#include <stdio.h>
#include <string.h>
#include "lora_log.h"
#include "../src/lora_config.h"
#include "lora_io.h"

int main(void) {
    const char *path1 = "../../legacy_gr_lora_sdr/gnuradio_out.bin";
    const char *path2 = "../../legacy_gr_lora_sdr/framework_out.bin";
    FILE *f1 = fopen(path1, "rb");
    FILE *f2 = fopen(path2, "rb");
    if (!f1 || !f2) {
        LORA_LOG_ERR("fopen");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return 1;
    }
    lora_io_t io1, io2;
    lora_io_init_file(&io1, f1);
    lora_io_init_file(&io2, f2);
    static unsigned char buf1[LORA_MAX_FILE_BYTES];
    static unsigned char buf2[LORA_MAX_FILE_BYTES];
    size_t len1 = 0, len2 = 0, n;
    while ((n = io1.read(io1.ctx, buf1 + len1, sizeof(buf1) - len1)) > 0)
        len1 += n;
    while ((n = io2.read(io2.ctx, buf2 + len2, sizeof(buf2) - len2)) > 0)
        len2 += n;
    fclose(f1);
    fclose(f2);
    if (len1 != len2) {
        LORA_LOG_INFO("Length mismatch");
        return 1;
    }
    int ok = memcmp(buf1, buf2, len1) == 0;
    if (ok) {
        LORA_LOG_INFO("GNURadio and framework outputs match");
        return 0;
    } else {
        LORA_LOG_INFO("Outputs differ");
        return 1;
    }
}
