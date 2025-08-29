#ifndef LORA_GRAYMAP_H
#define LORA_GRAYMAP_H

#include <stddef.h>
#include <stdint.h>

/* Map binary symbols to Gray-coded symbols. */
void lora_gray_map(const uint32_t *restrict in, uint32_t *restrict out,
                   uint8_t sf, size_t len);

/* Reverse operation of lora_gray_map. */
void lora_gray_demap(const uint32_t *restrict in, uint32_t *restrict out,
                     uint8_t sf, size_t len);

#endif /* LORA_GRAYMAP_H */
