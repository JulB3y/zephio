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
 * @brief Write an arbitrary byte sequence to the terminal.
 *
 * No-op if the terminal is not initialized.
 *
 * @param t    Pointer to the Terminal struct.
 * @param seq  Byte sequence to write.
 * @param len  Number of bytes.
 */
void terminal_write_seq(Terminal *t, const char *seq, size_t len);

extern Terminal g_terminal;

#endif
