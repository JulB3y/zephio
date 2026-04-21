/**
 * @file zephio_context_menu.h
 * @brief Right-click context menu popup.
 *
 * A standalone popup overlay that appears at an arbitrary screen
 * position. Items are added before each show; the menu is pushed
 * onto the overlay stack by zephio_context_menu_show().
 *
 * Usage:
 *   1. zephio_context_menu_init().
 *   2. zephio_context_menu_add_item() / add_separator().
 *   3. zephio_context_menu_set_on_select() for the callback.
 *   4. On right-click call zephio_context_menu_show(menu, app, row, col).
 *   5. In the callback call zephio_context_menu_hide() (or it auto-hides).
 */

#ifndef ZEPHIO_CONTEXT_MENU_H
#define ZEPHIO_CONTEXT_MENU_H

#include "zephio_widget.h"

#define ZEPHIO_CTX_ITEMS_INIT_CAP 8

typedef struct ZephioContextMenu ZephioContextMenu;

typedef void (*ZephioContextMenuCallback)(ZephioContextMenu *menu,
                                       int index,
                                       const char *label,
                                       void *user_data);

typedef struct {
    char *label;
    int   is_separator;
} ZephioContextMenuItem;

struct ZephioContextMenu {
    ZephioWidget           base;
    ZephioContextMenuItem  *items;
    int                 item_count;
    int                 item_capacity;
    int                 highlighted;
    int                 is_visible;
    void               *app;

    ZephioColor fg;
    ZephioColor bg;
    ZephioColor fg_highlight;
    ZephioColor bg_highlight;
    ZephioAttr  attr;

    ZephioContextMenuCallback on_select;
    void                  *user_data;
};

ZephioResult zephio_context_menu_init_ctx(ZephioContextMenu *menu, ZephioContext *ctx);

ZephioResult zephio_context_menu_add_item(ZephioContextMenu *menu, const char *label);

void zephio_context_menu_add_separator(ZephioContextMenu *menu);

void zephio_context_menu_clear(ZephioContextMenu *menu);

void zephio_context_menu_set_colors(ZephioContextMenu *menu,
                                 ZephioColor fg, ZephioColor bg,
                                 ZephioColor fg_hl, ZephioColor bg_hl);

void zephio_context_menu_set_on_select(ZephioContextMenu *menu,
                                    ZephioContextMenuCallback callback,
                                    void *user_data);

void zephio_context_menu_show(ZephioContextMenu *menu, void *app,
                           int row, int col);

void zephio_context_menu_hide(ZephioContextMenu *menu);

#endif
