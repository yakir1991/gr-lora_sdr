#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char separator;
    uint8_t counter;
} lora_payload_id_t;

/* Initialize the payload ID generator with the desired separator (e.g., ':'). */
void lora_payload_id_init(lora_payload_id_t *ctx, char separator);

/*
 * Increment the internal counter and format a new message.
 * The prefix of in_msg up to and including the separator and following space
 * is copied to out_msg, followed by the updated counter value.
 * out_msg must provide enough space for the result.
 */
void lora_payload_id_next(lora_payload_id_t *ctx, const char *in_msg,
                          char *out_msg, size_t out_len);

#ifdef __cplusplus
}
#endif

