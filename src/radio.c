#define _POSIX_C_SOURCE 200809L

#include "tui_radio.h"
#include "tui_context.h"

#include <stdlib.h>
#include <string.h>

static void radio_render(TuiWidget *widget)
{
    TuiRadio *radio = (TuiRadio *)widget;

    for (int i = 0; i < widget->height; i++) {
        int idx = i;
        if (idx >= radio->option_count) {
            tui_screen_fill(tui_current_ctx, widget->abs_y + i, widget->abs_x,
                            widget->width, 1, " ",
                            radio->fg, radio->bg, radio->attr);
            continue;
        }

        TuiColor fg;
        TuiColor bg;
        TuiAttr attr;

        if (widget->theme) {
            TuiWidgetState state = TUI_STATE_NORMAL;
            if (widget->disabled) {
                state = TUI_STATE_DISABLED;
            } else if (idx == radio->selected && widget->focused) {
                state = TUI_STATE_FOCUSED;
            }
            TuiStyle style = widget->theme->styles[state];
            fg   = style.fg;
            bg   = style.bg;
            attr = style.attr;
        } else {
            fg   = radio->fg;
            bg   = radio->bg;
            attr = radio->attr;

            if (idx == radio->selected && widget->focused) {
                fg   = radio->fg_selected;
                bg   = radio->bg_selected;
                attr |= TUI_ATTR_REVERSE;
            }
        }

        tui_screen_fill(tui_current_ctx, widget->abs_y + i, widget->abs_x,
                        widget->width, 1, " ", fg, bg, attr);

        const char *marker = (idx == radio->selected) ? "(x)" : "( )";
        tui_screen_write(tui_current_ctx, widget->abs_y + i, widget->abs_x,
                          marker, fg, bg, attr);

        if (radio->options[idx]) {
            int label_start = 4;
            int max_width = widget->width - label_start;
            if (max_width < 0) max_width = 0;

            int item_len = (int)strlen(radio->options[idx]);
            int write_len = item_len < max_width ? item_len : max_width;

            char buf[256];
            int copy_len = write_len < (int)sizeof(buf) - 1
                           ? write_len : (int)sizeof(buf) - 1;
            memcpy(buf, radio->options[idx], (size_t)copy_len);
            buf[copy_len] = '\0';

            tui_screen_write(tui_current_ctx, widget->abs_y + i,
                              widget->abs_x + label_start,
                              buf, fg, bg, attr);
        }
    }
}

static int radio_handle_input(TuiWidget *widget, const TuiEvent *event)
{
    TuiRadio *radio = (TuiRadio *)widget;

    if (radio->option_count == 0) return 0;

    if (event->key == TUI_KEY_UP) {
        if (radio->selected > 0) {
            radio->selected--;
            widget->dirty = 1;
            if (radio->on_change) {
                radio->on_change(widget, radio->selected,
                                 radio->options[radio->selected],
                                 radio->user_data);
            }
        }
        return 1;
    }

    if (event->key == TUI_KEY_DOWN) {
        if (radio->selected < radio->option_count - 1) {
            radio->selected++;
            widget->dirty = 1;
            if (radio->on_change) {
                radio->on_change(widget, radio->selected,
                                 radio->options[radio->selected],
                                 radio->user_data);
            }
        }
        return 1;
    }

    if (event->key == TUI_KEY_ENTER) {
        if (radio->on_change && radio->option_count > 0) {
            radio->on_change(widget, radio->selected,
                             radio->options[radio->selected],
                             radio->user_data);
        }
        return 1;
    }

    return 0;
}

static int radio_handle_mouse(TuiWidget *widget, const TuiMouseEvent *mouse)
{
    TuiRadio *radio = (TuiRadio *)widget;

    if (mouse->action != TUI_MOUSE_PRESS || mouse->button != TUI_MOUSE_BTN_LEFT)
        return 0;

    int idx = mouse->row - widget->abs_y;
    if (idx < 0 || idx >= radio->option_count)
        return 0;

    if (radio->selected != idx) {
        radio->selected = idx;
        widget->dirty = 1;
        if (radio->on_change) {
            radio->on_change(widget, idx, radio->options[idx],
                             radio->user_data);
        }
    }

    return 1;
}

static void radio_destroy(TuiWidget *widget)
{
    TuiRadio *radio = (TuiRadio *)widget;

    for (int i = 0; i < radio->option_count; i++) {
        free(radio->options[i]);
    }
    free(radio->options);
    radio->options        = NULL;
    radio->option_count   = 0;
    radio->option_capacity = 0;
}

static TuiWidgetVTable radio_vtable = {
    .render       = radio_render,
    .handle_input = radio_handle_input,
    .handle_mouse = radio_handle_mouse,
    .destroy      = radio_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_radio_init(TuiRadio *radio, int x, int y, int width, int height)
{
    if (!radio) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init(&radio->base, x, y, width, height,
                                    &radio_vtable, NULL);
    if (res != TUI_OK) return res;

    radio->base.focusable = 1;

    radio->options         = NULL;
    radio->option_count    = 0;
    radio->option_capacity = 0;
    radio->selected        = 0;
    radio->fg              = TUI_COLOR_INDEX(15);
    radio->bg              = TUI_COLOR_INDEX(0);
    radio->fg_selected     = TUI_COLOR_INDEX(0);
    radio->bg_selected     = TUI_COLOR_INDEX(12);
    radio->attr            = TUI_ATTR_NONE;
    radio->on_change       = NULL;
    radio->user_data       = NULL;

    return TUI_OK;
}

TuiResult tui_radio_add_option(TuiRadio *radio, const char *option)
{
    if (!radio) return TUI_ERR_MEMORY;

    if (radio->option_count >= radio->option_capacity) {
        int new_cap = radio->option_capacity == 0
                      ? TUI_RADIO_OPTIONS_INITIAL
                      : radio->option_capacity * 2;

        char **new_options = (char **)realloc(
            radio->options, (size_t)new_cap * sizeof(char *));
        if (!new_options) return TUI_ERR_MEMORY;

        radio->options        = new_options;
        radio->option_capacity = new_cap;
    }

    radio->options[radio->option_count] = option ? strdup(option) : NULL;
    radio->option_count++;
    radio->base.dirty = 1;

    return TUI_OK;
}

void tui_radio_remove_option(TuiRadio *radio, int index)
{
    if (!radio || index < 0 || index >= radio->option_count) return;

    free(radio->options[index]);
    memmove(&radio->options[index], &radio->options[index + 1],
            (size_t)(radio->option_count - index - 1) * sizeof(char *));
    radio->option_count--;

    if (radio->selected >= radio->option_count && radio->option_count > 0) {
        radio->selected = radio->option_count - 1;
    }

    radio->base.dirty = 1;
}

void tui_radio_clear(TuiRadio *radio)
{
    if (!radio) return;

    for (int i = 0; i < radio->option_count; i++) {
        free(radio->options[i]);
    }
    radio->option_count = 0;
    radio->selected     = 0;
    radio->base.dirty   = 1;
}

void tui_radio_set_selected(TuiRadio *radio, int index)
{
    if (!radio) return;
    if (index < 0) index = 0;
    if (index >= radio->option_count) index = radio->option_count - 1;
    radio->selected = index;
    radio->base.dirty = 1;
}

int tui_radio_get_selected(TuiRadio *radio)
{
    if (!radio) return -1;
    return radio->selected;
}

const char *tui_radio_get_selected_option(TuiRadio *radio)
{
    if (!radio || radio->option_count == 0) return NULL;
    return radio->options[radio->selected];
}

void tui_radio_set_colors(TuiRadio *radio,
                          TuiColor fg, TuiColor bg,
                          TuiColor fg_selected, TuiColor bg_selected)
{
    if (!radio) return;
    radio->fg           = fg;
    radio->bg           = bg;
    radio->fg_selected  = fg_selected;
    radio->bg_selected  = bg_selected;
    radio->base.dirty   = 1;
}

void tui_radio_set_on_change(TuiRadio *radio, TuiRadioCallback callback,
                             void *user_data)
{
    if (!radio) return;
    radio->on_change = callback;
    radio->user_data = user_data;
}
