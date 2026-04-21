#define _POSIX_C_SOURCE 200809L

#include "zephio_input_field.h"
#include "zephio_context.h"
#include "zephio_text.h"

#include <stdlib.h>
#include <string.h>

static void input_field_update_scroll(ZephioInputField *field)
{
    if (!field->text) {
        field->scroll_offset = 0;
        return;
    }

    int text_len = (int)strlen(field->text);
    int visible = field->base.width;

    int cursor_col = zephio_text_index_to_col(field->text, (size_t)text_len, (size_t)field->cursor_pos);
    int scroll_col = zephio_text_index_to_col(field->text, (size_t)text_len, (size_t)field->scroll_offset);

    if (cursor_col < scroll_col) {
        scroll_col = cursor_col;
    }

    if (cursor_col >= scroll_col + visible) {
        scroll_col = cursor_col - visible + 1;
    }

    field->scroll_offset = zephio_text_col_to_index(field->text, (size_t)text_len, scroll_col);
}

static void input_field_render(ZephioWidget *widget)
{
    ZephioInputField *field = (ZephioInputField *)widget;

    ZephioColor fg;
    ZephioColor bg;
    ZephioAttr attr;

    if (widget->theme) {
        ZephioStyle style = zephio_widget_get_style(widget);
        fg   = style.fg;
        bg   = style.bg;
        attr = style.attr;
    } else {
        fg   = field->fg;
        bg   = field->bg;
        attr = field->attr;
    }

    zephio_screen_fill(widget->ctx, widget->abs_y, widget->abs_x,
                    widget->width, 1, " ", fg, bg, attr);

    if (field->text && field->text[0]) {
        int text_len = (int)strlen(field->text);
        int visible = widget->width;
        int start = field->scroll_offset;

        if (start < text_len) {
            size_t clip_len;
            zephio_text_clip(field->text + start, (size_t)(text_len - start),
                          visible, &clip_len);

            char buf[1024];
            int copy_len = (int)clip_len;
            if (copy_len >= (int)sizeof(buf)) copy_len = (int)sizeof(buf) - 1;
            memcpy(buf, field->text + start, (size_t)copy_len);
            buf[copy_len] = '\0';

            zephio_screen_write(widget->ctx, widget->abs_y, widget->abs_x,
                             buf, fg, bg, attr);
        }
    }

    if (widget->focused && !widget->disabled) {
        int text_len = field->text ? (int)strlen(field->text) : 0;
        int cursor_col = 0;
        int pos = field->scroll_offset;
        while (pos < field->cursor_pos && pos < text_len) {
            uint32_t cp;
            int clen = tui_utf8_next(field->text + pos, text_len - pos, &cp);
            if (clen <= 0) break;
            cursor_col += tui_utf8_char_width(cp);
            pos += clen;
        }
        if (cursor_col >= 0 && cursor_col < widget->width) {
            ZephioColor cursor_fg = field->cursor_fg;
            ZephioColor cursor_bg = field->cursor_bg;
            if (widget->theme) {
                ZephioStyle focused = widget->theme->styles[ZEPHIO_STATE_FOCUSED];
                cursor_fg = focused.bg;
                cursor_bg = focused.fg;
            }
            const char *cursor_ch = " ";
            if (field->cursor_pos < text_len) {
                cursor_ch = field->text + field->cursor_pos;
            }
            zephio_screen_set_cell(widget->ctx, widget->abs_y, widget->abs_x + cursor_col,
                                cursor_ch, cursor_fg, cursor_bg,
                                ZEPHIO_ATTR_NONE);
        }
    }
}

static int input_field_handle_input(ZephioWidget *widget, const ZephioEvent *event)
{
    ZephioInputField *field = (ZephioInputField *)widget;
    int text_len = field->text ? (int)strlen(field->text) : 0;

    if (event->key == ZEPHIO_KEY_LEFT) {
        if (field->cursor_pos > 0) {
            int back = 1;
            while (back < field->cursor_pos &&
                   ((unsigned char)field->text[field->cursor_pos - back] & 0xC0) == 0x80) {
                back++;
            }
            field->cursor_pos -= back;
            input_field_update_scroll(field);
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == ZEPHIO_KEY_RIGHT) {
        if (field->cursor_pos < text_len) {
            int fwd = tui_utf8_char_len((unsigned char)field->text[field->cursor_pos]);
            if (field->cursor_pos + fwd > text_len) fwd = text_len - field->cursor_pos;
            field->cursor_pos += fwd;
            input_field_update_scroll(field);
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == ZEPHIO_KEY_HOME) {
        field->cursor_pos = 0;
        input_field_update_scroll(field);
        widget->dirty = 1;
        return 1;
    }

    if (event->key == ZEPHIO_KEY_END) {
        field->cursor_pos = text_len;
        input_field_update_scroll(field);
        widget->dirty = 1;
        return 1;
    }

    if (event->key == ZEPHIO_KEY_BACKSPACE) {
        if (field->cursor_pos > 0 && text_len > 0) {
            int back = 1;
            while (back < field->cursor_pos &&
                   ((unsigned char)field->text[field->cursor_pos - back] & 0xC0) == 0x80) {
                back++;
            }
            memmove(field->text + field->cursor_pos - back,
                    field->text + field->cursor_pos,
                    (size_t)(text_len - field->cursor_pos + 1));
            field->cursor_pos -= back;
            input_field_update_scroll(field);
            widget->dirty = 1;
            if (field->on_change) {
                field->on_change(widget, field->text, field->user_data);
            }
        }
        return 1;
    }

    if (event->key == ZEPHIO_KEY_DELETE) {
        if (field->cursor_pos < text_len && text_len > 0) {
            int fwd = tui_utf8_char_len((unsigned char)field->text[field->cursor_pos]);
            if (field->cursor_pos + fwd > text_len) fwd = text_len - field->cursor_pos;
            memmove(field->text + field->cursor_pos,
                    field->text + field->cursor_pos + fwd,
                    (size_t)(text_len - field->cursor_pos - fwd + 1));
            widget->dirty = 1;
            if (field->on_change) {
                field->on_change(widget, field->text, field->user_data);
            }
        }
        return 1;
    }

    if (event->key == ZEPHIO_KEY_ENTER) {
        if (field->on_submit) {
            field->on_submit(widget, field->text, field->user_data);
        }
        return 1;
    }

    if (event->codepoint >= 32 && event->key == ZEPHIO_KEY_UNKNOWN) {
        char enc[4];
        int char_len = tui_utf8_encode(event->codepoint, enc, sizeof(enc));
        if (char_len == 0) return 1;

        if (text_len + char_len < field->text_capacity) {
            if (!field->text) {
                field->text = (char *)calloc((size_t)field->text_capacity, 1);
                if (!field->text) return 1;
                text_len = 0;
            }

            memmove(field->text + field->cursor_pos + char_len,
                    field->text + field->cursor_pos,
                    (size_t)(text_len - field->cursor_pos + 1));
            memcpy(field->text + field->cursor_pos, enc, (size_t)char_len);
            field->cursor_pos += char_len;
            input_field_update_scroll(field);
            widget->dirty = 1;

            if (field->on_change) {
                field->on_change(widget, field->text, field->user_data);
            }
        }
        return 1;
    }

    return 0;
}

static void input_field_destroy(ZephioWidget *widget)
{
    ZephioInputField *field = (ZephioInputField *)widget;
    free(field->text);
    field->text = NULL;
}

static ZephioWidgetVTable input_field_vtable = {
    .render       = input_field_render,
    .handle_input = input_field_handle_input,
    .handle_mouse = NULL,
    .destroy      = input_field_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

ZephioResult zephio_input_field_init_ctx(ZephioInputField *field, ZephioContext *ctx, int x, int y, int width,
                                   int capacity)
{
    if (!field) return TUI_ERR_MEMORY;
    if (capacity <= 0) capacity = 256;

    ZephioResult res = zephio_widget_init_ctx(&field->base, x, y, width, 1,
                                        &input_field_vtable, ctx, NULL);
    if (res != ZEPHIO_OK) return res;

    field->base.focusable = 1;

    field->text           = NULL;
    field->text_capacity  = capacity;
    field->cursor_pos     = 0;
    field->scroll_offset  = 0;
    field->fg             = ZEPHIO_COLOR_INDEX(15);
    field->bg             = ZEPHIO_COLOR_INDEX(234);
    field->cursor_fg      = ZEPHIO_COLOR_INDEX(0);
    field->cursor_bg      = ZEPHIO_COLOR_INDEX(15);
    field->attr           = ZEPHIO_ATTR_NONE;
    field->on_change      = NULL;
    field->on_submit      = NULL;
    field->user_data      = NULL;

    return ZEPHIO_OK;
}

void zephio_input_field_set_text(ZephioInputField *field, const char *text)
{
    if (!field) return;
    free(field->text);

    if (text) {
        int len = (int)strlen(text);
        if (len >= field->text_capacity) len = field->text_capacity - 1;
        field->text = (char *)calloc((size_t)field->text_capacity, 1);
        if (field->text) {
            memcpy(field->text, text, (size_t)len);
            field->text[len] = '\0';
        }
        field->cursor_pos = len;
    } else {
        field->text = NULL;
        field->cursor_pos = 0;
    }

    input_field_update_scroll(field);
    field->base.dirty = 1;
}

const char *zephio_input_field_get_text(ZephioInputField *field)
{
    if (!field) return NULL;
    return field->text ? field->text : "";
}

void zephio_input_field_set_colors(ZephioInputField *field, ZephioColor fg, ZephioColor bg,
                                ZephioColor cursor_fg, ZephioColor cursor_bg)
{
    if (!field) return;
    field->fg         = fg;
    field->bg         = bg;
    field->cursor_fg  = cursor_fg;
    field->cursor_bg  = cursor_bg;
    field->base.dirty = 1;
}

void zephio_input_field_set_on_change(ZephioInputField *field,
                                   ZephioInputFieldCallback callback,
                                   void *user_data)
{
    if (!field) return;
    field->on_change = callback;
    field->user_data = user_data;
}

void zephio_input_field_set_on_submit(ZephioInputField *field,
                                   ZephioInputFieldCallback callback,
                                   void *user_data)
{
    if (!field) return;
    field->on_submit = callback;
    field->user_data = user_data;
}
