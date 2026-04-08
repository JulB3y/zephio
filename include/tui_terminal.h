/**
 * @file tui_terminal.h
 * @brief Low-level terminal control (raw mode, alt screen, cursor).
 *
 * This module wraps termios, ioctl, and ANSI escape-sequence I/O.
 * It is used internally by tui_init() / tui_shutdown() but can also
 * be called directly for fine-grained control.
 *
 * The global Terminal instance g_terminal holds the current state.
 */

#ifndef TUI_TERMINAL_H
#define TUI_TERMINAL_H

#include "tui.h"

/**
 * @brief Internal terminal state.
 */
typedef struct {
    int  fd;
    int  rows;
    int  cols;
    int  initialized;
    int  truecolor;
} Terminal;

/**
 * @brief Enable raw (non-canonical, no-echo) mode on the terminal.
 *
 * Saves the original termios for later restoration.
 *
 * @param t  Pointer to the Terminal struct.
 * @return TUI_OK on success.
 */
TuiResult terminal_raw_mode_enable(Terminal *t);

/**
 * @brief Disable raw mode, restoring the saved termios state.
 *
 * @param t  Pointer to the Terminal struct.
 * @return TUI_OK on success.
 */
TuiResult terminal_raw_mode_disable(Terminal *t);

/**
 * @brief Switch to the alternate screen buffer.
 *
 * Preserves the user's original terminal content.
 *
 * @param t  Pointer to the Terminal struct.
 * @return TUI_OK on success.
 */
TuiResult terminal_enter_alt_screen(Terminal *t);

/**
 * @brief Switch back from the alternate screen buffer.
 *
 * @param t  Pointer to the Terminal struct.
 * @return TUI_OK on success.
 */
TuiResult terminal_exit_alt_screen(Terminal *t);

/**
 * @brief Hide the terminal cursor.
 *
 * @param t  Pointer to the Terminal struct.
 * @return TUI_OK on success.
 */
TuiResult terminal_cursor_hide(Terminal *t);

/**
 * @brief Show the terminal cursor.
 *
 * @param t  Pointer to the Terminal struct.
 * @return TUI_OK on success.
 */
TuiResult terminal_cursor_show(Terminal *t);

/**
 * @brief Clear the entire screen.
 *
 * @param t  Pointer to the Terminal struct.
 * @return TUI_OK on success.
 */
TuiResult terminal_clear_screen(Terminal *t);

/**
 * @brief Query terminal rows and cols via ioctl.
 *
 * @param t  Pointer to the Terminal struct (rows/cols updated).
 * @return TUI_OK on success, TUI_ERR_IOCTL on failure.
 */
TuiResult terminal_get_size(Terminal *t);

/**
 * @brief Write an arbitrary byte sequence to the terminal.
 *
 * No-op if the terminal is not initialized.
 *
 * @param t    Pointer to the Terminal struct.
 * @param seq  Byte sequence to write.
 * @param len  Number of bytes.
 */
void terminal_write_seq(Terminal *t, const char *seq, size_t len);

/**
 * @brief Ensure the terminal is restored (called from atexit / signal handler).
 *
 * Restores raw mode, exits alt screen, shows cursor, disables mouse.
 */
void terminal_ensure_cleanup(void);

extern Terminal g_terminal;

#endif
