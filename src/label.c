#define _POSIX_C_SOURCE 200809L

#include "tui_label.h"
#include "tui_context.h"

#include <stdlib.h>
#include <string.h>

static void label_render(TuiWidget *widget)
{
    TuiLabel *label = (TuiLabel *)widget;
    if (!label->text) return;

    if (widget->theme) {
        TuiStyle style = tui_widget_get_style(widget);
        tui_screen_write(widget->ctx, widget->abs_y, widget->abs_x,
                         label->text, style.fg, style.bg, style.attr);
    } else {
        tui_screen_write(widget->ctx, widget->abs_y, widget->abs_x,
                          label->text, label->fg, label->bg, label->attr);
    }
}

static void label_destroy(TuiWidget *widget)
{
    TuiLabel *label = (TuiLabel *)widget;
    free(label->text);
    label->text = NULL;
}

static TuiWidgetVTable label_vtable = {
    .render       = label_render,
    .handle_input = NULL,
    .handle_mouse = NULL,
    .destroy      = label_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_label_init_ctx(TuiLabel *label, TuiContext *ctx, int x, int y, int width, int height,
                             const char *text)
{
    if (!label) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init_ctx(&label->base, x, y, width, height,
                                        &label_vtable, ctx, NULL);
    if (res != TUI_OK) return res;

    label->base.focusable = 0;

    label->fg   = TUI_COLOR_INDEX(15);
    label->bg   = TUI_COLOR_INDEX(0);
    label->attr = TUI_ATTR_NONE;

    if (text) {
        label->text = strdup(text);
        if (!label->text) return TUI_ERR_MEMORY;
    } else {
        label->text = NULL;
    }

    return TUI_OK;
}

void tui_label_set_text(TuiLabel *label, const char *text)
{
    if (!label) return;
    free(label->text);
    label->text = text ? strdup(text) : NULL;
    label->base.dirty = 1;
}

void tui_label_set_colors(TuiLabel *label, TuiColor fg, TuiColor bg)
{
    if (!label) return;
    label->fg = fg;
    label->bg = bg;
    label->base.dirty = 1;
}

void tui_label_set_attr(TuiLabel *label, TuiAttr attr)
{
    if (!label) return;
    label->attr = attr;
    label->base.dirty = 1;
}
