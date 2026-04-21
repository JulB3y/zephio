/**
 * @file zephio_input.h
 * @brief Keyboard input parsing and event loop.
 *
 * Reads raw bytes from stdin and decodes them into ZephioEvent structs.
 * Supports ASCII, UTF-8, Ctrl/Alt/Shift modifiers, function keys,
 * arrow keys, and terminal resize events.
 *
 * Usage:
 *   1. zephio_input_init(ctx)          — after zephio_init()
 *   2. zephio_input_poll(ctx, &evt)    — blocking poll for a single event
 *      or
 *      zephio_input_loop(ctx, cb, ud)  — event loop with callback
 *   3. zephio_input_shutdown(ctx)      — before zephio_shutdown()
 */

#ifndef ZEPHIO_INPUT_H
#define ZEPHIO_INPUT_H

#include "zephio_context.h"
#include "zephio_mouse.h"
#include "zephio_export.h"

/**
 * @brief Keyboard modifier flags (bitmask).
 */
typedef enum {
    ZEPHIO_MOD_NONE  = 0,
    ZEPHIO_MOD_SHIFT = 1 << 0,
    ZEPHIO_MOD_ALT   = 1 << 1,
    ZEPHIO_MOD_CTRL  = 1 << 2
} ZephioModifier;

/**
 * @brief Key codes for special keys.
 *
 * Printable characters arrive as ZEPHIO_KEY_UNKNOWN with the codepoint
 * field set. Special keys use values >= 0x1000.
 */
typedef enum {
    ZEPHIO_KEY_UNKNOWN = 0,

    ZEPHIO_KEY_ENTER       = 0x0D,
    ZEPHIO_KEY_TAB         = 0x09,
    ZEPHIO_KEY_BACKSPACE   = 0x7F,
    ZEPHIO_KEY_ESCAPE      = 0x1B,

    ZEPHIO_KEY_CTRL_A = 1,  ZEPHIO_KEY_CTRL_B = 2,  ZEPHIO_KEY_CTRL_C = 3,
    ZEPHIO_KEY_CTRL_D = 4,  ZEPHIO_KEY_CTRL_E = 5,  ZEPHIO_KEY_CTRL_F = 6,
    ZEPHIO_KEY_CTRL_G = 7,  ZEPHIO_KEY_CTRL_H = 8,  ZEPHIO_KEY_CTRL_I = 9,
    ZEPHIO_KEY_CTRL_J = 10, ZEPHIO_KEY_CTRL_K = 11, ZEPHIO_KEY_CTRL_L = 12,
    ZEPHIO_KEY_CTRL_M = 13, ZEPHIO_KEY_CTRL_N = 14, ZEPHIO_KEY_CTRL_O = 15,
    ZEPHIO_KEY_CTRL_P = 16, ZEPHIO_KEY_CTRL_Q = 17, ZEPHIO_KEY_CTRL_R = 18,
    ZEPHIO_KEY_CTRL_S = 19, ZEPHIO_KEY_CTRL_T = 20, ZEPHIO_KEY_CTRL_U = 21,
    ZEPHIO_KEY_CTRL_V = 22, ZEPHIO_KEY_CTRL_W = 23, ZEPHIO_KEY_CTRL_X = 24,
    ZEPHIO_KEY_CTRL_Y = 25, ZEPHIO_KEY_CTRL_Z = 26,

    ZEPHIO_KEY_UP    = 0x1000,
    ZEPHIO_KEY_DOWN,
    ZEPHIO_KEY_RIGHT,
    ZEPHIO_KEY_LEFT,

    ZEPHIO_KEY_HOME,
    ZEPHIO_KEY_END,
    ZEPHIO_KEY_INSERT,
    ZEPHIO_KEY_DELETE,
    ZEPHIO_KEY_PAGE_UP,
    ZEPHIO_KEY_PAGE_DOWN,

    ZEPHIO_KEY_F1,
    ZEPHIO_KEY_F2,
    ZEPHIO_KEY_F3,
    ZEPHIO_KEY_F4,
    ZEPHIO_KEY_F5,
    ZEPHIO_KEY_F6,
    ZEPHIO_KEY_F7,
    ZEPHIO_KEY_F8,
    ZEPHIO_KEY_F9,
    ZEPHIO_KEY_F10,
    ZEPHIO_KEY_F11,
    ZEPHIO_KEY_F12,

    ZEPHIO_EVENT_RESIZE = 0x2000,
    ZEPHIO_EVENT_MOUSE
} ZephioKey;

/**
 * @brief Input event (key press, resize, or mouse).
 */
typedef struct {
    ZephioKey        key;
    int           modifiers;
    uint32_t      codepoint;
    ZephioSize       size;
    ZephioMouseEvent mouse;
} ZephioEvent;

/**
 * @brief Callback type for zephio_input_loop().
 *
 * @param event      The input event.
 * @param user_data  User-provided context.
 * @return 1 to stop the loop, 0 to continue.
 */
typedef int (*ZephioInputCallback)(const ZephioEvent *event, void *user_data);

/**
 * @brief Initialize the input subsystem.
 *
 * Installs a SIGWINCH handler for resize detection.
 *
 * @param ctx  TUI context.
 * @return ZEPHIO_OK on success.
 */
ZEPHIO_API ZephioResult zephio_input_init(ZephioContext *ctx);

/**
 * @brief Shut down the input subsystem.
 *
 * @param ctx  TUI context.
 */
ZEPHIO_API void zephio_input_shutdown(ZephioContext *ctx);

/**
 * @brief Block until an input event is available, then return it.
 *
 * Handles resize events transparently by querying the new terminal size.
 *
 * @param ctx         TUI context.
 * @param[out] event  Populated with the next input event.
 * @return ZEPHIO_OK on success.
 */
ZEPHIO_API ZephioResult zephio_input_poll(ZephioContext *ctx, ZephioEvent *event);

/**
 * @brief Run a blocking input loop with a callback.
 *
 * Returns when the callback returns 1.
 *
 * @param ctx        TUI context.
 * @param callback   Called for every event.
 * @param user_data  Passed through to the callback.
 * @return ZEPHIO_OK on success.
 */
ZEPHIO_API int zephio_input_loop(ZephioContext *ctx, ZephioInputCallback callback, void *user_data);

/**
 * @brief Return a human-readable name for a key code.
 *
 * For printable characters this is the character itself.
 * For special keys it returns names like "Up", "F1", "Enter".
 *
 * @param key  The key code.
 * @return Static string (do not free).
 */
ZEPHIO_API const char *zephio_key_name(ZephioKey key);

/**
 * @brief Return a human-readable modifier description.
 *
 * @param mod  Bitmask of ZEPHIO_MOD_* flags.
 * @return Static string (do not free).
 */
ZEPHIO_API const char *zephio_modifier_name(int mod);

#endif
