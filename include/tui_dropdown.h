/**
 * @file tui_dropdown.h
 * @brief Dropdown/ComboBox widget with overlay popup.
 *
 * Displays a closed selector showing the current selection and a 'v'
 * indicator. When activated (click, Enter, or Down arrow), a popup
 * list is pushed onto the app's overlay stack.
 *
 * Usage:
 *   1. tui_dropdown_init() with position and width.
 *   2. tui_dropdown_add_item() for each option.
 *   3. tui_dropdown_set_on_change() for the selection callback.
 *   4. Add to widget tree via tui_widget_add_child().
 *   5. On activation the popup auto-pushes via tui_app_push_overlay().
 */

#ifndef ZEPHIO_DROPDOWN_H
#define ZEPHIO_DROPDOWN_H

#include "tui_widget.h"

#define ZEPHIO_DROPDOWN_MAX_VISIBLE 10
#define ZEPHIO_DROPDOWN_ITEMS_INIT_CAP 8

typedef struct TuiDropdown TuiDropdown;

typedef void (*TuiDropdownCallback)(TuiDropdown *dropdown, int index,
                                     const char *item, void *user_data);

typedef struct {
    TuiWidget base;
    TuiDropdown *owner;
    int highlighted;
    int scroll_offset;
} TuiDropdownPopup;

struct TuiDropdown {
    TuiWidget         base;
    char            **items;
    int               item_count;
    int               item_capacity;
    int               selected;
    int               max_visible;
    int               is_open;
    TuiDropdownPopup  popup;
    void             *app;

    TuiColor fg;
    TuiColor bg;
    TuiColor fg_popup;
    TuiColor bg_popup;
    TuiColor fg_selected;
    TuiColor bg_selected;
    TuiAttr  attr;

    TuiDropdownCallback on_change;
    void               *user_data;
};

TuiResult tui_dropdown_init_ctx(TuiDropdown *dropdown, TuiContext *ctx, int x, int y, int width);

TuiResult tui_dropdown_add_item(TuiDropdown *dropdown, const char *item);

void tui_dropdown_clear(TuiDropdown *dropdown);

int tui_dropdown_get_selected(const TuiDropdown *dropdown);

const char *tui_dropdown_get_selected_item(const TuiDropdown *dropdown);

void tui_dropdown_set_selected(TuiDropdown *dropdown, int index);

void tui_dropdown_set_colors(TuiDropdown *dropdown,
                             TuiColor fg, TuiColor bg,
                             TuiColor fg_focused, TuiColor bg_focused);

void tui_dropdown_set_popup_colors(TuiDropdown *dropdown,
                                   TuiColor fg, TuiColor bg,
                                   TuiColor fg_hl, TuiColor bg_hl);

void tui_dropdown_set_on_change(TuiDropdown *dropdown,
                                TuiDropdownCallback callback,
                                void *user_data);

void tui_dropdown_set_max_visible(TuiDropdown *dropdown, int max_visible);

void tui_dropdown_open(TuiDropdown *dropdown, void *app);

void tui_dropdown_close(TuiDropdown *dropdown);

#endif
