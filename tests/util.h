#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_test_run = 0;
static int g_test_pass = 0;
static int g_test_fail = 0;

#define TEST_ASSERT(expr)                                                  \
    do {                                                                   \
        g_test_run++;                                                      \
        if (expr) {                                                        \
            g_test_pass++;                                                 \
        } else {                                                           \
            g_test_fail++;                                                 \
            fprintf(stderr, "  FAIL %s:%d: %s\n", __FILE__, __LINE__,      \
                    #expr);                                                \
        }                                                                  \
    } while (0)

#define TEST_EQ(a, b) TEST_ASSERT((a) == (b))
#define TEST_NE(a, b) TEST_ASSERT((a) != (b))
#define TEST_STR_EQ(a, b) TEST_ASSERT(strcmp((a), (b)) == 0)

#define TEST_BEGIN(name)                                                   \
    static void test_##name(void);                                         \
    static void test_##name(void)

#define TEST_RUN(name)                                                     \
    do {                                                                   \
        fprintf(stderr, "  %s ...\n", #name);                              \
        test_##name();                                                     \
    } while (0)

#define TEST_SUMMARY()                                                     \
    do {                                                                   \
        fprintf(stderr, "\n%d tests: %d passed, %d failed\n",             \
                g_test_run, g_test_pass, g_test_fail);                     \
        return g_test_fail > 0 ? 1 : 0;                                   \
    } while (0)

#ifdef ZEPHIO_TEST_CAPTURE
#include <unistd.h>
#include "zephio_context.h"

static char g_output_buf[8192];
static int g_output_len = 0;
static int g_cap_r = -1;
static int g_cap_w = -1;
static ZephioContext g_test_ctx;

static inline void capture_start(void)
{
    if (g_cap_r >= 0) close(g_cap_r);
    if (g_cap_w >= 0) close(g_cap_w);
    int p[2];
    pipe(p);
    g_cap_r = p[0];
    g_cap_w = p[1];
    g_test_ctx.terminal.fd = g_cap_w;
    memset(g_output_buf, 0, sizeof(g_output_buf));
    g_output_len = 0;
}

static inline void capture_drain(void)
{
    if (g_cap_w >= 0) close(g_cap_w);
    g_test_ctx.terminal.fd = -1;
    ssize_t total = 0;
    if (g_cap_r >= 0) {
        ssize_t n;
        while ((n = read(g_cap_r, g_output_buf + total,
                         sizeof(g_output_buf) - 1 - (size_t)total)) > 0) {
            total += n;
        }
        close(g_cap_r);
    }
    g_output_buf[total] = '\0';
    g_output_len = (int)total;
    g_cap_r = -1;
    g_cap_w = -1;
}

static inline void capture_done(void)
{
    if (g_cap_r >= 0) close(g_cap_r);
    if (g_cap_w >= 0) close(g_cap_w);
    g_cap_r = -1;
    g_cap_w = -1;
    g_test_ctx.terminal.fd = -1;
}
#endif

#endif
