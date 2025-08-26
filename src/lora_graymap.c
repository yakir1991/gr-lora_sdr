#include "lora_graymap.h"
#include "lora_gray_lut.h"

void lora_gray_map(const uint32_t *in, uint32_t *out, uint8_t sf, size_t len) {
  const uint32_t mask = (1u << sf) - 1u;
  for (size_t i = 0; i < len; ++i) {
    out[i] = LORA_GRAY_MAP_LUT[in[i] & mask] & mask;
  }
}

void lora_gray_demap(const uint32_t *in, uint32_t *out, uint8_t sf,
                     size_t len) {
  const uint32_t mask = (1u << sf) - 1u;
  for (size_t i = 0; i < len; ++i) {
    out[i] = LORA_GRAY_DEMAP_LUT[in[i] & mask] & mask;
  }
}
