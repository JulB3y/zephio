#define TUI_TEST_CAPTURE
#include "util.h"
#include "tui_ansi.h"

/* ── cursor movement ────────────────────────────────────────────── */

TEST_BEGIN(ansi_move_cursor)
{
    capture_start();
    ansi_move_cursor(5, 10);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[5;10H");
}

TEST_BEGIN(ansi_move_up)
{
    capture_start();
    ansi_move_up(3);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[3A");
}

TEST_BEGIN(ansi_move_down)
{
    capture_start();
    ansi_move_down(2);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[2B");
}

TEST_BEGIN(ansi_move_left)
{
    capture_start();
    ansi_move_left(1);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[1D");
}

TEST_BEGIN(ansi_move_right)
{
    capture_start();
    ansi_move_right(4);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[4C");
}

/* ── cursor save/restore ────────────────────────────────────────── */

TEST_BEGIN(ansi_save_cursor)
{
    capture_start();
    ansi_save_cursor();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[s");
}

TEST_BEGIN(ansi_restore_cursor)
{
    capture_start();
    ansi_restore_cursor();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[u");
}

/* ── clear sequences ────────────────────────────────────────────── */

TEST_BEGIN(ansi_clear_screen)
{
    capture_start();
    ansi_clear_screen();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[2J\033[H");
}

TEST_BEGIN(ansi_clear_line)
{
    capture_start();
    ansi_clear_line();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[2K");
}

TEST_BEGIN(ansi_clear_line_right)
{
    capture_start();
    ansi_clear_line_right();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[K");
}

TEST_BEGIN(ansi_clear_line_left)
{
    capture_start();
    ansi_clear_line_left();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[1K");
}

TEST_BEGIN(ansi_clear_screen_above)
{
    capture_start();
    ansi_clear_screen_above();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[1J");
}

TEST_BEGIN(ansi_clear_screen_below)
{
    capture_start();
    ansi_clear_screen_below();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[0J");
}

/* ── color sequences ────────────────────────────────────────────── */

TEST_BEGIN(ansi_set_fg)
{
    capture_start();
    ansi_set_fg(15);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[38;5;15m");
}

TEST_BEGIN(ansi_set_bg)
{
    capture_start();
    ansi_set_bg(0);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[48;5;0m");
}

TEST_BEGIN(ansi_set_fg_rgb)
{
    capture_start();
    ansi_set_fg_rgb(255, 128, 0);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[38;2;255;128;0m");
}

TEST_BEGIN(ansi_set_bg_rgb)
{
    capture_start();
    ansi_set_bg_rgb(100, 200, 50);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[48;2;100;200;50m");
}

/* ── attribute sequences ────────────────────────────────────────── */

TEST_BEGIN(ansi_reset)
{
    capture_start();
    ansi_reset();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[0m");
}

TEST_BEGIN(ansi_set_bold)
{
    capture_start();
    ansi_set_bold();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[1m");
}

TEST_BEGIN(ansi_set_dim)
{
    capture_start();
    ansi_set_dim();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[2m");
}

TEST_BEGIN(ansi_set_italic)
{
    capture_start();
    ansi_set_italic();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[3m");
}

TEST_BEGIN(ansi_set_underline)
{
    capture_start();
    ansi_set_underline();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[4m");
}

TEST_BEGIN(ansi_set_blink)
{
    capture_start();
    ansi_set_blink();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[5m");
}

TEST_BEGIN(ansi_set_reverse)
{
    capture_start();
    ansi_set_reverse();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[7m");
}

TEST_BEGIN(ansi_set_strikethrough)
{
    capture_start();
    ansi_set_strikethrough();
    capture_drain();
    TEST_STR_EQ(g_output_buf, "\033[9m");
}

/* ── write ──────────────────────────────────────────────────────── */

TEST_BEGIN(ansi_write_text)
{
    capture_start();
    ansi_write("Hello", 5);
    capture_drain();
    TEST_STR_EQ(g_output_buf, "Hello");
}

TEST_BEGIN(ansi_write_at)
{
    capture_start();
    ansi_write_at(3, 7, "X", 1);
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
