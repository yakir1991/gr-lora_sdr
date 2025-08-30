#include "lora_payload_id.h"

#include <stdio.h>
#include <string.h>

void lora_payload_id_init(lora_payload_id_t *ctx, char separator)
{
    ctx->separator = separator;
    ctx->counter = 0;
}


static inline size_t u8_to_dec(uint8_t v, char *dst, size_t cap)
{
    if (cap == 0) return 0;
    if (v < 10) {
        dst[0] = (char)('0' + v);
        return 1;
    } else if (v < 100) {
        if (cap < 2) return 0;
        uint8_t d10 = (uint8_t)(v / 10);
        uint8_t d1  = (uint8_t)(v - d10 * 10);
        dst[0] = (char)('0' + d10);
        dst[1] = (char)('0' + d1);
        return 2;
    } else {
        if (cap < 3) return 0;
        uint8_t r = (uint8_t)(v - (v >= 200 ? 200 : 100));
        dst[0] = (char)('1' + (v >= 200));
        dst[1] = (char)('0' + (r / 10));
        dst[2] = (char)('0' + (r % 10));
        return 3;
    }
}

void lora_payload_id_next(lora_payload_id_t *ctx, const char *in_msg,
                          char *out_msg, size_t out_len)
{
    const char *sep = strchr(in_msg, ctx->separator);
    size_t prefix_len = 0;

    if (sep) {
        prefix_len = (size_t)(sep - in_msg) + 2; // include separator and following char
        if (prefix_len > out_len)
            prefix_len = out_len;
        if (prefix_len)
            memcpy(out_msg, in_msg, prefix_len);
    }

    ctx->counter = (uint8_t)(ctx->counter + 1); // wraps automatically at 256
    if (prefix_len < out_len) {
        size_t cap = out_len - prefix_len;
        size_t n = u8_to_dec(ctx->counter, out_msg + prefix_len, cap);
        if (prefix_len + n < out_len)
            out_msg[prefix_len + n] = '\0';
        else if (out_len > 0)
            out_msg[out_len - 1] = '\0';
    } else if (out_len > 0) {
        out_msg[out_len - 1] = '\0';
    }
}

