/**
 * @file tui_context_menu.h
 * @brief Right-click context menu popup.
 *
 * A standalone popup overlay that appears at an arbitrary screen
 * position. Items are added before each show; the menu is pushed
 * onto the overlay stack by tui_context_menu_show().
 *
 * Usage:
 *   1. tui_context_menu_init().
 *   2. tui_context_menu_add_item() / add_separator().
 *   3. tui_context_menu_set_on_select() for the callback.
 *   4. On right-click call tui_context_menu_show(menu, app, row, col).
 *   5. In the callback call tui_context_menu_hide() (or it auto-hides).
 */

#ifndef TUI_CONTEXT_MENU_H
#define TUI_CONTEXT_MENU_H

#include "tui_widget.h"

#define TUI_CTX_ITEMS_INIT_CAP 8

typedef struct TuiContextMenu TuiContextMenu;

typedef void (*TuiContextMenuCallback)(TuiContextMenu *menu,
                                       int index,
                                       const char *label,
                                       void *user_data);

typedef struct {
    char *label;
    int   is_separator;
} TuiContextMenuItem;

struct TuiContextMenu {
    TuiWidget           base;
    TuiContextMenuItem  *items;
    int                 item_count;
    int                 item_capacity;
    int                 highlighted;
    int                 is_visible;
    void               *app;

    TuiColor fg;
    TuiColor bg;
    TuiColor fg_highlight;
    TuiColor bg_highlight;
    TuiAttr  attr;

    TuiContextMenuCallback on_select;
    void                  *user_data;
};

TuiResult tui_context_menu_init(TuiContextMenu *menu);

TuiResult tui_context_menu_add_item(TuiContextMenu *menu, const char *label);

void tui_context_menu_add_separator(TuiContextMenu *menu);

void tui_context_menu_clear(TuiContextMenu *menu);

void tui_context_menu_set_colors(TuiContextMenu *menu,
                                 TuiColor fg, TuiColor bg,
                                 TuiColor fg_hl, TuiColor bg_hl);

void tui_context_menu_set_on_select(TuiContextMenu *menu,
                                    TuiContextMenuCallback callback,
                                    void *user_data);

void tui_context_menu_show(TuiContextMenu *menu, void *app,
                           int row, int col);

void tui_context_menu_hide(TuiContextMenu *menu);

#endif
