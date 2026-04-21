#define _POSIX_C_SOURCE 200809L

#include "zephio_container.h"
#include "zephio_context.h"

static void container_render(ZephioWidget *widget)
{
    ZephioContainer *container = (ZephioContainer *)widget;

    if (widget->theme) {
        ZephioStyle style = zephio_widget_get_style(widget);
        zephio_screen_fill(widget->ctx, widget->abs_y, widget->abs_x,
                        widget->width, widget->height,
                        " ", style.fg, style.bg, style.attr);
    } else {
        zephio_screen_fill(widget->ctx, widget->abs_y, widget->abs_x,
                        widget->width, widget->height,
                        " ", ZEPHIO_COLOR_INDEX(0), container->bg, ZEPHIO_ATTR_NONE);
    }
}

static ZephioWidgetVTable container_vtable = {
    .render       = container_render,
    .handle_input = NULL,
    .handle_mouse = NULL,
    .destroy      = NULL,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

ZephioResult zephio_container_init_ctx(ZephioContainer *container, ZephioContext *ctx, int x, int y,
                                  int width, int height)
{
    if (!container) return TUI_ERR_MEMORY;

    ZephioResult res = zephio_widget_init_ctx(&container->base, x, y, width, height,
                                        &container_vtable, ctx, NULL);
    if (res != ZEPHIO_OK) return res;

    container->base.focusable = 0;
    container->bg             = ZEPHIO_COLOR_INDEX(0);

    return ZEPHIO_OK;
}

void zephio_container_set_bg(ZephioContainer *container, ZephioColor bg)
{
    if (!container) return;
    container->bg = bg;
    container->base.dirty = 1;
}
