/**
 * @file zephio_button.h
 * @brief Clickable button widget with keyboard and mouse support.
 *
 * Renders centered text in a bordered region. Responds to Enter/Space
 * (keyboard) and left-click (mouse) via the on_click callback.
 * Supports focused/unfocused color states.
 */

#ifndef ZEPHIO_BUTTON_H
#define ZEPHIO_BUTTON_H

#include "zephio_widget.h"

/** @brief Callback invoked when a button is activated. */
typedef void (*ZephioButtonCallback)(ZephioWidget *widget, void *user_data);

/**
 * @brief Button widget data.
 */
typedef struct {
    ZephioWidget         base;
    char             *text;
    ZephioColor          fg;
    ZephioColor          bg;
    ZephioColor          fg_focused;
    ZephioColor          bg_focused;
    ZephioAttr           attr;
    ZephioButtonCallback on_click;
    void             *user_data;
} ZephioButton;

/**
 * @brief Initialize a button widget with context.
 *
 * @param button  Button struct to initialize.
 * @param ctx     TUI context.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param width   Width in columns.
 * @param height  Height in rows.
 * @param text    Button label text (copied).
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_button_init_ctx(ZephioButton *button, ZephioContext *ctx, int x, int y, int width, int height,
                              const char *text);

/**
 * @brief Update the button text.
 */
void zephio_button_set_text(ZephioButton *button, const char *text);

/**
 * @brief Set normal and focused color pairs.
 *
 * @param button      Button widget.
 * @param fg          Normal foreground.
 * @param bg          Normal background.
 * @param fg_focused  Focused foreground.
 * @param bg_focused  Focused background.
 */
void zephio_button_set_colors(ZephioButton *button, ZephioColor fg, ZephioColor bg,
                           ZephioColor fg_focused, ZephioColor bg_focused);

/**
 * @brief Set the click callback.
 *
 * @param button     Button widget.
 * @param callback   Function called when the button is activated.
 * @param user_data  Opaque pointer passed to the callback.
 */
void zephio_button_set_on_click(ZephioButton *button, ZephioButtonCallback callback,
                             void *user_data);

#endif
