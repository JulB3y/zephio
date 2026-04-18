#define _POSIX_C_SOURCE 200809L

#include "tui_container.h"
#include "tui_context.h"

static void container_render(TuiWidget *widget)
{
    TuiContainer *container = (TuiContainer *)widget;

    if (widget->theme) {
        TuiStyle style = tui_widget_get_style(widget);
        tui_screen_fill(tui_current_ctx, widget->abs_y, widget->abs_x,
                        widget->width, widget->height,
                        " ", style.fg, style.bg, style.attr);
    } else {
        tui_screen_fill(tui_current_ctx, widget->abs_y, widget->abs_x,
                        widget->width, widget->height,
                        " ", TUI_COLOR_INDEX(0), container->bg, TUI_ATTR_NONE);
    }
}

static TuiWidgetVTable container_vtable = {
    .render       = container_render,
    .handle_input = NULL,
    .handle_mouse = NULL,
    .destroy      = NULL,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_container_init(TuiContainer *container, int x, int y,
                             int width, int height)
{
    if (!container) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init(&container->base, x, y, width, height,
                                    &container_vtable, NULL);
    if (res != TUI_OK) return res;

    container->base.focusable = 0;
    container->bg             = TUI_COLOR_INDEX(0);

    return TUI_OK;
}

void tui_container_set_bg(TuiContainer *container, TuiColor bg)
{
    if (!container) return;
    container->bg = bg;
    container->base.dirty = 1;
}
