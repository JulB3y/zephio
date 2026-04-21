#define _POSIX_C_SOURCE 200809L

#include "zephio_menubar.h"
#include "zephio_context.h"
#include "zephio_app.h"
#include "zephio_screen.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static void menubar_recompute_layout(ZephioMenuBar *mb)
{
    int x = 0;
    for (int i = 0; i < mb->menu_count; i++) {
        mb->menus[i].start_x     = x;
        mb->menus[i].label_width = mb->menus[i].label
                                   ? (int)strlen(mb->menus[i].label) : 0;
        x += mb->menus[i].label_width + 2;
    }
}

static void menu_popup_close(ZephioMenuPopup *popup)
{
    ZephioMenuBar *mb = popup->owner;
    if (!mb->is_open) return;
    mb->is_open = 0;
    ZephioApp *app = (ZephioApp *)mb->app;
    if (app) zephio_app_pop_overlay(app);
    zephio_widget_focus(&mb->base);
}

static void popup_render(ZephioWidget *widget)
{
    ZephioMenuPopup *popup = (ZephioMenuPopup *)widget;
    ZephioMenuBar *mb  = popup->owner;
    ZephioMenu *menu = &mb->menus[popup->menu_index];

    ZephioColor fg  = mb->fg_popup;
    ZephioColor bg  = mb->bg_popup;
    ZephioColor bfg = ZEPHIO_COLOR_INDEX(14);

    zephio_screen_fill(widget->ctx, widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ", fg, bg, ZEPHIO_ATTR_NONE);
    zephio_screen_box_single(widget->ctx, widget->abs_y, widget->abs_x,
                          widget->width, widget->height, bfg, bg, ZEPHIO_ATTR_BOLD);

    for (int i = 0; i < menu->item_count; i++) {
        ZephioMenuItem *item = &menu->items[i];
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

        if (i == popup->highlighted) {
            ifg = mb->fg_popup_hl;
            ibg = mb->bg_popup_hl;
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

static int popup_next_item(ZephioMenu *menu, int from, int dir)
{
    int idx = from + dir;
    while (idx >= 0 && idx < menu->item_count) {
        if (!menu->items[idx].is_separator) return idx;
        idx += dir;
    }
    return from;
}

static void popup_navigate(ZephioMenuPopup *popup, ZephioMenu *menu, int dir, ZephioWidget *widget)
{
    int next = popup_next_item(menu, popup->highlighted, dir);
    if (next != popup->highlighted) {
        popup->highlighted = next;
        widget->dirty = 1;
    }
}

static void popup_switch_menu(ZephioMenuPopup *popup, ZephioMenuBar *mb, int new_idx)
{
    if (new_idx >= 0 && new_idx < mb->menu_count) {
        menu_popup_close(popup);
        zephio_menubar_open_menu(mb, new_idx);
    }
}

static void popup_activate(ZephioMenuPopup *popup, ZephioMenuBar *mb, ZephioMenu *menu)
{
    if (popup->highlighted >= 0 && popup->highlighted < menu->item_count) {
        ZephioMenuItem *item = &menu->items[popup->highlighted];
        if (!item->is_separator && mb->on_select) {
            mb->on_select(mb, menu->label,
                          popup->highlighted, item->label, mb->user_data);
        }
    }
    menu_popup_close(popup);
}

static int popup_handle_input(ZephioWidget *widget, const ZephioEvent *event)
{
    ZephioMenuPopup *popup = (ZephioMenuPopup *)widget;
    ZephioMenuBar *mb  = popup->owner;
    ZephioMenu *menu = &mb->menus[popup->menu_index];

    if (event->key == ZEPHIO_KEY_ESCAPE) {
        menu_popup_close(popup);
        return 1;
    }

    if (event->key == ZEPHIO_KEY_UP) {
        popup_navigate(popup, menu, -1, widget);
        return 1;
    }

    if (event->key == ZEPHIO_KEY_DOWN) {
        popup_navigate(popup, menu, 1, widget);
        return 1;
    }

    if (event->key == ZEPHIO_KEY_LEFT) {
        popup_switch_menu(popup, mb, popup->menu_index - 1);
        return 1;
    }

    if (event->key == ZEPHIO_KEY_RIGHT) {
        popup_switch_menu(popup, mb, popup->menu_index + 1);
        return 1;
    }

    if (event->key == ZEPHIO_KEY_ENTER) {
        popup_activate(popup, mb, menu);
        return 1;
    }

    return 1;
}

static int popup_handle_mouse(ZephioWidget *widget, const ZephioMouseEvent *mouse)
{
    ZephioMenuPopup *popup = (ZephioMenuPopup *)widget;
    ZephioMenuBar *mb  = popup->owner;
    ZephioMenu *menu = &mb->menus[popup->menu_index];

    if (mouse->action == ZEPHIO_MOUSE_MOTION) {
        if (zephio_widget_contains(widget, mouse->row, mouse->col)) {
            int idx = mouse->row - widget->abs_y - 1;
            if (idx >= 0 && idx < menu->item_count &&
                !menu->items[idx].is_separator &&
                popup->highlighted != idx) {
                popup->highlighted = idx;
                widget->dirty = 1;
            }
        }
        return 1;
    }

    if (mouse->action == ZEPHIO_MOUSE_PRESS && mouse->button == ZEPHIO_MOUSE_BTN_LEFT) {
        if (!zephio_widget_contains(widget, mouse->row, mouse->col)) {
            menu_popup_close(popup);
            return 1;
        }
        int idx = mouse->row - widget->abs_y - 1;
        if (idx >= 0 && idx < menu->item_count &&
            !menu->items[idx].is_separator) {
            if (mb->on_select)
                mb->on_select(mb, menu->label, idx,
                              menu->items[idx].label, mb->user_data);
            menu_popup_close(popup);
        }
        return 1;
    }

    return 1;
}

static ZephioWidgetVTable popup_vtable = {
    .render       = popup_render,
    .handle_input = popup_handle_input,
    .handle_mouse = popup_handle_mouse,
    .destroy      = NULL,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

static void menubar_render(ZephioWidget *widget)
{
    ZephioMenuBar *mb = (ZephioMenuBar *)widget;

    zephio_screen_fill(widget->ctx, widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ",
                    mb->fg, mb->bg, mb->attr);

    for (int i = 0; i < mb->menu_count; i++) {
        ZephioMenu *m = &mb->menus[i];
        ZephioColor fg = mb->fg;
        ZephioColor bg = mb->bg;
        ZephioAttr  at = mb->attr;

        if (i == mb->active_menu && (widget->focused || mb->is_open)) {
            fg = mb->fg_active;
            bg = mb->bg_active;
            at |= ZEPHIO_ATTR_REVERSE;
        }

        char buf[128];
        int clen = m->label_width + 2;
        if (clen > (int)sizeof(buf) - 1) clen = (int)sizeof(buf) - 1;
        buf[0] = ' ';
        if (m->label) {
            int copy = clen - 1;
            if (copy > m->label_width) copy = m->label_width;
            memcpy(buf + 1, m->label, (size_t)copy);
            buf[1 + copy] = ' ';
            buf[1 + copy + 1] = '\0';
        } else {
            buf[1] = ' ';
            buf[2] = '\0';
        }

        zephio_screen_write(widget->ctx, widget->abs_y,
                         widget->abs_x + m->start_x,
                         buf, fg, bg, at);

        if (m->mnemonic && m->label) {
            for (int j = 0; m->label[j]; j++) {
                if (tolower((unsigned char)m->label[j]) == tolower((unsigned char)m->mnemonic)) {
                    zephio_screen_set_cell(widget->ctx, widget->abs_y,
                                        widget->abs_x + m->start_x + 1 + j,
                                        &m->label[j], fg, bg,
                                        at | ZEPHIO_ATTR_UNDERLINE);
                    break;
                }
            }
        }
    }
}

static int menubar_handle_input(ZephioWidget *widget, const ZephioEvent *event)
{
    ZephioMenuBar *mb = (ZephioMenuBar *)widget;
    if (mb->is_open) return 0;

    if (event->key == ZEPHIO_KEY_LEFT) {
        if (mb->active_menu > 0) {
            mb->active_menu--;
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == ZEPHIO_KEY_RIGHT) {
        if (mb->active_menu < mb->menu_count - 1) {
            mb->active_menu++;
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == ZEPHIO_KEY_DOWN || event->key == ZEPHIO_KEY_ENTER) {
        if (mb->active_menu >= 0 && mb->app &&
            mb->menus[mb->active_menu].item_count > 0) {
            zephio_menubar_open_menu(mb, mb->active_menu);
        }
        return 1;
    }

    if (event->modifiers & ZEPHIO_MOD_ALT && event->codepoint) {
        char ch = tolower(event->codepoint);
        for (int i = 0; i < mb->menu_count; i++) {
            if (mb->menus[i].mnemonic &&
                tolower((unsigned char)mb->menus[i].mnemonic) == ch) {
                mb->active_menu = i;
                widget->dirty = 1;
                if (mb->app && mb->menus[i].item_count > 0)
                    zephio_menubar_open_menu(mb, i);
                return 1;
            }
        }
    }

    return 0;
}

static int menubar_handle_mouse(ZephioWidget *widget, const ZephioMouseEvent *mouse)
{
    ZephioMenuBar *mb = (ZephioMenuBar *)widget;
    if (mb->is_open) return 0;

    if (mouse->action == ZEPHIO_MOUSE_PRESS && mouse->button == ZEPHIO_MOUSE_BTN_LEFT) {
        int col = mouse->col - widget->abs_x;
        for (int i = 0; i < mb->menu_count; i++) {
            int end = mb->menus[i].start_x + mb->menus[i].label_width + 2;
            if (col >= mb->menus[i].start_x && col < end) {
                mb->active_menu = i;
                widget->dirty = 1;
                if (mb->app && mb->menus[i].item_count > 0)
                    zephio_menubar_open_menu(mb, i);
                return 1;
            }
        }
    }
    return 0;
}

static void destroy_menu(ZephioMenu *menu)
{
    for (int i = 0; i < menu->item_count; i++)
        free(menu->items[i].label);
    free(menu->items);
    free(menu->label);
    menu->items      = NULL;
    menu->item_count = 0;
    menu->label      = NULL;
}

static void menubar_destroy(ZephioWidget *widget)
{
    ZephioMenuBar *mb = (ZephioMenuBar *)widget;
    if (mb->is_open && mb->app) {
        ZephioApp *app = (ZephioApp *)mb->app;
        zephio_app_pop_overlay(app);
        mb->is_open = 0;
    }
    for (int i = 0; i < mb->menu_count; i++)
        destroy_menu(&mb->menus[i]);
    free(mb->menus);
    mb->menus      = NULL;
    mb->menu_count = 0;
}

static ZephioWidgetVTable menubar_vtable = {
    .render       = menubar_render,
    .handle_input = menubar_handle_input,
    .handle_mouse = menubar_handle_mouse,
    .destroy      = menubar_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

ZephioResult zephio_menubar_init_ctx(ZephioMenuBar *menubar, ZephioContext *ctx, int x, int y, int width)
{
    if (!menubar) return TUI_ERR_MEMORY;

    ZephioResult res = zephio_widget_init_ctx(&menubar->base, x, y, width, 1,
                                        &menubar_vtable, ctx, NULL);
    if (res != ZEPHIO_OK) return res;

    menubar->base.focusable = 1;

    menubar->menus        = NULL;
    menubar->menu_count   = 0;
    menubar->menu_capacity = 0;
    menubar->active_menu  = 0;
    menubar->is_open      = 0;
    menubar->app          = NULL;

    memset(&menubar->popup, 0, sizeof(menubar->popup));

    menubar->fg          = ZEPHIO_COLOR_INDEX(15);
    menubar->bg          = ZEPHIO_COLOR_INDEX(4);
    menubar->fg_active   = ZEPHIO_COLOR_INDEX(0);
    menubar->bg_active   = ZEPHIO_COLOR_INDEX(14);
    menubar->fg_popup    = ZEPHIO_COLOR_INDEX(15);
    menubar->bg_popup    = ZEPHIO_COLOR_INDEX(234);
    menubar->fg_popup_hl = ZEPHIO_COLOR_INDEX(0);
    menubar->bg_popup_hl = ZEPHIO_COLOR_INDEX(12);
    menubar->attr        = ZEPHIO_ATTR_BOLD;

    menubar->on_select = NULL;
    menubar->user_data = NULL;

    return ZEPHIO_OK;
}

int zephio_menubar_add_menu(ZephioMenuBar *menubar, const char *label, char mnemonic)
{
    if (!menubar || !label) return -1;

    if (menubar->menu_count >= menubar->menu_capacity) {
        int newcap = menubar->menu_capacity == 0
                     ? ZEPHIO_MENUS_INIT_CAP
                     : menubar->menu_capacity * 2;
        ZephioMenu *nm = (ZephioMenu *)realloc(menubar->menus,
                                         (size_t)newcap * sizeof(ZephioMenu));
        if (!nm) return -1;
        menubar->menus        = nm;
        menubar->menu_capacity = newcap;
    }

    int idx = menubar->menu_count++;
    ZephioMenu *m = &menubar->menus[idx];
    m->label        = strdup(label);
    m->mnemonic     = mnemonic;
    m->items        = NULL;
    m->item_count   = 0;
    m->item_capacity = 0;

    menubar_recompute_layout(menubar);
    menubar->base.dirty = 1;
    return idx;
}

static int ensure_item_capacity(ZephioMenu *m)
{
    if (m->item_count >= m->item_capacity) {
        int newcap = m->item_capacity == 0
                     ? ZEPHIO_MENU_ITEMS_INIT_CAP
                     : m->item_capacity * 2;
        ZephioMenuItem *ni = (ZephioMenuItem *)realloc(m->items,
                            (size_t)newcap * sizeof(ZephioMenuItem));
        if (!ni) return -1;
        m->items        = ni;
        m->item_capacity = newcap;
    }
    return 0;
}

ZephioResult zephio_menubar_add_menu_item(ZephioMenuBar *menubar, int menu_index,
                                    const char *label)
{
    if (!menubar || menu_index < 0 || menu_index >= menubar->menu_count)
        return TUI_ERR_MEMORY;

    ZephioMenu *m = &menubar->menus[menu_index];
    if (ensure_item_capacity(m) < 0) return TUI_ERR_MEMORY;

    ZephioMenuItem *it = &m->items[m->item_count++];
    it->label        = label ? strdup(label) : NULL;
    it->is_separator = 0;

    menubar->base.dirty = 1;
    return ZEPHIO_OK;
}

void zephio_menubar_add_menu_separator(ZephioMenuBar *menubar, int menu_index)
{
    if (!menubar || menu_index < 0 || menu_index >= menubar->menu_count) return;

    ZephioMenu *m = &menubar->menus[menu_index];
    if (ensure_item_capacity(m) < 0) return;

    ZephioMenuItem *it = &m->items[m->item_count++];
    it->label        = NULL;
    it->is_separator = 1;

    menubar->base.dirty = 1;
}

void zephio_menubar_set_colors(ZephioMenuBar *menubar,
                            ZephioColor fg, ZephioColor bg,
                            ZephioColor fg_active, ZephioColor bg_active)
{
    if (!menubar) return;
    menubar->fg        = fg;
    menubar->bg        = bg;
    menubar->fg_active = fg_active;
    menubar->bg_active = bg_active;
    menubar->base.dirty = 1;
}

void zephio_menubar_set_popup_colors(ZephioMenuBar *menubar,
                                  ZephioColor fg, ZephioColor bg,
                                  ZephioColor fg_hl, ZephioColor bg_hl)
{
    if (!menubar) return;
    menubar->fg_popup    = fg;
    menubar->bg_popup    = bg;
    menubar->fg_popup_hl = fg_hl;
    menubar->bg_popup_hl = bg_hl;
    menubar->base.dirty  = 1;
}

void zephio_menubar_set_on_select(ZephioMenuBar *menubar,
                               ZephioMenuBarCallback callback,
                               void *user_data)
{
    if (!menubar) return;
    menubar->on_select = callback;
    menubar->user_data = user_data;
}

void zephio_menubar_open_menu(ZephioMenuBar *menubar, int menu_index)
{
    if (!menubar || menu_index < 0 || menu_index >= menubar->menu_count)
        return;
    if (!menubar->app) return;

    ZephioMenu *menu = &menubar->menus[menu_index];
    if (menu->item_count == 0) return;

    menubar->active_menu = menu_index;
    menubar->is_open     = 1;
    menubar->base.dirty  = 1;

    int max_label_len = 0;
    for (int i = 0; i < menu->item_count; i++) {
        if (menu->items[i].label) {
            int len = (int)strlen(menu->items[i].label);
            if (len > max_label_len) max_label_len = len;
        }
    }
    int pw = max_label_len + 4;
    int ph = menu->item_count + 2;

    int px = menubar->base.abs_x + menu->start_x;
    int py = menubar->base.abs_y + menubar->base.height;

    zephio_widget_init_ctx(&menubar->popup.base, px, py, pw, ph,
                        &popup_vtable, menubar->base.ctx, NULL);
    menubar->popup.base.focusable = 1;
    menubar->popup.owner       = menubar;
    menubar->popup.menu_index  = menu_index;
    menubar->popup.highlighted = popup_next_item(menu, -1, 1);

    zephio_app_push_overlay((ZephioApp *)menubar->app, &menubar->popup.base);
}

void zephio_menubar_close_menu(ZephioMenuBar *menubar)
{
    if (!menubar || !menubar->is_open) return;
    menu_popup_close(&menubar->popup);
}
