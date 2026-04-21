#define _POSIX_C_SOURCE 200809L

#include "zephio_textarea.h"
#include "zephio_context.h"
#include "zephio_text.h"

#include <stdlib.h>
#include <string.h>

#define LINES_INITIAL_CAPACITY 16
#define RENDER_BUF_SIZE 4096

static void textarea_free_lines(ZephioTextArea *ta)
{
    if (!ta->lines) return;
    for (int i = 0; i < ta->line_count; i++) {
        free(ta->lines[i]);
    }
    free(ta->lines);
    ta->lines = NULL;
    ta->line_count = 0;
    ta->line_capacity = 0;
}

static int textarea_ensure_capacity(ZephioTextArea *ta, int needed)
{
    if (needed <= ta->line_capacity) return 0;
    int new_cap = ta->line_capacity > 0 ? ta->line_capacity * 2 : LINES_INITIAL_CAPACITY;
    while (new_cap < needed) new_cap *= 2;
    char **new_lines = (char **)realloc(ta->lines, (size_t)new_cap * sizeof(char *));
    if (!new_lines) return -1;
    ta->lines = new_lines;
    ta->line_capacity = new_cap;
    return 0;
}

static void textarea_insert_line_at(ZephioTextArea *ta, int index, const char *text)
{
    if (textarea_ensure_capacity(ta, ta->line_count + 1) != 0) return;
    memmove(&ta->lines[index + 1], &ta->lines[index],
            (size_t)(ta->line_count - index) * sizeof(char *));
    ta->lines[index] = text ? strdup(text) : strdup("");
    ta->line_count++;
}

static void textarea_delete_line_at(ZephioTextArea *ta, int index)
{
    if (index < 0 || index >= ta->line_count) return;
    free(ta->lines[index]);
    memmove(&ta->lines[index], &ta->lines[index + 1],
            (size_t)(ta->line_count - index - 1) * sizeof(char *));
    ta->line_count--;
}

static inline int line_len(ZephioTextArea *ta, int row)
{
    const char *l = ta->lines[row];
    return l ? (int)strlen(l) : 0;
}

static void textarea_ensure_cursor_visible(ZephioTextArea *ta)
{
    if (ta->cursor_row < ta->scroll_y) {
        ta->scroll_y = ta->cursor_row;
    }
    if (ta->cursor_row >= ta->scroll_y + ta->base.height) {
        ta->scroll_y = ta->cursor_row - ta->base.height + 1;
    }

    int ll = line_len(ta, ta->cursor_row);
    int cursor_display = zephio_text_index_to_col(ta->lines[ta->cursor_row],
                                                (size_t)ll, (size_t)ta->cursor_col);

    if (cursor_display < ta->scroll_x) {
        ta->scroll_x = cursor_display;
    }
    if (cursor_display >= ta->scroll_x + ta->base.width) {
        ta->scroll_x = cursor_display - ta->base.width + 1;
    }
}

static inline void cursor_moved(ZephioTextArea *ta)
{
    textarea_ensure_cursor_visible(ta);
    ta->base.dirty = 1;
}

static void textarea_render(ZephioWidget *widget)
{
    ZephioTextArea *ta = (ZephioTextArea *)widget;

    ZephioColor fg = ta->fg;
    ZephioColor bg = ta->bg;
    ZephioAttr attr = ta->attr;

    if (widget->theme) {
        ZephioStyle style = zephio_widget_get_style(widget);
        fg = style.fg;
        bg = style.bg;
        attr = style.attr;
    }

    zephio_screen_fill(widget->ctx, widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ", fg, bg, attr);

    char buf[RENDER_BUF_SIZE];
    int vp_height = widget->height;
    int vp_width = widget->width;

    for (int r = 0; r < vp_height; r++) {
        int line_idx = ta->scroll_y + r;
        if (line_idx >= ta->line_count) break;

        int ll = line_len(ta, line_idx);
        if (ll == 0) continue;

        const char *line = ta->lines[line_idx];

        int skip_bytes = zephio_text_col_to_index(line, (size_t)ll, ta->scroll_x);
        int remaining = ll - skip_bytes;
        if (remaining <= 0) continue;

        size_t visible_bytes;
        zephio_text_clip(line + skip_bytes, (size_t)remaining,
                      vp_width, &visible_bytes);

        if (visible_bytes > 0) {
            size_t copy = visible_bytes < sizeof(buf) - 1
                          ? visible_bytes : sizeof(buf) - 1;
            memcpy(buf, line + skip_bytes, copy);
            buf[copy] = '\0';
            zephio_screen_write(widget->ctx, widget->abs_y + r, widget->abs_x,
                             buf, fg, bg, attr);
        }
    }

    if (widget->focused && !widget->disabled) {
        int cursor_screen_row = ta->cursor_row - ta->scroll_y;
        if (cursor_screen_row >= 0 && cursor_screen_row < vp_height) {
            int ll = line_len(ta, ta->cursor_row);
            int cursor_display = zephio_text_index_to_col(ta->lines[ta->cursor_row],
                                                        (size_t)ll, (size_t)ta->cursor_col);
            int screen_col = cursor_display - ta->scroll_x;
            if (screen_col >= 0 && screen_col < vp_width) {
                zephio_screen_invert_cell(widget->ctx, widget->abs_y + cursor_screen_row,
                                       widget->abs_x + screen_col);
            }
        }
    }
}

static int textarea_get_display_col(ZephioTextArea *ta)
{
    int ll = line_len(ta, ta->cursor_row);
    return zephio_text_index_to_col(ta->lines[ta->cursor_row],
                                  (size_t)ll, (size_t)ta->cursor_col);
}

static void textarea_set_cursor_to_display_col(ZephioTextArea *ta, int display_col)
{
    int ll = line_len(ta, ta->cursor_row);
    ta->cursor_col = zephio_text_col_to_index(ta->lines[ta->cursor_row],
                                            (size_t)ll, display_col);
}

static void move_vertical(ZephioTextArea *ta, int dir)
{
    int target = ta->cursor_row + dir;
    if (target < 0 || target >= ta->line_count) return;

    if (!ta->has_preferred_col) {
        ta->preferred_col = textarea_get_display_col(ta);
        ta->has_preferred_col = 1;
    }
    ta->cursor_row = target;
    textarea_set_cursor_to_display_col(ta, ta->preferred_col);
    cursor_moved(ta);
}

static int utf8_backward_bytes(const char *line, int cursor_col)
{
    int back = 1;
    while (back < cursor_col &&
           ((unsigned char)line[cursor_col - back] & 0xC0) == 0x80) {
        back++;
    }
    return back;
}

static void merge_lines(ZephioTextArea *ta, int upper_row, int lower_row, int upper_keep_len)
{
    char *upper = ta->lines[upper_row];
    char *lower = ta->lines[lower_row];
    int lower_len = (int)strlen(lower);

    char *merged = (char *)malloc((size_t)(upper_keep_len + lower_len + 1));
    if (!merged) return;
    memcpy(merged, upper, (size_t)upper_keep_len);
    memcpy(merged + upper_keep_len, lower, (size_t)(lower_len + 1));
    free(ta->lines[upper_row]);
    ta->lines[upper_row] = merged;
    textarea_delete_line_at(ta, lower_row);
}

static int textarea_handle_input(ZephioWidget *widget, const ZephioEvent *event)
{
    ZephioTextArea *ta = (ZephioTextArea *)widget;

    switch (event->key) {
    case ZEPHIO_KEY_UP:
        move_vertical(ta, -1);
        return 1;

    case ZEPHIO_KEY_DOWN:
        move_vertical(ta, 1);
        return 1;

    case ZEPHIO_KEY_LEFT:
        ta->has_preferred_col = 0;
        if (ta->cursor_col > 0) {
            ta->cursor_col -= utf8_backward_bytes(ta->lines[ta->cursor_row], ta->cursor_col);
            cursor_moved(ta);
        } else if (ta->cursor_row > 0) {
            ta->cursor_row--;
            ta->cursor_col = line_len(ta, ta->cursor_row);
            cursor_moved(ta);
        }
        return 1;

    case ZEPHIO_KEY_RIGHT:
        ta->has_preferred_col = 0;
        {
            int ll = line_len(ta, ta->cursor_row);
            if (ta->cursor_col < ll) {
                int fwd = tui_utf8_char_len((unsigned char)ta->lines[ta->cursor_row][ta->cursor_col]);
                if (ta->cursor_col + fwd > ll) fwd = ll - ta->cursor_col;
                ta->cursor_col += fwd;
                cursor_moved(ta);
            } else if (ta->cursor_row < ta->line_count - 1) {
                ta->cursor_row++;
                ta->cursor_col = 0;
                cursor_moved(ta);
            }
        }
        return 1;

    case ZEPHIO_KEY_HOME:
        ta->has_preferred_col = 0;
        ta->cursor_col = 0;
        ta->scroll_x = 0;
        cursor_moved(ta);
        return 1;

    case ZEPHIO_KEY_END:
        ta->has_preferred_col = 0;
        ta->cursor_col = line_len(ta, ta->cursor_row);
        cursor_moved(ta);
        return 1;

    case ZEPHIO_KEY_PAGE_UP:
    case ZEPHIO_KEY_PAGE_DOWN:
        ta->has_preferred_col = 0;
        {
            int jump = widget->height > 1 ? widget->height - 1 : 1;
            if (event->key == ZEPHIO_KEY_PAGE_UP) {
                ta->cursor_row -= jump;
                if (ta->cursor_row < 0) ta->cursor_row = 0;
            } else {
                ta->cursor_row += jump;
                if (ta->cursor_row >= ta->line_count)
                    ta->cursor_row = ta->line_count - 1;
            }
            int ll = line_len(ta, ta->cursor_row);
            if (ta->cursor_col > ll) ta->cursor_col = ll;
            cursor_moved(ta);
        }
        return 1;

    case ZEPHIO_KEY_BACKSPACE:
        ta->has_preferred_col = 0;
        if (ta->cursor_col > 0) {
            char *line = ta->lines[ta->cursor_row];
            int ll = (int)strlen(line);
            int back = utf8_backward_bytes(line, ta->cursor_col);
            memmove(line + ta->cursor_col - back,
                    line + ta->cursor_col,
                    (size_t)(ll - ta->cursor_col + 1));
            ta->cursor_col -= back;
            cursor_moved(ta);
        } else if (ta->cursor_row > 0) {
            int prev_len = line_len(ta, ta->cursor_row - 1);
            merge_lines(ta, ta->cursor_row - 1, ta->cursor_row, prev_len);
            ta->cursor_row--;
            ta->cursor_col = prev_len;
            cursor_moved(ta);
        }
        return 1;

    case ZEPHIO_KEY_DELETE:
        ta->has_preferred_col = 0;
        {
            int ll = line_len(ta, ta->cursor_row);
            if (ta->cursor_col < ll) {
                char *line = ta->lines[ta->cursor_row];
                int fwd = tui_utf8_char_len((unsigned char)line[ta->cursor_col]);
                if (ta->cursor_col + fwd > ll) fwd = ll - ta->cursor_col;
                memmove(line + ta->cursor_col,
                        line + ta->cursor_col + fwd,
                        (size_t)(ll - ta->cursor_col - fwd + 1));
                widget->dirty = 1;
            } else if (ta->cursor_row < ta->line_count - 1) {
                merge_lines(ta, ta->cursor_row, ta->cursor_row + 1, ll);
                widget->dirty = 1;
            }
        }
        textarea_ensure_cursor_visible(ta);
        return 1;

    case ZEPHIO_KEY_ENTER:
        ta->has_preferred_col = 0;
        {
            char *old_line = ta->lines[ta->cursor_row];

            char *after = strdup(old_line + ta->cursor_col);
            old_line[ta->cursor_col] = '\0';

            char *before = strdup(old_line);
            free(ta->lines[ta->cursor_row]);
            ta->lines[ta->cursor_row] = before;

            textarea_insert_line_at(ta, ta->cursor_row + 1, after);
            free(after);

            ta->cursor_row++;
            ta->cursor_col = 0;
            cursor_moved(ta);
        }
        return 1;

    default:
        break;
    }

    if (event->codepoint >= 32 && event->key == ZEPHIO_KEY_UNKNOWN) {
        ta->has_preferred_col = 0;
        char enc[4];
        int char_len = tui_utf8_encode(event->codepoint, enc, sizeof(enc));
        if (char_len == 0) return 1;

        int ll = line_len(ta, ta->cursor_row);
        char *new_line = (char *)malloc((size_t)(ll + char_len + 1));
        if (!new_line) return 1;

        memcpy(new_line, ta->lines[ta->cursor_row], (size_t)ta->cursor_col);
        memcpy(new_line + ta->cursor_col, enc, (size_t)char_len);
        memcpy(new_line + ta->cursor_col + char_len,
               ta->lines[ta->cursor_row] + ta->cursor_col,
               (size_t)(ll - ta->cursor_col + 1));

        free(ta->lines[ta->cursor_row]);
        ta->lines[ta->cursor_row] = new_line;
        ta->cursor_col += char_len;

        cursor_moved(ta);
        return 1;
    }

    return 0;
}

static int textarea_handle_mouse(ZephioWidget *widget, const ZephioMouseEvent *mouse)
{
    ZephioTextArea *ta = (ZephioTextArea *)widget;

    if (mouse->action == ZEPHIO_MOUSE_PRESS && mouse->button == ZEPHIO_MOUSE_BTN_LEFT) {
        int rel_row = mouse->row - widget->abs_y;
        int rel_col = mouse->col - widget->abs_x;

        if (rel_row >= 0 && rel_row < widget->height &&
            rel_col >= 0 && rel_col < widget->width) {
            int target_row = ta->scroll_y + rel_row;
            if (target_row < 0) target_row = 0;
            if (target_row >= ta->line_count) target_row = ta->line_count - 1;

            ta->cursor_row = target_row;
            int display_col = ta->scroll_x + rel_col;
            int ll = line_len(ta, ta->cursor_row);
            ta->cursor_col = zephio_text_col_to_index(ta->lines[ta->cursor_row],
                                                    (size_t)ll, display_col);

            ta->has_preferred_col = 0;
            widget->dirty = 1;
        }
        return 1;
    }

    if (mouse->action == ZEPHIO_MOUSE_WHEEL_UP) {
        if (ta->scroll_y > 0) {
            ta->scroll_y -= 3;
            if (ta->scroll_y < 0) ta->scroll_y = 0;
            widget->dirty = 1;
        }
        return 1;
    }

    if (mouse->action == ZEPHIO_MOUSE_WHEEL_DOWN) {
        int max_scroll = ta->line_count - widget->height;
        if (max_scroll < 0) max_scroll = 0;
        if (ta->scroll_y < max_scroll) {
            ta->scroll_y += 3;
            if (ta->scroll_y > max_scroll) ta->scroll_y = max_scroll;
            widget->dirty = 1;
        }
        return 1;
    }

    return 0;
}

static void textarea_destroy(ZephioWidget *widget)
{
    ZephioTextArea *ta = (ZephioTextArea *)widget;
    textarea_free_lines(ta);
}

static ZephioWidgetVTable textarea_vtable = {
    .render       = textarea_render,
    .handle_input = textarea_handle_input,
    .handle_mouse = textarea_handle_mouse,
    .destroy      = textarea_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

ZephioResult zephio_textarea_init_ctx(ZephioTextArea *ta, ZephioContext *ctx, int x, int y,
                                  int width, int height)
{
    if (!ta) return TUI_ERR_MEMORY;

    ZephioResult res = zephio_widget_init_ctx(&ta->base, x, y, width, height,
                                        &textarea_vtable, ctx, NULL);
    if (res != ZEPHIO_OK) return res;

    ta->base.focusable = 1;

    ta->lines = NULL;
    ta->line_count = 0;
    ta->line_capacity = 0;

    textarea_ensure_capacity(ta, 1);
    if (ta->lines) {
        ta->lines[0] = strdup("");
        ta->line_count = 1;
    }

    ta->cursor_row = 0;
    ta->cursor_col = 0;
    ta->scroll_y = 0;
    ta->scroll_x = 0;
    ta->preferred_col = 0;
    ta->has_preferred_col = 0;

    ta->fg   = ZEPHIO_COLOR_INDEX(15);
    ta->bg   = ZEPHIO_COLOR_INDEX(234);
    ta->attr = ZEPHIO_ATTR_NONE;

    return ZEPHIO_OK;
}

void zephio_textarea_set_text(ZephioTextArea *ta, const char *text)
{
    if (!ta) return;
    textarea_free_lines(ta);

    if (!text) {
        textarea_ensure_capacity(ta, 1);
        if (ta->lines) {
            ta->lines[0] = strdup("");
            ta->line_count = 1;
        }
        ta->cursor_row = 0;
        ta->cursor_col = 0;
        ta->scroll_y = 0;
        ta->scroll_x = 0;
        ta->base.dirty = 1;
        return;
    }

    int nl_count = 1;
    for (const char *p = text; *p; p++) {
        if (*p == '\n') nl_count++;
    }
    textarea_ensure_capacity(ta, nl_count);

    const char *start = text;
    int idx = 0;
    for (const char *p = text; ; p++) {
        if (*p == '\n' || *p == '\0') {
            int len = (int)(p - start);
            char *line = (char *)malloc((size_t)(len + 1));
            if (line) {
                memcpy(line, start, (size_t)len);
                line[len] = '\0';
                if (idx < ta->line_capacity) {
                    ta->lines[idx++] = line;
                } else {
                    free(line);
                }
            }
            if (*p == '\0') break;
            start = p + 1;
        }
    }
    ta->line_count = idx;

    if (ta->line_count == 0) {
        ta->lines[0] = strdup("");
        ta->line_count = 1;
    }

    ta->cursor_row = 0;
    ta->cursor_col = 0;
    ta->scroll_y = 0;
    ta->scroll_x = 0;
    ta->base.dirty = 1;
}

char *zephio_textarea_get_text(ZephioTextArea *ta)
{
    if (!ta || !ta->lines) return strdup("");

    int total = 0;
    for (int i = 0; i < ta->line_count; i++) {
        total += (int)strlen(ta->lines[i]) + 1;
    }

    char *result = (char *)malloc((size_t)total + 1);
    if (!result) return NULL;

    int pos = 0;
    for (int i = 0; i < ta->line_count; i++) {
        int len = (int)strlen(ta->lines[i]);
        memcpy(result + pos, ta->lines[i], (size_t)len);
        pos += len;
        if (i < ta->line_count - 1) {
            result[pos++] = '\n';
        }
    }
    result[pos] = '\0';
    return result;
}

void zephio_textarea_set_colors(ZephioTextArea *ta, ZephioColor fg, ZephioColor bg,
                              ZephioAttr attr)
{
    if (!ta) return;
    ta->fg = fg;
    ta->bg = bg;
    ta->attr = attr;
    ta->base.dirty = 1;
}

int zephio_textarea_get_cursor_row(ZephioTextArea *ta)
{
    if (!ta) return 0;
    return ta->cursor_row;
}

int zephio_textarea_get_cursor_col(ZephioTextArea *ta)
{
    if (!ta) return 0;
    return ta->cursor_col;
}

int zephio_textarea_get_scroll_y(ZephioTextArea *ta)
{
    if (!ta) return 0;
    return ta->scroll_y;
}

int zephio_textarea_get_line_count(ZephioTextArea *ta)
{
    if (!ta) return 0;
    return ta->line_count;
}
