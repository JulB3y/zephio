/**
 * @file tui_menubar.h
 * @brief Horizontal menu bar with dropdown submenus.
 *
 * Renders a horizontal bar with menu labels (File, Edit, etc.).
 * Menus are activated by click, Alt+mnemonic, or arrow keys + Enter.
 * The dropdown submenu is pushed as an overlay.
 *
 * Usage:
 *   1. tui_menubar_init() with position and width.
 *   2. tui_menubar_add_menu() for each top-level menu (returns index).
 *   3. tui_menubar_add_menu_item() / add_menu_separator() per menu.
 *   4. tui_menubar_set_on_select() for the activation callback.
 *   5. Add to widget tree via tui_widget_add_child().
 */

#ifndef TUI_MENUBAR_H
#define TUI_MENUBAR_H

#include "tui_widget.h"

#define TUI_MENU_ITEMS_INIT_CAP 8
#define TUI_MENUS_INIT_CAP 8

typedef struct TuiMenuBar      TuiMenuBar;
typedef struct TuiMenu         TuiMenu;
typedef struct TuiMenuItem     TuiMenuItem;
typedef struct TuiMenuPopup    TuiMenuPopup;

typedef void (*TuiMenuBarCallback)(TuiMenuBar *menubar,
                                   const char *menu_label,
                                   int item_index,
                                   const char *item_label,
                                   void *user_data);

struct TuiMenuItem {
    char *label;
    int   is_separator;
};

struct TuiMenu {
    char         *label;
    char          mnemonic;
    int           start_x;
    int           label_width;
    TuiMenuItem  *items;
    int           item_count;
    int           item_capacity;
};

struct TuiMenuPopup {
    TuiWidget  base;
    TuiMenuBar *owner;
    int         menu_index;
    int         highlighted;
};

struct TuiMenuBar {
    TuiWidget  base;
    TuiMenu   *menus;
    int        menu_count;
    int        menu_capacity;
    int        active_menu;
    TuiMenuPopup popup;
    int        is_open;
    void      *app;

    TuiColor fg;
    TuiColor bg;
    TuiColor fg_active;
    TuiColor bg_active;
    TuiColor fg_popup;
    TuiColor bg_popup;
    TuiColor fg_popup_hl;
    TuiColor bg_popup_hl;
    TuiAttr  attr;

    TuiMenuBarCallback on_select;
    void              *user_data;
};

TuiResult tui_menubar_init(TuiMenuBar *menubar, int x, int y, int width);

int tui_menubar_add_menu(TuiMenuBar *menubar, const char *label, char mnemonic);

TuiResult tui_menubar_add_menu_item(TuiMenuBar *menubar, int menu_index,
                                    const char *label);

void tui_menubar_add_menu_separator(TuiMenuBar *menubar, int menu_index);

void tui_menubar_set_colors(TuiMenuBar *menubar,
                            TuiColor fg, TuiColor bg,
                            TuiColor fg_active, TuiColor bg_active);

void tui_menubar_set_popup_colors(TuiMenuBar *menubar,
                                  TuiColor fg, TuiColor bg,
                                  TuiColor fg_hl, TuiColor bg_hl);

void tui_menubar_set_on_select(TuiMenuBar *menubar,
                               TuiMenuBarCallback callback,
                               void *user_data);

void tui_menubar_open_menu(TuiMenuBar *menubar, int menu_index);

void tui_menubar_close_menu(TuiMenuBar *menubar);

#endif
