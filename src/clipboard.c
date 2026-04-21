#include "zephio_clipboard.h"
#include "zephio_terminal.h"
#include "zephio_context.h"

#include <stdlib.h>
#include <string.h>

static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static size_t base64_encode(const unsigned char *src, size_t len,
                            char *dst, size_t dst_size)
{
    size_t out = 0;

    for (size_t i = 0; i < len && out + 4 <= dst_size; i += 3) {
        unsigned int n = (unsigned int)src[i] << 16;
        if (i + 1 < len) n |= (unsigned int)src[i + 1] << 8;
        if (i + 2 < len) n |= (unsigned int)src[i + 2];

        dst[out++] = b64_table[(n >> 18) & 0x3F];
        dst[out++] = b64_table[(n >> 12) & 0x3F];
        dst[out++] = (i + 1 < len) ? b64_table[(n >> 6) & 0x3F] : '=';
        dst[out++] = (i + 2 < len) ? b64_table[n & 0x3F] : '=';
    }

    if (out < dst_size) dst[out] = '\0';
    return out;
}

static size_t base64_encoded_size(size_t len)
{
    return ((len + 2) / 3) * 4;
}

int zephio_clipboard_copy(ZephioContext *ctx, const char *text)
{
    if (!text) return -1;
    return zephio_clipboard_copy_n(ctx, text, strlen(text));
}

int zephio_clipboard_copy_n(ZephioContext *ctx, const char *data, size_t len)
{
    if (!data || len == 0) return -1;

    size_t b64_len = base64_encoded_size(len);
    size_t seq_len = 4 + 2 + b64_len + 2;

    char *buf = (char *)malloc(seq_len + 1);
    if (!buf) return -1;

    memcpy(buf, "\033]52;c;", 7);
    size_t encoded = base64_encode((const unsigned char *)data, len,
                                   buf + 7, b64_len + 1);
    buf[7 + encoded] = '\007';

    terminal_write_seq(&ctx->terminal, buf, 7 + encoded + 1);
    free(buf);
    return 0;
}
