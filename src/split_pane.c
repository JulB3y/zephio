#define _POSIX_C_SOURCE 200809L

#include "tui_split_pane.h"
#include "tui_screen.h"

static void clamp_split_pos(TuiSplitPane *pane)
{
    int total;
    if (pane->orientation == TUI_SPLIT_HORIZONTAL) {
        total = pane->base.width;
    } else {
        total = pane->base.height;
    }
    total -= pane->separator_size;
    if (total < 0) total = 0;

    int min1 = pane->min_pane1;
    int min2 = pane->min_pane2;
    if (min1 + min2 > total) {
        int excess = min1 + min2 - total;
        min1 -= excess / 2;
        min2 -= excess - excess / 2;
    }
    if (min1 < 0) min1 = 0;
    if (min2 < 0) min2 = 0;

    if (pane->split_pos < min1) pane->split_pos = min1;
    if (pane->split_pos > total - min2) pane->split_pos = total - min2;
}

static void split_render(TuiWidget *widget)
{
    TuiSplitPane *pane = (TuiSplitPane *)widget;
    clamp_split_pos(pane);

    TuiStyle style = tui_widget_get_style(widget);
    tui_screen_fill(widget->abs_y, widget->abs_x,
                    widget->width, widget->height,
                    " ", style.fg, style.bg, style.attr);

    int p1_x, p1_y, p1_w, p1_h;
    int p2_x, p2_y, p2_w, p2_h;

    if (pane->orientation == TUI_SPLIT_HORIZONTAL) {
        p1_x = widget->abs_x;
        p1_y = widget->abs_y;
        p1_w = pane->split_pos;
        p1_h = widget->height;

        p2_x = widget->abs_x + pane->split_pos + pane->separator_size;
        p2_y = widget->abs_y;
        p2_w = widget->width - pane->split_pos - pane->separator_size;
        p2_h = widget->height;

        int sep_x = widget->abs_x + pane->split_pos;
        for (int r = 0; r < widget->height; r++) {
            tui_screen_set_cell(widget->abs_y + r, sep_x,
                                "\xe2\x94\x83",
                                pane->sep_fg, pane->sep_bg,
                                pane->sep_attr);
        }
    } else {
        p1_x = widget->abs_x;
        p1_y = widget->abs_y;
        p1_w = widget->width;
        p1_h = pane->split_pos;

        p2_x = widget->abs_x;
        p2_y = widget->abs_y + pane->split_pos + pane->separator_size;
        p2_w = widget->width;
        p2_h = widget->height - pane->split_pos - pane->separator_size;

        int sep_y = widget->abs_y + pane->split_pos;
        for (int c = 0; c < widget->width; c++) {
            tui_screen_set_cell(sep_y, widget->abs_x + c,
                                "\xe2\x94\x81",
                                pane->sep_fg, pane->sep_bg,
                                pane->sep_attr);
        }
    }

    if (widget->child_count >= 1) {
        TuiWidget *child = widget->children[0];
        if (child->visible) {
            child->abs_x = p1_x;
            child->abs_y = p1_y;
            if (child->vtable && child->vtable->on_resize) {
                child->vtable->on_resize(child, p1_w, p1_h);
            }
            child->width  = p1_w;
            child->height = p1_h;
            tui_widget_render(child);
        }
    }

    if (widget->child_count >= 2) {
        TuiWidget *child = widget->children[1];
        if (child->visible) {
            child->abs_x = p2_x;
            child->abs_y = p2_y;
            if (child->vtable && child->vtable->on_resize) {
                child->vtable->on_resize(child, p2_w, p2_h);
            }
            child->width  = p2_w;
            child->height = p2_h;
            tui_widget_render(child);
        }
    }
}

static int split_handle_input(TuiWidget *widget, const TuiEvent *event)
{
    TuiSplitPane *pane = (TuiSplitPane *)widget;

    if (event->modifiers & TUI_MOD_CTRL) {
        if (event->key == TUI_KEY_LEFT && pane->orientation == TUI_SPLIT_HORIZONTAL) {
            if (pane->split_pos > pane->min_pane1) {
                pane->split_pos--;
                clamp_split_pos(pane);
                widget->dirty = 1;
            }
            return 1;
        }
        if (event->key == TUI_KEY_RIGHT && pane->orientation == TUI_SPLIT_HORIZONTAL) {
            pane->split_pos++;
            clamp_split_pos(pane);
            widget->dirty = 1;
            return 1;
        }
        if (event->key == TUI_KEY_UP && pane->orientation == TUI_SPLIT_VERTICAL) {
            if (pane->split_pos > pane->min_pane1) {
                pane->split_pos--;
                clamp_split_pos(pane);
                widget->dirty = 1;
            }
            return 1;
        }
        if (event->key == TUI_KEY_DOWN && pane->orientation == TUI_SPLIT_VERTICAL) {
            pane->split_pos++;
            clamp_split_pos(pane);
            widget->dirty = 1;
            return 1;
        }
    }

    for (int i = 0; i < widget->child_count; i++) {
        TuiWidget *child = widget->children[i];
        if (child->focused && child->vtable && child->vtable->handle_input) {
            int consumed = child->vtable->handle_input(child, event);
            if (consumed) return 1;
        }
    }

    return 0;
}

static int is_on_separator(TuiSplitPane *pane, int row, int col)
{
    if (pane->orientation == TUI_SPLIT_HORIZONTAL) {
        int sep_col = pane->base.abs_x + pane->split_pos;
        return col == sep_col &&
               row >= pane->base.abs_y &&
               row < pane->base.abs_y + pane->base.height;
    } else {
        int sep_row = pane->base.abs_y + pane->split_pos;
        return row == sep_row &&
               col >= pane->base.abs_x &&
               col < pane->base.abs_x + pane->base.width;
    }
}

static int split_handle_mouse(TuiWidget *widget, const TuiMouseEvent *mouse)
{
    TuiSplitPane *pane = (TuiSplitPane *)widget;

    if (mouse->action == TUI_MOUSE_PRESS && mouse->button == TUI_MOUSE_BTN_LEFT) {
        if (is_on_separator(pane, mouse->row, mouse->col)) {
            pane->dragging = 1;
            if (pane->orientation == TUI_SPLIT_HORIZONTAL) {
                pane->drag_origin = mouse->col;
            } else {
                pane->drag_origin = mouse->row;
            }
            pane->drag_split_origin = pane->split_pos;
            return 1;
        }
    }

    if (pane->dragging && mouse->action == TUI_MOUSE_MOTION) {
        int delta;
        if (pane->orientation == TUI_SPLIT_HORIZONTAL) {
            delta = mouse->col - pane->drag_origin;
        } else {
            delta = mouse->row - pane->drag_origin;
        }
        pane->split_pos = pane->drag_split_origin + delta;
        clamp_split_pos(pane);
        widget->dirty = 1;
        return 1;
    }

    if (pane->dragging && mouse->action == TUI_MOUSE_RELEASE) {
        pane->dragging = 0;
        return 1;
    }

    for (int i = 0; i < widget->child_count; i++) {
        TuiWidget *child = widget->children[i];
        if (child->visible && tui_widget_contains(child, mouse->row, mouse->col)) {
            int consumed = tui_widget_handle_mouse(child, mouse);
            if (consumed) return 1;
        }
    }

    return 0;
}

static void split_on_resize(TuiWidget *widget, int width, int height)
{
    TuiSplitPane *pane = (TuiSplitPane *)widget;
    widget->width  = width;
    widget->height = height;
    clamp_split_pos(pane);
    widget->dirty = 1;
}

static TuiWidgetVTable split_vtable = {
    .render       = split_render,
    .handle_input = split_handle_input,
    .handle_mouse = split_handle_mouse,
    .destroy      = NULL,
    .on_resize    = split_on_resize,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_split_pane_init(TuiSplitPane *pane, int x, int y,
                              int width, int height,
                              TuiSplitOrientation orientation)
{
    if (!pane) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init(&pane->base, x, y, width, height,
                                    &split_vtable, NULL);
    if (res != TUI_OK) return res;

    pane->base.focusable        = 0;
    pane->base.manages_children = 1;

    pane->orientation     = orientation;
    pane->separator_size  = 1;
    pane->min_pane1       = TUI_SPLIT_MIN_PANE;
    pane->min_pane2       = TUI_SPLIT_MIN_PANE;
    pane->dragging        = 0;
    pane->drag_origin     = 0;
    pane->drag_split_origin = 0;

    if (orientation == TUI_SPLIT_HORIZONTAL) {
        pane->split_pos = width / 2;
    } else {
        pane->split_pos = height / 2;
    }

    pane->sep_fg   = TUI_COLOR_INDEX(TUI_COLOR_BRIGHT_CYAN);
    pane->sep_bg   = TUI_COLOR_INDEX(TUI_COLOR_BG_DARK);
    pane->sep_attr = TUI_ATTR_DIM;

    clamp_split_pos(pane);

    return TUI_OK;
}

void tui_split_pane_set_position(TuiSplitPane *pane, int pos)
{
    if (!pane) return;
    pane->split_pos = pos;
    clamp_split_pos(pane);
    pane->base.dirty = 1;
}

int tui_split_pane_get_position(const TuiSplitPane *pane)
{
    if (!pane) return 0;
    return pane->split_pos;
}

void tui_split_pane_set_min_sizes(TuiSplitPane *pane, int min_pane1,
                                  int min_pane2)
{
    if (!pane) return;
    pane->min_pane1 = min_pane1;
    pane->min_pane2 = min_pane2;
    clamp_split_pos(pane);
    pane->base.dirty = 1;
}

void tui_split_pane_set_separator_style(TuiSplitPane *pane, TuiColor fg,
                                        TuiColor bg, TuiAttr attr)
{
    if (!pane) return;
    pane->sep_fg   = fg;
    pane->sep_bg   = bg;
    pane->sep_attr = attr;
    pane->base.dirty = 1;
}

void tui_split_pane_set_panes(TuiSplitPane *pane, TuiWidget *pane1,
                              TuiWidget *pane2)
{
    if (!pane) return;

    tui_widget_remove_all_children(&pane->base);

    if (pane1) tui_widget_add_child(&pane->base, pane1);
    if (pane2) tui_widget_add_child(&pane->base, pane2);

    pane->base.dirty = 1;
}

void tui_split_pane_get_pane1_rect(const TuiSplitPane *pane,
                                   int *x, int *y, int *w, int *h)
{
    if (!pane) return;

    if (pane->orientation == TUI_SPLIT_HORIZONTAL) {
        if (x) *x = pane->base.abs_x;
        if (y) *y = pane->base.abs_y;
        if (w) *w = pane->split_pos;
        if (h) *h = pane->base.height;
    } else {
        if (x) *x = pane->base.abs_x;
        if (y) *y = pane->base.abs_y;
        if (w) *w = pane->base.width;
        if (h) *h = pane->split_pos;
    }
}

void tui_split_pane_get_pane2_rect(const TuiSplitPane *pane,
                                   int *x, int *y, int *w, int *h)
{
    if (!pane) return;

    if (pane->orientation == TUI_SPLIT_HORIZONTAL) {
        if (x) *x = pane->base.abs_x + pane->split_pos + pane->separator_size;
        if (y) *y = pane->base.abs_y;
        if (w) *w = pane->base.width - pane->split_pos - pane->separator_size;
        if (h) *h = pane->base.height;
    } else {
        if (x) *x = pane->base.abs_x;
        if (y) *y = pane->base.abs_y + pane->split_pos + pane->separator_size;
        if (w) *w = pane->base.width;
        if (h) *h = pane->base.height - pane->split_pos - pane->separator_size;
    }
}
