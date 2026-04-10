/**
 * @file tui_dialog.h
 * @brief Modal dialog/popup widget.
 *
 * Renders a centered popup with a title, message body, and a row of
 * buttons. The dialog is meant to be pushed onto the app's overlay
 * stack so it renders on top of all other widgets and blocks input
 * to the underlying widget tree.
 *
 * Usage:
 *   1. tui_dialog_init() with title/message/button labels.
 *   2. tui_dialog_set_on_button() for the response callback.
 *   3. tui_app_push_overlay(app, &dialog.base) to show.
 *   4. In the callback, call tui_app_pop_overlay(app) to close.
 */

#ifndef TUI_DIALOG_H
#define TUI_DIALOG_H

#include "tui_widget.h"

#define TUI_DIALOG_MAX_BUTTONS 4
#define TUI_DIALOG_MAX_BUTTON_LABEL 16

typedef struct TuiDialog TuiDialog;

typedef void (*TuiDialogCallback)(TuiDialog *dialog, int button_index,
                                  void *user_data);

struct TuiDialog {
    TuiWidget base;

    char *title;
    char *message;

    char   button_labels[TUI_DIALOG_MAX_BUTTONS][TUI_DIALOG_MAX_BUTTON_LABEL];
    int    button_count;
    int    selected_button;

    TuiDialogCallback on_button;
    void             *user_data;
};

/**
 * @brief Initialize a dialog widget.
 *
 * The dialog is centered on screen. Width/height are auto-calculated
 * from the message content, or can be overridden with tui_widget_set_size().
 *
 * @param dialog     Dialog struct to initialize.
 * @param title      Title string (copied). May be NULL.
 * @param message    Body message (copied). May be NULL.
 * @return TUI_OK on success.
 */
TuiResult tui_dialog_init(TuiDialog *dialog, const char *title,
                          const char *message);

/**
 * @brief Add a button to the dialog.
 *
 * @param dialog  Dialog widget.
 * @param label   Button label (copied, max 15 chars).
 * @return Button index (0..3), or -1 on failure.
 */
int tui_dialog_add_button(TuiDialog *dialog, const char *label);

/**
 * @brief Set the button-activation callback.
 *
 * @param dialog     Dialog widget.
 * @param callback   Called when a button is activated (Enter/Space/click).
 * @param user_data  Opaque pointer passed to the callback.
 */
void tui_dialog_set_on_button(TuiDialog *dialog, TuiDialogCallback callback,
                              void *user_data);

/**
 * @brief Center the dialog in the terminal.
 *
 * Should be called after init, before pushing to the overlay stack.
 * Recalculates x/y to center based on current screen size.
 */
void tui_dialog_center(TuiDialog *dialog);

/**
 * @brief Get the selected button index after the dialog closes.
 */
int tui_dialog_get_selected(TuiDialog *dialog);

#endif
