/**
 * @file zephio_dropdown.h
 * @brief Dropdown/ComboBox widget with overlay popup.
 *
 * Displays a closed selector showing the current selection and a 'v'
 * indicator. When activated (click, Enter, or Down arrow), a popup
 * list is pushed onto the app's overlay stack.
 *
 * Usage:
 *   1. zephio_dropdown_init() with position and width.
 *   2. zephio_dropdown_add_item() for each option.
 *   3. zephio_dropdown_set_on_change() for the selection callback.
 *   4. Add to widget tree via zephio_widget_add_child().
 *   5. On activation the popup auto-pushes via zephio_app_push_overlay().
 */

#ifndef ZEPHIO_DROPDOWN_H
#define ZEPHIO_DROPDOWN_H

#include "zephio_widget.h"

#define ZEPHIO_DROPDOWN_MAX_VISIBLE 10
#define ZEPHIO_DROPDOWN_ITEMS_INIT_CAP 8

typedef struct ZephioDropdown ZephioDropdown;

typedef void (*ZephioDropdownCallback)(ZephioDropdown *dropdown, int index,
                                     const char *item, void *user_data);

typedef struct {
    ZephioWidget base;
    ZephioDropdown *owner;
    int highlighted;
    int scroll_offset;
} ZephioDropdownPopup;

struct ZephioDropdown {
    ZephioWidget         base;
    char            **items;
    int               item_count;
    int               item_capacity;
    int               selected;
    int               max_visible;
    int               is_open;
    ZephioDropdownPopup  popup;
    void             *app;

    ZephioColor fg;
    ZephioColor bg;
    ZephioColor fg_popup;
    ZephioColor bg_popup;
    ZephioColor fg_selected;
    ZephioColor bg_selected;
    ZephioAttr  attr;

    ZephioDropdownCallback on_change;
    void               *user_data;
};

ZephioResult zephio_dropdown_init_ctx(ZephioDropdown *dropdown, ZephioContext *ctx, int x, int y, int width);

ZephioResult zephio_dropdown_add_item(ZephioDropdown *dropdown, const char *item);

void zephio_dropdown_clear(ZephioDropdown *dropdown);

int zephio_dropdown_get_selected(const ZephioDropdown *dropdown);

const char *zephio_dropdown_get_selected_item(const ZephioDropdown *dropdown);

void zephio_dropdown_set_selected(ZephioDropdown *dropdown, int index);

void zephio_dropdown_set_colors(ZephioDropdown *dropdown,
                             ZephioColor fg, ZephioColor bg,
                             ZephioColor fg_focused, ZephioColor bg_focused);

void zephio_dropdown_set_popup_colors(ZephioDropdown *dropdown,
                                   ZephioColor fg, ZephioColor bg,
                                   ZephioColor fg_hl, ZephioColor bg_hl);

void zephio_dropdown_set_on_change(ZephioDropdown *dropdown,
                                ZephioDropdownCallback callback,
                                void *user_data);

void zephio_dropdown_set_max_visible(ZephioDropdown *dropdown, int max_visible);

void zephio_dropdown_open(ZephioDropdown *dropdown, void *app);

void zephio_dropdown_close(ZephioDropdown *dropdown);

#endif
