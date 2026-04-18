#include "tui_terminal.h"
#include "tui_ansi.h"
#include "tui_context.h"

#include <stdio.h>
#include <unistd.h>

void ansi_move_cursor(TuiContext *ctx, int row, int col)
{
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "\033[%d;%dH", row, col);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_move_up(TuiContext *ctx, int n)
{
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "\033[%dA", n);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_move_down(TuiContext *ctx, int n)
{
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "\033[%dB", n);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_move_left(TuiContext *ctx, int n)
{
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "\033[%dD", n);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_move_right(TuiContext *ctx, int n)
{
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "\033[%dC", n);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_save_cursor(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_SAVE_CURSOR, sizeof(ANSI_SAVE_CURSOR) - 1);
}

void ansi_restore_cursor(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_RESTORE_CURSOR, sizeof(ANSI_RESTORE_CURSOR) - 1);
}

void ansi_clear_screen(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_CLEAR_SCREEN, sizeof(ANSI_CLEAR_SCREEN) - 1);
    terminal_write_seq(&ctx->terminal, ANSI_CURSOR_HOME, sizeof(ANSI_CURSOR_HOME) - 1);
}

void ansi_clear_line(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_CLEAR_LINE, sizeof(ANSI_CLEAR_LINE) - 1);
}

void ansi_clear_line_right(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_CLEAR_LINE_RIGHT, sizeof(ANSI_CLEAR_LINE_RIGHT) - 1);
}

void ansi_clear_line_left(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_CLEAR_LINE_LEFT, sizeof(ANSI_CLEAR_LINE_LEFT) - 1);
}

void ansi_clear_screen_above(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_CLEAR_SCREEN_ABOVE, sizeof(ANSI_CLEAR_SCREEN_ABOVE) - 1);
}

void ansi_clear_screen_below(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_CLEAR_SCREEN_BELOW, sizeof(ANSI_CLEAR_SCREEN_BELOW) - 1);
}

void ansi_set_fg(TuiContext *ctx, int color)
{
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "\033[38;5;%dm", color);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_set_bg(TuiContext *ctx, int color)
{
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "\033[48;5;%dm", color);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_set_fg_rgb(TuiContext *ctx, int r, int g, int b)
{
    char buf[48];
    int len = snprintf(buf, sizeof(buf), "\033[38;2;%d;%d;%dm", r, g, b);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_set_bg_rgb(TuiContext *ctx, int r, int g, int b)
{
    char buf[48];
    int len = snprintf(buf, sizeof(buf), "\033[48;2;%d;%d;%dm", r, g, b);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_reset(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_RESET, sizeof(ANSI_RESET) - 1);
}

void ansi_set_bold(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_BOLD, sizeof(ANSI_BOLD) - 1);
}

void ansi_set_dim(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_DIM, sizeof(ANSI_DIM) - 1);
}

void ansi_set_italic(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_ITALIC, sizeof(ANSI_ITALIC) - 1);
}

void ansi_set_underline(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_UNDERLINE, sizeof(ANSI_UNDERLINE) - 1);
}

void ansi_set_blink(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_BLINK, sizeof(ANSI_BLINK) - 1);
}

void ansi_set_reverse(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_REVERSE, sizeof(ANSI_REVERSE) - 1);
}

void ansi_set_strikethrough(TuiContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_STRIKETHROUGH, sizeof(ANSI_STRIKETHROUGH) - 1);
}

void ansi_write(TuiContext *ctx, const char *text, size_t len)
{
    terminal_write_seq(&ctx->terminal, text, len);
}

void ansi_write_at(TuiContext *ctx, int row, int col, const char *text, size_t len)
{
    ansi_move_cursor(ctx, row, col);
    terminal_write_seq(&ctx->terminal, text, len);
}
