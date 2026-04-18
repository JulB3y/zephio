#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "tui_terminal.h"
#include "tui_ansi.h"
#include "tui_input.h"
#include "tui_context.h"

#include <stdio.h>
#include <string.h>

static int center_col(int text_len, int cols)
{
    int c = (cols - text_len) / 2 + 1;
    return c < 1 ? 1 : c;
}

static void draw_box_single(TuiContext *ctx, int row, int col, int w, int h)
{
    ansi_move_cursor(ctx, row, col);
    ansi_write(ctx, "\xe2\x95\x94", 3);
    for (int i = 0; i < w - 2; i++) {
        ansi_write(ctx, "\xe2\x95\x90", 3);
    }
    ansi_write(ctx, "\xe2\x95\x97", 3);

    for (int r = 1; r < h - 1; r++) {
        ansi_move_cursor(ctx, row + r, col);
        ansi_write(ctx, "\xe2\x95\x91", 3);
        for (int i = 0; i < w - 2; i++) {
            ansi_write(ctx, " ", 1);
        }
        ansi_write(ctx, "\xe2\x95\x91", 3);
    }

    ansi_move_cursor(ctx, row + h - 1, col);
    ansi_write(ctx, "\xe2\x95\x9a", 3);
    for (int i = 0; i < w - 2; i++) {
        ansi_write(ctx, "\xe2\x95\x90", 3);
    }
    ansi_write(ctx, "\xe2\x95\x9d", 3);
}

static void draw_color_bar(TuiContext *ctx, int row, int col, int width)
{
    for (int i = 0; i < 16 && i < width; i++) {
        ansi_move_cursor(ctx, row, col + i * 3);
        ansi_set_bg(ctx, i);
        ansi_write(ctx, "   ", 3);
    }
    ansi_reset(ctx);
}

static void draw(TuiContext *ctx, int rows, int cols)
{
    ansi_clear_screen(ctx);

    int row = rows / 2 - 6;
    int col;

    col = center_col(42, cols);
    ansi_write_at(ctx, row, col, "Phase 1 - ANSI & Terminal Abstraction Demo", 42);
    ansi_reset(ctx);

    col = center_col(30, cols);
    ansi_move_cursor(ctx, row + 2, col);
    ansi_set_bold(ctx);
    ansi_set_fg(ctx, 2);
    ansi_write(ctx, "Hello from TUI framework!", 26);
    ansi_reset(ctx);

    draw_box_single(ctx, row + 4, center_col(40, cols), 40, 3);
    ansi_move_cursor(ctx, row + 5, center_col(32, cols));
    ansi_set_italic(ctx);
    ansi_set_fg(ctx, 6);
    ansi_write(ctx, "Text inside a box with italic style", 34);
    ansi_reset(ctx);

    ansi_move_cursor(ctx, row + 8, center_col(24, cols));
    ansi_set_bold(ctx);
    ansi_write(ctx, "Bold ", 5);
    ansi_reset(ctx);
    ansi_set_dim(ctx);
    ansi_write(ctx, "Dim ", 4);
    ansi_reset(ctx);
    ansi_set_underline(ctx);
    ansi_write(ctx, "Underline ", 10);
    ansi_reset(ctx);
    ansi_set_italic(ctx);
    ansi_write(ctx, "Italic ", 7);
    ansi_reset(ctx);
    ansi_set_reverse(ctx);
    ansi_write(ctx, "Reverse", 7);
    ansi_reset(ctx);

    draw_color_bar(ctx, row + 10, center_col(48, cols), cols);

    col = center_col(16, cols);
    ansi_move_cursor(ctx, row + 12, col);
    ansi_write(ctx, "RGB: ", 5);
    ansi_set_fg_rgb(ctx, 255, 100, 50);
    ansi_write(ctx, "Orange", 6);
    ansi_write(ctx, " ", 1);
    ansi_set_fg_rgb(ctx, 50, 200, 255);
    ansi_write(ctx, "Cyan", 4);
    ansi_write(ctx, " ", 1);
    ansi_set_fg_rgb(ctx, 100, 255, 100);
    ansi_write(ctx, "Green", 5);
    ansi_reset(ctx);

    ansi_move_cursor(ctx, rows - 1, 2);
    ansi_set_fg(ctx, 8);
    char info[64];
    int info_len = snprintf(info, sizeof(info), "Terminal: %dx%d  |  Press 'q' to quit", cols, rows);
    ansi_write(ctx, info, (size_t)info_len);
    ansi_reset(ctx);
}

static int input_callback(const TuiEvent *event, void *user_data)
{
    TuiContext *ctx = (TuiContext *)user_data;

    if (event->key == TUI_EVENT_RESIZE) {
        TuiSize size = {0, 0};
        if (tui_get_size(ctx, &size) == TUI_OK) {
            draw(ctx, size.rows, size.cols);
        }
        return 0;
    }

    if (event->key == TUI_KEY_ESCAPE || event->key == TUI_KEY_CTRL_C) {
        return 1;
    }
    if (event->codepoint == 'q' && event->modifiers == TUI_MOD_NONE) {
        return 1;
    }

    return 0;
}

int main(void)
{
    TuiContext ctx;
    TuiResult res = tui_init(&ctx);
    if (res != TUI_OK) {
        fprintf(stderr, "tui_init failed: %d\n", res);
        return 1;
    }

    tui_input_init(&ctx);

    TuiSize size = {0, 0};
    if (tui_get_size(&ctx, &size) == TUI_OK) {
        draw(&ctx, size.rows, size.cols);
    }

    tui_input_loop(&ctx, input_callback, &ctx);

    tui_input_shutdown(&ctx);
    tui_shutdown(&ctx);
    return 0;
}
