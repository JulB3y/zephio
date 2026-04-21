/**
 * @file tui.h
 * @brief Core initialization and terminal lifecycle.
 *
 * This is the primary entry point for the TUI framework. Call zephio_init()
 * before using any other API. Call zephio_shutdown() when done to restore
 * the terminal to its original state.
 *
 * All other headers are self-contained and can be included independently,
 * but zephio_init() must be called first.
 */

#ifndef ZEPHIO_H
#define ZEPHIO_H

#include <stddef.h>
#include <stdint.h>
#include "zephio_export.h"

typedef struct ZephioContext ZephioContext;

/**
 * @brief Terminal dimensions (rows x cols).
 */
typedef struct {
    int rows;
    int cols;
} ZephioSize;

/**
 * @brief Framework return codes.
 *
 * All public API functions return ZephioResult. ZEPHIO_OK (0) indicates success;
 * any other value indicates an error.
 */
typedef enum {
    ZEPHIO_OK = 0,
    TUI_ERR_IOCTL,
    TUI_ERR_TCGETATTR,
    TUI_ERR_TCSETATTR,
    TUI_ERR_WRITE,
    TUI_ERR_MEMORY
} ZephioResult;

/**
 * @brief Initialize the TUI framework.
 *
 * Enables raw terminal mode, enters the alternate screen buffer, hides
 * the cursor, registers atexit() cleanup and signal handlers (SIGINT,
 * SIGTERM, SIGQUIT). Must be called before any other TUI function.
 *
 * @return ZEPHIO_OK on success, or TUI_ERR_* on failure.
 */
ZEPHIO_API ZephioResult zephio_init(ZephioContext *ctx);

/**
 * @brief Shut down the TUI framework.
 *
 * Restores the original terminal state: exits alternate screen, shows
 * the cursor, disables raw mode, disables mouse tracking. Safe to call
 * multiple times.
 */
ZEPHIO_API void zephio_shutdown(ZephioContext *ctx);

/**
 * @brief Query the current terminal size.
 *
 * @param[out] size  Populated with rows and cols on success.
 * @return ZEPHIO_OK on success, or TUI_ERR_IOCTL on failure.
 */
ZEPHIO_API ZephioResult zephio_get_size(ZephioContext *ctx, ZephioSize *size);

#endif
