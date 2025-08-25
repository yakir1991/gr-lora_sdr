#include <stdio.h>
#include <string.h>
#include "lora_io.h"

int main(void) {
    const char *path = "../../legacy_gr_lora_sdr/data/GRC_default/example_tx_source.txt";
    FILE *fp = fopen(path, "rb");
    if(!fp) {
        perror("fopen");
        return 1;
    }
    lora_io_t io;
    lora_io_init_file(&io, fp);
    unsigned char buf1[4096];
    size_t n1 = io.read(io.ctx, buf1, sizeof(buf1));
    fclose(fp);

    fp = fopen(path, "rb");
    if(!fp) {
        perror("fopen");
        return 1;
    }
    unsigned char buf2[4096];
    size_t n2 = fread(buf2, 1, sizeof(buf2), fp);
    fclose(fp);

    int ok = (n1 == n2) && (memcmp(buf1, buf2, n1) == 0);
    if(ok) {
        printf("Read %zu bytes successfully\n", n1);
        return 0;
    } else {
        printf("Mismatch between data source and file\n");
        return 1;
    }
}
