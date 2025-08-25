#include "lora_data_source.h"
#include <stdlib.h>
#include <string.h>

lora_data_source_t *lora_data_source_open(const char *filename) {
    lora_data_source_t *src = (lora_data_source_t*)malloc(sizeof(lora_data_source_t));
    if(!src) return NULL;
    src->fp = fopen(filename, "rb");
    if(!src->fp) {
        free(src);
        return NULL;
    }
    return src;
}

void lora_data_source_close(lora_data_source_t *src) {
    if(!src) return;
    if(src->fp) {
        fclose(src->fp);
    }
    free(src);
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
