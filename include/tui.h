/**
 * @file tui.h
 * @brief Core initialization and terminal lifecycle.
 *
 * This is the primary entry point for the TUI framework. Call tui_init()
 * before using any other API. Call tui_shutdown() when done to restore
 * the terminal to its original state.
 *
 * All other headers are self-contained and can be included independently,
 * but tui_init() must be called first.
 */

#ifndef TUI_H
#define TUI_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Terminal dimensions (rows x cols).
 */
typedef struct {
    int rows;
    int cols;
} TuiSize;

/**
 * @brief Framework return codes.
 *
 * All public API functions return TuiResult. TUI_OK (0) indicates success;
 * any other value indicates an error.
 */
typedef enum {
    TUI_OK = 0,
    TUI_ERR_IOCTL,
    TUI_ERR_TCGETATTR,
    TUI_ERR_TCSETATTR,
    TUI_ERR_WRITE,
    TUI_ERR_MEMORY
} TuiResult;

/**
 * @brief Initialize the TUI framework.
 *
 * Enables raw terminal mode, enters the alternate screen buffer, hides
 * the cursor, registers atexit() cleanup and signal handlers (SIGINT,
 * SIGTERM, SIGQUIT). Must be called before any other TUI function.
 *
 * @return TUI_OK on success, or TUI_ERR_* on failure.
 */
TuiResult tui_init(void);

/**
 * @brief Shut down the TUI framework.
 *
 * Restores the original terminal state: exits alternate screen, shows
 * the cursor, disables raw mode, disables mouse tracking. Safe to call
 * multiple times.
 */
void tui_shutdown(void);

/**
 * @brief Query the current terminal size.
 *
 * @param[out] size  Populated with rows and cols on success.
 * @return TUI_OK on success, or TUI_ERR_IOCTL on failure.
 */
TuiResult tui_get_size(TuiSize *size);

#endif
