#define _POSIX_C_SOURCE 200809L

#include "tui_box.h"

#include <stdlib.h>
#include <string.h>

static void box_render(TuiWidget *widget)
{
    TuiBox *box = (TuiBox *)widget;

    if (widget->width < 2 || widget->height < 2) return;

    uint8_t fg;
    uint8_t bg;
    TuiAttr attr;

    if (widget->theme) {
        TuiStyle style = tui_widget_get_style(widget);
        fg   = style.fg;
        bg   = style.bg;
        attr = style.attr;
    } else {
        fg   = box->fg;
        bg   = box->bg;
        attr = box->attr;
    }

    tui_screen_fill(widget->abs_y + 1, widget->abs_x + 1,
                    widget->width - 2, widget->height - 2,
                    " ", fg, bg, attr);

    if (box->border_style == TUI_BOX_DOUBLE) {
        tui_screen_box_double(widget->abs_y, widget->abs_x,
                              widget->width, widget->height,
                              fg, bg, attr);
    } else {
        tui_screen_box_single(widget->abs_y, widget->abs_x,
                              widget->width, widget->height,
                              fg, bg, attr);
    }

    if (box->title) {
        int title_len = (int)strlen(box->title);
        int max_title = widget->width - 4;
        if (max_title < 0) max_title = 0;
        int write_len = title_len < max_title ? title_len : max_title;

        if (write_len > 0) {
            char buf[256];
            int copy_len = write_len < (int)sizeof(buf) - 1 ? write_len : (int)sizeof(buf) - 1;
            memcpy(buf, box->title, (size_t)copy_len);
            buf[copy_len] = '\0';

            tui_screen_write(widget->abs_y, widget->abs_x + 2,
                             buf, fg, bg, attr | TUI_ATTR_BOLD);
        }
    }
}

static void box_on_resize(TuiWidget *widget, int width, int height)
{
    (void)width;
    (void)height;
    for (int i = 0; i < widget->child_count; i++) {
        TuiWidget *child = widget->children[i];
        int inner_x = 1 + ((TuiBox *)widget)->padding;
        int inner_y = 1 + ((TuiBox *)widget)->padding;
        int inner_w = widget->width - 2 - 2 * ((TuiBox *)widget)->padding;
        int inner_h = widget->height - 2 - 2 * ((TuiBox *)widget)->padding;
        if (inner_w < 0) inner_w = 0;
        if (inner_h < 0) inner_h = 0;

        tui_widget_set_position(child, inner_x, inner_y);
        tui_widget_set_size(child, inner_w, inner_h);
    }
}

static void box_destroy(TuiWidget *widget)
{
    TuiBox *box = (TuiBox *)widget;
    free(box->title);
    box->title = NULL;
}

static TuiWidgetVTable box_vtable = {
    .render       = box_render,
    .handle_input = NULL,
    .handle_mouse = NULL,
    .destroy      = box_destroy,
    .on_resize    = box_on_resize,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_box_init(TuiBox *box, int x, int y, int width, int height,
                       int border_style)
{
    if (!box) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init(&box->base, x, y, width, height,
                                    &box_vtable, NULL);
    if (res != TUI_OK) return res;

    box->base.focusable = 0;

    box->title         = NULL;
    box->fg            = 15;
    box->bg            = 0;
    box->attr          = TUI_ATTR_NONE;
    box->border_style  = border_style;
    box->padding       = 0;

    return TUI_OK;
}

void tui_box_set_title(TuiBox *box, const char *title)
{
    if (!box) return;
    free(box->title);
    box->title = title ? strdup(title) : NULL;
    box->base.dirty = 1;
}

void tui_box_set_colors(TuiBox *box, uint8_t fg, uint8_t bg)
{
    if (!box) return;
    box->fg = fg;
    box->bg = bg;
    box->base.dirty = 1;
}

void tui_box_set_attr(TuiBox *box, TuiAttr attr)
{
    if (!box) return;
    box->attr = attr;
    box->base.dirty = 1;
}

void tui_box_set_padding(TuiBox *box, int padding)
{
    if (!box) return;
    box->padding = padding;
    box->base.dirty = 1;
}
