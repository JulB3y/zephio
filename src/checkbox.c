#define _POSIX_C_SOURCE 200809L

#include "zephio_checkbox.h"
#include "zephio_context.h"

#include <stdlib.h>
#include <string.h>

static void checkbox_toggle(ZephioCheckbox *cb)
{
    if (cb->tristate) {
        switch (cb->state) {
        case TUI_CHECK_UNCHECKED:
            cb->state = TUI_CHECK_CHECKED;
            break;
        case TUI_CHECK_CHECKED:
            cb->state = TUI_CHECK_INDETERMINATE;
            break;
        case TUI_CHECK_INDETERMINATE:
            cb->state = TUI_CHECK_UNCHECKED;
            break;
        }
    } else {
        cb->state = (cb->state == TUI_CHECK_CHECKED)
                    ? TUI_CHECK_UNCHECKED
                    : TUI_CHECK_CHECKED;
    }
}

static void checkbox_render(ZephioWidget *widget)
{
    ZephioCheckbox *cb = (ZephioCheckbox *)widget;

    ZephioColor fg;
    ZephioColor bg;
    ZephioAttr attr;

    if (widget->theme) {
        ZephioStyle style = zephio_widget_get_style(widget);
        fg   = style.fg;
        bg   = style.bg;
        attr = style.attr;
    } else {
        fg   = cb->fg;
        bg   = cb->bg;
        attr = cb->attr;

        if (widget->focused) {
            fg   = cb->fg_focused;
            bg   = cb->bg_focused;
            attr |= ZEPHIO_ATTR_BOLD;
        }
    }

    zephio_screen_fill(widget->ctx, widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ", fg, bg, attr);

    const char *mark;
    switch (cb->state) {
    case TUI_CHECK_CHECKED:      mark = "[x]"; break;
    case TUI_CHECK_INDETERMINATE: mark = "[-]"; break;
    default:                      mark = "[ ]"; break;
    }

    zephio_screen_write(widget->ctx, widget->abs_y, widget->abs_x, mark, fg, bg, attr);

    if (cb->label) {
        int label_start = 4;
        int max_label = widget->width - label_start;
        if (max_label < 0) max_label = 0;

        int label_len = (int)strlen(cb->label);
        int write_len = label_len < max_label ? label_len : max_label;

        char buf[256];
        int copy_len = write_len < (int)sizeof(buf) - 1 ? write_len : (int)sizeof(buf) - 1;
        memcpy(buf, cb->label, (size_t)copy_len);
        buf[copy_len] = '\0';

        zephio_screen_write(widget->ctx, widget->abs_y, widget->abs_x + label_start,
                           buf, fg, bg, attr);
    }
}

static int checkbox_handle_input(ZephioWidget *widget, const ZephioEvent *event)
{
    ZephioCheckbox *cb = (ZephioCheckbox *)widget;

    if (event->codepoint == ' ' || event->key == ZEPHIO_KEY_ENTER) {
        checkbox_toggle(cb);
        widget->dirty = 1;
        if (cb->on_change) {
            cb->on_change(widget, cb->state, cb->user_data);
        }
        return 1;
    }

    return 0;
}

static int checkbox_handle_mouse(ZephioWidget *widget, const ZephioMouseEvent *mouse)
{
    ZephioCheckbox *cb = (ZephioCheckbox *)widget;

    if (mouse->action == ZEPHIO_MOUSE_PRESS && mouse->button == ZEPHIO_MOUSE_BTN_LEFT) {
        checkbox_toggle(cb);
        widget->dirty = 1;
        if (cb->on_change) {
            cb->on_change(widget, cb->state, cb->user_data);
        }
        return 1;
    }

    return 0;
}

static void checkbox_destroy(ZephioWidget *widget)
{
    ZephioCheckbox *cb = (ZephioCheckbox *)widget;
    free(cb->label);
    cb->label = NULL;
}

static ZephioWidgetVTable checkbox_vtable = {
    .render       = checkbox_render,
    .handle_input = checkbox_handle_input,
    .handle_mouse = checkbox_handle_mouse,
    .destroy      = checkbox_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

ZephioResult zephio_checkbox_init_ctx(ZephioCheckbox *checkbox, ZephioContext *ctx, int x, int y,
                                int width, int height, const char *label)
{
    if (!checkbox) return TUI_ERR_MEMORY;

    ZephioResult res = zephio_widget_init_ctx(&checkbox->base, x, y, width, height,
                                        &checkbox_vtable, ctx, NULL);
    if (res != ZEPHIO_OK) return res;

    checkbox->base.focusable = 1;

    checkbox->state     = TUI_CHECK_UNCHECKED;
    checkbox->tristate  = 0;
    checkbox->fg          = ZEPHIO_COLOR_INDEX(15);
    checkbox->bg          = ZEPHIO_COLOR_INDEX(0);
    checkbox->fg_focused  = ZEPHIO_COLOR_INDEX(0);
    checkbox->bg_focused  = ZEPHIO_COLOR_INDEX(12);
    checkbox->attr        = ZEPHIO_ATTR_NONE;
    checkbox->on_change   = NULL;
    checkbox->user_data   = NULL;

    if (label) {
        checkbox->label = strdup(label);
        if (!checkbox->label) return TUI_ERR_MEMORY;
    } else {
        checkbox->label = NULL;
    }

    return ZEPHIO_OK;
}

void zephio_checkbox_set_state(ZephioCheckbox *checkbox, ZephioCheckState state)
{
    if (!checkbox) return;
    checkbox->state = state;
    checkbox->base.dirty = 1;
}

ZephioCheckState zephio_checkbox_get_state(ZephioCheckbox *checkbox)
{
    if (!checkbox) return TUI_CHECK_UNCHECKED;
    return checkbox->state;
}

void zephio_checkbox_set_tristate(ZephioCheckbox *checkbox, int tristate)
{
    if (!checkbox) return;
    checkbox->tristate = tristate;
}

void zephio_checkbox_set_label(ZephioCheckbox *checkbox, const char *label)
{
    if (!checkbox) return;
    free(checkbox->label);
    checkbox->label = label ? strdup(label) : NULL;
    checkbox->base.dirty = 1;
}

void zephio_checkbox_set_colors(ZephioCheckbox *checkbox,
                             ZephioColor fg, ZephioColor bg,
                             ZephioColor fg_focused, ZephioColor bg_focused)
{
    if (!checkbox) return;
    checkbox->fg          = fg;
    checkbox->bg          = bg;
    checkbox->fg_focused  = fg_focused;
    checkbox->bg_focused  = bg_focused;
    checkbox->base.dirty  = 1;
}

void zephio_checkbox_set_on_change(ZephioCheckbox *checkbox,
                                ZephioCheckboxCallback callback,
                                void *user_data)
{
    if (!checkbox) return;
    checkbox->on_change  = callback;
    checkbox->user_data  = user_data;
}
