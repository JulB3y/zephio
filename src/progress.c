#define _POSIX_C_SOURCE 200809L

#include "zephio_progress.h"
#include "zephio_context.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void progress_render(ZephioWidget *widget)
{
    ZephioProgress *p = (ZephioProgress *)widget;

    ZephioColor fg = p->fg_fill;
    ZephioColor bg = p->bg_fill;
    ZephioAttr attr = p->attr;

    if (widget->theme) {
        ZephioStyle style = zephio_widget_get_style(widget);
        fg = style.fg;
        bg = style.bg;
        attr = style.attr;
    }

    zephio_screen_fill(widget->ctx, widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ",
                    p->fg_empty, p->bg_empty, attr);

    int col = widget->abs_x;
    int label_len = 0;

    if (p->label) {
        label_len = (int)strlen(p->label);
        int max_label = widget->width / 3;
        if (max_label < 0) max_label = 0;
        if (label_len > max_label) label_len = max_label;

        zephio_screen_write(widget->ctx, widget->abs_y, col, p->label,
                         p->fg_label, p->bg_label, attr);
        col += label_len;

        if (col < widget->abs_x + widget->width) {
            zephio_screen_set_cell(widget->ctx, widget->abs_y, col, " ",
                                p->fg_label, p->bg_label, attr);
            col++;
        }
    }

    int remaining = widget->width - (col - widget->abs_x);

    int pct_width = 0;
    char pct_buf[8];
    if (p->show_percent && remaining >= 5) {
        snprintf(pct_buf, sizeof(pct_buf), "%3d%%", p->value);
        pct_width = 4;
    }

    int bar_width = remaining - pct_width;
    if (bar_width < 0) bar_width = 0;

    int fill_width = (p->value * bar_width) / 100;

    for (int i = 0; i < fill_width && col + i < widget->abs_x + widget->width; i++) {
        zephio_screen_set_cell(widget->ctx, widget->abs_y, col + i, p->fill_char,
                            fg, bg, attr);
    }

    for (int i = fill_width; i < bar_width && col + i < widget->abs_x + widget->width; i++) {
        zephio_screen_set_cell(widget->ctx, widget->abs_y, col + i, p->empty_char,
                            p->fg_empty, p->bg_empty, attr);
    }

    if (pct_width > 0) {
        zephio_screen_write(widget->ctx, widget->abs_y, col + bar_width, pct_buf,
                         fg, bg, attr);
    }
}

static void progress_destroy(ZephioWidget *widget)
{
    ZephioProgress *p = (ZephioProgress *)widget;
    free(p->label);
    p->label = NULL;
}

static ZephioWidgetVTable progress_vtable = {
    .render       = progress_render,
    .handle_input = NULL,
    .handle_mouse = NULL,
    .destroy      = progress_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

ZephioResult zephio_progress_init_ctx(ZephioProgress *progress, ZephioContext *ctx, int x, int y, int width, int height)
{
    if (!progress) return TUI_ERR_MEMORY;

    ZephioResult res = zephio_widget_init_ctx(&progress->base, x, y, width, height,
                                        &progress_vtable, ctx, NULL);
    if (res != ZEPHIO_OK) return res;

    progress->base.focusable = 0;

    progress->value = 0;
    memcpy(progress->fill_char, "\xe2\x96\x93", 4);
    memcpy(progress->empty_char, "\xe2\x96\x91", 4);
    progress->label = NULL;
    progress->show_percent = 1;

    progress->fg_fill  = ZEPHIO_COLOR_INDEX(10);
    progress->bg_fill  = ZEPHIO_COLOR_INDEX(10);
    progress->fg_empty = ZEPHIO_COLOR_INDEX(8);
    progress->bg_empty = ZEPHIO_COLOR_INDEX(8);
    progress->fg_label = ZEPHIO_COLOR_INDEX(15);
    progress->bg_label = ZEPHIO_COLOR_INDEX(0);
    progress->attr     = ZEPHIO_ATTR_NONE;

    return ZEPHIO_OK;
}

void zephio_progress_set_value(ZephioProgress *progress, int value)
{
    if (!progress) return;
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    progress->value = value;
    progress->base.dirty = 1;
}

int zephio_progress_get_value(ZephioProgress *progress)
{
    if (!progress) return 0;
    return progress->value;
}

void zephio_progress_set_chars(ZephioProgress *progress, const char *fill, const char *empty)
{
    if (!progress) return;

    if (fill) {
        size_t len = strlen(fill);
        if (len > 3) len = 3;
        memcpy(progress->fill_char, fill, len);
        progress->fill_char[len] = '\0';
    }

    if (empty) {
        size_t len = strlen(empty);
        if (len > 3) len = 3;
        memcpy(progress->empty_char, empty, len);
        progress->empty_char[len] = '\0';
    }

    progress->base.dirty = 1;
}

void zephio_progress_set_label(ZephioProgress *progress, const char *label)
{
    if (!progress) return;
    free(progress->label);
    progress->label = label ? strdup(label) : NULL;
    progress->base.dirty = 1;
}

void zephio_progress_set_show_percent(ZephioProgress *progress, int show)
{
    if (!progress) return;
    progress->show_percent = show;
    progress->base.dirty = 1;
}

void zephio_progress_set_colors(ZephioProgress *progress,
                             ZephioColor fg_fill, ZephioColor bg_fill,
                             ZephioColor fg_empty, ZephioColor bg_empty)
{
    if (!progress) return;
    progress->fg_fill  = fg_fill;
    progress->bg_fill  = bg_fill;
    progress->fg_empty = fg_empty;
    progress->bg_empty = bg_empty;
    progress->base.dirty = 1;
}

void zephio_progress_set_label_colors(ZephioProgress *progress,
                                   ZephioColor fg, ZephioColor bg)
{
    if (!progress) return;
    progress->fg_label = fg;
    progress->bg_label = bg;
    progress->base.dirty = 1;
}

void zephio_progress_set_attr(ZephioProgress *progress, ZephioAttr attr)
{
    if (!progress) return;
    progress->attr = attr;
    progress->base.dirty = 1;
}
