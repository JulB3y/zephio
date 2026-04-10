#define _POSIX_C_SOURCE 200809L

#include "tui_screen.h"
#include "tui_terminal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TuiScreen g_screen = {
    .front = NULL,
    .back = NULL,
    .rows = 0,
    .cols = 0,
    .initialized = 0
};

static TuiCell DEFAULT_CELL;

static int default_cell_ready = 0;

static void init_default_cell(void)
{
    if (default_cell_ready) return;
    DEFAULT_CELL.ch[0] = ' ';
    DEFAULT_CELL.ch[1] = 0;
    DEFAULT_CELL.ch[2] = 0;
    DEFAULT_CELL.ch[3] = 0;
    DEFAULT_CELL.fg.type = TUI_COLOR_TYPE_INDEX;
    DEFAULT_CELL.fg.index = 0;
    DEFAULT_CELL.bg.type = TUI_COLOR_TYPE_INDEX;
    DEFAULT_CELL.bg.index = 0;
    DEFAULT_CELL.attr = 0;
    default_cell_ready = 1;
}

static int utf8_char_len(unsigned char c)
{
    if ((c & 0x80) == 0x00) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

static void cell_set_char(TuiCell *cell, const char *ch)
{
    int len = utf8_char_len((unsigned char)ch[0]);
    memset(cell->ch, 0, 4);
    memcpy(cell->ch, ch, len);
}

static void clear_buffer(TuiCell *buf, int count)
{
    init_default_cell();
    for (int i = 0; i < count; i++) {
        buf[i] = DEFAULT_CELL;
    }
}

TuiResult tui_screen_init(int rows, int cols)
{
    if (rows <= 0 || cols <= 0) {
        return TUI_ERR_MEMORY;
    }

    if (g_screen.initialized) {
        tui_screen_free();
    }

    int total = rows * cols;
    size_t buf_size = (size_t)total * sizeof(TuiCell);

    g_screen.front = (TuiCell *)malloc(buf_size);
    if (!g_screen.front) {
        return TUI_ERR_MEMORY;
    }

    g_screen.back = (TuiCell *)malloc(buf_size);
    if (!g_screen.back) {
        free(g_screen.front);
        g_screen.front = NULL;
        return TUI_ERR_MEMORY;
    }

    clear_buffer(g_screen.front, total);
    clear_buffer(g_screen.back, total);

    g_screen.rows = rows;
    g_screen.cols = cols;
    g_screen.initialized = 1;

    return TUI_OK;
}

void tui_screen_free(void)
{
    if (!g_screen.initialized) {
        return;
    }

    free(g_screen.front);
    free(g_screen.back);

    g_screen.front = NULL;
    g_screen.back = NULL;
    g_screen.rows = 0;
    g_screen.cols = 0;
    g_screen.initialized = 0;
}

TuiResult tui_screen_resize(int rows, int cols)
{
    if (!g_screen.initialized) {
        return tui_screen_init(rows, cols);
    }

    if (rows <= 0 || cols <= 0) {
        return TUI_ERR_MEMORY;
    }

    if (rows == g_screen.rows && cols == g_screen.cols) {
        return TUI_OK;
    }

    TuiCell *new_front = (TuiCell *)malloc((size_t)rows * (size_t)cols * sizeof(TuiCell));
    if (!new_front) {
        return TUI_ERR_MEMORY;
    }

    TuiCell *new_back = (TuiCell *)malloc((size_t)rows * (size_t)cols * sizeof(TuiCell));
    if (!new_back) {
        free(new_front);
        return TUI_ERR_MEMORY;
    }

    clear_buffer(new_front, rows * cols);
    clear_buffer(new_back, rows * cols);

    int copy_rows = rows < g_screen.rows ? rows : g_screen.rows;
    int copy_cols = cols < g_screen.cols ? cols : g_screen.cols;

    for (int r = 0; r < copy_rows; r++) {
        TuiCell *old_front_row = g_screen.front + r * g_screen.cols;
        TuiCell *old_back_row = g_screen.back + r * g_screen.cols;
        TuiCell *new_front_row = new_front + r * cols;
        TuiCell *new_back_row = new_back + r * cols;

        memcpy(new_front_row, old_front_row, (size_t)copy_cols * sizeof(TuiCell));
        memcpy(new_back_row, old_back_row, (size_t)copy_cols * sizeof(TuiCell));
    }

    free(g_screen.front);
    free(g_screen.back);

    g_screen.front = new_front;
    g_screen.back = new_back;
    g_screen.rows = rows;
    g_screen.cols = cols;

    return TUI_OK;
}

void tui_screen_clear(void)
{
    if (!g_screen.initialized) return;
    clear_buffer(g_screen.back, g_screen.rows * g_screen.cols);
}

void tui_screen_set_cell(int row, int col, const char *ch, TuiColor fg, TuiColor bg, TuiAttr attr)
{
    if (!g_screen.initialized) return;
    if (row < 0 || row >= g_screen.rows) return;
    if (col < 0 || col >= g_screen.cols) return;
    if (!ch) return;

    TuiCell *cell = &g_screen.back[row * g_screen.cols + col];
    cell_set_char(cell, ch);
    cell->fg = fg;
    cell->bg = bg;
    cell->attr = attr;
}

void tui_screen_write(int row, int col, const char *text, TuiColor fg, TuiColor bg, TuiAttr attr)
{
    if (!g_screen.initialized || !text) return;

    int c = col;
    const unsigned char *p = (const unsigned char *)text;

    while (*p && c < g_screen.cols) {
        int len = utf8_char_len(*p);

        if (row >= 0 && row < g_screen.rows && c >= 0) {
            TuiCell *cell = &g_screen.back[row * g_screen.cols + c];
            memset(cell->ch, 0, 4);
            memcpy(cell->ch, p, len);
            cell->fg = fg;
            cell->bg = bg;
            cell->attr = attr;
        }

        p += len;
        c++;
    }
}

void tui_screen_fill(int row, int col, int width, int height, const char *ch, TuiColor fg, TuiColor bg, TuiAttr attr)
{
    if (!g_screen.initialized || !ch) return;

    for (int r = row; r < row + height; r++) {
        for (int c = col; c < col + width; c++) {
            tui_screen_set_cell(r, c, ch, fg, bg, attr);
        }
    }
}

void tui_screen_box_single(int row, int col, int width, int height, TuiColor fg, TuiColor bg, TuiAttr attr)
{
    if (!g_screen.initialized || width < 2 || height < 2) return;

    tui_screen_set_cell(row, col, "\xe2\x94\x8c", fg, bg, attr);
    tui_screen_set_cell(row, col + width - 1, "\xe2\x94\x90", fg, bg, attr);
    tui_screen_set_cell(row + height - 1, col, "\xe2\x94\x94", fg, bg, attr);
    tui_screen_set_cell(row + height - 1, col + width - 1, "\xe2\x94\x98", fg, bg, attr);

    for (int c = col + 1; c < col + width - 1; c++) {
        tui_screen_set_cell(row, c, "\xe2\x94\x80", fg, bg, attr);
        tui_screen_set_cell(row + height - 1, c, "\xe2\x94\x80", fg, bg, attr);
    }

    for (int r = row + 1; r < row + height - 1; r++) {
        tui_screen_set_cell(r, col, "\xe2\x94\x82", fg, bg, attr);
        tui_screen_set_cell(r, col + width - 1, "\xe2\x94\x82", fg, bg, attr);
    }
}

void tui_screen_box_double(int row, int col, int width, int height, TuiColor fg, TuiColor bg, TuiAttr attr)
{
    if (!g_screen.initialized || width < 2 || height < 2) return;

    tui_screen_set_cell(row, col, "\xe2\x95\x94", fg, bg, attr);
    tui_screen_set_cell(row, col + width - 1, "\xe2\x95\x97", fg, bg, attr);
    tui_screen_set_cell(row + height - 1, col, "\xe2\x95\x9a", fg, bg, attr);
    tui_screen_set_cell(row + height - 1, col + width - 1, "\xe2\x95\x9d", fg, bg, attr);

    for (int c = col + 1; c < col + width - 1; c++) {
        tui_screen_set_cell(row, c, "\xe2\x95\x90", fg, bg, attr);
        tui_screen_set_cell(row + height - 1, c, "\xe2\x95\x90", fg, bg, attr);
    }

    for (int r = row + 1; r < row + height - 1; r++) {
        tui_screen_set_cell(r, col, "\xe2\x95\x91", fg, bg, attr);
        tui_screen_set_cell(r, col + width - 1, "\xe2\x95\x91", fg, bg, attr);
    }
}

void tui_screen_invert_cell(int row, int col)
{
    if (!g_screen.initialized) return;
    if (row < 0 || row >= g_screen.rows) return;
    if (col < 0 || col >= g_screen.cols) return;

    TuiCell *cell = &g_screen.back[row * g_screen.cols + col];
    TuiColor tmp = cell->fg;
    cell->fg = cell->bg;
    cell->bg = tmp;
}

TuiSize tui_screen_size(void)
{
    TuiSize s = {g_screen.rows, g_screen.cols};
    return s;
}

#define RENDER_BUF_SIZE 16384

void tui_screen_render(void)
{
    if (!g_screen.initialized) return;

    char rbuf[RENDER_BUF_SIZE];
    size_t rpos = 0;
    int total = g_screen.rows * g_screen.cols;

    int cur_row = -1, cur_col = -1;
    TuiColor cur_fg;
    TuiColor cur_bg;
    memset(&cur_fg, 0, sizeof(cur_fg));
    cur_fg.type = TUI_COLOR_TYPE_NONE;
    memset(&cur_bg, 0, sizeof(cur_bg));
    cur_bg.type = TUI_COLOR_TYPE_NONE;
    uint16_t cur_attr = 0xFFFF;

    int use_truecolor = g_terminal.truecolor;

    for (int i = 0; i < total; i++) {
        TuiCell *b = &g_screen.back[i];
        TuiCell *f = &g_screen.front[i];

        if (b->ch[0] == f->ch[0] && b->ch[1] == f->ch[1] &&
            b->ch[2] == f->ch[2] && b->ch[3] == f->ch[3] &&
            tui_color_eq(b->fg, f->fg) && tui_color_eq(b->bg, f->bg) &&
            b->attr == f->attr) continue;

        int row = i / g_screen.cols;
        int col = i % g_screen.cols;

        if (rpos + 80 > RENDER_BUF_SIZE) {
            terminal_write_seq(&g_terminal, rbuf, rpos);
            rpos = 0;
        }

        if (row != cur_row || col != cur_col) {
            rpos += (size_t)snprintf(rbuf + rpos, RENDER_BUF_SIZE - rpos,
                                     "\033[%d;%dH", row + 1, col + 1);
            cur_row = row;
            cur_col = col;
        }

        if (b->attr != cur_attr) {
            memcpy(rbuf + rpos, "\033[0m", 4); rpos += 4;
            memset(&cur_fg, 0, sizeof(cur_fg));
            cur_fg.type = TUI_COLOR_TYPE_NONE;
            memset(&cur_bg, 0, sizeof(cur_bg));
            cur_bg.type = TUI_COLOR_TYPE_NONE;
            if (b->attr & TUI_ATTR_BOLD)          { memcpy(rbuf + rpos, "\033[1m", 4); rpos += 4; }
            if (b->attr & TUI_ATTR_DIM)           { memcpy(rbuf + rpos, "\033[2m", 4); rpos += 4; }
            if (b->attr & TUI_ATTR_ITALIC)        { memcpy(rbuf + rpos, "\033[3m", 4); rpos += 4; }
            if (b->attr & TUI_ATTR_UNDERLINE)     { memcpy(rbuf + rpos, "\033[4m", 4); rpos += 4; }
            if (b->attr & TUI_ATTR_BLINK)         { memcpy(rbuf + rpos, "\033[5m", 4); rpos += 4; }
            if (b->attr & TUI_ATTR_REVERSE)       { memcpy(rbuf + rpos, "\033[7m", 4); rpos += 4; }
            if (b->attr & TUI_ATTR_STRIKETHROUGH) { memcpy(rbuf + rpos, "\033[9m", 4); rpos += 4; }
            cur_attr = b->attr;
        }

        if (!tui_color_eq(b->fg, cur_fg)) {
            if (use_truecolor && b->fg.type == TUI_COLOR_TYPE_RGB) {
                rpos += (size_t)snprintf(rbuf + rpos, RENDER_BUF_SIZE - rpos,
                                         "\033[38;2;%d;%d;%dm", b->fg.rgb.r, b->fg.rgb.g, b->fg.rgb.b);
            } else {
                rpos += (size_t)snprintf(rbuf + rpos, RENDER_BUF_SIZE - rpos,
                                         "\033[38;5;%dm", b->fg.type == TUI_COLOR_TYPE_INDEX ? b->fg.index : 0);
            }
            cur_fg = b->fg;
        }
        if (!tui_color_eq(b->bg, cur_bg)) {
            if (use_truecolor && b->bg.type == TUI_COLOR_TYPE_RGB) {
                rpos += (size_t)snprintf(rbuf + rpos, RENDER_BUF_SIZE - rpos,
                                         "\033[48;2;%d;%d;%dm", b->bg.rgb.r, b->bg.rgb.g, b->bg.rgb.b);
            } else {
                rpos += (size_t)snprintf(rbuf + rpos, RENDER_BUF_SIZE - rpos,
                                         "\033[48;5;%dm", b->bg.type == TUI_COLOR_TYPE_INDEX ? b->bg.index : 0);
            }
            cur_bg = b->bg;
        }

        int clen = utf8_char_len((unsigned char)b->ch[0]);
        memcpy(rbuf + rpos, b->ch, clen);
        rpos += clen;

        cur_col++;
    }

    if (rpos > 0) {
        terminal_write_seq(&g_terminal, rbuf, rpos);
    }

    memcpy(g_screen.front, g_screen.back, (size_t)total * sizeof(TuiCell));
}
