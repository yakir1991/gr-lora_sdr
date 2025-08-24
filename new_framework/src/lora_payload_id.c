#include "lora_payload_id.h"

#include <stdio.h>
#include <string.h>

void lora_payload_id_init(lora_payload_id_t *ctx, char separator)
{
    ctx->separator = separator;
    ctx->counter = 0;
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
    if (prefix_len < out_len)
        snprintf(out_msg + prefix_len, out_len - prefix_len, "%u", ctx->counter);
    else if (out_len > 0)
        out_msg[out_len - 1] = '\0';
}

