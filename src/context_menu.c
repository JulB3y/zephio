#define _POSIX_C_SOURCE 200809L

#include "zephio_context_menu.h"
#include "zephio_context.h"
#include "zephio_app.h"
#include "zephio_screen.h"

#include <stdlib.h>
#include <string.h>

static int ctx_next_item(ZephioContextMenu *menu, int from, int dir)
{
    int idx = from + dir;
    while (idx >= 0 && idx < menu->item_count) {
        if (!menu->items[idx].is_separator) return idx;
        idx += dir;
    }
    return from;
}

static void ctx_hide(ZephioContextMenu *menu)
{
    if (!menu->is_visible) return;
    menu->is_visible = 0;
    ZephioApp *app = (ZephioApp *)menu->app;
    if (app) zephio_app_pop_overlay(app);
}

static void ctx_render(ZephioWidget *widget)
{
    ZephioContextMenu *menu = (ZephioContextMenu *)widget;

    ZephioColor fg  = menu->fg;
    ZephioColor bg  = menu->bg;
    ZephioColor bfg = ZEPHIO_COLOR_INDEX(14);

    zephio_screen_fill(widget->ctx, widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ", fg, bg, ZEPHIO_ATTR_NONE);
    zephio_screen_box_single(widget->ctx, widget->abs_y, widget->abs_x,
                          widget->width, widget->height, bfg, bg, ZEPHIO_ATTR_BOLD);

    for (int i = 0; i < menu->item_count; i++) {
        ZephioContextMenuItem *item = &menu->items[i];
        int row = widget->abs_y + 1 + i;

        if (item->is_separator) {
            for (int c = 1; c < widget->width - 1; c++)
                zephio_screen_set_cell(widget->ctx, row, widget->abs_x + c,
                                    "\xe2\x94\x80", bfg, bg, ZEPHIO_ATTR_DIM);
            continue;
        }

        ZephioColor ifg = fg;
        ZephioColor ibg = bg;
        ZephioAttr  iat = ZEPHIO_ATTR_NONE;

        if (i == menu->highlighted) {
            ifg = menu->fg_highlight;
            ibg = menu->bg_highlight;
            iat = ZEPHIO_ATTR_REVERSE;
        }

        zephio_screen_fill(widget->ctx, row, widget->abs_x + 1,
                        widget->width - 2, 1, " ", ifg, ibg, iat);

        if (item->label) {
            int len = (int)strlen(item->label);
            int maxw = widget->width - 4;
            int wlen = len < maxw ? len : maxw;
            char buf[256];
            int clen = wlen < (int)sizeof(buf) - 1 ? wlen : (int)sizeof(buf) - 1;
            memcpy(buf, item->label, (size_t)clen);
            buf[clen] = '\0';
            zephio_screen_write(widget->ctx, row, widget->abs_x + 2, buf, ifg, ibg, iat);
        }
    }
}

static int ctx_handle_input(ZephioWidget *widget, const ZephioEvent *event)
{
    ZephioContextMenu *menu = (ZephioContextMenu *)widget;

    if (event->key == ZEPHIO_KEY_ESCAPE) {
        ctx_hide(menu);
        return 1;
    }

    if (event->key == ZEPHIO_KEY_UP) {
        int next = ctx_next_item(menu, menu->highlighted, -1);
        if (next != menu->highlighted) {
            menu->highlighted = next;
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == ZEPHIO_KEY_DOWN) {
        int next = ctx_next_item(menu, menu->highlighted, 1);
        if (next != menu->highlighted) {
            menu->highlighted = next;
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == ZEPHIO_KEY_ENTER) {
        if (menu->highlighted >= 0 && menu->highlighted < menu->item_count) {
            ZephioContextMenuItem *item = &menu->items[menu->highlighted];
            if (!item->is_separator && menu->on_select)
                menu->on_select(menu, menu->highlighted,
                                item->label, menu->user_data);
        }
        ctx_hide(menu);
        return 1;
    }

    return 1;
}

static int ctx_handle_mouse(ZephioWidget *widget, const ZephioMouseEvent *mouse)
{
    ZephioContextMenu *menu = (ZephioContextMenu *)widget;

    if (mouse->action == ZEPHIO_MOUSE_MOTION) {
        if (zephio_widget_contains(widget, mouse->row, mouse->col)) {
            int idx = mouse->row - widget->abs_y - 1;
            if (idx >= 0 && idx < menu->item_count &&
                !menu->items[idx].is_separator &&
                menu->highlighted != idx) {
                menu->highlighted = idx;
                widget->dirty = 1;
            }
        }
        return 1;
    }

    if (mouse->action == ZEPHIO_MOUSE_PRESS && mouse->button == ZEPHIO_MOUSE_BTN_LEFT) {
        if (!zephio_widget_contains(widget, mouse->row, mouse->col)) {
            ctx_hide(menu);
            return 1;
        }
        int idx = mouse->row - widget->abs_y - 1;
        if (idx >= 0 && idx < menu->item_count &&
            !menu->items[idx].is_separator) {
            if (menu->on_select)
                menu->on_select(menu, idx,
                                menu->items[idx].label, menu->user_data);
        }
        ctx_hide(menu);
        return 1;
    }

    return 1;
}

static void ctx_destroy(ZephioWidget *widget)
{
    ZephioContextMenu *menu = (ZephioContextMenu *)widget;
    if (menu->is_visible && menu->app) {
        ZephioApp *app = (ZephioApp *)menu->app;
        zephio_app_pop_overlay(app);
        menu->is_visible = 0;
    }
    for (int i = 0; i < menu->item_count; i++)
        free(menu->items[i].label);
    free(menu->items);
    menu->items      = NULL;
    menu->item_count = 0;
}

static ZephioWidgetVTable ctx_vtable = {
    .render       = ctx_render,
    .handle_input = ctx_handle_input,
    .handle_mouse = ctx_handle_mouse,
    .destroy      = ctx_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

ZephioResult zephio_context_menu_init_ctx(ZephioContextMenu *menu, ZephioContext *ctx)
{
    if (!menu) return TUI_ERR_MEMORY;

    ZephioResult res = zephio_widget_init_ctx(&menu->base, 0, 0, 2, 2,
                                        &ctx_vtable, ctx, NULL);
    if (res != ZEPHIO_OK) return res;

    menu->base.focusable = 1;

    menu->items        = NULL;
    menu->item_count   = 0;
    menu->item_capacity = 0;
    menu->highlighted  = -1;
    menu->is_visible   = 0;
    menu->app          = NULL;

    menu->fg           = ZEPHIO_COLOR_INDEX(15);
    menu->bg           = ZEPHIO_COLOR_INDEX(234);
    menu->fg_highlight = ZEPHIO_COLOR_INDEX(0);
    menu->bg_highlight = ZEPHIO_COLOR_INDEX(12);
    menu->attr         = ZEPHIO_ATTR_NONE;

    menu->on_select  = NULL;
    menu->user_data  = NULL;

    return ZEPHIO_OK;
}

ZephioResult zephio_context_menu_add_item(ZephioContextMenu *menu, const char *label)
{
    if (!menu) return TUI_ERR_MEMORY;

    if (menu->item_count >= menu->item_capacity) {
        int newcap = menu->item_capacity == 0
                     ? ZEPHIO_CTX_ITEMS_INIT_CAP
                     : menu->item_capacity * 2;
        ZephioContextMenuItem *ni = (ZephioContextMenuItem *)realloc(
            menu->items, (size_t)newcap * sizeof(ZephioContextMenuItem));
        if (!ni) return TUI_ERR_MEMORY;
        menu->items        = ni;
        menu->item_capacity = newcap;
    }

    ZephioContextMenuItem *it = &menu->items[menu->item_count++];
    it->label        = label ? strdup(label) : NULL;
    it->is_separator = 0;
    return ZEPHIO_OK;
}

void zephio_context_menu_add_separator(ZephioContextMenu *menu)
{
    if (!menu) return;

    if (menu->item_count >= menu->item_capacity) {
        int newcap = menu->item_capacity == 0
                     ? ZEPHIO_CTX_ITEMS_INIT_CAP
                     : menu->item_capacity * 2;
        ZephioContextMenuItem *ni = (ZephioContextMenuItem *)realloc(
            menu->items, (size_t)newcap * sizeof(ZephioContextMenuItem));
        if (!ni) return;
        menu->items        = ni;
        menu->item_capacity = newcap;
    }

    ZephioContextMenuItem *it = &menu->items[menu->item_count++];
    it->label        = NULL;
    it->is_separator = 1;
}

void zephio_context_menu_clear(ZephioContextMenu *menu)
{
    if (!menu) return;
    for (int i = 0; i < menu->item_count; i++)
        free(menu->items[i].label);
    menu->item_count  = 0;
    menu->highlighted = -1;
}

void zephio_context_menu_set_colors(ZephioContextMenu *menu,
                                 ZephioColor fg, ZephioColor bg,
                                 ZephioColor fg_hl, ZephioColor bg_hl)
{
    if (!menu) return;
    menu->fg           = fg;
    menu->bg           = bg;
    menu->fg_highlight = fg_hl;
    menu->bg_highlight = bg_hl;
}

void zephio_context_menu_set_on_select(ZephioContextMenu *menu,
                                    ZephioContextMenuCallback callback,
                                    void *user_data)
{
    if (!menu) return;
    menu->on_select = callback;
    menu->user_data = user_data;
}

void zephio_context_menu_show(ZephioContextMenu *menu, void *app,
                           int row, int col)
{
    if (!menu || !app || menu->item_count == 0) return;

    menu->app = app;

    int max_label_len = 0;
    for (int i = 0; i < menu->item_count; i++) {
        if (menu->items[i].label) {
            int len = (int)strlen(menu->items[i].label);
            if (len > max_label_len) max_label_len = len;
        }
    }
    int w = max_label_len + 4;
    int h = menu->item_count + 2;

    zephio_widget_init_ctx(&menu->base, col, row, w, h, &ctx_vtable, menu->base.ctx, NULL);
    menu->base.focusable = 1;
    menu->highlighted    = ctx_next_item(menu, -1, 1);
    menu->is_visible     = 1;
    menu->base.dirty     = 1;

    zephio_app_push_overlay((ZephioApp *)app, &menu->base);
}

void zephio_context_menu_hide(ZephioContextMenu *menu)
{
    if (!menu) return;
    ctx_hide(menu);
}
