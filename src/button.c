#define _POSIX_C_SOURCE 200809L

#include "zephio_button.h"
#include "zephio_context.h"

#include <stdlib.h>
#include <string.h>

static void button_render(ZephioWidget *widget)
{
    ZephioButton *button = (ZephioButton *)widget;

    ZephioColor fg;
    ZephioColor bg;
    ZephioAttr attr;

    if (widget->theme) {
        ZephioStyle style = zephio_widget_get_style(widget);
        fg   = style.fg;
        bg   = style.bg;
        attr = style.attr;
    } else {
        fg   = button->fg;
        bg   = button->bg;
        attr = button->attr;

        if (widget->focused) {
            fg   = button->fg_focused;
            bg   = button->bg_focused;
            attr |= ZEPHIO_ATTR_BOLD;
        }
    }

    zephio_screen_fill(widget->ctx, widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ", fg, bg, attr);

    if (button->text) {
        int text_len = (int)strlen(button->text);
        int max_width = widget->width - 2;
        if (max_width < 0) max_width = 0;
        int write_len = text_len < max_width ? text_len : max_width;
        int col = widget->abs_x + (widget->width - write_len) / 2;

        char buf[256];
        int copy_len = write_len < (int)sizeof(buf) - 1 ? write_len : (int)sizeof(buf) - 1;
        memcpy(buf, button->text, (size_t)copy_len);
        buf[copy_len] = '\0';

        zephio_screen_write(widget->ctx, widget->abs_y + widget->height / 2, col,
                         buf, fg, bg, attr);
    }
}

static int button_handle_input(ZephioWidget *widget, const ZephioEvent *event)
{
    ZephioButton *button = (ZephioButton *)widget;

    if (event->key == ZEPHIO_KEY_ENTER || event->codepoint == ' ') {
        if (button->on_click) {
            button->on_click(widget, button->user_data);
        }
        return 1;
    }

    return 0;
}

static int button_handle_mouse(ZephioWidget *widget, const ZephioMouseEvent *mouse)
{
    ZephioButton *button = (ZephioButton *)widget;

    if (mouse->action == ZEPHIO_MOUSE_PRESS && mouse->button == ZEPHIO_MOUSE_BTN_LEFT) {
        if (button->on_click) {
            button->on_click(widget, button->user_data);
        }
        return 1;
    }

    return 0;
}

static void button_destroy(ZephioWidget *widget)
{
    ZephioButton *button = (ZephioButton *)widget;
    free(button->text);
    button->text = NULL;
}

static ZephioWidgetVTable button_vtable = {
    .render       = button_render,
    .handle_input = button_handle_input,
    .handle_mouse = button_handle_mouse,
    .destroy      = button_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

ZephioResult zephio_button_init_ctx(ZephioButton *button, ZephioContext *ctx, int x, int y, int width, int height,
                              const char *text)
{
    if (!button) return TUI_ERR_MEMORY;

    ZephioResult res = zephio_widget_init_ctx(&button->base, x, y, width, height,
                                        &button_vtable, ctx, NULL);
    if (res != ZEPHIO_OK) return res;

    button->base.focusable = 1;

    button->fg          = ZEPHIO_COLOR_INDEX(15);
    button->bg          = ZEPHIO_COLOR_INDEX(0);
    button->fg_focused  = ZEPHIO_COLOR_INDEX(0);
    button->bg_focused  = ZEPHIO_COLOR_INDEX(12);
    button->attr        = ZEPHIO_ATTR_NONE;
    button->on_click    = NULL;
    button->user_data   = NULL;

    if (text) {
        button->text = strdup(text);
        if (!button->text) return TUI_ERR_MEMORY;
    } else {
        button->text = NULL;
    }

    return ZEPHIO_OK;
}

void zephio_button_set_text(ZephioButton *button, const char *text)
{
    if (!button) return;
    free(button->text);
    button->text = text ? strdup(text) : NULL;
    button->base.dirty = 1;
}

void zephio_button_set_colors(ZephioButton *button, ZephioColor fg, ZephioColor bg,
                           ZephioColor fg_focused, ZephioColor bg_focused)
{
    if (!button) return;
    button->fg          = fg;
    button->bg          = bg;
    button->fg_focused  = fg_focused;
    button->bg_focused  = bg_focused;
    button->base.dirty  = 1;
}

void zephio_button_set_on_click(ZephioButton *button, ZephioButtonCallback callback,
                             void *user_data)
{
    if (!button) return;
    button->on_click  = callback;
    button->user_data = user_data;
}
