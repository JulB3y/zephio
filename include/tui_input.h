/**
 * @file tui_input.h
 * @brief Keyboard input parsing and event loop.
 *
 * Reads raw bytes from stdin and decodes them into TuiEvent structs.
 * Supports ASCII, UTF-8, Ctrl/Alt/Shift modifiers, function keys,
 * arrow keys, and terminal resize events.
 *
 * Usage:
 *   1. tui_input_init(ctx)          — after tui_init()
 *   2. tui_input_poll(ctx, &evt)    — blocking poll for a single event
 *      or
 *      tui_input_loop(ctx, cb, ud)  — event loop with callback
 *   3. tui_input_shutdown(ctx)      — before tui_shutdown()
 */

#ifndef TUI_INPUT_H
#define TUI_INPUT_H

#include "tui_context.h"
#include "tui_mouse.h"

/**
 * @brief Keyboard modifier flags (bitmask).
 */
typedef enum {
    TUI_MOD_NONE  = 0,
    TUI_MOD_SHIFT = 1 << 0,
    TUI_MOD_ALT   = 1 << 1,
    TUI_MOD_CTRL  = 1 << 2
} TuiModifier;

/**
 * @brief Key codes for special keys.
 *
 * Printable characters arrive as TUI_KEY_UNKNOWN with the codepoint
 * field set. Special keys use values >= 0x1000.
 */
typedef enum {
    TUI_KEY_UNKNOWN = 0,

    TUI_KEY_ENTER       = 0x0D,
    TUI_KEY_TAB         = 0x09,
    TUI_KEY_BACKSPACE   = 0x7F,
    TUI_KEY_ESCAPE      = 0x1B,

    TUI_KEY_CTRL_A = 1,  TUI_KEY_CTRL_B = 2,  TUI_KEY_CTRL_C = 3,
    TUI_KEY_CTRL_D = 4,  TUI_KEY_CTRL_E = 5,  TUI_KEY_CTRL_F = 6,
    TUI_KEY_CTRL_G = 7,  TUI_KEY_CTRL_H = 8,  TUI_KEY_CTRL_I = 9,
    TUI_KEY_CTRL_J = 10, TUI_KEY_CTRL_K = 11, TUI_KEY_CTRL_L = 12,
    TUI_KEY_CTRL_M = 13, TUI_KEY_CTRL_N = 14, TUI_KEY_CTRL_O = 15,
    TUI_KEY_CTRL_P = 16, TUI_KEY_CTRL_Q = 17, TUI_KEY_CTRL_R = 18,
    TUI_KEY_CTRL_S = 19, TUI_KEY_CTRL_T = 20, TUI_KEY_CTRL_U = 21,
    TUI_KEY_CTRL_V = 22, TUI_KEY_CTRL_W = 23, TUI_KEY_CTRL_X = 24,
    TUI_KEY_CTRL_Y = 25, TUI_KEY_CTRL_Z = 26,

    TUI_KEY_UP    = 0x1000,
    TUI_KEY_DOWN,
    TUI_KEY_RIGHT,
    TUI_KEY_LEFT,

    TUI_KEY_HOME,
    TUI_KEY_END,
    TUI_KEY_INSERT,
    TUI_KEY_DELETE,
    TUI_KEY_PAGE_UP,
    TUI_KEY_PAGE_DOWN,

    TUI_KEY_F1,
    TUI_KEY_F2,
    TUI_KEY_F3,
    TUI_KEY_F4,
    TUI_KEY_F5,
    TUI_KEY_F6,
    TUI_KEY_F7,
    TUI_KEY_F8,
    TUI_KEY_F9,
    TUI_KEY_F10,
    TUI_KEY_F11,
    TUI_KEY_F12,

    TUI_EVENT_RESIZE = 0x2000,
    TUI_EVENT_MOUSE
} TuiKey;

/**
 * @brief Input event (key press, resize, or mouse).
 */
typedef struct {
    TuiKey        key;
    int           modifiers;
    uint32_t      codepoint;
    TuiSize       size;
    TuiMouseEvent mouse;
} TuiEvent;

/**
 * @brief Callback type for tui_input_loop().
 *
 * @param event      The input event.
 * @param user_data  User-provided context.
 * @return 1 to stop the loop, 0 to continue.
 */
typedef int (*TuiInputCallback)(const TuiEvent *event, void *user_data);

/**
 * @brief Initialize the input subsystem.
 *
 * Installs a SIGWINCH handler for resize detection.
 *
 * @param ctx  TUI context.
 * @return TUI_OK on success.
 */
TuiResult tui_input_init(TuiContext *ctx);

/**
 * @brief Shut down the input subsystem.
 *
 * @param ctx  TUI context.
 */
void tui_input_shutdown(TuiContext *ctx);

/**
 * @brief Block until an input event is available, then return it.
 *
 * Handles resize events transparently by querying the new terminal size.
 *
 * @param ctx         TUI context.
 * @param[out] event  Populated with the next input event.
 * @return TUI_OK on success.
 */
TuiResult tui_input_poll(TuiContext *ctx, TuiEvent *event);

/**
 * @brief Run a blocking input loop with a callback.
 *
 * Returns when the callback returns 1.
 *
 * @param ctx        TUI context.
 * @param callback   Called for every event.
 * @param user_data  Passed through to the callback.
 * @return TUI_OK on success.
 */
int tui_input_loop(TuiContext *ctx, TuiInputCallback callback, void *user_data);

/**
 * @brief Return a human-readable name for a key code.
 *
 * For printable characters this is the character itself.
 * For special keys it returns names like "Up", "F1", "Enter".
 *
 * @param key  The key code.
 * @return Static string (do not free).
 */
const char *tui_key_name(TuiKey key);

/**
 * @brief Return a human-readable modifier description.
 *
 * @param mod  Bitmask of TUI_MOD_* flags.
 * @return Static string (do not free).
 */
const char *tui_modifier_name(int mod);

#endif
