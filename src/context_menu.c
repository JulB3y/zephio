#define _POSIX_C_SOURCE 200809L

#include "tui_context_menu.h"
#include "tui_app.h"
#include "tui_screen.h"

#include <stdlib.h>
#include <string.h>

static int ctx_next_item(TuiContextMenu *menu, int from, int dir)
{
    int idx = from + dir;
    while (idx >= 0 && idx < menu->item_count) {
        if (!menu->items[idx].is_separator) return idx;
        idx += dir;
    }
    return from;
}

static void ctx_hide(TuiContextMenu *menu)
{
    if (!menu->is_visible) return;
    menu->is_visible = 0;
    TuiApp *app = (TuiApp *)menu->app;
    if (app) tui_app_pop_overlay(app);
}

static void ctx_render(TuiWidget *widget)
{
    TuiContextMenu *menu = (TuiContextMenu *)widget;

    TuiColor fg  = menu->fg;
    TuiColor bg  = menu->bg;
    TuiColor bfg = TUI_COLOR_INDEX(14);

    tui_screen_fill(widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ", fg, bg, TUI_ATTR_NONE);
    tui_screen_box_single(widget->abs_y, widget->abs_x,
                          widget->width, widget->height, bfg, bg, TUI_ATTR_BOLD);

    for (int i = 0; i < menu->item_count; i++) {
        TuiContextMenuItem *item = &menu->items[i];
        int row = widget->abs_y + 1 + i;

        if (item->is_separator) {
            for (int c = 1; c < widget->width - 1; c++)
                tui_screen_set_cell(row, widget->abs_x + c,
                                    "\xe2\x94\x80", bfg, bg, TUI_ATTR_DIM);
            continue;
        }

        TuiColor ifg = fg;
        TuiColor ibg = bg;
        TuiAttr  iat = TUI_ATTR_NONE;

        if (i == menu->highlighted) {
            ifg = menu->fg_highlight;
            ibg = menu->bg_highlight;
            iat = TUI_ATTR_REVERSE;
        }

        tui_screen_fill(row, widget->abs_x + 1,
                        widget->width - 2, 1, " ", ifg, ibg, iat);

        if (item->label) {
            int len = (int)strlen(item->label);
            int maxw = widget->width - 4;
            int wlen = len < maxw ? len : maxw;
            char buf[256];
            int clen = wlen < (int)sizeof(buf) - 1 ? wlen : (int)sizeof(buf) - 1;
            memcpy(buf, item->label, (size_t)clen);
            buf[clen] = '\0';
            tui_screen_write(row, widget->abs_x + 2, buf, ifg, ibg, iat);
        }
    }
}

static int ctx_handle_input(TuiWidget *widget, const TuiEvent *event)
{
    TuiContextMenu *menu = (TuiContextMenu *)widget;

    if (event->key == TUI_KEY_ESCAPE) {
        ctx_hide(menu);
        return 1;
    }

    if (event->key == TUI_KEY_UP) {
        int next = ctx_next_item(menu, menu->highlighted, -1);
        if (next != menu->highlighted) {
            menu->highlighted = next;
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == TUI_KEY_DOWN) {
        int next = ctx_next_item(menu, menu->highlighted, 1);
        if (next != menu->highlighted) {
            menu->highlighted = next;
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == TUI_KEY_ENTER) {
        if (menu->highlighted >= 0 && menu->highlighted < menu->item_count) {
            TuiContextMenuItem *item = &menu->items[menu->highlighted];
            if (!item->is_separator && menu->on_select)
                menu->on_select(menu, menu->highlighted,
                                item->label, menu->user_data);
        }
        ctx_hide(menu);
        return 1;
    }

    return 1;
}

static int ctx_handle_mouse(TuiWidget *widget, const TuiMouseEvent *mouse)
{
    TuiContextMenu *menu = (TuiContextMenu *)widget;

    if (mouse->action == TUI_MOUSE_MOTION) {
        if (tui_widget_contains(widget, mouse->row, mouse->col)) {
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

    if (mouse->action == TUI_MOUSE_PRESS && mouse->button == TUI_MOUSE_BTN_LEFT) {
        if (!tui_widget_contains(widget, mouse->row, mouse->col)) {
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

static void ctx_destroy(TuiWidget *widget)
{
    TuiContextMenu *menu = (TuiContextMenu *)widget;
    if (menu->is_visible && menu->app) {
        TuiApp *app = (TuiApp *)menu->app;
        tui_app_pop_overlay(app);
        menu->is_visible = 0;
    }
    for (int i = 0; i < menu->item_count; i++)
        free(menu->items[i].label);
    free(menu->items);
    menu->items      = NULL;
    menu->item_count = 0;
}

static TuiWidgetVTable ctx_vtable = {
    .render       = ctx_render,
    .handle_input = ctx_handle_input,
    .handle_mouse = ctx_handle_mouse,
    .destroy      = ctx_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_context_menu_init(TuiContextMenu *menu)
{
    if (!menu) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init(&menu->base, 0, 0, 2, 2,
                                    &ctx_vtable, NULL);
    if (res != TUI_OK) return res;

    menu->base.focusable = 1;

    menu->items        = NULL;
    menu->item_count   = 0;
    menu->item_capacity = 0;
    menu->highlighted  = -1;
    menu->is_visible   = 0;
    menu->app          = NULL;

    menu->fg           = TUI_COLOR_INDEX(15);
    menu->bg           = TUI_COLOR_INDEX(234);
    menu->fg_highlight = TUI_COLOR_INDEX(0);
    menu->bg_highlight = TUI_COLOR_INDEX(12);
    menu->attr         = TUI_ATTR_NONE;

    menu->on_select  = NULL;
    menu->user_data  = NULL;

    return TUI_OK;
}

TuiResult tui_context_menu_add_item(TuiContextMenu *menu, const char *label)
{
    if (!menu) return TUI_ERR_MEMORY;

    if (menu->item_count >= menu->item_capacity) {
        int newcap = menu->item_capacity == 0
                     ? TUI_CTX_ITEMS_INIT_CAP
                     : menu->item_capacity * 2;
        TuiContextMenuItem *ni = (TuiContextMenuItem *)realloc(
            menu->items, (size_t)newcap * sizeof(TuiContextMenuItem));
        if (!ni) return TUI_ERR_MEMORY;
        menu->items        = ni;
        menu->item_capacity = newcap;
    }

    TuiContextMenuItem *it = &menu->items[menu->item_count++];
    it->label        = label ? strdup(label) : NULL;
    it->is_separator = 0;
    return TUI_OK;
}

void tui_context_menu_add_separator(TuiContextMenu *menu)
{
    if (!menu) return;

    if (menu->item_count >= menu->item_capacity) {
        int newcap = menu->item_capacity == 0
                     ? TUI_CTX_ITEMS_INIT_CAP
                     : menu->item_capacity * 2;
        TuiContextMenuItem *ni = (TuiContextMenuItem *)realloc(
            menu->items, (size_t)newcap * sizeof(TuiContextMenuItem));
        if (!ni) return;
        menu->items        = ni;
        menu->item_capacity = newcap;
    }

    TuiContextMenuItem *it = &menu->items[menu->item_count++];
    it->label        = NULL;
    it->is_separator = 1;
}

void tui_context_menu_clear(TuiContextMenu *menu)
{
    if (!menu) return;
    for (int i = 0; i < menu->item_count; i++)
        free(menu->items[i].label);
    menu->item_count  = 0;
    menu->highlighted = -1;
}

void tui_context_menu_set_colors(TuiContextMenu *menu,
                                 TuiColor fg, TuiColor bg,
                                 TuiColor fg_hl, TuiColor bg_hl)
{
    if (!menu) return;
    menu->fg           = fg;
    menu->bg           = bg;
    menu->fg_highlight = fg_hl;
    menu->bg_highlight = bg_hl;
}

void tui_context_menu_set_on_select(TuiContextMenu *menu,
                                    TuiContextMenuCallback callback,
                                    void *user_data)
{
    if (!menu) return;
    menu->on_select = callback;
    menu->user_data = user_data;
}

void tui_context_menu_show(TuiContextMenu *menu, void *app,
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

    tui_widget_init(&menu->base, col, row, w, h, &ctx_vtable, NULL);
    menu->base.focusable = 1;
    menu->highlighted    = ctx_next_item(menu, -1, 1);
    menu->is_visible     = 1;
    menu->base.dirty     = 1;

    tui_app_push_overlay((TuiApp *)app, &menu->base);
}

void tui_context_menu_hide(TuiContextMenu *menu)
{
    if (!menu) return;
    ctx_hide(menu);
}
