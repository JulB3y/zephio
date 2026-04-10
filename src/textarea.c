#define _POSIX_C_SOURCE 200809L

#include "tui_textarea.h"
#include "tui_text.h"

#include <stdlib.h>
#include <string.h>

#define LINES_INITIAL_CAPACITY 16
#define RENDER_BUF_SIZE 4096

static void textarea_free_lines(TuiTextArea *ta)
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

static int textarea_ensure_capacity(TuiTextArea *ta, int needed)
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

static void textarea_insert_line_at(TuiTextArea *ta, int index, const char *text)
{
    if (textarea_ensure_capacity(ta, ta->line_count + 1) != 0) return;
    memmove(&ta->lines[index + 1], &ta->lines[index],
            (size_t)(ta->line_count - index) * sizeof(char *));
    ta->lines[index] = text ? strdup(text) : strdup("");
    ta->line_count++;
}

static void textarea_delete_line_at(TuiTextArea *ta, int index)
{
    if (index < 0 || index >= ta->line_count) return;
    free(ta->lines[index]);
    memmove(&ta->lines[index], &ta->lines[index + 1],
            (size_t)(ta->line_count - index - 1) * sizeof(char *));
    ta->line_count--;
}

static void textarea_ensure_cursor_visible(TuiTextArea *ta)
{
    if (ta->cursor_row < ta->scroll_y) {
        ta->scroll_y = ta->cursor_row;
    }
    if (ta->cursor_row >= ta->scroll_y + ta->base.height) {
        ta->scroll_y = ta->cursor_row - ta->base.height + 1;
    }

    const char *line = ta->lines[ta->cursor_row];
    int line_len = line ? (int)strlen(line) : 0;
    int cursor_display = tui_text_index_to_col(line, (size_t)line_len,
                                                (size_t)ta->cursor_col);

    if (cursor_display < ta->scroll_x) {
        ta->scroll_x = cursor_display;
    }
    if (cursor_display >= ta->scroll_x + ta->base.width) {
        ta->scroll_x = cursor_display - ta->base.width + 1;
    }
}

static void textarea_render(TuiWidget *widget)
{
    TuiTextArea *ta = (TuiTextArea *)widget;

    TuiColor fg = ta->fg;
    TuiColor bg = ta->bg;
    TuiAttr attr = ta->attr;

    if (widget->theme) {
        TuiStyle style = tui_widget_get_style(widget);
        fg = style.fg;
        bg = style.bg;
        attr = style.attr;
    }

    tui_screen_fill(widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ", fg, bg, attr);

    char buf[RENDER_BUF_SIZE];
    int vp_height = widget->height;
    int vp_width = widget->width;

    for (int r = 0; r < vp_height; r++) {
        int line_idx = ta->scroll_y + r;
        if (line_idx >= ta->line_count) break;

        const char *line = ta->lines[line_idx];
        int line_len = line ? (int)strlen(line) : 0;
        if (line_len == 0) continue;

        int skip_bytes = tui_text_col_to_index(line, (size_t)line_len,
                                                ta->scroll_x);
        int remaining = line_len - skip_bytes;
        if (remaining <= 0) continue;

        size_t visible_bytes;
        tui_text_clip(line + skip_bytes, (size_t)remaining,
                      vp_width, &visible_bytes);

        if (visible_bytes > 0) {
            size_t copy = visible_bytes < sizeof(buf) - 1
                          ? visible_bytes : sizeof(buf) - 1;
            memcpy(buf, line + skip_bytes, copy);
            buf[copy] = '\0';
            tui_screen_write(widget->abs_y + r, widget->abs_x,
                             buf, fg, bg, attr);
        }
    }

    if (widget->focused && !widget->disabled) {
        int cursor_screen_row = ta->cursor_row - ta->scroll_y;
        if (cursor_screen_row >= 0 && cursor_screen_row < vp_height) {
            const char *line = ta->lines[ta->cursor_row];
            int line_len = line ? (int)strlen(line) : 0;
            int cursor_display = tui_text_index_to_col(line, (size_t)line_len,
                                                        (size_t)ta->cursor_col);
            int screen_col = cursor_display - ta->scroll_x;
            if (screen_col >= 0 && screen_col < vp_width) {
                tui_screen_invert_cell(widget->abs_y + cursor_screen_row,
                                       widget->abs_x + screen_col);
            }
        }
    }
}

static int textarea_get_display_col(TuiTextArea *ta)
{
    const char *line = ta->lines[ta->cursor_row];
    int line_len = line ? (int)strlen(line) : 0;
    return tui_text_index_to_col(line, (size_t)line_len,
                                  (size_t)ta->cursor_col);
}

static void textarea_set_cursor_to_display_col(TuiTextArea *ta, int display_col)
{
    const char *line = ta->lines[ta->cursor_row];
    int line_len = line ? (int)strlen(line) : 0;
    ta->cursor_col = tui_text_col_to_index(line, (size_t)line_len, display_col);
}

static int textarea_handle_input(TuiWidget *widget, const TuiEvent *event)
{
    TuiTextArea *ta = (TuiTextArea *)widget;

    switch (event->key) {
    case TUI_KEY_UP:
        if (ta->cursor_row > 0) {
            if (!ta->has_preferred_col) {
                ta->preferred_col = textarea_get_display_col(ta);
                ta->has_preferred_col = 1;
            }
            ta->cursor_row--;
            textarea_set_cursor_to_display_col(ta, ta->preferred_col);
            textarea_ensure_cursor_visible(ta);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_DOWN:
        if (ta->cursor_row < ta->line_count - 1) {
            if (!ta->has_preferred_col) {
                ta->preferred_col = textarea_get_display_col(ta);
                ta->has_preferred_col = 1;
            }
            ta->cursor_row++;
            textarea_set_cursor_to_display_col(ta, ta->preferred_col);
            textarea_ensure_cursor_visible(ta);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_LEFT:
        ta->has_preferred_col = 0;
        if (ta->cursor_col > 0) {
            int back = 1;
            while (back < ta->cursor_col &&
                   ((unsigned char)ta->lines[ta->cursor_row][ta->cursor_col - back] & 0xC0) == 0x80) {
                back++;
            }
            ta->cursor_col -= back;
            textarea_ensure_cursor_visible(ta);
            widget->dirty = 1;
        } else if (ta->cursor_row > 0) {
            ta->cursor_row--;
            ta->cursor_col = (int)strlen(ta->lines[ta->cursor_row]);
            textarea_ensure_cursor_visible(ta);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_RIGHT:
        ta->has_preferred_col = 0;
        {
            int line_len = (int)strlen(ta->lines[ta->cursor_row]);
            if (ta->cursor_col < line_len) {
                int fwd = tui_utf8_char_len((unsigned char)ta->lines[ta->cursor_row][ta->cursor_col]);
                if (ta->cursor_col + fwd > line_len) fwd = line_len - ta->cursor_col;
                ta->cursor_col += fwd;
                textarea_ensure_cursor_visible(ta);
                widget->dirty = 1;
            } else if (ta->cursor_row < ta->line_count - 1) {
                ta->cursor_row++;
                ta->cursor_col = 0;
                textarea_ensure_cursor_visible(ta);
                widget->dirty = 1;
            }
        }
        return 1;

    case TUI_KEY_HOME:
        ta->has_preferred_col = 0;
        ta->cursor_col = 0;
        ta->scroll_x = 0;
        textarea_ensure_cursor_visible(ta);
        widget->dirty = 1;
        return 1;

    case TUI_KEY_END:
        ta->has_preferred_col = 0;
        ta->cursor_col = (int)strlen(ta->lines[ta->cursor_row]);
        textarea_ensure_cursor_visible(ta);
        widget->dirty = 1;
        return 1;

    case TUI_KEY_PAGE_UP:
        ta->has_preferred_col = 0;
        {
            int jump = widget->height > 1 ? widget->height - 1 : 1;
            ta->cursor_row -= jump;
            if (ta->cursor_row < 0) ta->cursor_row = 0;
            int line_len = (int)strlen(ta->lines[ta->cursor_row]);
            if (ta->cursor_col > line_len) ta->cursor_col = line_len;
            textarea_ensure_cursor_visible(ta);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_PAGE_DOWN:
        ta->has_preferred_col = 0;
        {
            int jump = widget->height > 1 ? widget->height - 1 : 1;
            ta->cursor_row += jump;
            if (ta->cursor_row >= ta->line_count) ta->cursor_row = ta->line_count - 1;
            int line_len = (int)strlen(ta->lines[ta->cursor_row]);
            if (ta->cursor_col > line_len) ta->cursor_col = line_len;
            textarea_ensure_cursor_visible(ta);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_BACKSPACE:
        ta->has_preferred_col = 0;
        if (ta->cursor_col > 0) {
            char *line = ta->lines[ta->cursor_row];
            int line_len = (int)strlen(line);
            int back = 1;
            while (back < ta->cursor_col &&
                   ((unsigned char)line[ta->cursor_col - back] & 0xC0) == 0x80) {
                back++;
            }
            memmove(line + ta->cursor_col - back,
                    line + ta->cursor_col,
                    (size_t)(line_len - ta->cursor_col + 1));
            ta->cursor_col -= back;
            textarea_ensure_cursor_visible(ta);
            widget->dirty = 1;
        } else if (ta->cursor_row > 0) {
            char *prev_line = ta->lines[ta->cursor_row - 1];
            char *cur_line = ta->lines[ta->cursor_row];
            int prev_len = (int)strlen(prev_line);
            int cur_len = (int)strlen(cur_line);

            char *merged = (char *)malloc((size_t)(prev_len + cur_len + 1));
            if (merged) {
                memcpy(merged, prev_line, (size_t)prev_len);
                memcpy(merged + prev_len, cur_line, (size_t)(cur_len + 1));
                free(ta->lines[ta->cursor_row - 1]);
                ta->lines[ta->cursor_row - 1] = merged;
                textarea_delete_line_at(ta, ta->cursor_row);
                ta->cursor_row--;
                ta->cursor_col = prev_len;
            }
            textarea_ensure_cursor_visible(ta);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_DELETE:
        ta->has_preferred_col = 0;
        {
            int line_len = (int)strlen(ta->lines[ta->cursor_row]);
            if (ta->cursor_col < line_len) {
                char *line = ta->lines[ta->cursor_row];
                int fwd = tui_utf8_char_len((unsigned char)line[ta->cursor_col]);
                if (ta->cursor_col + fwd > line_len) fwd = line_len - ta->cursor_col;
                memmove(line + ta->cursor_col,
                        line + ta->cursor_col + fwd,
                        (size_t)(line_len - ta->cursor_col - fwd + 1));
                widget->dirty = 1;
            } else if (ta->cursor_row < ta->line_count - 1) {
                char *cur_line = ta->lines[ta->cursor_row];
                char *next_line = ta->lines[ta->cursor_row + 1];
                int cur_len = (int)strlen(cur_line);
                int next_len = (int)strlen(next_line);

                char *merged = (char *)malloc((size_t)(cur_len + next_len + 1));
                if (merged) {
                    memcpy(merged, cur_line, (size_t)cur_len);
                    memcpy(merged + cur_len, next_line, (size_t)(next_len + 1));
                    free(ta->lines[ta->cursor_row]);
                    ta->lines[ta->cursor_row] = merged;
                    textarea_delete_line_at(ta, ta->cursor_row + 1);
                }
                widget->dirty = 1;
            }
        }
        textarea_ensure_cursor_visible(ta);
        return 1;

    case TUI_KEY_ENTER:
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
            textarea_ensure_cursor_visible(ta);
            widget->dirty = 1;
        }
        return 1;

    default:
        break;
    }

    if (event->codepoint >= 32 && event->key == TUI_KEY_UNKNOWN) {
        ta->has_preferred_col = 0;
        char enc[4];
        int char_len = tui_utf8_encode(event->codepoint, enc, sizeof(enc));
        if (char_len == 0) return 1;

        char *line = ta->lines[ta->cursor_row];
        int line_len = (int)strlen(line);

        char *new_line = (char *)malloc((size_t)(line_len + char_len + 1));
        if (!new_line) return 1;

        memcpy(new_line, line, (size_t)ta->cursor_col);
        memcpy(new_line + ta->cursor_col, enc, (size_t)char_len);
        memcpy(new_line + ta->cursor_col + char_len,
               line + ta->cursor_col,
               (size_t)(line_len - ta->cursor_col + 1));

        free(ta->lines[ta->cursor_row]);
        ta->lines[ta->cursor_row] = new_line;
        ta->cursor_col += char_len;

        textarea_ensure_cursor_visible(ta);
        widget->dirty = 1;
        return 1;
    }

    return 0;
}

static int textarea_handle_mouse(TuiWidget *widget, const TuiMouseEvent *mouse)
{
    TuiTextArea *ta = (TuiTextArea *)widget;

    if (mouse->action == TUI_MOUSE_PRESS && mouse->button == TUI_MOUSE_BTN_LEFT) {
        int rel_row = mouse->row - widget->abs_y;
        int rel_col = mouse->col - widget->abs_x;

        if (rel_row >= 0 && rel_row < widget->height &&
            rel_col >= 0 && rel_col < widget->width) {
            int target_row = ta->scroll_y + rel_row;
            if (target_row < 0) target_row = 0;
            if (target_row >= ta->line_count) target_row = ta->line_count - 1;

            ta->cursor_row = target_row;
            int display_col = ta->scroll_x + rel_col;
            const char *line = ta->lines[ta->cursor_row];
            int line_len = line ? (int)strlen(line) : 0;
            ta->cursor_col = tui_text_col_to_index(line, (size_t)line_len, display_col);

            ta->has_preferred_col = 0;
            widget->dirty = 1;
        }
        return 1;
    }

    if (mouse->action == TUI_MOUSE_WHEEL_UP) {
        if (ta->scroll_y > 0) {
            ta->scroll_y -= 3;
            if (ta->scroll_y < 0) ta->scroll_y = 0;
            widget->dirty = 1;
        }
        return 1;
    }

    if (mouse->action == TUI_MOUSE_WHEEL_DOWN) {
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

static void textarea_destroy(TuiWidget *widget)
{
    TuiTextArea *ta = (TuiTextArea *)widget;
    textarea_free_lines(ta);
}

static TuiWidgetVTable textarea_vtable = {
    .render       = textarea_render,
    .handle_input = textarea_handle_input,
    .handle_mouse = textarea_handle_mouse,
    .destroy      = textarea_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_textarea_init(TuiTextArea *ta, int x, int y,
                             int width, int height)
{
    if (!ta) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init(&ta->base, x, y, width, height,
                                     &textarea_vtable, NULL);
    if (res != TUI_OK) return res;

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

    ta->fg   = TUI_COLOR_INDEX(15);
    ta->bg   = TUI_COLOR_INDEX(234);
    ta->attr = TUI_ATTR_NONE;

    return TUI_OK;
}

void tui_textarea_set_text(TuiTextArea *ta, const char *text)
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

char *tui_textarea_get_text(TuiTextArea *ta)
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

void tui_textarea_set_colors(TuiTextArea *ta, TuiColor fg, TuiColor bg,
                              TuiAttr attr)
{
    if (!ta) return;
    ta->fg = fg;
    ta->bg = bg;
    ta->attr = attr;
    ta->base.dirty = 1;
}

int tui_textarea_get_cursor_row(TuiTextArea *ta)
{
    if (!ta) return 0;
    return ta->cursor_row;
}

int tui_textarea_get_cursor_col(TuiTextArea *ta)
{
    if (!ta) return 0;
    return ta->cursor_col;
}

int tui_textarea_get_scroll_y(TuiTextArea *ta)
{
    if (!ta) return 0;
    return ta->scroll_y;
}

int tui_textarea_get_line_count(TuiTextArea *ta)
{
    if (!ta) return 0;
    return ta->line_count;
}
