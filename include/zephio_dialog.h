/**
 * @file zephio_dialog.h
 * @brief Modal dialog/popup widget.
 *
 * Renders a centered popup with a title, message body, and a row of
 * buttons. The dialog is meant to be pushed onto the app's overlay
 * stack so it renders on top of all other widgets and blocks input
 * to the underlying widget tree.
 *
 * Usage:
 *   1. zephio_dialog_init() with title/message/button labels.
 *   2. zephio_dialog_set_on_button() for the response callback.
 *   3. zephio_app_push_overlay(app, &dialog.base) to show.
 *   4. In the callback, call zephio_app_pop_overlay(app) to close.
 */

#ifndef ZEPHIO_DIALOG_H
#define ZEPHIO_DIALOG_H

#include "zephio_widget.h"

#define ZEPHIO_DIALOG_MAX_BUTTONS 4
#define ZEPHIO_DIALOG_MAX_BUTTON_LABEL 16

typedef struct ZephioDialog ZephioDialog;

typedef void (*ZephioDialogCallback)(ZephioDialog *dialog, int button_index,
                                  void *user_data);

struct ZephioDialog {
    ZephioWidget base;

    char *title;
    char *message;

    char   button_labels[ZEPHIO_DIALOG_MAX_BUTTONS][ZEPHIO_DIALOG_MAX_BUTTON_LABEL];
    int    button_count;
    int    selected_button;

    ZephioDialogCallback on_button;
    void             *user_data;
};

/**
 * @brief Initialize a dialog widget with context.
 *
 * The dialog is centered on screen. Width/height are auto-calculated
 * from to message content, or can be overridden with zephio_widget_set_size().
 *
 * @param dialog     Dialog struct to initialize.
 * @param ctx        TUI context.
 * @param title      Title string (copied). May be NULL.
 * @param message    Body message (copied). May be NULL.
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_dialog_init_ctx(ZephioDialog *dialog, ZephioContext *ctx, const char *title,
                              const char *message);

/**
 * @brief Add a button to the dialog.
 *
 * @param dialog  Dialog widget.
 * @param label   Button label (copied, max 15 chars).
 * @return Button index (0..3), or -1 on failure.
 */
int zephio_dialog_add_button(ZephioDialog *dialog, const char *label);

/**
 * @brief Set the button-activation callback.
 *
 * @param dialog     Dialog widget.
 * @param callback   Called when a button is activated (Enter/Space/click).
 * @param user_data  Opaque pointer passed to the callback.
 */
void zephio_dialog_set_on_button(ZephioDialog *dialog, ZephioDialogCallback callback,
                              void *user_data);

/**
 * @brief Center dialog in terminal.
 *
 * Should be called after init, before pushing to overlay stack.
 * Recalculates x/y to center based on current screen size.
 *
 * @param ctx  ZephioContext for screen size query.
 * @param dialog  Dialog widget.
 */
void zephio_dialog_center(ZephioContext *ctx, ZephioDialog *dialog);

/**
 * @brief Get the selected button index after the dialog closes.
 */
int zephio_dialog_get_selected(ZephioDialog *dialog);

#endif
