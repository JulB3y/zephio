#ifndef TEST_UTIL_H
#define TEST_UTIL_H

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

#endif
