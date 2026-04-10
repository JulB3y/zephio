/**
 * @file tui_checkbox.h
 * @brief Checkbox widget with tristate support.
 *
 * Renders a toggleable checkbox with [x]/[ ]/[-] states.
 * Supports keyboard (Space) and mouse (click) toggling.
 * Optional tristate mode for indeterminate ([-]) state.
 */

#ifndef TUI_CHECKBOX_H
#define TUI_CHECKBOX_H

#include "tui_widget.h"

typedef enum {
    TUI_CHECK_UNCHECKED    = 0,
    TUI_CHECK_CHECKED      = 1,
    TUI_CHECK_INDETERMINATE = 2
} TuiCheckState;

/** @brief Callback invoked when the checkbox state changes. */
typedef void (*TuiCheckboxCallback)(TuiWidget *widget, TuiCheckState state,
                                    void *user_data);

typedef struct {
    TuiWidget          base;
    TuiCheckState      state;
    int                tristate;
    char              *label;
    TuiColor           fg;
    TuiColor           bg;
    TuiColor           fg_focused;
    TuiColor           bg_focused;
    TuiAttr            attr;
    TuiCheckboxCallback on_change;
    void              *user_data;
} TuiCheckbox;

/**
 * @brief Initialize a checkbox widget.
 *
 * @param checkbox  Checkbox struct to initialize.
 * @param x         Column offset.
 * @param y         Row offset.
 * @param width     Width in columns (should accommodate "[x] label").
 * @param height    Height in rows (typically 1).
 * @param label     Label text (copied; may be NULL).
 * @return TUI_OK on success.
 */
TuiResult tui_checkbox_init(TuiCheckbox *checkbox, int x, int y,
                            int width, int height, const char *label);

/**
 * @brief Set the checkbox state.
 */
void tui_checkbox_set_state(TuiCheckbox *checkbox, TuiCheckState state);

/**
 * @brief Get the current checkbox state.
 */
TuiCheckState tui_checkbox_get_state(TuiCheckbox *checkbox);

/**
 * @brief Enable or disable tristate mode.
 *
 * In tristate mode, cycling goes: unchecked -> checked -> indeterminate.
 * In bistate mode (default), cycling goes: unchecked -> checked.
 */
void tui_checkbox_set_tristate(TuiCheckbox *checkbox, int tristate);

/**
 * @brief Update the label text.
 */
void tui_checkbox_set_label(TuiCheckbox *checkbox, const char *label);

/**
 * @brief Set normal and focused color pairs.
 */
void tui_checkbox_set_colors(TuiCheckbox *checkbox,
                             TuiColor fg, TuiColor bg,
                             TuiColor fg_focused, TuiColor bg_focused);

/**
 * @brief Set the state-change callback.
 */
void tui_checkbox_set_on_change(TuiCheckbox *checkbox,
                                TuiCheckboxCallback callback,
                                void *user_data);

#endif
