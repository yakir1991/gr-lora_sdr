#include "lora_data_source.h"
#include <string.h>

int lora_data_source_open(lora_data_source_t *src, const char *filename) {
    if(!src) return -1;
    src->fp = fopen(filename, "rb");
    if(!src->fp) {
        return -1;
    }
    return 0;
}

void lora_data_source_close(lora_data_source_t *src) {
    if(!src) return;
    if(src->fp) {
        fclose(src->fp);
    }
    src->fp = NULL;
}

size_t lora_data_source_read(lora_data_source_t *src, unsigned char *buf, size_t len) {
    if(!src || !src->fp || !buf) return 0;
    return fread(buf, 1, len, src->fp);
}

void lora_data_source_random_string(char *out, size_t nbytes) {
    const char *charmap = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t charmap_len = strlen(charmap);
    for(size_t i = 0; i < nbytes; ++i) {
        out[i] = charmap[rand() % charmap_len];
    }
}
