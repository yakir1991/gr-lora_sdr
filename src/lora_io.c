#include "lora_io.h"

size_t lora_io_file_read(void *ctx, uint8_t *buf, size_t len) {
    FILE *f = (FILE *)ctx;
    return fread(buf, 1, len, f);
}

size_t lora_io_file_write(void *ctx, const uint8_t *buf, size_t len) {
    FILE *f = (FILE *)ctx;
    return fwrite(buf, 1, len, f);
}

size_t lora_io_uart_read(void *ctx, uint8_t *buf, size_t len) {
    int fd = (int)(intptr_t)ctx;
    ssize_t rd = read(fd, buf, len);
    if (rd < 0) return 0;
    return (size_t)rd;
}

size_t lora_io_uart_write(void *ctx, const uint8_t *buf, size_t len) {
    int fd = (int)(intptr_t)ctx;
    ssize_t wr = write(fd, buf, len);
    if (wr < 0) return 0;
    return (size_t)wr;
}
