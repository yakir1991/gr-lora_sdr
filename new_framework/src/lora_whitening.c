#include "lora_whitening.h"
#include "lora_utils.h"

void lora_whiten(const uint8_t *in, uint8_t *out, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        out[i] = in[i] ^ lora_whitening_seq[i % LORA_WHITENING_SEQ_LEN];
    }
}

void lora_dewhiten(const uint8_t *in, uint8_t *out, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        out[i] = in[i] ^ lora_whitening_seq[i % LORA_WHITENING_SEQ_LEN];
    }
}
