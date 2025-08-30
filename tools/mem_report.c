#include <stdio.h>
#include "lora_config.h"

int main(void) {
  printf("metric,value\n");
  printf("LORA_MAX_NSYM,%u\n", (unsigned)LORA_MAX_NSYM);
  printf("LORA_MAX_PAYLOAD_LEN,%u\n", (unsigned)LORA_MAX_PAYLOAD_LEN);
  printf("rx_bytes_saved,%u\n", (unsigned)(2u * LORA_MAX_NSYM));
  printf("tx_bytes_saved,%u\n", (unsigned)(LORA_MAX_PAYLOAD_LEN + 2u));
  return 0;
}

