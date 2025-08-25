#ifndef LORA_IO_H
#define LORA_IO_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *ctx;
    size_t (*read)(void *ctx, uint8_t *buf, size_t len);
    size_t (*write)(void *ctx, const uint8_t *buf, size_t len);
} lora_io_t;

/* File-backed implementations for host builds */
size_t lora_io_file_read(void *ctx, uint8_t *buf, size_t len);
size_t lora_io_file_write(void *ctx, const uint8_t *buf, size_t len);
static inline void lora_io_init_file(lora_io_t *io, FILE *fp) {
    io->ctx = fp;
    io->read = lora_io_file_read;
    io->write = lora_io_file_write;
}

/* Example UART implementations for embedded targets */
size_t lora_io_uart_read(void *ctx, uint8_t *buf, size_t len);
size_t lora_io_uart_write(void *ctx, const uint8_t *buf, size_t len);
static inline void lora_io_init_uart(lora_io_t *io, int fd) {
    io->ctx = (void*)(intptr_t)fd;
    io->read = lora_io_uart_read;
    io->write = lora_io_uart_write;
}

#ifdef __cplusplus
}
#endif

#endif /* LORA_IO_H */
