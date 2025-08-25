#ifndef LORA_LOG_H
#define LORA_LOG_H

#if LORA_LITE_ENABLE_LOGGING
#include <stdio.h>
#ifndef LORA_LOG_PRINTF
#define LORA_LOG_PRINTF printf
#endif
#ifndef LORA_LOG_ERRPRINTF
#define LORA_LOG_ERRPRINTF fprintf
#endif
#define LORA_LOG_DEBUG(fmt, ...) LORA_LOG_PRINTF("DEBUG: " fmt "\n", ##__VA_ARGS__)
#define LORA_LOG_INFO(fmt, ...)  LORA_LOG_PRINTF("INFO: " fmt "\n", ##__VA_ARGS__)
#define LORA_LOG_WARN(fmt, ...)  LORA_LOG_PRINTF("WARN: " fmt "\n", ##__VA_ARGS__)
#define LORA_LOG_ERR(fmt, ...)   LORA_LOG_ERRPRINTF(stderr, "ERROR: " fmt "\n", ##__VA_ARGS__)
#else
#define LORA_LOG_DEBUG(fmt, ...) do { (void)0; } while(0)
#define LORA_LOG_INFO(fmt, ...)  do { (void)0; } while(0)
#define LORA_LOG_WARN(fmt, ...)  do { (void)0; } while(0)
#define LORA_LOG_ERR(fmt, ...)   do { (void)0; } while(0)
#endif

#endif // LORA_LOG_H
