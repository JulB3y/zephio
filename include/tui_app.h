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
#ifdef TUI_FEATURE_TOAST
#include "tui_toast.h"
#endif
#include "tui_widget.h"

#define TUI_APP_MAX_OVERLAYS 16

typedef struct TuiAnimator TuiAnimator;
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

    TuiWidget *overlays[TUI_APP_MAX_OVERLAYS];
    int        overlay_count;

    TuiAnimator *animator;
    double       last_tick_ms;

    TuiToastManager toasts;
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

/**
 * @brief Push a widget onto the overlay stack.
 *
 * Overlays render on top of all other widgets. When an overlay is
 * present, input events are routed exclusively to the topmost overlay.
 *
 * @param app     Application.
 * @param widget  Widget to push (must remain valid until popped).
 * @return TUI_OK on success, TUI_ERR_MEMORY if the stack is full.
 */
TuiResult tui_app_push_overlay(TuiApp *app, TuiWidget *widget);

/**
 * @brief Remove the topmost overlay from the stack.
 *
 * Does not destroy the widget.
 *
 * @param app  Application.
 * @return The removed widget, or NULL if the stack was empty.
 */
TuiWidget *tui_app_pop_overlay(TuiApp *app);

/**
 * @brief Return the topmost overlay, or NULL if none.
 */
TuiWidget *tui_app_top_overlay(TuiApp *app);

/**
 * @brief Render all overlays in stack order (bottom to top).
 *
 * Called by the app's render logic after the main widget tree.
 */
void tui_app_render_overlays(TuiApp *app);

/**
 * @brief Dispatch an input event to the topmost overlay.
 *
 * If no overlay is active, returns 0.
 *
 * @return 1 if the event was consumed, 0 otherwise.
 */
int tui_app_handle_overlay_input(TuiApp *app, const TuiEvent *event);

/**
 * @brief Dispatch a mouse event to the topmost overlay.
 *
 * If no overlay is active, returns 0.
 *
 * @return 1 if the event was consumed, 0 otherwise.
 */
int tui_app_handle_overlay_mouse(TuiApp *app, const TuiMouseEvent *mouse);

/**
 * @brief Return the app's animation manager.
 *
 * The animator is auto-updated each tick. Use this to create and
 * control animations.
 *
 * @return Pointer to the animator, or NULL if not initialized.
 */
TuiAnimator *tui_app_get_animator(TuiApp *app);

/**
 * @brief Return the app's toast manager.
 *
 * Use this to show, dismiss, or query toast notifications.
 *
 * @return Pointer to the toast manager.
 */
TuiToastManager *tui_app_get_toasts(TuiApp *app);

/**
 * @brief Convenience: show a toast via the app's toast manager.
 *
 * @param app         Application.
 * @param severity    Toast severity level.
 * @param message     Message text.
 * @param duration_ms Auto-dismiss time (0 = default 3000ms).
 * @return Toast ID, or -1 on failure.
 */
int tui_app_toast(TuiApp *app, TuiToastSeverity severity,
                  const char *message, double duration_ms);

/**
 * @brief Render all active toasts on top of everything else.
 *
 * Called automatically by the app's render loop after overlays.
 * Can also be called manually in custom render loops.
 */
void tui_app_render_toasts(TuiApp *app);

#endif
