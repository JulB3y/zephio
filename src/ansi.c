#include "zephio_terminal.h"
#include "zephio_ansi.h"
#include "zephio_context.h"

#include <stdio.h>
#include <unistd.h>

void ansi_move_cursor(ZephioContext *ctx, int row, int col)
{
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "\033[%d;%dH", row, col);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_move_up(ZephioContext *ctx, int n)
{
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "\033[%dA", n);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_move_down(ZephioContext *ctx, int n)
{
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "\033[%dB", n);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_move_left(ZephioContext *ctx, int n)
{
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "\033[%dD", n);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_move_right(ZephioContext *ctx, int n)
{
    char buf[16];
    int len = snprintf(buf, sizeof(buf), "\033[%dC", n);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_save_cursor(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_SAVE_CURSOR, sizeof(ANSI_SAVE_CURSOR) - 1);
}

void ansi_restore_cursor(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_RESTORE_CURSOR, sizeof(ANSI_RESTORE_CURSOR) - 1);
}

void ansi_clear_screen(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_CLEAR_SCREEN, sizeof(ANSI_CLEAR_SCREEN) - 1);
    terminal_write_seq(&ctx->terminal, ANSI_CURSOR_HOME, sizeof(ANSI_CURSOR_HOME) - 1);
}

void ansi_clear_line(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_CLEAR_LINE, sizeof(ANSI_CLEAR_LINE) - 1);
}

void ansi_clear_line_right(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_CLEAR_LINE_RIGHT, sizeof(ANSI_CLEAR_LINE_RIGHT) - 1);
}

void ansi_clear_line_left(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_CLEAR_LINE_LEFT, sizeof(ANSI_CLEAR_LINE_LEFT) - 1);
}

void ansi_clear_screen_above(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_CLEAR_SCREEN_ABOVE, sizeof(ANSI_CLEAR_SCREEN_ABOVE) - 1);
}

void ansi_clear_screen_below(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_CLEAR_SCREEN_BELOW, sizeof(ANSI_CLEAR_SCREEN_BELOW) - 1);
}

void ansi_set_fg(ZephioContext *ctx, int color)
{
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "\033[38;5;%dm", color);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_set_bg(ZephioContext *ctx, int color)
{
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "\033[48;5;%dm", color);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_set_fg_rgb(ZephioContext *ctx, int r, int g, int b)
{
    char buf[48];
    int len = snprintf(buf, sizeof(buf), "\033[38;2;%d;%d;%dm", r, g, b);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_set_bg_rgb(ZephioContext *ctx, int r, int g, int b)
{
    char buf[48];
    int len = snprintf(buf, sizeof(buf), "\033[48;2;%d;%d;%dm", r, g, b);
    terminal_write_seq(&ctx->terminal, buf, (size_t)len);
}

void ansi_reset(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_RESET, sizeof(ANSI_RESET) - 1);
}

void ansi_set_bold(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_BOLD, sizeof(ANSI_BOLD) - 1);
}

void ansi_set_dim(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_DIM, sizeof(ANSI_DIM) - 1);
}

void ansi_set_italic(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_ITALIC, sizeof(ANSI_ITALIC) - 1);
}

void ansi_set_underline(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_UNDERLINE, sizeof(ANSI_UNDERLINE) - 1);
}

void ansi_set_blink(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_BLINK, sizeof(ANSI_BLINK) - 1);
}

void ansi_set_reverse(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_REVERSE, sizeof(ANSI_REVERSE) - 1);
}

void ansi_set_strikethrough(ZephioContext *ctx)
{
    terminal_write_seq(&ctx->terminal, ANSI_STRIKETHROUGH, sizeof(ANSI_STRIKETHROUGH) - 1);
}

void ansi_write(ZephioContext *ctx, const char *text, size_t len)
{
    terminal_write_seq(&ctx->terminal, text, len);
}

void ansi_write_at(ZephioContext *ctx, int row, int col, const char *text, size_t len)
{
    ansi_move_cursor(ctx, row, col);
    terminal_write_seq(&ctx->terminal, text, len);
}
