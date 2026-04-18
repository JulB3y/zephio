/**
 * @file tui_radio.h
 * @brief Radio button group widget.
 *
 * Displays a vertical list of mutually-exclusive options. Exactly one
 * option is selected at a time. Supports arrow key navigation and
 * mouse click selection.
 */

#ifndef TUI_RADIO_H
#define TUI_RADIO_H

#include "tui_widget.h"

#define TUI_RADIO_OPTIONS_INITIAL 8

/** @brief Callback invoked when the selected option changes. */
typedef void (*TuiRadioCallback)(TuiWidget *widget, int index,
                                 const char *option, void *user_data);

typedef struct {
    TuiWidget       base;
    char          **options;
    int             option_count;
    int             option_capacity;
    int             selected;
    TuiColor        fg;
    TuiColor        bg;
    TuiColor        fg_selected;
    TuiColor        bg_selected;
    TuiAttr         attr;
    TuiRadioCallback on_change;
    void           *user_data;
} TuiRadio;

/**
 * @brief Initialize a radio group widget with context.
 *
 * @param radio   Radio struct to initialize.
 * @param ctx     TUI context.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param width   Width in columns.
 * @param height  Height in rows (number of visible options).
 * @return TUI_OK on success.
 */
TuiResult tui_radio_init_ctx(TuiRadio *radio, TuiContext *ctx, int x, int y, int width, int height);

/**
 * @brief Add an option to the radio group.
 *
 * The first option added is automatically selected.
 *
 * @return TUI_OK on success, TUI_ERR_MEMORY on failure.
 */
TuiResult tui_radio_add_option(TuiRadio *radio, const char *option);

/**
 * @brief Remove an option by index.
 */
void tui_radio_remove_option(TuiRadio *radio, int index);

/**
 * @brief Remove all options.
 */
void tui_radio_clear(TuiRadio *radio);

/**
 * @brief Set the selected option by index.
 */
void tui_radio_set_selected(TuiRadio *radio, int index);

/**
 * @brief Get the index of the currently selected option.
 */
int tui_radio_get_selected(TuiRadio *radio);

/**
 * @brief Get the text of the currently selected option, or NULL.
 */
const char *tui_radio_get_selected_option(TuiRadio *radio);

/**
 * @brief Set normal and selected color pairs.
 */
void tui_radio_set_colors(TuiRadio *radio,
                          TuiColor fg, TuiColor bg,
                          TuiColor fg_selected, TuiColor bg_selected);

/**
 * @brief Set the selection-change callback.
 */
void tui_radio_set_on_change(TuiRadio *radio, TuiRadioCallback callback,
                             void *user_data);

#endif
