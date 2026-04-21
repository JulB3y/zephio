/**
 * @file zephio_mouse.h
 * @brief Mouse tracking enable/disable and event types.
 *
 * Uses SGR (1006) mouse mode for reliable coordinate parsing.
 * The input parser in zephio_input.c decodes mouse events into
 * ZephioMouseEvent structs.
 *
 * Usage:
 *   1. zephio_mouse_enable(ctx)   — after zephio_init()
 *   2. Handle ZEPHIO_EVENT_MOUSE events via zephio_input_poll / zephio_input_loop
 *   3. zephio_mouse_disable(ctx)  — before zephio_shutdown()
 */

#ifndef ZEPHIO_MOUSE_H
#define ZEPHIO_MOUSE_H

#include "zephio_context.h"

/**
 * @brief Mouse buttons.
 */
typedef enum {
    ZEPHIO_MOUSE_BTN_NONE   = 0,
    ZEPHIO_MOUSE_BTN_LEFT   = 1,
    ZEPHIO_MOUSE_BTN_MIDDLE = 2,
    ZEPHIO_MOUSE_BTN_RIGHT  = 3
} ZephioMouseButton;

/**
 * @brief Mouse actions.
 */
typedef enum {
    ZEPHIO_MOUSE_PRESS      = 0,
    ZEPHIO_MOUSE_RELEASE    = 1,
    ZEPHIO_MOUSE_MOTION     = 2,
    ZEPHIO_MOUSE_WHEEL_UP   = 3,
    ZEPHIO_MOUSE_WHEEL_DOWN = 4
} ZephioMouseAction;

/**
 * @brief Mouse event data.
 */
typedef struct {
    int             row;
    int             col;
    ZephioMouseButton  button;
    ZephioMouseAction  action;
    int             modifiers;
} ZephioMouseEvent;

/**
 * @brief Enable mouse tracking (basic + drag + SGR mode).
 *
 * @param ctx  TUI context.
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_mouse_enable(ZephioContext *ctx);

/**
 * @brief Disable mouse tracking.
 *
 * @param ctx  TUI context.
 */
void zephio_mouse_disable(ZephioContext *ctx);

/**
 * @brief Return a human-readable mouse action name.
 *
 * @param action  The mouse action.
 * @return Static string (do not free).
 */
const char *zephio_mouse_action_name(ZephioMouseAction action);

/**
 * @brief Return a human-readable mouse button name.
 *
 * @param button  The mouse button.
 * @return Static string (do not free).
 */
const char *zephio_mouse_button_name(ZephioMouseButton button);

#endif
