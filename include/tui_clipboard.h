/**
 * @file tui_clipboard.h
 * @brief Terminal clipboard support via OSC 52 escape sequences.
 *
 * Uses OSC 52 to set the terminal clipboard. Not all terminal
 * emulators support this; the function silently does nothing if
 * the terminal does not acknowledge OSC 52.
 *
 * Base64 encoding is implemented internally (no external deps).
 */

#ifndef ZEPHIO_CLIPBOARD_H
#define ZEPHIO_CLIPBOARD_H

#include "tui_context.h"
#include <stddef.h>

/**
 * @brief Copy text to the system clipboard using OSC 52.
 *
 * Encodes the text as base64 and emits the OSC 52 escape sequence.
 * The terminal must support OSC 52 for this to have any effect.
 *
 * @param ctx   TUI context.
 * @param text  UTF-8 text to copy (must be NUL-terminated).
 * @return 0 on success, -1 on failure.
 */
int tui_clipboard_copy(TuiContext *ctx, const char *text);

/**
 * @brief Copy a bounded byte range to the clipboard using OSC 52.
 *
 * @param ctx   TUI context.
 * @param data  Pointer to the data.
 * @param len   Number of bytes.
 * @return 0 on success, -1 on failure.
 */
int tui_clipboard_copy_n(TuiContext *ctx, const char *data, size_t len);

#endif
