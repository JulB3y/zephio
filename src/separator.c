#define _POSIX_C_SOURCE 200809L

#include "tui_separator.h"

static void separator_render(TuiWidget *widget)
{
    TuiSeparator *sep = (TuiSeparator *)widget;

    uint8_t fg;
    uint8_t bg;
    TuiAttr attr;

    if (widget->theme) {
        TuiStyle style = tui_widget_get_style(widget);
        fg   = style.fg;
        bg   = style.bg;
        attr = style.attr;
    } else {
        fg   = sep->fg;
        bg   = sep->bg;
        attr = sep->attr;
    }

    if (sep->horizontal) {
        for (int c = 0; c < widget->width; c++) {
            tui_screen_set_cell(widget->abs_y, widget->abs_x + c,
                                "\xe2\x94\x80", fg, bg, attr);
        }
    } else {
        for (int r = 0; r < widget->height; r++) {
            tui_screen_set_cell(widget->abs_y + r, widget->abs_x,
                                "\xe2\x94\x82", fg, bg, attr);
        }
    }
}

static TuiWidgetVTable separator_vtable = {
    .render       = separator_render,
    .handle_input = NULL,
    .handle_mouse = NULL,
    .destroy      = NULL,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_separator_init_h(TuiSeparator *sep, int x, int y, int width)
{
    if (!sep) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init(&sep->base, x, y, width, 1,
                                    &separator_vtable, NULL);
    if (res != TUI_OK) return res;

    sep->base.focusable = 0;
    sep->horizontal     = 1;
    sep->fg             = 8;
    sep->bg             = 0;
    sep->attr           = TUI_ATTR_NONE;

    return TUI_OK;
}

TuiResult tui_separator_init_v(TuiSeparator *sep, int x, int y, int height)
{
    if (!sep) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init(&sep->base, x, y, 1, height,
                                    &separator_vtable, NULL);
    if (res != TUI_OK) return res;

    sep->base.focusable = 0;
    sep->horizontal     = 0;
    sep->fg             = 8;
    sep->bg             = 0;
    sep->attr           = TUI_ATTR_NONE;

    return TUI_OK;
}

void tui_separator_set_colors(TuiSeparator *sep, uint8_t fg, uint8_t bg)
{
    if (!sep) return;
    sep->fg = fg;
    sep->bg = bg;
    sep->base.dirty = 1;
}

void tui_separator_set_attr(TuiSeparator *sep, TuiAttr attr)
{
    if (!sep) return;
    sep->attr = attr;
    sep->base.dirty = 1;
}
