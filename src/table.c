#define _POSIX_C_SOURCE 200809L

#include "tui_table.h"

#include <stdlib.h>
#include <string.h>

#define INITIAL_CAP 8

static void rebuild_sort_mapping(TuiTable *table)
{
    if (table->sort_mapping) {
        free(table->sort_mapping);
        table->sort_mapping = NULL;
    }

    if (table->row_count == 0) return;

    table->sort_mapping = (int *)malloc((size_t)table->row_count * sizeof(int));
    if (!table->sort_mapping) return;

    for (int i = 0; i < table->row_count; i++)
        table->sort_mapping[i] = i;

    if (table->sort_col < 0 || table->sort_col >= table->col_count)
        return;

    TuiSortOrder order = table->columns[table->sort_col].sort_order;
    if (order == TUI_SORT_NONE) return;

    int col = table->sort_col;

    for (int i = 1; i < table->row_count; i++) {
        int key = table->sort_mapping[i];
        int j = i - 1;

        while (j >= 0) {
            int idx_j = table->sort_mapping[j];
            const char *a = (col < table->col_count && table->rows[idx_j])
                                ? table->rows[idx_j][col] : "";
            const char *b = (col < table->col_count && table->rows[key])
                                ? table->rows[key][col] : "";
            int cmp = strcmp(a, b);
            if (order == TUI_SORT_DESC) cmp = -cmp;
            if (cmp > 0) {
                table->sort_mapping[j + 1] = table->sort_mapping[j];
                j--;
            } else {
                break;
            }
        }
        table->sort_mapping[j + 1] = key;
    }
}

static int mapped_row(TuiTable *table, int logical)
{
    if (table->sort_mapping && logical >= 0 && logical < table->row_count)
        return table->sort_mapping[logical];
    return logical;
}

static void ensure_visible(TuiTable *table)
{
    int body_height = table->base.height - 1;
    if (body_height < 1) body_height = 1;

    if (table->selected < table->scroll_y) {
        table->scroll_y = table->selected;
    }
    if (table->selected >= table->scroll_y + body_height) {
        table->scroll_y = table->selected - body_height + 1;
    }
}

static void render_clipped_text(int row, int col, const char *text,
                                 int max_w, TuiColor fg, TuiColor bg, TuiAttr attr)
{
    int len = (int)strlen(text);
    int w = len < max_w ? len : max_w;
    char buf[256];
    int cl = w < (int)sizeof(buf) - 1 ? w : (int)sizeof(buf) - 1;
    memcpy(buf, text, (size_t)cl);
    buf[cl] = '\0';
    tui_screen_write(row, col, buf, fg, bg, attr);
}

static void render_columns(TuiTable *table, int row, int screen_row,
                            int wx, int widget_width, TuiColor fg, TuiColor bg, TuiAttr attr)
{
    int col_x = wx - table->scroll_x;
    for (int c = 0; c < table->col_count; c++) {
        int cw = table->columns[c].width;
        if (col_x + cw <= wx) { col_x += cw; continue; }
        if (col_x >= wx + widget_width) break;

        const char *cell = (c < table->col_count && table->rows[row] && table->rows[row][c])
                               ? table->rows[row][c] : "";
        int text_x = col_x < wx ? wx : col_x;
        int max_w = (col_x + cw) - text_x;
        if (max_w > widget_width - (text_x - wx))
            max_w = widget_width - (text_x - wx);
        if (max_w > 0 && cell[0]) {
            render_clipped_text(screen_row, text_x, cell, max_w, fg, bg, attr);
        }
        col_x += cw;
    }
}

static void resolve_style(TuiWidget *widget, int logical, int selected,
                           TuiColor fg_def, TuiColor bg_def,
                           TuiColor fg_sel, TuiColor bg_sel,
                           TuiColor *fg, TuiColor *bg, TuiAttr *attr)
{
    if (widget->theme) {
        TuiWidgetState state = TUI_STATE_NORMAL;
        if (widget->disabled)
            state = TUI_STATE_DISABLED;
        else if (logical == selected && widget->focused)
            state = TUI_STATE_FOCUSED;
        TuiStyle s = widget->theme->styles[state];
        *fg = s.fg; *bg = s.bg; *attr = s.attr;
    } else {
        *fg = fg_def; *bg = bg_def; *attr = TUI_ATTR_NONE;
        if (logical == selected && widget->focused) {
            *fg = fg_sel; *bg = bg_sel;
        }
    }
}

static void table_render(TuiWidget *widget)
{
    TuiTable *table = (TuiTable *)widget;
    int wx = widget->abs_x;
    int wy = widget->abs_y;

    TuiStyle header_style = tui_widget_get_style(widget);
    int body_height = widget->height - 1;
    if (body_height < 0) body_height = 0;

    {
        TuiColor hfg = table->fg_header;
        TuiColor hbg = table->bg_header;
        TuiAttr  hat = TUI_ATTR_BOLD;

        if (widget->theme) {
            hfg = header_style.fg;
            hbg = header_style.bg;
            hat = header_style.attr | TUI_ATTR_BOLD;
        }

        tui_screen_fill(wy, wx, widget->width, 1, " ", hfg, hbg, hat);

        int col_x = wx - table->scroll_x;
        for (int c = 0; c < table->col_count; c++) {
            int cw = table->columns[c].width;
            if (col_x + cw <= wx) { col_x += cw; continue; }
            if (col_x >= wx + widget->width) break;

            if (table->columns[c].label && col_x < wx + widget->width) {
                int text_x = col_x < wx ? wx : col_x;
                int max_w = (col_x + cw) - text_x;
                if (max_w > widget->width - (text_x - wx)) max_w = widget->width - (text_x - wx);
                if (max_w > 0) {
                    int len = (int)strlen(table->columns[c].label);
                    int w = len < max_w ? len : max_w;
                    char buf[256];
                    int cl = w < (int)sizeof(buf) - 3 ? w : (int)sizeof(buf) - 3;
                    memcpy(buf, table->columns[c].label, (size_t)cl);

                    if (table->columns[c].sort_order != TUI_SORT_NONE && cl + 2 < max_w) {
                        buf[cl++] = ' ';
                        buf[cl++] = table->columns[c].sort_order == TUI_SORT_ASC ? '^' : 'v';
                    }
                    buf[cl] = '\0';
                    tui_screen_write(wy, text_x, buf, hfg, hbg, hat);
                }
            }
            col_x += cw;
        }
    }

    {
        for (int i = 0; i < body_height; i++) {
            int logical = table->scroll_y + i;
            if (logical >= table->row_count) break;
            int r = mapped_row(table, logical);

            TuiColor fg, bg;
            TuiAttr  attr;
            resolve_style(widget, logical, table->selected,
                          table->fg, table->bg,
                          table->fg_selected, table->bg_selected,
                          &fg, &bg, &attr);

            tui_screen_fill(wy + 1 + i, wx, widget->width, 1, " ", fg, bg, attr);

            if (!table->rows[r]) continue;
            render_columns(table, r, wy + 1 + i, wx, widget->width, fg, bg, attr);
        }
    }

    if (table->row_count > body_height && body_height > 0) {
        int scroll_col = wx + widget->width - 1;
        TuiColor track_fg = TUI_COLOR_INDEX(TUI_COLOR_GRAY_DARK);
        TuiColor track_bg = table->bg;
        tui_screen_fill(wy + 1, scroll_col, 1, body_height, " ",
                        track_fg, track_bg, TUI_ATTR_DIM);

        int max_scroll = table->row_count - body_height;
        if (max_scroll > 0) {
            int thumb_h = body_height * body_height / table->row_count;
            if (thumb_h < 1) thumb_h = 1;
            int thumb_y = table->scroll_y * (body_height - thumb_h) / max_scroll;
            for (int t = 0; t < thumb_h; t++) {
                tui_screen_set_cell(wy + 1 + thumb_y + t, scroll_col,
                                    "\xe2\x96\x88",
                                    TUI_COLOR_INDEX(TUI_COLOR_BRIGHT_WHITE),
                                    TUI_COLOR_INDEX(TUI_COLOR_GRAY_MID),
                                    TUI_ATTR_NONE);
            }
        }
    }
}

static void table_handle_sort(TuiTable *table, int col)
{
    if (col < 0 || col >= table->col_count) return;

    if (table->sort_col >= 0 && table->sort_col != col) {
        table->columns[table->sort_col].sort_order = TUI_SORT_NONE;
    }

    TuiSortOrder cur = table->columns[col].sort_order;
    switch (cur) {
    case TUI_SORT_NONE: table->columns[col].sort_order = TUI_SORT_ASC;  break;
    case TUI_SORT_ASC:  table->columns[col].sort_order = TUI_SORT_DESC; break;
    case TUI_SORT_DESC: table->columns[col].sort_order = TUI_SORT_NONE; break;
    }
    table->sort_col = col;

    rebuild_sort_mapping(table);
    table->base.dirty = 1;
}

static int table_handle_input(TuiWidget *widget, const TuiEvent *event)
{
    TuiTable *table = (TuiTable *)widget;

    int body_height = widget->height - 1;
    if (body_height < 1) body_height = 1;

    switch (event->key) {
    case TUI_KEY_UP:
        if (table->selected > 0) {
            table->selected--;
            ensure_visible(table);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_DOWN:
        if (table->selected < table->row_count - 1) {
            table->selected++;
            ensure_visible(table);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_LEFT:
        if (table->scroll_x > 0) {
            table->scroll_x -= 5;
            if (table->scroll_x < 0) table->scroll_x = 0;
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_RIGHT: {
        int total_w = 0;
        for (int c = 0; c < table->col_count; c++) total_w += table->columns[c].width;
        if (table->scroll_x < total_w - widget->width) {
            table->scroll_x += 5;
            widget->dirty = 1;
        }
        return 1;
    }

    case TUI_KEY_PAGE_UP:
        if (table->selected > 0) {
            table->selected -= body_height;
            if (table->selected < 0) table->selected = 0;
            ensure_visible(table);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_PAGE_DOWN:
        if (table->row_count > 0) {
            table->selected += body_height;
            if (table->selected >= table->row_count)
                table->selected = table->row_count - 1;
            ensure_visible(table);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_HOME:
        table->selected = 0;
        table->scroll_y = 0;
        widget->dirty = 1;
        return 1;

    case TUI_KEY_END:
        if (table->row_count > 0) {
            table->selected = table->row_count - 1;
            ensure_visible(table);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_ENTER:
        if (table->on_select && table->row_count > 0) {
            table->on_select(widget, table->selected, table->user_data);
        }
        return 1;

    default:
        break;
    }

    return 0;
}

static int table_handle_mouse(TuiWidget *widget, const TuiMouseEvent *mouse)
{
    TuiTable *table = (TuiTable *)widget;

    if (mouse->action == TUI_MOUSE_WHEEL_UP) {
        if (table->scroll_y > 0) {
            table->scroll_y -= 3;
            if (table->scroll_y < 0) table->scroll_y = 0;
            widget->dirty = 1;
        }
        return 1;
    }

    if (mouse->action == TUI_MOUSE_WHEEL_DOWN) {
        int body_height = widget->height - 1;
        if (body_height < 1) body_height = 1;
        int max_off = table->row_count - body_height;
        if (max_off < 0) max_off = 0;
        if (table->scroll_y < max_off) {
            table->scroll_y += 3;
            if (table->scroll_y > max_off) table->scroll_y = max_off;
            widget->dirty = 1;
        }
        return 1;
    }

    if (mouse->action != TUI_MOUSE_PRESS || mouse->button != TUI_MOUSE_BTN_LEFT)
        return 0;

    int rel_row = mouse->row - widget->abs_y;
    int rel_col = mouse->col - widget->abs_x;

    if (rel_row == 0) {
        int col_x = 0;
        for (int c = 0; c < table->col_count; c++) {
            int cw = table->columns[c].width;
            int sx = col_x - table->scroll_x;
            if (rel_col >= sx && rel_col < sx + cw) {
                table_handle_sort(table, c);
                return 1;
            }
            col_x += cw;
        }
        return 0;
    }

    if (rel_row >= 1) {
        int logical = table->scroll_y + (rel_row - 1);
        if (logical >= 0 && logical < table->row_count) {
            table->selected = logical;
            ensure_visible(table);
            widget->dirty = 1;

            if (table->on_select) {
                table->on_select(widget, table->selected, table->user_data);
            }
            return 1;
        }
    }

    (void)rel_col;
    return 0;
}

static void table_destroy(TuiWidget *widget)
{
    TuiTable *table = (TuiTable *)widget;

    for (int c = 0; c < table->col_count; c++)
        free(table->columns[c].label);
    free(table->columns);

    for (int r = 0; r < table->row_count; r++) {
        if (table->rows[r]) {
            for (int c = 0; c < table->col_count; c++)
                free(table->rows[r][c]);
            free(table->rows[r]);
        }
    }
    free(table->rows);
    free(table->sort_mapping);

    table->columns       = NULL;
    table->rows          = NULL;
    table->sort_mapping  = NULL;
    table->col_count     = 0;
    table->row_count     = 0;
}

static TuiWidgetVTable table_vtable = {
    .render       = table_render,
    .handle_input = table_handle_input,
    .handle_mouse = table_handle_mouse,
    .destroy      = table_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_table_init(TuiTable *table, int x, int y, int width, int height)
{
    if (!table) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init(&table->base, x, y, width, height,
                                    &table_vtable, NULL);
    if (res != TUI_OK) return res;

    table->base.focusable = 1;

    table->columns      = NULL;
    table->col_count     = 0;
    table->col_capacity  = 0;
    table->rows          = NULL;
    table->row_count     = 0;
    table->row_capacity  = 0;
    table->sort_mapping  = NULL;
    table->selected      = 0;
    table->scroll_y      = 0;
    table->scroll_x      = 0;
    table->sort_col      = -1;
    table->on_select     = NULL;
    table->user_data     = NULL;

    table->fg            = TUI_COLOR_INDEX(15);
    table->bg            = TUI_COLOR_INDEX(0);
    table->fg_header     = TUI_COLOR_INDEX(15);
    table->bg_header     = TUI_COLOR_INDEX(240);
    table->fg_selected   = TUI_COLOR_INDEX(0);
    table->bg_selected   = TUI_COLOR_INDEX(12);

    return TUI_OK;
}

TuiResult tui_table_add_column(TuiTable *table, const char *label, int width)
{
    if (!table) return TUI_ERR_MEMORY;

    if (table->col_count >= table->col_capacity) {
        int new_cap = table->col_capacity == 0
            ? INITIAL_CAP : table->col_capacity * 2;
        TuiTableColumn *nc = (TuiTableColumn *)realloc(
            table->columns, (size_t)new_cap * sizeof(TuiTableColumn));
        if (!nc) return TUI_ERR_MEMORY;
        table->columns      = nc;
        table->col_capacity = new_cap;
    }

    table->columns[table->col_count].label      = label ? strdup(label) : NULL;
    table->columns[table->col_count].width       = width > 0 ? width : 10;
    table->columns[table->col_count].sort_order  = TUI_SORT_NONE;
    table->col_count++;
    table->base.dirty = 1;
    return TUI_OK;
}

TuiResult tui_table_add_row(TuiTable *table, const char **cells, int cell_count)
{
    if (!table) return TUI_ERR_MEMORY;

    if (table->row_count >= table->row_capacity) {
        int new_cap = table->row_capacity == 0
            ? INITIAL_CAP : table->row_capacity * 2;
        char ***nr = (char ***)realloc(
            table->rows, (size_t)new_cap * sizeof(char **));
        if (!nr) return TUI_ERR_MEMORY;
        table->rows        = nr;
        table->row_capacity = new_cap;
    }

    int alloc_count = cell_count < table->col_count
        ? table->col_count : cell_count;
    char **row = (char **)calloc((size_t)alloc_count, sizeof(char *));
    if (!row) return TUI_ERR_MEMORY;

    int copy = cell_count < table->col_count ? cell_count : table->col_count;
    for (int c = 0; c < copy; c++)
        row[c] = cells[c] ? strdup(cells[c]) : NULL;
    for (int c = copy; c < table->col_count; c++)
        row[c] = NULL;

    table->rows[table->row_count] = row;
    table->row_count++;

    if (table->sort_col >= 0 && table->sort_col < table->col_count
        && table->columns[table->sort_col].sort_order != TUI_SORT_NONE) {
        rebuild_sort_mapping(table);
    }

    table->base.dirty = 1;
    return TUI_OK;
}

void tui_table_remove_row(TuiTable *table, int row)
{
    if (!table || row < 0 || row >= table->row_count) return;

    if (table->rows[row]) {
        for (int c = 0; c < table->col_count; c++)
            free(table->rows[row][c]);
        free(table->rows[row]);
    }
    memmove(&table->rows[row], &table->rows[row + 1],
            (size_t)(table->row_count - row - 1) * sizeof(char **));
    table->row_count--;

    if (table->selected >= table->row_count && table->row_count > 0)
        table->selected = table->row_count - 1;

    ensure_visible(table);
    rebuild_sort_mapping(table);
    table->base.dirty = 1;
}

void tui_table_clear_rows(TuiTable *table)
{
    if (!table) return;
    for (int r = 0; r < table->row_count; r++) {
        if (table->rows[r]) {
            for (int c = 0; c < table->col_count; c++)
                free(table->rows[r][c]);
            free(table->rows[r]);
        }
    }
    table->row_count    = 0;
    table->selected     = 0;
    table->scroll_y     = 0;
    table->scroll_x     = 0;
    free(table->sort_mapping);
    table->sort_mapping = NULL;
    table->base.dirty   = 1;
}

int tui_table_get_selected(TuiTable *table)
{
    if (!table) return -1;
    return table->selected;
}

void tui_table_set_colors(TuiTable *table, TuiColor fg, TuiColor bg,
                          TuiColor fg_header, TuiColor bg_header,
                          TuiColor fg_selected, TuiColor bg_selected)
{
    if (!table) return;
    table->fg           = fg;
    table->bg           = bg;
    table->fg_header    = fg_header;
    table->bg_header    = bg_header;
    table->fg_selected  = fg_selected;
    table->bg_selected  = bg_selected;
    table->base.dirty   = 1;
}

void tui_table_set_on_select(TuiTable *table, TuiTableCallback callback,
                             void *user_data)
{
    if (!table) return;
    table->on_select = callback;
    table->user_data = user_data;
}

void tui_table_sort_by(TuiTable *table, int col, TuiSortOrder order)
{
    if (!table || col < 0 || col >= table->col_count) return;
    if (table->sort_col >= 0 && table->sort_col != col)
        table->columns[table->sort_col].sort_order = TUI_SORT_NONE;
    table->sort_col = col;
    table->columns[col].sort_order = order;
    rebuild_sort_mapping(table);
    table->base.dirty = 1;
}
