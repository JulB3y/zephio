#define _POSIX_C_SOURCE 200809L

#include "zephio_separator.h"
#include "zephio_context.h"

static void separator_render(ZephioWidget *widget)
{
    ZephioSeparator *sep = (ZephioSeparator *)widget;

    ZephioColor fg;
    ZephioColor bg;
    ZephioAttr attr;

    if (widget->theme) {
        ZephioStyle style = zephio_widget_get_style(widget);
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
            zephio_screen_set_cell(widget->ctx, widget->abs_y, widget->abs_x + c,
                                "\xe2\x94\x80", fg, bg, attr);
        }
    } else {
        for (int r = 0; r < widget->height; r++) {
            zephio_screen_set_cell(widget->ctx, widget->abs_y + r, widget->abs_x,
                                "\xe2\x94\x82", fg, bg, attr);
        }
    }
}

static ZephioWidgetVTable separator_vtable = {
    .render       = separator_render,
    .handle_input = NULL,
    .handle_mouse = NULL,
    .destroy      = NULL,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

ZephioResult zephio_separator_init_h_ctx(ZephioSeparator *sep, ZephioContext *ctx, int x, int y, int width)
{
    if (!sep) return TUI_ERR_MEMORY;

    ZephioResult res = zephio_widget_init_ctx(&sep->base, x, y, width, 1,
                                        &separator_vtable, ctx, NULL);
    if (res != ZEPHIO_OK) return res;

    sep->base.focusable = 0;
    sep->horizontal     = 1;
    sep->fg             = ZEPHIO_COLOR_INDEX(8);
    sep->bg             = ZEPHIO_COLOR_INDEX(0);
    sep->attr           = ZEPHIO_ATTR_NONE;

    return ZEPHIO_OK;
}

ZephioResult zephio_separator_init_v_ctx(ZephioSeparator *sep, ZephioContext *ctx, int x, int y, int height)
{
    if (!sep) return TUI_ERR_MEMORY;

    ZephioResult res = zephio_widget_init_ctx(&sep->base, x, y, 1, height,
                                        &separator_vtable, ctx, NULL);
    if (res != ZEPHIO_OK) return res;

    sep->base.focusable = 0;
    sep->horizontal     = 0;
    sep->fg             = ZEPHIO_COLOR_INDEX(8);
    sep->bg             = ZEPHIO_COLOR_INDEX(0);
    sep->attr           = ZEPHIO_ATTR_NONE;

    return ZEPHIO_OK;
}

void zephio_separator_set_colors(ZephioSeparator *sep, ZephioColor fg, ZephioColor bg)
{
    if (!sep) return;
    sep->fg = fg;
    sep->bg = bg;
    sep->base.dirty = 1;
}

void zephio_separator_set_attr(ZephioSeparator *sep, ZephioAttr attr)
{
    if (!sep) return;
    sep->attr = attr;
    sep->base.dirty = 1;
}
