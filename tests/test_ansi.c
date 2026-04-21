#define ZEPHIO_TEST_CAPTURE
#include "util.h"
#include "zephio_ansi.h"
#include "zephio_context.h"

extern ZephioContext g_test_ctx;

/* ── cursor movement ────────────────────────────────────────────── */

TEST_BEGIN(ansi_move_cursor)
{
    capture_start();
    ansi_move_cursor(&g_test_ctx, 5, 10);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[5;10H");
}

TEST_BEGIN(ansi_move_up)
{
    capture_start();
    ansi_move_up(&g_test_ctx, 3);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[3A");
}

TEST_BEGIN(ansi_move_down)
{
    capture_start();
    ansi_move_down(&g_test_ctx, 2);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[2B");
}

TEST_BEGIN(ansi_move_left)
{
    capture_start();
    ansi_move_left(&g_test_ctx, 1);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[1D");
}

TEST_BEGIN(ansi_move_right)
{
    capture_start();
    ansi_move_right(&g_test_ctx, 4);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[4C");
}

/* ── cursor save/restore ────────────────────────────────────────── */

TEST_BEGIN(ansi_save_cursor)
{
    capture_start();
    ansi_save_cursor(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[s");
}

TEST_BEGIN(ansi_restore_cursor)
{
    capture_start();
    ansi_restore_cursor(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[u");
}

/* ── clear sequences ────────────────────────────────────────────── */

TEST_BEGIN(ansi_clear_screen)
{
    capture_start();
    ansi_clear_screen(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[2J\033[H");
}

TEST_BEGIN(ansi_clear_line)
{
    capture_start();
    ansi_clear_line(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[2K");
}

TEST_BEGIN(ansi_clear_line_right)
{
    capture_start();
    ansi_clear_line_right(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[K");
}

TEST_BEGIN(ansi_clear_line_left)
{
    capture_start();
    ansi_clear_line_left(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[1K");
}

TEST_BEGIN(ansi_clear_screen_above)
{
    capture_start();
    ansi_clear_screen_above(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[1J");
}

TEST_BEGIN(ansi_clear_screen_below)
{
    capture_start();
    ansi_clear_screen_below(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[0J");
}

/* ── color sequences ────────────────────────────────────────────── */

TEST_BEGIN(ansi_set_fg)
{
    capture_start();
    ansi_set_fg(&g_test_ctx, 15);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[38;5;15m");
}

TEST_BEGIN(ansi_set_bg)
{
    capture_start();
    ansi_set_bg(&g_test_ctx, 0);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[48;5;0m");
}

TEST_BEGIN(ansi_set_fg_rgb)
{
    capture_start();
    ansi_set_fg_rgb(&g_test_ctx, 255, 128, 0);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[38;2;255;128;0m");
}

TEST_BEGIN(ansi_set_bg_rgb)
{
    capture_start();
    ansi_set_bg_rgb(&g_test_ctx, 100, 200, 50);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[48;2;100;200;50m");
}

/* ── attribute sequences ────────────────────────────────────────── */

TEST_BEGIN(ansi_reset)
{
    capture_start();
    ansi_reset(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[0m");
}

TEST_BEGIN(ansi_set_bold)
{
    capture_start();
    ansi_set_bold(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[1m");
}

TEST_BEGIN(ansi_set_dim)
{
    capture_start();
    ansi_set_dim(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[2m");
}

TEST_BEGIN(ansi_set_italic)
{
    capture_start();
    ansi_set_italic(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[3m");
}

TEST_BEGIN(ansi_set_underline)
{
    capture_start();
    ansi_set_underline(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[4m");
}

TEST_BEGIN(ansi_set_blink)
{
    capture_start();
    ansi_set_blink(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[5m");
}

TEST_BEGIN(ansi_set_reverse)
{
    capture_start();
    ansi_set_reverse(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[7m");
}

TEST_BEGIN(ansi_set_strikethrough)
{
    capture_start();
    ansi_set_strikethrough(&g_test_ctx);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[9m");
}

/* ── write ──────────────────────────────────────────────────────── */

TEST_BEGIN(ansi_write_text)
{
    capture_start();
    ansi_write(&g_test_ctx, "Hello", 5);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "Hello");
}

TEST_BEGIN(ansi_write_at)
{
    capture_start();
    ansi_write_at(&g_test_ctx, 3, 7, "X", 1);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[3;7HX");
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void)
{
    fprintf(stderr, "Running ansi tests...\n\n");

    TEST_RUN(ansi_move_cursor);
    TEST_RUN(ansi_move_up);
    TEST_RUN(ansi_move_down);
    TEST_RUN(ansi_move_left);
    TEST_RUN(ansi_move_right);

    TEST_RUN(ansi_save_cursor);
    TEST_RUN(ansi_restore_cursor);

    TEST_RUN(ansi_clear_screen);
    TEST_RUN(ansi_clear_line);
    TEST_RUN(ansi_clear_line_right);
    TEST_RUN(ansi_clear_line_left);
    TEST_RUN(ansi_clear_screen_above);
    TEST_RUN(ansi_clear_screen_below);

    TEST_RUN(ansi_set_fg);
    TEST_RUN(ansi_set_bg);
    TEST_RUN(ansi_set_fg_rgb);
    TEST_RUN(ansi_set_bg_rgb);

    TEST_RUN(ansi_reset);
    TEST_RUN(ansi_set_bold);
    TEST_RUN(ansi_set_dim);
    TEST_RUN(ansi_set_italic);
    TEST_RUN(ansi_set_underline);
    TEST_RUN(ansi_set_blink);
    TEST_RUN(ansi_set_reverse);
    TEST_RUN(ansi_set_strikethrough);

    TEST_RUN(ansi_write_text);
    TEST_RUN(ansi_write_at);

    capture_done();

    TEST_SUMMARY();
}
