/**
 * @file zephio_menubar.h
 * @brief Horizontal menu bar with dropdown submenus.
 *
 * Renders a horizontal bar with menu labels (File, Edit, etc.).
 * Menus are activated by click, Alt+mnemonic, or arrow keys + Enter.
 * The dropdown submenu is pushed as an overlay.
 *
 * Usage:
 *   1. zephio_menubar_init() with position and width.
 *   2. zephio_menubar_add_menu() for each top-level menu (returns index).
 *   3. zephio_menubar_add_menu_item() / add_menu_separator() per menu.
 *   4. zephio_menubar_set_on_select() for the activation callback.
 *   5. Add to widget tree via zephio_widget_add_child().
 */

#ifndef ZEPHIO_MENUBAR_H
#define ZEPHIO_MENUBAR_H

#include "zephio_widget.h"

#define ZEPHIO_MENU_ITEMS_INIT_CAP 8
#define ZEPHIO_MENUS_INIT_CAP 8

typedef struct ZephioMenuBar      ZephioMenuBar;
typedef struct ZephioMenu         ZephioMenu;
typedef struct ZephioMenuItem     ZephioMenuItem;
typedef struct ZephioMenuPopup    ZephioMenuPopup;

typedef void (*ZephioMenuBarCallback)(ZephioMenuBar *menubar,
                                   const char *menu_label,
                                   int item_index,
                                   const char *item_label,
                                   void *user_data);

struct ZephioMenuItem {
    char *label;
    int   is_separator;
};

struct ZephioMenu {
    char         *label;
    char          mnemonic;
    int           start_x;
    int           label_width;
    ZephioMenuItem  *items;
    int           item_count;
    int           item_capacity;
};

struct ZephioMenuPopup {
    ZephioWidget  base;
    ZephioMenuBar *owner;
    int         menu_index;
    int         highlighted;
};

struct ZephioMenuBar {
    ZephioWidget  base;
    ZephioMenu   *menus;
    int        menu_count;
    int        menu_capacity;
    int        active_menu;
    ZephioMenuPopup popup;
    int        is_open;
    void      *app;

    ZephioColor fg;
    ZephioColor bg;
    ZephioColor fg_active;
    ZephioColor bg_active;
    ZephioColor fg_popup;
    ZephioColor bg_popup;
    ZephioColor fg_popup_hl;
    ZephioColor bg_popup_hl;
    ZephioAttr  attr;

    ZephioMenuBarCallback on_select;
    void              *user_data;
};

ZephioResult zephio_menubar_init_ctx(ZephioMenuBar *menubar, ZephioContext *ctx, int x, int y, int width);

int zephio_menubar_add_menu(ZephioMenuBar *menubar, const char *label, char mnemonic);

ZephioResult zephio_menubar_add_menu_item(ZephioMenuBar *menubar, int menu_index,
                                    const char *label);

void zephio_menubar_add_menu_separator(ZephioMenuBar *menubar, int menu_index);

void zephio_menubar_set_colors(ZephioMenuBar *menubar,
                            ZephioColor fg, ZephioColor bg,
                            ZephioColor fg_active, ZephioColor bg_active);

void zephio_menubar_set_popup_colors(ZephioMenuBar *menubar,
                                  ZephioColor fg, ZephioColor bg,
                                  ZephioColor fg_hl, ZephioColor bg_hl);

void zephio_menubar_set_on_select(ZephioMenuBar *menubar,
                               ZephioMenuBarCallback callback,
                               void *user_data);

void zephio_menubar_open_menu(ZephioMenuBar *menubar, int menu_index);

void zephio_menubar_close_menu(ZephioMenuBar *menubar);

#endif
