/**
 * @file tui_button.h
 * @brief Clickable button widget with keyboard and mouse support.
 *
 * Renders centered text in a bordered region. Responds to Enter/Space
 * (keyboard) and left-click (mouse) via the on_click callback.
 * Supports focused/unfocused color states.
 */

#ifndef TUI_BUTTON_H
#define TUI_BUTTON_H

#include "tui_widget.h"

/** @brief Callback invoked when a button is activated. */
typedef void (*TuiButtonCallback)(TuiWidget *widget, void *user_data);

/**
 * @brief Button widget data.
 */
typedef struct {
    TuiWidget         base;
    char             *text;
    uint8_t           fg;
    uint8_t           bg;
    uint8_t           fg_focused;
    uint8_t           bg_focused;
    TuiAttr           attr;
    TuiButtonCallback on_click;
    void             *user_data;
} TuiButton;

/**
 * @brief Initialize a button widget.
 *
 * @param button  Button struct to initialize.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param width   Width in columns.
 * @param height  Height in rows.
 * @param text    Button label text (copied).
 * @return TUI_OK on success.
 */
TuiResult tui_button_init(TuiButton *button, int x, int y, int width, int height,
                          const char *text);

/**
 * @brief Update the button text.
 */
void tui_button_set_text(TuiButton *button, const char *text);

/**
 * @brief Set normal and focused color pairs.
 *
 * @param button      Button widget.
 * @param fg          Normal foreground.
 * @param bg          Normal background.
 * @param fg_focused  Focused foreground.
 * @param bg_focused  Focused background.
 */
void tui_button_set_colors(TuiButton *button, uint8_t fg, uint8_t bg,
                           uint8_t fg_focused, uint8_t bg_focused);

/**
 * @brief Set the click callback.
 *
 * @param button     Button widget.
 * @param callback   Function called when the button is activated.
 * @param user_data  Opaque pointer passed to the callback.
 */
void tui_button_set_on_click(TuiButton *button, TuiButtonCallback callback,
                             void *user_data);

#endif
