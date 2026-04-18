#define _POSIX_C_SOURCE 200809L

#include "tui_button.h"
#include "tui_context.h"

#include <stdlib.h>
#include <string.h>

static void button_render(TuiWidget *widget)
{
    TuiButton *button = (TuiButton *)widget;

    TuiColor fg;
    TuiColor bg;
    TuiAttr attr;

    if (widget->theme) {
        TuiStyle style = tui_widget_get_style(widget);
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
            attr |= TUI_ATTR_BOLD;
        }
    }

    tui_screen_fill(tui_current_ctx,widget->abs_y, widget->abs_x,
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

        tui_screen_write(tui_current_ctx,widget->abs_y + widget->height / 2, col,
                         buf, fg, bg, attr);
    }
}

static int button_handle_input(TuiWidget *widget, const TuiEvent *event)
{
    TuiButton *button = (TuiButton *)widget;

    if (event->key == TUI_KEY_ENTER || event->codepoint == ' ') {
        if (button->on_click) {
            button->on_click(widget, button->user_data);
        }
        return 1;
    }

    return 0;
}

static int button_handle_mouse(TuiWidget *widget, const TuiMouseEvent *mouse)
{
    TuiButton *button = (TuiButton *)widget;

    if (mouse->action == TUI_MOUSE_PRESS && mouse->button == TUI_MOUSE_BTN_LEFT) {
        if (button->on_click) {
            button->on_click(widget, button->user_data);
        }
        return 1;
    }

    return 0;
}

static void button_destroy(TuiWidget *widget)
{
    TuiButton *button = (TuiButton *)widget;
    free(button->text);
    button->text = NULL;
}

static TuiWidgetVTable button_vtable = {
    .render       = button_render,
    .handle_input = button_handle_input,
    .handle_mouse = button_handle_mouse,
    .destroy      = button_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_button_init(TuiButton *button, int x, int y, int width, int height,
                          const char *text)
{
    if (!button) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init(&button->base, x, y, width, height,
                                    &button_vtable, NULL);
    if (res != TUI_OK) return res;

    button->base.focusable = 1;

    button->fg          = TUI_COLOR_INDEX(15);
    button->bg          = TUI_COLOR_INDEX(0);
    button->fg_focused  = TUI_COLOR_INDEX(0);
    button->bg_focused  = TUI_COLOR_INDEX(12);
    button->attr        = TUI_ATTR_NONE;
    button->on_click    = NULL;
    button->user_data   = NULL;

    if (text) {
        button->text = strdup(text);
        if (!button->text) return TUI_ERR_MEMORY;
    } else {
        button->text = NULL;
    }

    return TUI_OK;
}

void tui_button_set_text(TuiButton *button, const char *text)
{
    if (!button) return;
    free(button->text);
    button->text = text ? strdup(text) : NULL;
    button->base.dirty = 1;
}

void tui_button_set_colors(TuiButton *button, TuiColor fg, TuiColor bg,
                           TuiColor fg_focused, TuiColor bg_focused)
{
    if (!button) return;
    button->fg          = fg;
    button->bg          = bg;
    button->fg_focused  = fg_focused;
    button->bg_focused  = bg_focused;
    button->base.dirty  = 1;
}

void tui_button_set_on_click(TuiButton *button, TuiButtonCallback callback,
                             void *user_data)
{
    if (!button) return;
    button->on_click  = callback;
    button->user_data = user_data;
}
