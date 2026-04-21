/**
 * @file zephio_checkbox.h
 * @brief Checkbox widget with tristate support.
 *
 * Renders a toggleable checkbox with [x]/[ ]/[-] states.
 * Supports keyboard (Space) and mouse (click) toggling.
 * Optional tristate mode for indeterminate ([-]) state.
 */

#ifndef ZEPHIO_CHECKBOX_H
#define ZEPHIO_CHECKBOX_H

#include "zephio_widget.h"

typedef enum {
    TUI_CHECK_UNCHECKED    = 0,
    TUI_CHECK_CHECKED      = 1,
    TUI_CHECK_INDETERMINATE = 2
} ZephioCheckState;

/** @brief Callback invoked when the checkbox state changes. */
typedef void (*ZephioCheckboxCallback)(ZephioWidget *widget, ZephioCheckState state,
                                    void *user_data);

typedef struct {
    ZephioWidget          base;
    ZephioCheckState      state;
    int                tristate;
    char              *label;
    ZephioColor           fg;
    ZephioColor           bg;
    ZephioColor           fg_focused;
    ZephioColor           bg_focused;
    ZephioAttr            attr;
    ZephioCheckboxCallback on_change;
    void              *user_data;
} ZephioCheckbox;

/**
 * @brief Initialize a checkbox widget with context.
 *
 * @param checkbox  Checkbox struct to initialize.
 * @param ctx       TUI context.
 * @param x         Column offset.
 * @param y         Row offset.
 * @param width     Width in columns (should accommodate "[x] label").
 * @param height    Height in rows (typically 1).
 * @param label     Label text (copied; may be NULL).
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_checkbox_init_ctx(ZephioCheckbox *checkbox, ZephioContext *ctx, int x, int y,
                                int width, int height, const char *label);

/**
 * @brief Set the checkbox state.
 */
void zephio_checkbox_set_state(ZephioCheckbox *checkbox, ZephioCheckState state);

/**
 * @brief Get the current checkbox state.
 */
ZephioCheckState zephio_checkbox_get_state(ZephioCheckbox *checkbox);

/**
 * @brief Enable or disable tristate mode.
 *
 * In tristate mode, cycling goes: unchecked -> checked -> indeterminate.
 * In bistate mode (default), cycling goes: unchecked -> checked.
 */
void zephio_checkbox_set_tristate(ZephioCheckbox *checkbox, int tristate);

/**
 * @brief Update the label text.
 */
void zephio_checkbox_set_label(ZephioCheckbox *checkbox, const char *label);

/**
 * @brief Set normal and focused color pairs.
 */
void zephio_checkbox_set_colors(ZephioCheckbox *checkbox,
                             ZephioColor fg, ZephioColor bg,
                             ZephioColor fg_focused, ZephioColor bg_focused);

/**
 * @brief Set the state-change callback.
 */
void zephio_checkbox_set_on_change(ZephioCheckbox *checkbox,
                                ZephioCheckboxCallback callback,
                                void *user_data);

#endif
