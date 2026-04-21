/**
 * @file tui_mouse.h
 * @brief Mouse tracking enable/disable and event types.
 *
 * Uses SGR (1006) mouse mode for reliable coordinate parsing.
 * The input parser in tui_input.c decodes mouse events into
 * TuiMouseEvent structs.
 *
 * Usage:
 *   1. tui_mouse_enable(ctx)   — after tui_init()
 *   2. Handle TUI_EVENT_MOUSE events via tui_input_poll / tui_input_loop
 *   3. tui_mouse_disable(ctx)  — before tui_shutdown()
 */

#ifndef ZEPHIO_MOUSE_H
#define ZEPHIO_MOUSE_H

#include "tui_context.h"

/**
 * @brief Mouse buttons.
 */
typedef enum {
    TUI_MOUSE_BTN_NONE   = 0,
    TUI_MOUSE_BTN_LEFT   = 1,
    TUI_MOUSE_BTN_MIDDLE = 2,
    TUI_MOUSE_BTN_RIGHT  = 3
} TuiMouseButton;

/**
 * @brief Mouse actions.
 */
typedef enum {
    TUI_MOUSE_PRESS      = 0,
    TUI_MOUSE_RELEASE    = 1,
    TUI_MOUSE_MOTION     = 2,
    TUI_MOUSE_WHEEL_UP   = 3,
    TUI_MOUSE_WHEEL_DOWN = 4
} TuiMouseAction;

/**
 * @brief Mouse event data.
 */
typedef struct {
    int             row;
    int             col;
    TuiMouseButton  button;
    TuiMouseAction  action;
    int             modifiers;
} TuiMouseEvent;

/**
 * @brief Enable mouse tracking (basic + drag + SGR mode).
 *
 * @param ctx  TUI context.
 * @return TUI_OK on success.
 */
TuiResult tui_mouse_enable(TuiContext *ctx);

/**
 * @brief Disable mouse tracking.
 *
 * @param ctx  TUI context.
 */
void tui_mouse_disable(TuiContext *ctx);

/**
 * @brief Return a human-readable mouse action name.
 *
 * @param action  The mouse action.
 * @return Static string (do not free).
 */
const char *tui_mouse_action_name(TuiMouseAction action);

/**
 * @brief Return a human-readable mouse button name.
 *
 * @param button  The mouse button.
 * @return Static string (do not free).
 */
const char *tui_mouse_button_name(TuiMouseButton button);

#endif
