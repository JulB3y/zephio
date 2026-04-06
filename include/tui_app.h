/**
 * @file tui_app.h
 * @brief High-level application runtime (event loop, lifecycle hooks).
 *
 * TuiApp wraps the init/render/input/shutdown cycle into a single
 * tui_app_run() call. The user provides a TuiAppConfig with callback
 * functions for each lifecycle phase.
 *
 * Usage:
 *   1. Fill a TuiAppConfig with callbacks.
 *   2. tui_app_new(&config)  — allocates a TuiApp.
 *   3. tui_app_run(app)       — enters the event loop.
 *   4. tui_app_free(app)      — after run returns.
 */

#ifndef TUI_APP_H
#define TUI_APP_H

#include "tui.h"
#include "tui_input.h"
#include "tui_mouse.h"

typedef struct TuiApp TuiApp;

/** @brief Called once after tui_init. Return 0 to continue, non-zero to abort. */
typedef int  (*TuiAppInitFn)(TuiApp *app, void *user_data);
/** @brief Called on terminal resize. */
typedef int  (*TuiAppResizeFn)(TuiApp *app, int rows, int cols, void *user_data);
/** @brief Called every frame to draw the UI. */
typedef int  (*TuiAppRenderFn)(TuiApp *app, void *user_data);
/** @brief Called on shutdown for cleanup. */
typedef void (*TuiAppShutdownFn)(TuiApp *app, void *user_data);
/** @brief Called for keyboard events. Return 1 to stop the loop. */
typedef int  (*TuiAppInputFn)(TuiApp *app, const TuiEvent *event, void *user_data);
/** @brief Called for mouse events. */
typedef int  (*TuiAppMouseFn)(TuiApp *app, const TuiMouseEvent *mouse, void *user_data);

/**
 * @brief Application configuration (lifecycle callbacks + user data).
 */
typedef struct {
    TuiAppInitFn     on_init;
    TuiAppResizeFn   on_resize;
    TuiAppRenderFn   on_render;
    TuiAppShutdownFn on_shutdown;
    TuiAppInputFn    on_input;
    TuiAppMouseFn    on_mouse;
    void            *user_data;
    int              tick_rate_ms;
} TuiAppConfig;

struct TuiApp {
    TuiAppConfig config;
    int          running;
    int          exit_code;
};

/**
 * @brief Allocate and initialize a TuiApp.
 *
 * Copies the config struct. Calls tui_init(), tui_input_init(),
 * tui_mouse_enable(), and config.on_init internally.
 *
 * @param config  Configuration.
 * @return Heap-allocated TuiApp, or NULL on failure.
 */
TuiApp *tui_app_new(const TuiAppConfig *config);

/**
 * @brief Free a TuiApp (does NOT call tui_shutdown).
 */
void tui_app_free(TuiApp *app);

/**
 * @brief Enter the main event loop.
 *
 * Blocks until tui_app_stop() is called or a callback returns non-zero.
 * Handles resize events, mouse events, keyboard events, and timed renders.
 *
 * @return Exit code (0 on success).
 */
int tui_app_run(TuiApp *app);

/**
 * @brief Signal the app to stop.
 *
 * Safe to call from any callback.
 */
void tui_app_stop(TuiApp *app);

#endif
