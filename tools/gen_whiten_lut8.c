#include <stdio.h>
#include <stdint.h>

int main(void) {
  printf("#include <stdint.h>\n\n");
  printf("const uint64_t LORA_WHITEN_LUT8_STATIC[256] = {\n");
  for (int s = 0; s < 256; ++s) {
    uint8_t lfsr = (uint8_t)s;
    uint8_t bytes[8];
    for (int i = 0; i < 8; ++i) {
      bytes[i] = lfsr;
      uint8_t new_bit = ((lfsr >> 7) ^ (lfsr >> 5) ^ (lfsr >> 4) ^ (lfsr >> 3)) & 0x01u;
      lfsr = (uint8_t)((lfsr << 1) | new_bit);
    }
    uint64_t mask = 0;
    for (int i = 0; i < 8; ++i) mask |= ((uint64_t)bytes[i]) << (8*i);
    printf("  0x%016llxULL,%s\n", (unsigned long long)mask, (s%4==3)?"":"");
  }
  printf("};\n\n");
  printf("const uint8_t LORA_WHITEN_NEXT8_STATIC[256] = {\n");
  for (int s = 0; s < 256; ++s) {
    uint8_t lfsr = (uint8_t)s;
    for (int i = 0; i < 8; ++i) {
      uint8_t new_bit = ((lfsr >> 7) ^ (lfsr >> 5) ^ (lfsr >> 4) ^ (lfsr >> 3)) & 0x01u;
      lfsr = (uint8_t)((lfsr << 1) | new_bit);
    }
    printf("  %u,%s\n", (unsigned)lfsr, (s%16==15)?"":"");
  }
  printf("};\n");
  return 0;
}

