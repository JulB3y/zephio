#include "tui.h"
#include "tui_terminal.h"
#include "tui_ansi.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int center_col(int text_len, int cols)
{
    int c = (cols - text_len) / 2 + 1;
    return c < 1 ? 1 : c;
}

static void draw_box_single(int row, int col, int w, int h)
{
    ansi_move_cursor(row, col);
    ansi_write("\xe2\x95\x94", 3);
    for (int i = 0; i < w - 2; i++) {
        ansi_write("\xe2\x95\x90", 3);
    }
    ansi_write("\xe2\x95\x97", 3);

    for (int r = 1; r < h - 1; r++) {
        ansi_move_cursor(row + r, col);
        ansi_write("\xe2\x95\x91", 3);
        for (int i = 0; i < w - 2; i++) {
            ansi_write(" ", 1);
        }
        ansi_write("\xe2\x95\x91", 3);
    }

    ansi_move_cursor(row + h - 1, col);
    ansi_write("\xe2\x95\x9a", 3);
    for (int i = 0; i < w - 2; i++) {
        ansi_write("\xe2\x95\x90", 3);
    }
    ansi_write("\xe2\x95\x9d", 3);
}

static void draw_color_bar(int row, int col, int width)
{
    for (int i = 0; i < 16 && i < width; i++) {
        ansi_move_cursor(row, col + i * 3);
        ansi_set_bg(i);
        ansi_write("   ", 3);
    }
    ansi_reset();
}

int main(void)
{
    TuiResult res;
    TuiSize size;

    res = tui_init();
    if (res != TUI_OK) {
        fprintf(stderr, "tui_init failed: %d\n", res);
        return 1;
    }

    res = tui_get_size(&size);
    if (res != TUI_OK) {
        tui_shutdown();
        return 1;
    }

    int row = size.rows / 2 - 6;
    int col;

    col = center_col(42, size.cols);
    ansi_write_at(row, col, "Phase 1 - ANSI & Terminal Abstraction Demo", 42);
    ansi_reset();

    col = center_col(30, size.cols);
    ansi_move_cursor(row + 2, col);
    ansi_set_bold();
    ansi_set_fg(2);
    ansi_write("Hello from TUI framework!", 26);
    ansi_reset();

    draw_box_single(row + 4, center_col(40, size.cols), 40, 3);
    ansi_move_cursor(row + 5, center_col(32, size.cols));
    ansi_set_italic();
    ansi_set_fg(6);
    ansi_write("Text inside a box with italic style", 34);
    ansi_reset();

    ansi_move_cursor(row + 8, center_col(24, size.cols));
    ansi_set_bold();
    ansi_write("Bold ", 5);
    ansi_reset();
    ansi_set_dim();
    ansi_write("Dim ", 4);
    ansi_reset();
    ansi_set_underline();
    ansi_write("Underline ", 10);
    ansi_reset();
    ansi_set_italic();
    ansi_write("Italic ", 7);
    ansi_reset();
    ansi_set_reverse();
    ansi_write("Reverse", 7);
    ansi_reset();

    draw_color_bar(row + 10, center_col(48, size.cols), size.cols);

    col = center_col(16, size.cols);
    ansi_move_cursor(row + 12, col);
    ansi_write("RGB: ", 5);
    ansi_set_fg_rgb(255, 100, 50);
    ansi_write("Orange", 6);
    ansi_write(" ", 1);
    ansi_set_fg_rgb(50, 200, 255);
    ansi_write("Cyan", 4);
    ansi_write(" ", 1);
    ansi_set_fg_rgb(100, 255, 100);
    ansi_write("Green", 5);
    ansi_reset();

    ansi_save_cursor();
    ansi_move_cursor(size.rows - 1, 2);
    ansi_set_fg(8);
    char info[64];
    int info_len = snprintf(info, sizeof(info), "Terminal: %dx%d  |  Press 'q' to quit", size.cols, size.rows);
    ansi_write(info, (size_t)info_len);
    ansi_reset();
    ansi_restore_cursor();

    while (1) {
        char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n <= 0) {
            break;
        }
        if (c == 'q') {
            break;
        }
    }

    tui_shutdown();
    return 0;
}
