#include <stdio.h>
#include <string.h>
#include "../src/lora_config.h"

int main(void) {
    const char *path1 = "../../legacy_gr_lora_sdr/gnuradio_out.bin";
    const char *path2 = "../../legacy_gr_lora_sdr/framework_out.bin";
    FILE *f1 = fopen(path1, "rb");
    FILE *f2 = fopen(path2, "rb");
    if (!f1 || !f2) {
        perror("fopen");
        return 1;
    }
    fseek(f1, 0, SEEK_END);
    fseek(f2, 0, SEEK_END);
    long len1 = ftell(f1);
    long len2 = ftell(f2);
    rewind(f1);
    rewind(f2);
    if (len1 != len2) {
        printf("Length mismatch\n");
        fclose(f1);
        fclose(f2);
        return 1;
    }
    if (len1 > LORA_MAX_FILE_BYTES || len2 > LORA_MAX_FILE_BYTES) {
        fclose(f1);
        fclose(f2);
        return 1;
    }
    static unsigned char buf1[LORA_MAX_FILE_BYTES];
    static unsigned char buf2[LORA_MAX_FILE_BYTES];
    fread(buf1, 1, len1, f1);
    fread(buf2, 1, len2, f2);
    fclose(f1);
    fclose(f2);
    int ok = memcmp(buf1, buf2, len1) == 0;
    if (ok) {
        printf("GNURadio and framework outputs match\n");
        return 0;
    } else {
        printf("Outputs differ\n");
        return 1;
    }
}
