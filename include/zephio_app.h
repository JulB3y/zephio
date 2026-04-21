/**
 * @file zephio_app.h
 * @brief High-level application runtime (event loop, lifecycle hooks).
 *
 * ZephioApp wraps the init/render/input/shutdown cycle into a single
 * zephio_app_run() call. The user provides a ZephioAppConfig with callback
 * functions for each lifecycle phase.
 *
 * Usage:
 *   1. Fill a ZephioAppConfig with callbacks.
 *   2. zephio_app_new(ctx, &config)  — allocates a ZephioApp.
 *   3. zephio_app_run(app)           — enters the event loop.
 *   4. zephio_app_free(app)          — after run returns.
 */

#ifndef ZEPHIO_APP_H
#define ZEPHIO_APP_H

#include "zephio_context.h"
#include "zephio_input.h"
#include "zephio_mouse.h"
#include "zephio_toast.h"
#include "zephio_widget.h"

#define ZEPHIO_APP_MAX_OVERLAYS 16

typedef struct ZephioAnimator ZephioAnimator;
typedef struct ZephioApp ZephioApp;

/** @brief Called once after zephio_init. Return 0 to continue, non-zero to abort. */
typedef int  (*ZephioAppInitFn)(ZephioApp *app, void *user_data);
/** @brief Called on terminal resize. */
typedef int  (*ZephioAppResizeFn)(ZephioApp *app, int rows, int cols, void *user_data);
/** @brief Called every frame to draw the UI. */
typedef int  (*ZephioAppRenderFn)(ZephioApp *app, void *user_data);
/** @brief Called on shutdown for cleanup. */
typedef void (*ZephioAppShutdownFn)(ZephioApp *app, void *user_data);
/** @brief Called for keyboard events. Return 1 to stop the loop. */
typedef int  (*ZephioAppInputFn)(ZephioApp *app, const ZephioEvent *event, void *user_data);
/** @brief Called for mouse events. */
typedef int  (*ZephioAppMouseFn)(ZephioApp *app, const ZephioMouseEvent *mouse, void *user_data);

/**
 * @brief Application configuration (lifecycle callbacks + user data).
 */
typedef struct {
    ZephioAppInitFn     on_init;
    ZephioAppResizeFn   on_resize;
    ZephioAppRenderFn   on_render;
    ZephioAppShutdownFn on_shutdown;
    ZephioAppInputFn    on_input;
    ZephioAppMouseFn    on_mouse;
    void            *user_data;
    int              tick_rate_ms;
} ZephioAppConfig;

struct ZephioApp {
    ZephioContext  *ctx;
    ZephioAppConfig config;
    int          running;
    int          exit_code;

    ZephioWidget *overlays[ZEPHIO_APP_MAX_OVERLAYS];
    int        overlay_count;

    ZephioAnimator *animator;
    double       last_tick_ms;

    ZephioToastManager toasts;
};

/**
 * @brief Allocate and initialize a ZephioApp.
 *
 * Copies the config struct. Calls zephio_init(), zephio_input_init(),
 * zephio_mouse_enable(), and config.on_init internally.
 *
 * @param ctx     TUI context.
 * @param config  Configuration.
 * @return Heap-allocated ZephioApp, or NULL on failure.
 */
ZephioApp *zephio_app_new(ZephioContext *ctx, const ZephioAppConfig *config);

/**
 * @brief Free a ZephioApp (does NOT call zephio_shutdown).
 */
void zephio_app_free(ZephioApp *app);

/**
 * @brief Enter the main event loop.
 *
 * Blocks until zephio_app_stop() is called or a callback returns non-zero.
 * Handles resize events, mouse events, keyboard events, and timed renders.
 *
 * @return Exit code (0 on success).
 */
int zephio_app_run(ZephioApp *app);

/**
 * @brief Push a widget onto the overlay stack.
 *
 * Overlays render on top of all other widgets. When an overlay is
 * present, input events are routed exclusively to the topmost overlay.
 *
 * @param app     Application.
 * @param widget  Widget to push (must remain valid until popped).
 * @return ZEPHIO_OK on success, TUI_ERR_MEMORY if the stack is full.
 */
ZephioResult zephio_app_push_overlay(ZephioApp *app, ZephioWidget *widget);

/**
 * @brief Remove the topmost overlay from the stack.
 *
 * Does not destroy the widget.
 *
 * @param app  Application.
 * @return The removed widget, or NULL if the stack was empty.
 */
ZephioWidget *zephio_app_pop_overlay(ZephioApp *app);

/**
 * @brief Render all overlays in stack order (bottom to top).
 *
 * Called by the app's render logic after the main widget tree.
 */
void zephio_app_render_overlays(ZephioApp *app);

/**
 * @brief Return the app's animation manager.
 *
 * The animator is auto-updated each tick. Use this to create and
 * control animations.
 *
 * @return Pointer to the animator, or NULL if not initialized.
 */
ZephioAnimator *zephio_app_get_animator(ZephioApp *app);

/**
 * @brief Return the app's toast manager.
 *
 * Use this to show, dismiss, or query toast notifications.
 *
 * @return Pointer to the toast manager.
 */
ZephioToastManager *zephio_app_get_toasts(ZephioApp *app);

/**
 * @brief Convenience: show a toast via the app's toast manager.
 *
 * @param app         Application.
 * @param severity    Toast severity level.
 * @param message     Message text.
 * @param duration_ms Auto-dismiss time (0 = default 3000ms).
 * @return Toast ID, or -1 on failure.
 */
int zephio_app_toast(ZephioApp *app, ZephioToastSeverity severity,
                  const char *message, double duration_ms);

/**
 * @brief Render all active toasts on top of everything else.
 *
 * Called automatically by the app's render loop after overlays.
 * Can also be called manually in custom render loops.
 */
void zephio_app_render_toasts(ZephioApp *app);

#endif
