#ifndef LORA_WHITENING_H
#define LORA_WHITENING_H

#include <stddef.h>
#include <stdint.h>

void lora_whiten(const uint8_t *in, uint8_t *out, size_t len);
void lora_dewhiten(const uint8_t *in, uint8_t *out, size_t len);

#endif // LORA_WHITENING_H
