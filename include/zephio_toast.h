/**
 * @file zephio_toast.h
 * @brief Toast/notification overlay system.
 *
 * Renders floating notification toasts stacked in the top-right corner.
 * Each toast has a severity level (info, success, warning, error),
 * a message, an auto-dismiss timer, and fade-in/fade-out animations.
 *
 * Toasts render on top of all other widgets (Z-order above overlays).
 *
 * Usage:
 *   1. zephio_toast_manager_init() — once after app creation.
 *   2. zephio_toast_show(severity, message, duration_ms) to post a toast.
 *   3. zephio_toast_update(delta_ms) each tick (auto-called by ZephioApp).
 *   4. zephio_toast_render() after all other rendering (auto-called by ZephioApp).
 *   5. zephio_toast_manager_free() on shutdown.
 */

#ifndef ZEPHIO_TOAST_H
#define ZEPHIO_TOAST_H

#include "zephio.h"
#include "zephio_screen.h"

#define ZEPHIO_TOAST_MAX_COUNT    8
#define ZEPHIO_TOAST_MAX_MESSAGE  128
#define ZEPHIO_TOAST_DEFAULT_MS   3000
#define ZEPHIO_TOAST_FADE_MS      300
#define ZEPHIO_TOAST_MIN_WIDTH    24
#define ZEPHIO_TOAST_MAX_WIDTH    52
#define ZEPHIO_TOAST_HEIGHT       3
#define ZEPHIO_TOAST_MARGIN       1

typedef struct ZephioToastManager ZephioToastManager;

/**
 * @brief Toast severity levels with distinct colors.
 */
typedef enum {
    ZEPHIO_TOAST_INFO    = 0,
    ZEPHIO_TOAST_SUCCESS = 1,
    ZEPHIO_TOAST_WARNING = 2,
    ZEPHIO_TOAST_ERROR   = 3
} ZephioToastSeverity;

/**
 * @brief Toast animation state.
 */
typedef enum {
    ZEPHIO_TOAST_FADE_IN  = 0,
    ZEPHIO_TOAST_VISIBLE  = 1,
    ZEPHIO_TOAST_FADE_OUT = 2,
    ZEPHIO_TOAST_DISMISSED = 3
} ZephioToastState;

typedef void (*ZephioToastDismissFn)(int toast_id, void *user_data);

/**
 * @brief A single toast notification instance.
 */
typedef struct {
    int              id;
    ZephioToastSeverity severity;
    char             message[ZEPHIO_TOAST_MAX_MESSAGE];
    double           duration_ms;
    double           elapsed_ms;
    double           anim_ms;
    ZephioToastState    state;
    int              width;
    int              row;
    int              col;
    ZephioToastDismissFn on_dismiss;
    void            *user_data;
} ZephioToast;

struct ZephioToastManager {
    ZephioToast toasts[ZEPHIO_TOAST_MAX_COUNT];
    int       count;
    int       next_id;
};

/**
 * @brief Initialize the toast manager.
 *
 * @param mgr  Toast manager struct to initialize.
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_toast_manager_init(ZephioToastManager *mgr);

/**
 * @brief Free the toast manager resources.
 *
 * @param mgr  Toast manager to free.
 */
void zephio_toast_manager_free(ZephioToastManager *mgr);

/**
 * @brief Show a toast notification.
 *
 * @param mgr           Toast manager.
 * @param severity      Severity level.
 * @param message       Message text (copied, max 127 chars).
 * @param duration_ms   Auto-dismiss time in ms (0 = use default 3000ms).
 * @return Toast ID (>= 0), or -1 on failure.
 */
int zephio_toast_show(ZephioToastManager *mgr, ZephioToastSeverity severity,
                   const char *message, double duration_ms);

/**
 * @brief Show a toast with a dismiss callback.
 *
 * @param mgr           Toast manager.
 * @param severity      Severity level.
 * @param message       Message text.
 * @param duration_ms   Auto-dismiss time in ms.
 * @param on_dismiss    Called when the toast is dismissed (may be NULL).
 * @param user_data     User data for the callback.
 * @return Toast ID (>= 0), or -1 on failure.
 */
int zephio_toast_show_cb(ZephioToastManager *mgr, ZephioToastSeverity severity,
                      const char *message, double duration_ms,
                      ZephioToastDismissFn on_dismiss, void *user_data);

/**
 * @brief Dismiss a specific toast by ID.
 *
 * Triggers the fade-out animation. The toast is removed once the
 * animation completes.
 *
 * @param mgr  Toast manager.
 * @param id   Toast ID returned by zephio_toast_show().
 */
void zephio_toast_dismiss(ZephioToastManager *mgr, int id);

/**
 * @brief Dismiss all active toasts.
 *
 * @param mgr  Toast manager.
 */
void zephio_toast_dismiss_all(ZephioToastManager *mgr);

/**
 * @brief Update toast animations and auto-dismiss timers.
 *
 * Call once per frame with the elapsed time in ms.
 *
 * @param mgr       Toast manager.
 * @param delta_ms  Elapsed time since last update.
 */
void zephio_toast_update(ZephioToastManager *mgr, double delta_ms);

/**
 * @brief Render all active toasts.
 *
 * Call after all other rendering (including overlays) so toasts
 * appear on top of everything.
 *
 * @param ctx        ZephioContext for rendering.
 * @param mgr        Toast manager.
 * @param screen_rows Terminal rows (for positioning).
 * @param screen_cols Terminal cols (for positioning).
 */
void zephio_toast_render(ZephioContext *ctx, ZephioToastManager *mgr, int screen_rows, int screen_cols);

/**
 * @brief Check if any toast is currently visible.
 *
 * @param mgr  Toast manager.
 * @return 1 if at least one toast is visible, 0 otherwise.
 */
int zephio_toast_has_active(const ZephioToastManager *mgr);

#endif
