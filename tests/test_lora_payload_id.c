#include "lora_payload_id.h"
#include <stdio.h>
#include <string.h>
#include "lora_log.h"

int main(void)
{
    lora_payload_id_t ctx;
    lora_payload_id_init(&ctx, ':');

    const char *base = "Seq: 0";
    char out[32];

    lora_payload_id_next(&ctx, base, out, sizeof(out));
    if (strcmp(out, "Seq: 1") != 0) {
        LORA_LOG_INFO("First increment failed: %s", out);
        return 1;
    }

    lora_payload_id_next(&ctx, base, out, sizeof(out));
    if (strcmp(out, "Seq: 2") != 0) {
        LORA_LOG_INFO("Second increment failed: %s", out);
        return 1;
    }

    for (int i = 0; i < 253; i++)
        lora_payload_id_next(&ctx, base, out, sizeof(out));

    lora_payload_id_next(&ctx, base, out, sizeof(out));
    if (strcmp(out, "Seq: 0") != 0) {
        LORA_LOG_INFO("Wrap-around failed: %s", out);
        return 1;
    }

    LORA_LOG_INFO("Payload ID test passed");
    return 0;
}

