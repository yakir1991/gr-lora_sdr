#include "lora_payload_id.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    lora_payload_id_t ctx;
    lora_payload_id_init(&ctx, ':');

    const char *base = "Seq: 0";
    char out[32];

    lora_payload_id_next(&ctx, base, out, sizeof(out));
    if (strcmp(out, "Seq: 1") != 0) {
        printf("First increment failed: %s\n", out);
        return 1;
    }

    lora_payload_id_next(&ctx, base, out, sizeof(out));
    if (strcmp(out, "Seq: 2") != 0) {
        printf("Second increment failed: %s\n", out);
        return 1;
    }

    for (int i = 0; i < 253; i++)
        lora_payload_id_next(&ctx, base, out, sizeof(out));

    lora_payload_id_next(&ctx, base, out, sizeof(out));
    if (strcmp(out, "Seq: 0") != 0) {
        printf("Wrap-around failed: %s\n", out);
        return 1;
    }

    printf("Payload ID test passed\n");
    return 0;
}

