/**
 * @file zephio_radio.h
 * @brief Radio button group widget.
 *
 * Displays a vertical list of mutually-exclusive options. Exactly one
 * option is selected at a time. Supports arrow key navigation and
 * mouse click selection.
 */

#ifndef ZEPHIO_RADIO_H
#define ZEPHIO_RADIO_H

#include "zephio_widget.h"

#define ZEPHIO_RADIO_OPTIONS_INITIAL 8

/** @brief Callback invoked when the selected option changes. */
typedef void (*ZephioRadioCallback)(ZephioWidget *widget, int index,
                                 const char *option, void *user_data);

typedef struct {
    ZephioWidget       base;
    char          **options;
    int             option_count;
    int             option_capacity;
    int             selected;
    ZephioColor        fg;
    ZephioColor        bg;
    ZephioColor        fg_selected;
    ZephioColor        bg_selected;
    ZephioAttr         attr;
    ZephioRadioCallback on_change;
    void           *user_data;
} ZephioRadio;

/**
 * @brief Initialize a radio group widget with context.
 *
 * @param radio   Radio struct to initialize.
 * @param ctx     TUI context.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param width   Width in columns.
 * @param height  Height in rows (number of visible options).
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_radio_init_ctx(ZephioRadio *radio, ZephioContext *ctx, int x, int y, int width, int height);

/**
 * @brief Add an option to the radio group.
 *
 * The first option added is automatically selected.
 *
 * @return ZEPHIO_OK on success, TUI_ERR_MEMORY on failure.
 */
ZephioResult zephio_radio_add_option(ZephioRadio *radio, const char *option);

/**
 * @brief Remove an option by index.
 */
void zephio_radio_remove_option(ZephioRadio *radio, int index);

/**
 * @brief Remove all options.
 */
void zephio_radio_clear(ZephioRadio *radio);

/**
 * @brief Set the selected option by index.
 */
void zephio_radio_set_selected(ZephioRadio *radio, int index);

/**
 * @brief Get the index of the currently selected option.
 */
int zephio_radio_get_selected(ZephioRadio *radio);

/**
 * @brief Get the text of the currently selected option, or NULL.
 */
const char *zephio_radio_get_selected_option(ZephioRadio *radio);

/**
 * @brief Set normal and selected color pairs.
 */
void zephio_radio_set_colors(ZephioRadio *radio,
                          ZephioColor fg, ZephioColor bg,
                          ZephioColor fg_selected, ZephioColor bg_selected);

/**
 * @brief Set the selection-change callback.
 */
void zephio_radio_set_on_change(ZephioRadio *radio, ZephioRadioCallback callback,
                             void *user_data);

#endif
