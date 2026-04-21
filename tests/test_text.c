#include "util.h"
#include "zephio_context.h"
#include "zephio_text.h"

#include <string.h>

/* ── utf8_valid ──────────────────────────────────────────────────── */

TEST_BEGIN(utf8_valid_ascii)
{
    const char *s = "Hello";
    TEST_ASSERT(tui_utf8_valid(s, strlen(s)));
}

TEST_BEGIN(utf8_valid_empty)
{
    TEST_ASSERT(tui_utf8_valid("", 0));
}


TEST_BEGIN(utf8_valid_null)
{
    TEST_ASSERT(!tui_utf8_valid(NULL, 5));
}


TEST_BEGIN(utf8_valid_two_byte)
{
    const char *s = "\xC3\xBC";
    TEST_ASSERT(tui_utf8_valid(s, 2));
}


TEST_BEGIN(utf8_valid_three_byte)
{
    const char *s = "\xE2\x82\xAC";
    TEST_ASSERT(tui_utf8_valid(s, 3));
}


TEST_BEGIN(utf8_valid_four_byte)
{
    const char *s = "\xF0\x9F\x98\x80";
    TEST_ASSERT(tui_utf8_valid(s, 4));
}


TEST_BEGIN(utf8_valid_truncated)
{
    const char *s = "\xC3";
    TEST_ASSERT(!tui_utf8_valid(s, 1));
}


TEST_BEGIN(utf8_valid_invalid_cont)
{
    const char *s = "\xC3\x00";
    TEST_ASSERT(!tui_utf8_valid(s, 2));
}


TEST_BEGIN(utf8_valid_surrogate)
{
    const char *s = "\xED\xA0\x80";
    TEST_ASSERT(!tui_utf8_valid(s, 3));
}


TEST_BEGIN(utf8_valid_overlong)
{
    const char *s = "\xC0\x80";
    TEST_ASSERT(!tui_utf8_valid(s, 2));
}


/* ── utf8_char_len ───────────────────────────────────────────────── */

TEST_BEGIN(utf8_char_len_ascii)
{
    TEST_EQ(tui_utf8_char_len('A'), 1);
    TEST_EQ(tui_utf8_char_len(0x7F), 1);
}


TEST_BEGIN(utf8_char_len_lead_bytes)
{
    TEST_EQ(tui_utf8_char_len(0xC3), 2);
    TEST_EQ(tui_utf8_char_len(0xE2), 3);
    TEST_EQ(tui_utf8_char_len(0xF0), 4);
}


TEST_BEGIN(utf8_char_len_invalid)
{
    TEST_EQ(tui_utf8_char_len(0x80), 1);
    TEST_EQ(tui_utf8_char_len(0xFF), 1);
}


/* ── utf8_next ───────────────────────────────────────────────────── */

TEST_BEGIN(utf8_next_ascii)
{
    uint32_t cp;
    int n = tui_utf8_next("A", 1, &cp);
    TEST_EQ(n, 1);
    TEST_EQ((int)cp, 'A');
}


TEST_BEGIN(utf8_next_two_byte)
{
    uint32_t cp;
    int n = tui_utf8_next("\xC3\xBC", 2, &cp);
    TEST_EQ(n, 2);
    TEST_EQ((int)cp, 0xFC);
}


TEST_BEGIN(utf8_next_three_byte)
{
    uint32_t cp;
    int n = tui_utf8_next("\xE2\x82\xAC", 3, &cp);
    TEST_EQ(n, 3);
    TEST_EQ((int)cp, 0x20AC);
}


TEST_BEGIN(utf8_next_four_byte)
{
    uint32_t cp;
    int n = tui_utf8_next("\xF0\x9F\x98\x80", 4, &cp);
    TEST_EQ(n, 4);
    TEST_EQ((int)cp, 0x1F600);
}


TEST_BEGIN(utf8_next_null)
{
    uint32_t cp = 0xDEAD;
    TEST_EQ(tui_utf8_next(NULL, 5, &cp), 0);
}


TEST_BEGIN(utf8_next_zero_len)
{
    uint32_t cp = 0xDEAD;
    TEST_EQ(tui_utf8_next("A", 0, &cp), 0);
}


/* ── utf8_char_width ─────────────────────────────────────────────── */

TEST_BEGIN(utf8_char_width_control)
{
    TEST_EQ(tui_utf8_char_width(0x00), 0);
    TEST_EQ(tui_utf8_char_width(0x0A), 0);
    TEST_EQ(tui_utf8_char_width(0x1F), 0);
    TEST_EQ(tui_utf8_char_width(0x7F), 0);
}


TEST_BEGIN(utf8_char_width_ascii_printable)
{
    TEST_EQ(tui_utf8_char_width('A'), 1);
    TEST_EQ(tui_utf8_char_width('z'), 1);
    TEST_EQ(tui_utf8_char_width(' '), 1);
}


TEST_BEGIN(utf8_char_width_cjk)
{
    TEST_EQ(tui_utf8_char_width(0x4E00), 2);
    TEST_EQ(tui_utf8_char_width(0x3040), 2);
    TEST_EQ(tui_utf8_char_width(0xAC00), 2);
}


TEST_BEGIN(utf8_char_width_emoji)
{
    TEST_EQ(tui_utf8_char_width(0x1F600), 2);
    TEST_EQ(tui_utf8_char_width(0x1F4A9), 2);
}


TEST_BEGIN(utf8_char_width_combining)
{
    TEST_EQ(tui_utf8_char_width(0xFE00), 0);
    TEST_EQ(tui_utf8_char_width(0xFE0F), 0);
}


/* ── text_width / text_str_width ─────────────────────────────────── */

TEST_BEGIN(text_width_ascii)
{
    const char *s = "Hello";
    TEST_EQ(zephio_text_width(s, strlen(s)), 5);
}


TEST_BEGIN(text_width_null)
{
    TEST_EQ(zephio_text_width(NULL, 0), 0);
}


TEST_BEGIN(text_width_mixed)
{
    const char *s = "A\xE2\x82\xAC""Z";
    TEST_EQ(zephio_text_width(s, strlen(s)), 3);
}


TEST_BEGIN(text_width_cjk)
{
    const char *s = "\xE4\xB8\xAD\xE6\x96\x87";
    TEST_EQ(zephio_text_width(s, strlen(s)), 4);
}


TEST_BEGIN(text_width_stops_at_newline)
{
    const char *s = "AB\nCD";
    TEST_EQ(zephio_text_width(s, strlen(s)), 2);
}


TEST_BEGIN(text_str_width_basic)
{
    TEST_EQ(zephio_text_str_width("abc"), 3);
    TEST_EQ(zephio_text_str_width(NULL), 0);
}


/* ── text_clip ───────────────────────────────────────────────────── */

TEST_BEGIN(text_clip_basic)
{
    const char *s = "Hello, world!";
    size_t out_len;
    int w = zephio_text_clip(s, strlen(s), 5, &out_len);
    TEST_EQ(w, 5);
    TEST_EQ((int)out_len, 5);
    TEST_ASSERT(memcmp(s, "Hello", 5) == 0);
}


TEST_BEGIN(text_clip_exact)
{
    const char *s = "Hello";
    size_t out_len;
    int w = zephio_text_clip(s, strlen(s), 5, &out_len);
    TEST_EQ(w, 5);
    TEST_EQ((int)out_len, 5);
}


TEST_BEGIN(text_clip_shorter)
{
    const char *s = "Hi";
    size_t out_len;
    int w = zephio_text_clip(s, strlen(s), 10, &out_len);
    TEST_EQ(w, 2);
    TEST_EQ((int)out_len, 2);
}


TEST_BEGIN(text_clip_zero_width)
{
    size_t out_len = 99;
    int w = zephio_text_clip("Hello", 5, 0, &out_len);
    TEST_EQ(w, 0);
    TEST_EQ((int)out_len, 0);
}


TEST_BEGIN(text_clip_null)
{
    size_t out_len = 99;
    int w = zephio_text_clip(NULL, 0, 5, &out_len);
    TEST_EQ(w, 0);
    TEST_EQ((int)out_len, 0);
}


TEST_BEGIN(text_clip_wide_char)
{
    const char *s = "\xE4\xB8\xAD""AB";
    size_t out_len;
    int w = zephio_text_clip(s, strlen(s), 3, &out_len);
    TEST_EQ(w, 3);
    TEST_EQ((int)out_len, 4);
}


TEST_BEGIN(text_clip_wide_overflow)
{
    const char *s = "A\xE4\xB8\xAD""B";
    size_t out_len;
    int w = zephio_text_clip(s, strlen(s), 2, &out_len);
    TEST_EQ(w, 1);
    TEST_EQ((int)out_len, 1);
}


/* ── text_truncate ───────────────────────────────────────────────── */

TEST_BEGIN(text_truncate_noop)
{
    char out[64];
    zephio_text_truncate("Hi", 2, 10, "~", out, sizeof(out));
    TEST_STR_EQ(out, "Hi");
}


TEST_BEGIN(text_truncate_with_ellipsis)
{
    char out[64];
    zephio_text_truncate("Hello, world!", 13, 7, "...", out, sizeof(out));
    TEST_STR_EQ(out, "Hell...");
}


TEST_BEGIN(text_truncate_null_text)
{
    char out[16] = "X";
    zephio_text_truncate(NULL, 0, 10, "~", out, sizeof(out));
    TEST_EQ(out[0], '\0');
}


TEST_BEGIN(text_truncate_small_buffer)
{
    char out[4];
    zephio_text_truncate("Hello", 5, 10, "~", out, sizeof(out));
    TEST_STR_EQ(out, "Hel");
}


/* ── text_word_wrap ──────────────────────────────────────────────── */

TEST_BEGIN(word_wrap_basic)
{
    const char *s = "Hello world foo";
    int breaks[8];
    int n = zephio_text_word_wrap(s, strlen(s), 6, breaks, 8);
    TEST_EQ(n, 2);
}


TEST_BEGIN(word_wrap_newline)
{
    const char *s = "Hello\nWorld";
    int breaks[4];
    int n = zephio_text_word_wrap(s, strlen(s), 80, breaks, 4);
    TEST_EQ(n, 1);
    TEST_EQ(breaks[0], 5);
}


TEST_BEGIN(word_wrap_null)
{
    int breaks[4];
    TEST_EQ(zephio_text_word_wrap(NULL, 0, 10, breaks, 4), 0);
}


TEST_BEGIN(word_wrap_no_wrap_needed)
{
    const char *s = "Short";
    int breaks[4];
    int n = zephio_text_word_wrap(s, strlen(s), 80, breaks, 4);
    TEST_EQ(n, 0);
}


/* ── text_index_to_col / text_col_to_index ───────────────────────── */

TEST_BEGIN(index_to_col_basic)
{
    const char *s = "Hello";
    TEST_EQ(zephio_text_index_to_col(s, strlen(s), 0), 0);
    TEST_EQ(zephio_text_index_to_col(s, strlen(s), 3), 3);
    TEST_EQ(zephio_text_index_to_col(s, strlen(s), 5), 5);
}


TEST_BEGIN(index_to_col_wide)
{
    const char *s = "\xE4\xB8\xAD""AB";
    TEST_EQ(zephio_text_index_to_col(s, strlen(s), 3), 2);
    TEST_EQ(zephio_text_index_to_col(s, strlen(s), 4), 3);
}


TEST_BEGIN(col_to_index_basic)
{
    const char *s = "Hello";
    TEST_EQ(zephio_text_col_to_index(s, strlen(s), 0), 0);
    TEST_EQ(zephio_text_col_to_index(s, strlen(s), 3), 3);
    TEST_EQ(zephio_text_col_to_index(s, strlen(s), 5), 5);
}


TEST_BEGIN(col_to_index_wide)
{
    const char *s = "\xE4\xB8\xAD""AB";
    TEST_EQ(zephio_text_col_to_index(s, strlen(s), 2), 3);
    TEST_EQ(zephio_text_col_to_index(s, strlen(s), 3), 4);
}


/* ── text_expand_tab / text_expand_tabs ──────────────────────────── */

TEST_BEGIN(expand_tab)
{
    TEST_EQ(zephio_text_expand_tab(0, 8), 8);
    TEST_EQ(zephio_text_expand_tab(4, 8), 4);
    TEST_EQ(zephio_text_expand_tab(7, 8), 1);
    TEST_EQ(zephio_text_expand_tab(8, 8), 8);
}


TEST_BEGIN(expand_tabs_basic)
{
    char out[64];
    int n = zephio_text_expand_tabs("\tHello", 6, 4, out, sizeof(out));
    TEST_EQ(n, 4 + 5);
    TEST_STR_EQ(out, "    Hello");
}


TEST_BEGIN(expand_tabs_mixed)
{
    char out[64];
    int n = zephio_text_expand_tabs("A\tB", 3, 4, out, sizeof(out));
    TEST_EQ(n, 5);
    TEST_STR_EQ(out, "A   B");
}


/* ── main ────────────────────────────────────────────────────────── */

int main(void)
{
    fprintf(stderr, "Running text tests...\n\n");

    TEST_RUN(utf8_valid_ascii);
    TEST_RUN(utf8_valid_empty);
    TEST_RUN(utf8_valid_null);
    TEST_RUN(utf8_valid_two_byte);
    TEST_RUN(utf8_valid_three_byte);
    TEST_RUN(utf8_valid_four_byte);
    TEST_RUN(utf8_valid_truncated);
    TEST_RUN(utf8_valid_invalid_cont);
    TEST_RUN(utf8_valid_surrogate);
    TEST_RUN(utf8_valid_overlong);

    TEST_RUN(utf8_char_len_ascii);
    TEST_RUN(utf8_char_len_lead_bytes);
    TEST_RUN(utf8_char_len_invalid);

    TEST_RUN(utf8_next_ascii);
    TEST_RUN(utf8_next_two_byte);
    TEST_RUN(utf8_next_three_byte);
    TEST_RUN(utf8_next_four_byte);
    TEST_RUN(utf8_next_null);
    TEST_RUN(utf8_next_zero_len);

    TEST_RUN(utf8_char_width_control);
    TEST_RUN(utf8_char_width_ascii_printable);
    TEST_RUN(utf8_char_width_cjk);
    TEST_RUN(utf8_char_width_emoji);
    TEST_RUN(utf8_char_width_combining);

    TEST_RUN(text_width_ascii);
    TEST_RUN(text_width_null);
    TEST_RUN(text_width_mixed);
    TEST_RUN(text_width_cjk);
    TEST_RUN(text_width_stops_at_newline);
    TEST_RUN(text_str_width_basic);

    TEST_RUN(text_clip_basic);
    TEST_RUN(text_clip_exact);
    TEST_RUN(text_clip_shorter);
    TEST_RUN(text_clip_zero_width);
    TEST_RUN(text_clip_null);
    TEST_RUN(text_clip_wide_char);
    TEST_RUN(text_clip_wide_overflow);

    TEST_RUN(text_truncate_noop);
    TEST_RUN(text_truncate_with_ellipsis);
    TEST_RUN(text_truncate_null_text);
    TEST_RUN(text_truncate_small_buffer);

    TEST_RUN(word_wrap_basic);
    TEST_RUN(word_wrap_newline);
    TEST_RUN(word_wrap_null);
    TEST_RUN(word_wrap_no_wrap_needed);

    TEST_RUN(index_to_col_basic);
    TEST_RUN(index_to_col_wide);
    TEST_RUN(col_to_index_basic);
    TEST_RUN(col_to_index_wide);

    TEST_RUN(expand_tab);
    TEST_RUN(expand_tabs_basic);
    TEST_RUN(expand_tabs_mixed);

    TEST_SUMMARY();
}
