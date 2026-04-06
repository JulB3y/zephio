#define _POSIX_C_SOURCE 200809L

#include "tui_input_field.h"

#include <stdlib.h>
#include <string.h>

static void input_field_update_scroll(TuiInputField *field)
{
    if (field->cursor_pos < field->scroll_offset) {
        field->scroll_offset = field->cursor_pos;
    }

    int visible = field->base.width;
    if (field->cursor_pos >= field->scroll_offset + visible) {
        field->scroll_offset = field->cursor_pos - visible + 1;
    }
}

static void input_field_render(TuiWidget *widget)
{
    TuiInputField *field = (TuiInputField *)widget;

    uint8_t fg;
    uint8_t bg;
    TuiAttr attr;

    if (widget->theme) {
        TuiStyle style = tui_widget_get_style(widget);
        fg   = style.fg;
        bg   = style.bg;
        attr = style.attr;
    } else {
        fg   = field->fg;
        bg   = field->bg;
        attr = field->attr;
    }

    tui_screen_fill(widget->abs_y, widget->abs_x,
                    widget->width, 1, " ", fg, bg, attr);

    if (field->text && field->text[0]) {
        int text_len = (int)strlen(field->text);
        int visible = widget->width;
        int start = field->scroll_offset;
        int end = start + visible;
        if (end > text_len) end = text_len;

        if (start < text_len) {
            char buf[1024];
            int copy_len = end - start;
            if (copy_len >= (int)sizeof(buf)) copy_len = (int)sizeof(buf) - 1;
            memcpy(buf, field->text + start, (size_t)copy_len);
            buf[copy_len] = '\0';

            tui_screen_write(widget->abs_y, widget->abs_x,
                             buf, fg, bg, attr);
        }
    }

    if (widget->focused && !widget->disabled) {
        int cursor_col = field->cursor_pos - field->scroll_offset;
        if (cursor_col >= 0 && cursor_col < widget->width) {
            char cursor_ch[4] = " ";
            if (field->text && field->cursor_pos < (int)strlen(field->text)) {
                int len = 1;
                unsigned char c = (unsigned char)field->text[field->cursor_pos];
                if ((c & 0x80) == 0x00) len = 1;
                else if ((c & 0xE0) == 0xC0) len = 2;
                else if ((c & 0xF0) == 0xE0) len = 3;
                else if ((c & 0xF8) == 0xF0) len = 4;
                memcpy(cursor_ch, field->text + field->cursor_pos, (size_t)len);
                cursor_ch[len] = '\0';
            }

            uint8_t cursor_fg = field->cursor_fg;
            uint8_t cursor_bg = field->cursor_bg;
            if (widget->theme) {
                TuiStyle focused = widget->theme->styles[TUI_STATE_FOCUSED];
                cursor_fg = focused.bg;
                cursor_bg = focused.fg;
            }
            tui_screen_set_cell(widget->abs_y, widget->abs_x + cursor_col,
                                cursor_ch, cursor_fg, cursor_bg,
                                TUI_ATTR_REVERSE);
        }
    }
}

static int input_field_handle_input(TuiWidget *widget, const TuiEvent *event)
{
    TuiInputField *field = (TuiInputField *)widget;
    int text_len = field->text ? (int)strlen(field->text) : 0;

    if (event->key == TUI_KEY_LEFT) {
        if (field->cursor_pos > 0) {
            field->cursor_pos--;
            input_field_update_scroll(field);
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == TUI_KEY_RIGHT) {
        if (field->cursor_pos < text_len) {
            field->cursor_pos++;
            input_field_update_scroll(field);
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == TUI_KEY_HOME) {
        field->cursor_pos = 0;
        input_field_update_scroll(field);
        widget->dirty = 1;
        return 1;
    }

    if (event->key == TUI_KEY_END) {
        field->cursor_pos = text_len;
        input_field_update_scroll(field);
        widget->dirty = 1;
        return 1;
    }

    if (event->key == TUI_KEY_BACKSPACE) {
        if (field->cursor_pos > 0 && text_len > 0) {
            memmove(field->text + field->cursor_pos - 1,
                    field->text + field->cursor_pos,
                    (size_t)(text_len - field->cursor_pos + 1));
            field->cursor_pos--;
            input_field_update_scroll(field);
            widget->dirty = 1;
            if (field->on_change) {
                field->on_change(widget, field->text, field->user_data);
            }
        }
        return 1;
    }

    if (event->key == TUI_KEY_DELETE) {
        if (field->cursor_pos < text_len && text_len > 0) {
            memmove(field->text + field->cursor_pos,
                    field->text + field->cursor_pos + 1,
                    (size_t)(text_len - field->cursor_pos));
            widget->dirty = 1;
            if (field->on_change) {
                field->on_change(widget, field->text, field->user_data);
            }
        }
        return 1;
    }

    if (event->key == TUI_KEY_ENTER) {
        if (field->on_submit) {
            field->on_submit(widget, field->text, field->user_data);
        }
        return 1;
    }

    if (event->codepoint >= 32 && event->codepoint < 127 && event->key == TUI_KEY_UNKNOWN) {
        if (text_len + 1 < field->text_capacity) {
            if (!field->text) {
                field->text = (char *)calloc((size_t)field->text_capacity, 1);
                if (!field->text) return 1;
                text_len = 0;
            }

            memmove(field->text + field->cursor_pos + 1,
                    field->text + field->cursor_pos,
                    (size_t)(text_len - field->cursor_pos + 1));
            field->text[field->cursor_pos] = (char)event->codepoint;
            field->cursor_pos++;
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

static void input_field_destroy(TuiWidget *widget)
{
    TuiInputField *field = (TuiInputField *)widget;
    free(field->text);
    field->text = NULL;
}

static TuiWidgetVTable input_field_vtable = {
    .render       = input_field_render,
    .handle_input = input_field_handle_input,
    .handle_mouse = NULL,
    .destroy      = input_field_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_input_field_init(TuiInputField *field, int x, int y, int width,
                               int capacity)
{
    if (!field) return TUI_ERR_MEMORY;
    if (capacity <= 0) capacity = 256;

    TuiResult res = tui_widget_init(&field->base, x, y, width, 1,
                                    &input_field_vtable, NULL);
    if (res != TUI_OK) return res;

    field->base.focusable = 1;

    field->text           = NULL;
    field->text_capacity  = capacity;
    field->cursor_pos     = 0;
    field->scroll_offset  = 0;
    field->fg             = 15;
    field->bg             = 234;
    field->cursor_fg      = 0;
    field->cursor_bg      = 15;
    field->attr           = TUI_ATTR_NONE;
    field->on_change      = NULL;
    field->on_submit      = NULL;
    field->user_data      = NULL;

    return TUI_OK;
}

void tui_input_field_set_text(TuiInputField *field, const char *text)
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

const char *tui_input_field_get_text(TuiInputField *field)
{
    if (!field) return NULL;
    return field->text ? field->text : "";
}

void tui_input_field_set_colors(TuiInputField *field, uint8_t fg, uint8_t bg,
                                uint8_t cursor_fg, uint8_t cursor_bg)
{
    if (!field) return;
    field->fg         = fg;
    field->bg         = bg;
    field->cursor_fg  = cursor_fg;
    field->cursor_bg  = cursor_bg;
    field->base.dirty = 1;
}

void tui_input_field_set_on_change(TuiInputField *field,
                                   TuiInputFieldCallback callback,
                                   void *user_data)
{
    if (!field) return;
    field->on_change = callback;
    field->user_data = user_data;
}

void tui_input_field_set_on_submit(TuiInputField *field,
                                   TuiInputFieldCallback callback,
                                   void *user_data)
{
    if (!field) return;
    field->on_submit = callback;
    field->user_data = user_data;
}
