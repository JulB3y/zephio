#include "tui_terminal.h"
#include "tui_ansi.h"

#include <stdio.h>
#include <unistd.h>

void ansi_move_cursor(int row, int col)
{
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "\033[%d;%dH", row, col);
    terminal_write_seq(&g_terminal, buf, (size_t)len);
}

void ansi_move_up(int n)
{
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "\033[%dA", n);
    terminal_write_seq(&g_terminal, buf, (size_t)len);
}

void ansi_move_down(int n)
{
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "\033[%dB", n);
    terminal_write_seq(&g_terminal, buf, (size_t)len);
}

void ansi_move_left(int n)
{
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "\033[%dD", n);
    terminal_write_seq(&g_terminal, buf, (size_t)len);
}

void ansi_move_right(int n)
{
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "\033[%dC", n);
    terminal_write_seq(&g_terminal, buf, (size_t)len);
}

void ansi_save_cursor(void)
{
    terminal_write_seq(&g_terminal, ANSI_SAVE_CURSOR, sizeof(ANSI_SAVE_CURSOR) - 1);
}

void ansi_restore_cursor(void)
{
    terminal_write_seq(&g_terminal, ANSI_RESTORE_CURSOR, sizeof(ANSI_RESTORE_CURSOR) - 1);
}

void ansi_clear_screen(void)
{
    terminal_write_seq(&g_terminal, ANSI_CLEAR_SCREEN, sizeof(ANSI_CLEAR_SCREEN) - 1);
    terminal_write_seq(&g_terminal, ANSI_CURSOR_HOME, sizeof(ANSI_CURSOR_HOME) - 1);
}

void ansi_clear_line(void)
{
    terminal_write_seq(&g_terminal, ANSI_CLEAR_LINE, sizeof(ANSI_CLEAR_LINE) - 1);
}

void ansi_clear_line_right(void)
{
    terminal_write_seq(&g_terminal, ANSI_CLEAR_LINE_RIGHT, sizeof(ANSI_CLEAR_LINE_RIGHT) - 1);
}

void ansi_clear_line_left(void)
{
    terminal_write_seq(&g_terminal, ANSI_CLEAR_LINE_LEFT, sizeof(ANSI_CLEAR_LINE_LEFT) - 1);
}

void ansi_clear_screen_above(void)
{
    terminal_write_seq(&g_terminal, ANSI_CLEAR_SCREEN_ABOVE, sizeof(ANSI_CLEAR_SCREEN_ABOVE) - 1);
}

void ansi_clear_screen_below(void)
{
    terminal_write_seq(&g_terminal, ANSI_CLEAR_SCREEN_BELOW, sizeof(ANSI_CLEAR_SCREEN_BELOW) - 1);
}

void ansi_set_fg(int color)
{
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "\033[38;5;%dm", color);
    terminal_write_seq(&g_terminal, buf, (size_t)len);
}

void ansi_set_bg(int color)
{
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "\033[48;5;%dm", color);
    terminal_write_seq(&g_terminal, buf, (size_t)len);
}

void ansi_set_fg_rgb(int r, int g, int b)
{
    char buf[48];
    int len = snprintf(buf, sizeof(buf), "\033[38;2;%d;%d;%dm", r, g, b);
    terminal_write_seq(&g_terminal, buf, (size_t)len);
}

void ansi_set_bg_rgb(int r, int g, int b)
{
    char buf[48];
    int len = snprintf(buf, sizeof(buf), "\033[48;2;%d;%d;%dm", r, g, b);
    terminal_write_seq(&g_terminal, buf, (size_t)len);
}

void ansi_reset(void)
{
    terminal_write_seq(&g_terminal, ANSI_RESET, sizeof(ANSI_RESET) - 1);
}

void ansi_set_bold(void)
{
    terminal_write_seq(&g_terminal, ANSI_BOLD, sizeof(ANSI_BOLD) - 1);
}

void ansi_set_dim(void)
{
    terminal_write_seq(&g_terminal, ANSI_DIM, sizeof(ANSI_DIM) - 1);
}

void ansi_set_italic(void)
{
    terminal_write_seq(&g_terminal, ANSI_ITALIC, sizeof(ANSI_ITALIC) - 1);
}

void ansi_set_underline(void)
{
    terminal_write_seq(&g_terminal, ANSI_UNDERLINE, sizeof(ANSI_UNDERLINE) - 1);
}

void ansi_set_blink(void)
{
    terminal_write_seq(&g_terminal, ANSI_BLINK, sizeof(ANSI_BLINK) - 1);
}

void ansi_set_reverse(void)
{
    terminal_write_seq(&g_terminal, ANSI_REVERSE, sizeof(ANSI_REVERSE) - 1);
}

void ansi_set_strikethrough(void)
{
    terminal_write_seq(&g_terminal, ANSI_STRIKETHROUGH, sizeof(ANSI_STRIKETHROUGH) - 1);
}

void ansi_write(const char *text, size_t len)
{
    terminal_write_seq(&g_terminal, text, len);
}

void ansi_write_at(int row, int col, const char *text, size_t len)
{
    ansi_move_cursor(row, col);
    terminal_write_seq(&g_terminal, text, len);
}
