#include "lora_graymap.h"
#include "lora_gray_lut.h"

void lora_gray_map(const uint32_t *restrict in, uint32_t *restrict out,
                   uint8_t sf, size_t len) {
  const uint32_t mask = (1u << sf) - 1u;
  size_t i = 0;
  size_t n4 = len & ~(size_t)3;
  for (; i < n4; i += 4) {
    uint32_t a = in[i + 0] & mask;
    uint32_t b = in[i + 1] & mask;
    uint32_t c = in[i + 2] & mask;
    uint32_t d = in[i + 3] & mask;
    out[i + 0] = LORA_GRAY_MAP_LUT[a] & mask;
    out[i + 1] = LORA_GRAY_MAP_LUT[b] & mask;
    out[i + 2] = LORA_GRAY_MAP_LUT[c] & mask;
    out[i + 3] = LORA_GRAY_MAP_LUT[d] & mask;
  }
  for (; i < len; ++i)
    out[i] = LORA_GRAY_MAP_LUT[in[i] & mask] & mask;
}

void lora_gray_demap(const uint32_t *restrict in, uint32_t *restrict out,
                     uint8_t sf, size_t len) {
  const uint32_t mask = (1u << sf) - 1u;
  size_t i = 0;
  size_t n4 = len & ~(size_t)3;
  for (; i < n4; i += 4) {
    uint32_t a = in[i + 0] & mask;
    uint32_t b = in[i + 1] & mask;
    uint32_t c = in[i + 2] & mask;
    uint32_t d = in[i + 3] & mask;
    out[i + 0] = LORA_GRAY_DEMAP_LUT[a] & mask;
    out[i + 1] = LORA_GRAY_DEMAP_LUT[b] & mask;
    out[i + 2] = LORA_GRAY_DEMAP_LUT[c] & mask;
    out[i + 3] = LORA_GRAY_DEMAP_LUT[d] & mask;
  }
  for (; i < len; ++i)
    out[i] = LORA_GRAY_DEMAP_LUT[in[i] & mask] & mask;
}
