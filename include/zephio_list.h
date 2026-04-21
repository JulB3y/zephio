/**
 * @file zephio_list.h
 * @brief Scrollable list widget with keyboard navigation.
 *
 * Displays a vertical list of text items. The user can navigate with
 * Up/Down keys and select with Enter. Supports scrolling when the
 * number of items exceeds the widget height.
 *
 * Mouse support: click to select items.
 */

#ifndef ZEPHIO_LIST_H
#define ZEPHIO_LIST_H

#include "zephio_widget.h"

/** @brief Callback invoked when an item is selected (Enter key). */
typedef void (*ZephioListCallback)(ZephioWidget *widget, int index,
                                const char *item, void *user_data);

/**
 * @brief List widget data.
 */
typedef struct {
    ZephioWidget       base;
    char          **items;
    int             item_count;
    int             item_capacity;
    int             selected;
    int             scroll_offset;
    ZephioColor        fg;
    ZephioColor        bg;
    ZephioColor        fg_selected;
    ZephioColor        bg_selected;
    ZephioAttr         attr;
    ZephioListCallback on_select;
    void           *user_data;
} ZephioList;

/**
 * @brief Initialize a list widget with context.
 *
 * @param list    List struct to initialize.
 * @param ctx     TUI context.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param width   Width in columns.
 * @param height  Height in rows.
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_list_init_ctx(ZephioList *list, ZephioContext *ctx, int x, int y, int width, int height);

/**
 * @brief Add an item to the list.
 *
 * @return ZEPHIO_OK on success, TUI_ERR_MEMORY on failure.
 */
ZephioResult zephio_list_add_item(ZephioList *list, const char *item);

/**
 * @brief Remove an item by index.
 */
void zephio_list_remove_item(ZephioList *list, int index);

/**
 * @brief Remove all items.
 */
void zephio_list_clear(ZephioList *list);

/** @brief Return the index of the currently selected item. */
int zephio_list_get_selected(ZephioList *list);

/** @brief Return the text of the currently selected item, or NULL. */
const char *zephio_list_get_selected_item(ZephioList *list);

/** @brief Set normal and selected color pairs. */
void zephio_list_set_colors(ZephioList *list, ZephioColor fg, ZephioColor bg,
                         ZephioColor fg_selected, ZephioColor bg_selected);

/** @brief Set the selection callback. */
void zephio_list_set_on_select(ZephioList *list, ZephioListCallback callback,
                            void *user_data);

#endif
