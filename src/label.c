#define _POSIX_C_SOURCE 200809L

#include "zephio_label.h"
#include "zephio_context.h"

#include <stdlib.h>
#include <string.h>

static void label_render(ZephioWidget *widget)
{
    ZephioLabel *label = (ZephioLabel *)widget;
    if (!label->text) return;

    if (widget->theme) {
        ZephioStyle style = zephio_widget_get_style(widget);
        zephio_screen_write(widget->ctx, widget->abs_y, widget->abs_x,
                         label->text, style.fg, style.bg, style.attr);
    } else {
        zephio_screen_write(widget->ctx, widget->abs_y, widget->abs_x,
                          label->text, label->fg, label->bg, label->attr);
    }
}

static void label_destroy(ZephioWidget *widget)
{
    ZephioLabel *label = (ZephioLabel *)widget;
    free(label->text);
    label->text = NULL;
}

static ZephioWidgetVTable label_vtable = {
    .render       = label_render,
    .handle_input = NULL,
    .handle_mouse = NULL,
    .destroy      = label_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

ZephioResult zephio_label_init_ctx(ZephioLabel *label, ZephioContext *ctx, int x, int y, int width, int height,
                             const char *text)
{
    if (!label) return TUI_ERR_MEMORY;

    ZephioResult res = zephio_widget_init_ctx(&label->base, x, y, width, height,
                                        &label_vtable, ctx, NULL);
    if (res != ZEPHIO_OK) return res;

    label->base.focusable = 0;

    label->fg   = ZEPHIO_COLOR_INDEX(15);
    label->bg   = ZEPHIO_COLOR_INDEX(0);
    label->attr = ZEPHIO_ATTR_NONE;

    if (text) {
        label->text = strdup(text);
        if (!label->text) return TUI_ERR_MEMORY;
    } else {
        label->text = NULL;
    }

    return ZEPHIO_OK;
}

void zephio_label_set_text(ZephioLabel *label, const char *text)
{
    if (!label) return;
    free(label->text);
    label->text = text ? strdup(text) : NULL;
    label->base.dirty = 1;
}

void zephio_label_set_colors(ZephioLabel *label, ZephioColor fg, ZephioColor bg)
{
    if (!label) return;
    label->fg = fg;
    label->bg = bg;
    label->base.dirty = 1;
}

void zephio_label_set_attr(ZephioLabel *label, ZephioAttr attr)
{
    if (!label) return;
    label->attr = attr;
    label->base.dirty = 1;
}
