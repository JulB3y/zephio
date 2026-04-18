#define _POSIX_C_SOURCE 200809L

#include "tui_screen.h"
#include "tui_terminal.h"
#include "tui_text.h"
#include "tui_context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static TuiCell DEFAULT_CELL;

static int default_cell_ready = 0;

static void init_default_cell(void)
{
    if (default_cell_ready) return;
    DEFAULT_CELL.ch[0] = ' ';
    DEFAULT_CELL.ch[1] = 0;
    DEFAULT_CELL.ch[2] = 0;
    DEFAULT_CELL.ch[3] = 0;
    DEFAULT_CELL.width = 1;
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

static int cell_char_width(const char *ch, int len)
{
    uint32_t cp = 0;
    tui_utf8_next(ch, (size_t)len, &cp);
    int w = tui_utf8_char_width(cp);
    return w > 0 ? w : 1;
}

static void clear_continuation(TuiScreen *screen, int row, int col)
{
    if (row < 0 || row >= screen->rows) return;
    if (col < 0 || col >= screen->cols) return;
    TuiCell *cell = &screen->back[row * screen->cols + col];
    if (cell->width == 2 && col + 1 < screen->cols) {
        TuiCell *cont = &screen->back[row * screen->cols + col + 1];
        if (cont->width == 0) {
            *cont = DEFAULT_CELL;
        }
    }
}

static void clear_leader(TuiScreen *screen, int row, int col)
{
    if (row < 0 || row >= screen->rows) return;
    if (col <= 0 || col >= screen->cols) return;
    TuiCell *cell = &screen->back[row * screen->cols + col];
    if (cell->width == 0) {
        TuiCell *leader = &screen->back[row * screen->cols + col - 1];
        if (leader->width == 2) {
            *leader = DEFAULT_CELL;
        }
    }
}

static void clear_buffer(TuiCell *buf, int count)
{
    init_default_cell();
    for (int i = 0; i < count; i++) {
        buf[i] = DEFAULT_CELL;
    }
}

TuiResult tui_screen_init(TuiContext *ctx, int rows, int cols)
{
    if (rows <= 0 || cols <= 0) {
        return TUI_ERR_MEMORY;
    }

    TuiScreen *screen = &ctx->screen;

    if (screen->initialized) {
        tui_screen_free(ctx);
    }

    int total = rows * cols;
    size_t buf_size = (size_t)total * sizeof(TuiCell);

    screen->front = (TuiCell *)malloc(buf_size);
    if (!screen->front) {
        return TUI_ERR_MEMORY;
    }

    screen->back = (TuiCell *)malloc(buf_size);
    if (!screen->back) {
        free(screen->front);
        screen->front = NULL;
        return TUI_ERR_MEMORY;
    }

    clear_buffer(screen->front, total);
    clear_buffer(screen->back, total);

    screen->rows = rows;
    screen->cols = cols;
    screen->initialized = 1;

    return TUI_OK;
}

void tui_screen_free(TuiContext *ctx)
{
    TuiScreen *screen = &ctx->screen;

    if (!screen->initialized) {
        return;
    }

    free(screen->front);
    free(screen->back);

    screen->front = NULL;
    screen->back = NULL;
    screen->rows = 0;
    screen->cols = 0;
    screen->initialized = 0;
}

TuiResult tui_screen_resize(TuiContext *ctx, int rows, int cols)
{
    TuiScreen *screen = &ctx->screen;

    if (!screen->initialized) {
        return tui_screen_init(ctx, rows, cols);
    }

    if (rows <= 0 || cols <= 0) {
        return TUI_ERR_MEMORY;
    }

    if (rows == screen->rows && cols == screen->cols) {
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

    int copy_rows = rows < screen->rows ? rows : screen->rows;
    int copy_cols = cols < screen->cols ? cols : screen->cols;

    for (int r = 0; r < copy_rows; r++) {
        TuiCell *old_front_row = screen->front + r * screen->cols;
        TuiCell *old_back_row = screen->back + r * screen->cols;
        TuiCell *new_front_row = new_front + r * cols;
        TuiCell *new_back_row = new_back + r * cols;

        memcpy(new_front_row, old_front_row, (size_t)copy_cols * sizeof(TuiCell));
        memcpy(new_back_row, old_back_row, (size_t)copy_cols * sizeof(TuiCell));
    }

    free(screen->front);
    free(screen->back);

    screen->front = new_front;
    screen->back = new_back;
    screen->rows = rows;
    screen->cols = cols;

    return TUI_OK;
}

void tui_screen_clear(TuiContext *ctx)
{
    TuiScreen *screen = &ctx->screen;
    if (!screen->initialized) return;
    clear_buffer(screen->back, screen->rows * screen->cols);
}

void tui_screen_set_cell(TuiContext *ctx, int row, int col, const char *ch, TuiColor fg, TuiColor bg, TuiAttr attr)
{
    TuiScreen *screen = &ctx->screen;
    if (!screen->initialized) return;
    if (row < 0 || row >= screen->rows) return;
    if (col < 0 || col >= screen->cols) return;
    if (!ch) return;

    init_default_cell();
    clear_leader(screen, row, col);
    clear_continuation(screen, row, col);

    int blen = utf8_char_len((unsigned char)ch[0]);
    int w = cell_char_width(ch, blen);

    if (w == 2 && col + 1 >= screen->cols) {
        w = 1;
    }

    TuiCell *cell = &screen->back[row * screen->cols + col];
    cell_set_char(cell, ch);
    cell->fg = fg;
    cell->bg = bg;
    cell->attr = attr;
    cell->width = (uint8_t)w;

    if (w == 2 && col + 1 < screen->cols) {
        TuiCell *cont = &screen->back[row * screen->cols + col + 1];
        clear_leader(screen, row, col + 1);
        clear_continuation(screen, row, col + 1);
        memset(cont->ch, 0, 4);
        cont->fg = fg;
        cont->bg = bg;
        cont->attr = attr;
        cont->width = 0;
    }
}

void tui_screen_write(TuiContext *ctx, int row, int col, const char *text, TuiColor fg, TuiColor bg, TuiAttr attr)
{
    TuiScreen *screen = &ctx->screen;
    if (!screen->initialized || !text) return;

    init_default_cell();
    int c = col;
    const unsigned char *p = (const unsigned char *)text;

    while (*p && c < screen->cols) {
        if (row >= 0 && row < screen->rows && c >= 0) {
            clear_leader(screen, row, c);
            clear_continuation(screen, row, c);
        }

        int blen = utf8_char_len(*p);
        int w = cell_char_width((const char *)p, blen);

        if (c + w > screen->cols) break;

        if (row >= 0 && row < screen->rows && c >= 0) {
            TuiCell *cell = &screen->back[row * screen->cols + c];
            memset(cell->ch, 0, 4);
            memcpy(cell->ch, p, blen);
            cell->fg = fg;
            cell->bg = bg;
            cell->attr = attr;
            cell->width = (uint8_t)w;

            if (w == 2 && c + 1 < screen->cols) {
                TuiCell *cont = &screen->back[row * screen->cols + c + 1];
                clear_leader(screen, row, c + 1);
                clear_continuation(screen, row, c + 1);
                memset(cont->ch, 0, 4);
                cont->fg = fg;
                cont->bg = bg;
                cont->attr = attr;
                cont->width = 0;
            }
        }

        p += blen;
        c += w;
    }
}

void tui_screen_fill(TuiContext *ctx, int row, int col, int width, int height, const char *ch, TuiColor fg, TuiColor bg, TuiAttr attr)
{
    if (!ctx->screen.initialized || !ch) return;

    for (int r = row; r < row + height; r++) {
        for (int c = col; c < col + width; c++) {
            tui_screen_set_cell(ctx, r, c, ch, fg, bg, attr);
        }
    }
}

void tui_screen_box_single(TuiContext *ctx, int row, int col, int width, int height, TuiColor fg, TuiColor bg, TuiAttr attr)
{
    if (!ctx->screen.initialized || width < 2 || height < 2) return;

    tui_screen_set_cell(ctx, row, col, "\xe2\x94\x8c", fg, bg, attr);
    tui_screen_set_cell(ctx, row, col + width - 1, "\xe2\x94\x90", fg, bg, attr);
    tui_screen_set_cell(ctx, row + height - 1, col, "\xe2\x94\x94", fg, bg, attr);
    tui_screen_set_cell(ctx, row + height - 1, col + width - 1, "\xe2\x94\x98", fg, bg, attr);

    for (int c = col + 1; c < col + width - 1; c++) {
        tui_screen_set_cell(ctx, row, c, "\xe2\x94\x80", fg, bg, attr);
        tui_screen_set_cell(ctx, row + height - 1, c, "\xe2\x94\x80", fg, bg, attr);
    }

    for (int r = row + 1; r < row + height - 1; r++) {
        tui_screen_set_cell(ctx, r, col, "\xe2\x94\x82", fg, bg, attr);
        tui_screen_set_cell(ctx, r, col + width - 1, "\xe2\x94\x82", fg, bg, attr);
    }
}

void tui_screen_box_double(TuiContext *ctx, int row, int col, int width, int height, TuiColor fg, TuiColor bg, TuiAttr attr)
{
    if (!ctx->screen.initialized || width < 2 || height < 2) return;

    tui_screen_set_cell(ctx, row, col, "\xe2\x95\x94", fg, bg, attr);
    tui_screen_set_cell(ctx, row, col + width - 1, "\xe2\x95\x97", fg, bg, attr);
    tui_screen_set_cell(ctx, row + height - 1, col, "\xe2\x95\x9a", fg, bg, attr);
    tui_screen_set_cell(ctx, row + height - 1, col + width - 1, "\xe2\x95\x9d", fg, bg, attr);

    for (int c = col + 1; c < col + width - 1; c++) {
        tui_screen_set_cell(ctx, row, c, "\xe2\x95\x90", fg, bg, attr);
        tui_screen_set_cell(ctx, row + height - 1, c, "\xe2\x95\x90", fg, bg, attr);
    }

    for (int r = row + 1; r < row + height - 1; r++) {
        tui_screen_set_cell(ctx, r, col, "\xe2\x95\x91", fg, bg, attr);
        tui_screen_set_cell(ctx, r, col + width - 1, "\xe2\x95\x91", fg, bg, attr);
    }
}

void tui_screen_invert_cell(TuiContext *ctx, int row, int col)
{
    TuiScreen *screen = &ctx->screen;
    if (!screen->initialized) return;
    if (row < 0 || row >= screen->rows) return;
    if (col < 0 || col >= screen->cols) return;

    TuiCell *cell = &screen->back[row * screen->cols + col];

    if (cell->width == 0 && col > 0) {
        TuiCell *leader = &screen->back[row * screen->cols + col - 1];
        if (leader->width == 2) {
            cell = leader;
        }
    }

    TuiColor tmp = cell->fg;
    cell->fg = cell->bg;
    cell->bg = tmp;
}

TuiSize tui_screen_size(TuiContext *ctx)
{
    TuiSize s = {ctx->screen.rows, ctx->screen.cols};
    return s;
}

#define RENDER_BUF_SIZE 16384

static int fmt_uint(char *buf, unsigned int v)
{
    if (v < 10) { buf[0] = (char)('0' + v); return 1; }
    if (v < 100) { buf[1] = (char)('0' + v % 10); buf[0] = (char)('0' + v / 10); return 2; }
    if (v < 1000) {
        buf[2] = (char)('0' + v % 10); v /= 10;
        buf[1] = (char)('0' + v % 10); v /= 10;
        buf[0] = (char)('0' + v);
        return 3;
    }
    buf[3] = (char)('0' + v % 10); v /= 10;
    buf[2] = (char)('0' + v % 10); v /= 10;
    buf[1] = (char)('0' + v % 10); v /= 10;
    buf[0] = (char)('0' + v);
    return 4;
}

static void render_flush(Terminal *terminal, char *rbuf, size_t *rpos)
{
    if (*rpos > 0) {
        terminal_write_seq(terminal, rbuf, *rpos);
        *rpos = 0;
    }
}

static size_t emit_cursor(char *buf, size_t pos, int row, int col)
{
    memcpy(buf + pos, "\033[", 2); pos += 2;
    pos += (size_t)fmt_uint(buf + pos, (unsigned int)(row + 1));
    buf[pos++] = ';';
    pos += (size_t)fmt_uint(buf + pos, (unsigned int)(col + 1));
    buf[pos++] = 'H';
    return pos;
}

static size_t emit_fg_256(char *buf, size_t pos, uint8_t idx)
{
    memcpy(buf + pos, "\033[38;5;", 7); pos += 7;
    pos += (size_t)fmt_uint(buf + pos, idx);
    buf[pos++] = 'm';
    return pos;
}

static size_t emit_fg_rgb(char *buf, size_t pos, uint8_t r, uint8_t g, uint8_t b)
{
    memcpy(buf + pos, "\033[38;2;", 7); pos += 7;
    pos += (size_t)fmt_uint(buf + pos, r); buf[pos++] = ';';
    pos += (size_t)fmt_uint(buf + pos, g); buf[pos++] = ';';
    pos += (size_t)fmt_uint(buf + pos, b); buf[pos++] = 'm';
    return pos;
}

static size_t emit_bg_256(char *buf, size_t pos, uint8_t idx)
{
    memcpy(buf + pos, "\033[48;5;", 7); pos += 7;
    pos += (size_t)fmt_uint(buf + pos, idx);
    buf[pos++] = 'm';
    return pos;
}

static size_t emit_bg_rgb(char *buf, size_t pos, uint8_t r, uint8_t g, uint8_t b)
{
    memcpy(buf + pos, "\033[48;2;", 7); pos += 7;
    pos += (size_t)fmt_uint(buf + pos, r); buf[pos++] = ';';
    pos += (size_t)fmt_uint(buf + pos, g); buf[pos++] = ';';
    pos += (size_t)fmt_uint(buf + pos, b); buf[pos++] = 'm';
    return pos;
}

void tui_screen_render(TuiContext *ctx)
{
    TuiScreen *screen = &ctx->screen;
    Terminal *terminal = &ctx->terminal;

    if (!screen->initialized) return;

    char rbuf[RENDER_BUF_SIZE];
    size_t rpos = 0;
    int total = screen->rows * screen->cols;

    int cur_row = -1, cur_col = -1;
    TuiColor cur_fg;
    TuiColor cur_bg;
    memset(&cur_fg, 0, sizeof(cur_fg));
    cur_fg.type = TUI_COLOR_TYPE_NONE;
    memset(&cur_bg, 0, sizeof(cur_bg));
    cur_bg.type = TUI_COLOR_TYPE_NONE;
    uint16_t cur_attr = 0xFFFF;

    int use_truecolor = terminal->truecolor;

    for (int i = 0; i < total; i++) {
        TuiCell *b = &screen->back[i];

        if (b->width == 0) continue;

        TuiCell *f = &screen->front[i];

        if (b->ch[0] == f->ch[0] && b->ch[1] == f->ch[1] &&
            b->ch[2] == f->ch[2] && b->ch[3] == f->ch[3] &&
            tui_color_eq(b->fg, f->fg) && tui_color_eq(b->bg, f->bg) &&
            b->attr == f->attr && b->width == f->width) continue;

        int row = i / screen->cols;
        int col = i % screen->cols;

        if (rpos + 80 > RENDER_BUF_SIZE) {
            render_flush(terminal, rbuf, &rpos);
        }

        if (row != cur_row || col != cur_col) {
            rpos = emit_cursor(rbuf, rpos, row, col);
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
                rpos = emit_fg_rgb(rbuf, rpos, b->fg.rgb.r, b->fg.rgb.g, b->fg.rgb.b);
            } else {
                rpos = emit_fg_256(rbuf, rpos, b->fg.type == TUI_COLOR_TYPE_INDEX ? b->fg.index : 0);
            }
            cur_fg = b->fg;
        }
        if (!tui_color_eq(b->bg, cur_bg)) {
            if (use_truecolor && b->bg.type == TUI_COLOR_TYPE_RGB) {
                rpos = emit_bg_rgb(rbuf, rpos, b->bg.rgb.r, b->bg.rgb.g, b->bg.rgb.b);
            } else {
                rpos = emit_bg_256(rbuf, rpos, b->bg.type == TUI_COLOR_TYPE_INDEX ? b->bg.index : 0);
            }
            cur_bg = b->bg;
        }

        int clen = utf8_char_len((unsigned char)b->ch[0]);
        memcpy(rbuf + rpos, b->ch, clen);
        rpos += clen;

        cur_col += b->width;
    }

    if (rpos > 0) {
        terminal_write_seq(terminal, rbuf, rpos);
    }

    memcpy(screen->front, screen->back, (size_t)total * sizeof(TuiCell));
}
