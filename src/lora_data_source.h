#ifndef LORA_DATA_SOURCE_H
#define LORA_DATA_SOURCE_H

#include <stdio.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    FILE *fp;
} lora_data_source_t;

/* Open a file for reading. Returns NULL on failure. */
lora_data_source_t *lora_data_source_open(const char *filename);

/* Close and free resources. */
void lora_data_source_close(lora_data_source_t *src);

/* Read up to len bytes into buf. Returns number of bytes read. */
size_t lora_data_source_read(lora_data_source_t *src, unsigned char *buf, size_t len);

/* Generate a random ASCII string matching data_source_impl logic. */
void lora_data_source_random_string(char *out, size_t nbytes);

#ifdef __cplusplus
}
#endif

#endif /* LORA_DATA_SOURCE_H */
