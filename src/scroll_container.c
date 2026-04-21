#define _POSIX_C_SOURCE 200809L

#include "zephio_scroll_container.h"
#include "zephio_context.h"
#include "zephio_screen.h"

void zephio_scroll_container_clamp_scroll(ZephioScrollContainer *sc, int cv_width, int cv_height)
{
    int max_y = sc->content_height - cv_height;
    if (max_y < 0) max_y = 0;
    if (sc->scroll_y < 0) sc->scroll_y = 0;
    if (sc->scroll_y > max_y) sc->scroll_y = max_y;

    int max_x = sc->content_width - cv_width;
    if (max_x < 0) max_x = 0;
    if (sc->scroll_x < 0) sc->scroll_x = 0;
    if (sc->scroll_x > max_x) sc->scroll_x = max_x;
}

static void render_scrollbars(ZephioWidget *widget, ZephioScrollContainer *sc,
                              int has_vscroll, int has_hscroll,
                              int cv_width, int cv_height)
{
    zephio_scroll_container_render_scrollbars(widget, sc, has_vscroll, has_hscroll,
                                           cv_width, cv_height);
}

void zephio_scroll_container_render_scrollbars(ZephioWidget *widget, ZephioScrollContainer *sc,
                                             int has_vscroll, int has_hscroll,
                                             int cv_width, int cv_height)
{
    int vx = widget->abs_x;
    int vy = widget->abs_y;

    ZephioColor track_fg = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_GRAY_DARK);
    ZephioStyle style = zephio_widget_get_style(widget);
    ZephioColor track_bg = style.bg;
    ZephioAttr track_attr = ZEPHIO_ATTR_DIM;

    ZephioColor thumb_fg = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_WHITE);
    ZephioColor thumb_bg = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_GRAY_MID);

    if (has_vscroll) {
        int scroll_col = vx + widget->width - 1;
        zephio_screen_fill(widget->ctx, vy, scroll_col, 1, cv_height, " ",
                        track_fg, track_bg, track_attr);

        int max_scroll = sc->content_height - cv_height;
        if (max_scroll > 0) {
            int thumb_h = cv_height * cv_height / sc->content_height;
            if (thumb_h < 1) thumb_h = 1;
            if (thumb_h > cv_height) thumb_h = cv_height;

            int thumb_y = sc->scroll_y * (cv_height - thumb_h) / max_scroll;

            for (int r = 0; r < thumb_h; r++) {
                zephio_screen_set_cell(widget->ctx, vy + thumb_y + r, scroll_col,
                                    "\xe2\x96\x88", thumb_fg, thumb_bg,
                                    ZEPHIO_ATTR_NONE);
            }
        }
    }

    if (has_hscroll) {
        int scroll_row = vy + widget->height - 1;
        zephio_screen_fill(widget->ctx, scroll_row, vx, cv_width, 1, " ",
                        track_fg, track_bg, track_attr);

        int max_scroll = sc->content_width - cv_width;
        if (max_scroll > 0) {
            int thumb_w = cv_width * cv_width / sc->content_width;
            if (thumb_w < 1) thumb_w = 1;
            if (thumb_w > cv_width) thumb_w = cv_width;

            int thumb_x = sc->scroll_x * (cv_width - thumb_w) / max_scroll;

            for (int c = 0; c < thumb_w; c++) {
                zephio_screen_set_cell(widget->ctx, scroll_row, vx + thumb_x + c,
                                    "\xe2\x96\x88", thumb_fg, thumb_bg,
                                    ZEPHIO_ATTR_NONE);
            }
        }
    }

    if (has_vscroll && has_hscroll) {
        zephio_screen_set_cell(widget->ctx, vy + widget->height - 1,
                            vx + widget->width - 1, " ",
                            track_fg, track_bg, track_attr);
    }
}

static void scroll_render(ZephioWidget *widget)
{
    ZephioScrollContainer *sc = (ZephioScrollContainer *)widget;

    int has_vscroll = sc->content_height > widget->height;
    int has_hscroll = sc->content_width > widget->width;
    int cv_width  = widget->width  - (has_vscroll ? 1 : 0);
    int cv_height = widget->height - (has_hscroll ? 1 : 0);
    if (cv_width < 0) cv_width = 0;
    if (cv_height < 0) cv_height = 0;

    zephio_scroll_container_clamp_scroll(sc, cv_width, cv_height);

    ZephioStyle style = zephio_widget_get_style(widget);
    zephio_screen_fill(widget->ctx, widget->abs_y, widget->abs_x,
                    widget->width, widget->height,
                    " ", style.fg, style.bg, style.attr);

    for (int i = 0; i < widget->child_count; i++) {
        ZephioWidget *child = widget->children[i];
        if (!child->visible) continue;

        int adj_x = child->x - sc->scroll_x;
        int adj_y = child->y - sc->scroll_y;

        if (adj_x + child->width <= 0 || adj_x >= cv_width ||
            adj_y + child->height <= 0 || adj_y >= cv_height) {
            continue;
        }

        child->abs_x = widget->abs_x + adj_x;
        child->abs_y = widget->abs_y + adj_y;

        int orig_x = child->x;
        int orig_y = child->y;
        child->x = adj_x;
        child->y = adj_y;

        zephio_widget_render(child);

        child->x = orig_x;
        child->y = orig_y;
    }

    render_scrollbars(widget, sc, has_vscroll, has_hscroll, cv_width, cv_height);
}

int zephio_scroll_container_handle_input(ZephioWidget *widget, const ZephioEvent *event)
{
    ZephioScrollContainer *sc = (ZephioScrollContainer *)widget;

    int has_vscroll = sc->content_height > widget->height;
    int has_hscroll = sc->content_width > widget->width;
    int cv_width  = widget->width  - (has_vscroll ? 1 : 0);
    int cv_height = widget->height - (has_hscroll ? 1 : 0);
    if (cv_width < 0) cv_width = 0;
    if (cv_height < 0) cv_height = 0;

    switch (event->key) {
    case ZEPHIO_KEY_UP:
        if (has_vscroll && sc->scroll_y > 0) {
            sc->scroll_y--;
            widget->dirty = 1;
            return 1;
        }
        break;

    case ZEPHIO_KEY_DOWN:
        if (has_vscroll) {
            int max_y = sc->content_height - cv_height;
            if (max_y < 0) max_y = 0;
            if (sc->scroll_y < max_y) {
                sc->scroll_y++;
                widget->dirty = 1;
                return 1;
            }
        }
        break;

    case ZEPHIO_KEY_LEFT:
        if (has_hscroll && sc->scroll_x > 0) {
            sc->scroll_x--;
            widget->dirty = 1;
            return 1;
        }
        break;

    case ZEPHIO_KEY_RIGHT:
        if (has_hscroll) {
            int max_x = sc->content_width - cv_width;
            if (max_x < 0) max_x = 0;
            if (sc->scroll_x < max_x) {
                sc->scroll_x++;
                widget->dirty = 1;
                return 1;
            }
        }
        break;

    case ZEPHIO_KEY_PAGE_UP:
        if (has_vscroll && sc->scroll_y > 0) {
            sc->scroll_y -= cv_height;
            if (sc->scroll_y < 0) sc->scroll_y = 0;
            widget->dirty = 1;
            return 1;
        }
        break;

    case ZEPHIO_KEY_PAGE_DOWN:
        if (has_vscroll) {
            int max_y = sc->content_height - cv_height;
            if (max_y < 0) max_y = 0;
            sc->scroll_y += cv_height;
            if (sc->scroll_y > max_y) sc->scroll_y = max_y;
            widget->dirty = 1;
            return 1;
        }
        break;

    case ZEPHIO_KEY_HOME:
        if ((has_vscroll && sc->scroll_y > 0) ||
            (has_hscroll && sc->scroll_x > 0)) {
            sc->scroll_y = 0;
            sc->scroll_x = 0;
            widget->dirty = 1;
            return 1;
        }
        break;

    case ZEPHIO_KEY_END:
        if (has_vscroll) {
            int max_y = sc->content_height - cv_height;
            if (max_y < 0) max_y = 0;
            if (sc->scroll_y < max_y) {
                sc->scroll_y = max_y;
                widget->dirty = 1;
                return 1;
            }
        }
        break;

    default:
        break;
    }

    return 0;
}

int zephio_scroll_container_handle_mouse(ZephioWidget *widget, const ZephioMouseEvent *mouse)
{
    ZephioScrollContainer *sc = (ZephioScrollContainer *)widget;

    int has_vscroll = sc->content_height > widget->height;
    int has_hscroll = sc->content_width > widget->width;
    int cv_width  = widget->width  - (has_vscroll ? 1 : 0);
    int cv_height = widget->height - (has_hscroll ? 1 : 0);
    if (cv_width < 0) cv_width = 0;
    if (cv_height < 0) cv_height = 0;

    if (mouse->action == ZEPHIO_MOUSE_WHEEL_UP) {
        if (sc->scroll_y > 0) {
            sc->scroll_y -= 3;
            if (sc->scroll_y < 0) sc->scroll_y = 0;
            widget->dirty = 1;
        }
        return 1;
    }

    if (mouse->action == ZEPHIO_MOUSE_WHEEL_DOWN) {
        int max_y = sc->content_height - cv_height;
        if (max_y < 0) max_y = 0;
        if (sc->scroll_y < max_y) {
            sc->scroll_y += 3;
            if (sc->scroll_y > max_y) sc->scroll_y = max_y;
            widget->dirty = 1;
        }
        return 1;
    }

    if (mouse->action == ZEPHIO_MOUSE_PRESS && mouse->button == ZEPHIO_MOUSE_BTN_LEFT) {
        int scroll_col = widget->abs_x + widget->width - 1;
        int scroll_row = widget->abs_y + widget->height - 1;

        if (has_vscroll && mouse->col == scroll_col &&
            mouse->row >= widget->abs_y &&
            mouse->row < widget->abs_y + cv_height) {
            int max_y = sc->content_height - cv_height;
            if (max_y > 0) {
                int thumb_h = cv_height * cv_height / sc->content_height;
                if (thumb_h < 1) thumb_h = 1;
                int rel = mouse->row - widget->abs_y - thumb_h / 2;
                if (rel < 0) rel = 0;
                sc->scroll_y = rel * max_y / (cv_height - thumb_h);
                if (sc->scroll_y > max_y) sc->scroll_y = max_y;
                sc->dragging = 1;
                widget->dirty = 1;
            }
            return 1;
        }

        if (has_hscroll && mouse->row == scroll_row &&
            mouse->col >= widget->abs_x &&
            mouse->col < widget->abs_x + cv_width) {
            int max_x = sc->content_width - cv_width;
            if (max_x > 0) {
                int thumb_w = cv_width * cv_width / sc->content_width;
                if (thumb_w < 1) thumb_w = 1;
                int rel = mouse->col - widget->abs_x - thumb_w / 2;
                if (rel < 0) rel = 0;
                sc->scroll_x = rel * max_x / (cv_width - thumb_w);
                if (sc->scroll_x > max_x) sc->scroll_x = max_x;
                sc->dragging = 2;
                widget->dirty = 1;
            }
            return 1;
        }
    }

    if (sc->dragging == 1 && mouse->action == ZEPHIO_MOUSE_MOTION) {
        int max_y = sc->content_height - cv_height;
        if (max_y > 0) {
            int thumb_h = cv_height * cv_height / sc->content_height;
            if (thumb_h < 1) thumb_h = 1;
            int rel = mouse->row - widget->abs_y - thumb_h / 2;
            if (rel < 0) rel = 0;
            sc->scroll_y = rel * max_y / (cv_height - thumb_h);
            if (sc->scroll_y > max_y) sc->scroll_y = max_y;
            widget->dirty = 1;
        }
        return 1;
    }

    if (sc->dragging == 2 && mouse->action == ZEPHIO_MOUSE_MOTION) {
        int max_x = sc->content_width - cv_width;
        if (max_x > 0) {
            int thumb_w = cv_width * cv_width / sc->content_width;
            if (thumb_w < 1) thumb_w = 1;
            int rel = mouse->col - widget->abs_x - thumb_w / 2;
            if (rel < 0) rel = 0;
            sc->scroll_x = rel * max_x / (cv_width - thumb_w);
            if (sc->scroll_x > max_x) sc->scroll_x = max_x;
            widget->dirty = 1;
        }
        return 1;
    }

    if (mouse->action == ZEPHIO_MOUSE_RELEASE) {
        if (sc->dragging) {
            sc->dragging = 0;
            return 1;
        }
    }

    return 0;
}

static ZephioWidgetVTable scroll_vtable = {
    .render       = scroll_render,
    .handle_input = zephio_scroll_container_handle_input,
    .handle_mouse = zephio_scroll_container_handle_mouse,
    .destroy      = NULL,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

ZephioResult zephio_scroll_container_init_ctx(ZephioScrollContainer *sc, ZephioContext *ctx, int x, int y,
                                        int width, int height)
{
    if (!sc) return TUI_ERR_MEMORY;

    ZephioResult res = zephio_widget_init_ctx(&sc->base, x, y, width, height,
                                        &scroll_vtable, ctx, NULL);
    if (res != ZEPHIO_OK) return res;

    sc->base.focusable        = 1;
    sc->base.manages_children = 1;

    sc->scroll_x       = 0;
    sc->scroll_y       = 0;
    sc->content_width  = width;
    sc->content_height = height;
    sc->dragging       = 0;

    return ZEPHIO_OK;
}

void zephio_scroll_container_set_content_size(ZephioScrollContainer *sc,
                                           int width, int height)
{
    if (!sc) return;
    sc->content_width  = width;
    sc->content_height = height;
    sc->base.dirty = 1;
}

void zephio_scroll_container_auto_content_size(ZephioScrollContainer *sc)
{
    if (!sc) return;
    int max_w = 0, max_h = 0;
    for (int i = 0; i < sc->base.child_count; i++) {
        ZephioWidget *child = sc->base.children[i];
        int w = child->x + child->width;
        int h = child->y + child->height;
        if (w > max_w) max_w = w;
        if (h > max_h) max_h = h;
    }
    sc->content_width  = max_w;
    sc->content_height = max_h;
    sc->base.dirty = 1;
}

void zephio_scroll_container_scroll_to(ZephioScrollContainer *sc, int x, int y)
{
    if (!sc) return;
    sc->scroll_x = x;
    sc->scroll_y = y;
    sc->base.dirty = 1;
}

void zephio_scroll_container_set_scroll_x(ZephioScrollContainer *sc, int x)
{
    if (!sc) return;
    sc->scroll_x = x;
    sc->base.dirty = 1;
}

void zephio_scroll_container_set_scroll_y(ZephioScrollContainer *sc, int y)
{
    if (!sc) return;
    sc->scroll_y = y;
    sc->base.dirty = 1;
}
