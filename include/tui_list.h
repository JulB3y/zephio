/**
 * @file tui_list.h
 * @brief Scrollable list widget with keyboard navigation.
 *
 * Displays a vertical list of text items. The user can navigate with
 * Up/Down keys and select with Enter. Supports scrolling when the
 * number of items exceeds the widget height.
 *
 * Mouse support is not yet implemented.
 */

#ifndef TUI_LIST_H
#define TUI_LIST_H

#include "tui_widget.h"

/** @brief Callback invoked when an item is selected (Enter key). */
typedef void (*TuiListCallback)(TuiWidget *widget, int index,
                                const char *item, void *user_data);

/**
 * @brief List widget data.
 */
typedef struct {
    TuiWidget       base;
    char          **items;
    int             item_count;
    int             item_capacity;
    int             selected;
    int             scroll_offset;
    uint8_t         fg;
    uint8_t         bg;
    uint8_t         fg_selected;
    uint8_t         bg_selected;
    TuiAttr         attr;
    TuiListCallback on_select;
    void           *user_data;
} TuiList;

/**
 * @brief Initialize a list widget.
 *
 * @param list    List struct to initialize.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param width   Width in columns.
 * @param height  Height in rows.
 * @return TUI_OK on success.
 */
TuiResult tui_list_init(TuiList *list, int x, int y, int width, int height);

/**
 * @brief Add an item to the list.
 *
 * @return TUI_OK on success, TUI_ERR_MEMORY on failure.
 */
TuiResult tui_list_add_item(TuiList *list, const char *item);

/**
 * @brief Remove an item by index.
 */
void tui_list_remove_item(TuiList *list, int index);

/**
 * @brief Remove all items.
 */
void tui_list_clear(TuiList *list);

/** @brief Return the index of the currently selected item. */
int tui_list_get_selected(TuiList *list);

/** @brief Return the text of the currently selected item, or NULL. */
const char *tui_list_get_selected_item(TuiList *list);

/** @brief Set normal and selected color pairs. */
void tui_list_set_colors(TuiList *list, uint8_t fg, uint8_t bg,
                         uint8_t fg_selected, uint8_t bg_selected);

/** @brief Set the selection callback. */
void tui_list_set_on_select(TuiList *list, TuiListCallback callback,
                            void *user_data);

#endif
