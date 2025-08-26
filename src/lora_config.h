#pragma once

#include <stdint.h>
#include <complex.h>

#ifndef LORA_MAX_SPS
#define LORA_MAX_SPS 4096U
#endif
_Static_assert(LORA_MAX_SPS > 0, "LORA_MAX_SPS must be > 0");

#ifndef LORA_MAX_NSYM
#define LORA_MAX_NSYM 256U
#endif
_Static_assert(LORA_MAX_NSYM > 0, "LORA_MAX_NSYM must be > 0");

#ifndef LORA_MAX_PAYLOAD_LEN
#define LORA_MAX_PAYLOAD_LEN 254U
#endif
_Static_assert(LORA_MAX_PAYLOAD_LEN + 2U <= LORA_MAX_NSYM, "LORA_MAX_PAYLOAD_LEN + 2 must be <= LORA_MAX_NSYM");

#ifndef LORA_KISSFFT_CFG_MAX
#define LORA_KISSFFT_CFG_MAX 131072U
#endif
_Static_assert(LORA_KISSFFT_CFG_MAX > 0, "LORA_KISSFFT_CFG_MAX must be > 0");

#ifndef LORA_MAX_CHIPS
#define LORA_MAX_CHIPS (LORA_MAX_SPS * LORA_MAX_NSYM)
#endif
_Static_assert(LORA_MAX_CHIPS > 0, "LORA_MAX_CHIPS must be > 0");
_Static_assert(LORA_MAX_CHIPS >= LORA_MAX_SPS * (LORA_MAX_PAYLOAD_LEN + 2U),
               "LORA_MAX_CHIPS must accommodate maximum payload");

#ifndef LORA_MAX_FILE_BYTES
#define LORA_MAX_FILE_BYTES (LORA_MAX_CHIPS * sizeof(float complex))
#endif
_Static_assert(LORA_MAX_FILE_BYTES > 0, "LORA_MAX_FILE_BYTES must be > 0");
