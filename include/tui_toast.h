/**
 * @file tui_toast.h
 * @brief Toast/notification overlay system.
 *
 * Renders floating notification toasts stacked in the top-right corner.
 * Each toast has a severity level (info, success, warning, error),
 * a message, an auto-dismiss timer, and fade-in/fade-out animations.
 *
 * Toasts render on top of all other widgets (Z-order above overlays).
 *
 * Usage:
 *   1. tui_toast_manager_init() — once after app creation.
 *   2. tui_toast_show(severity, message, duration_ms) to post a toast.
 *   3. tui_toast_update(delta_ms) each tick (auto-called by TuiApp).
 *   4. tui_toast_render() after all other rendering (auto-called by TuiApp).
 *   5. tui_toast_manager_free() on shutdown.
 */

#ifndef TUI_TOAST_H
#define TUI_TOAST_H

#include "tui.h"
#include "tui_screen.h"

#define TUI_TOAST_MAX_COUNT    8
#define TUI_TOAST_MAX_MESSAGE  128
#define TUI_TOAST_DEFAULT_MS   3000
#define TUI_TOAST_FADE_MS      300
#define TUI_TOAST_MIN_WIDTH    24
#define TUI_TOAST_MAX_WIDTH    52
#define TUI_TOAST_HEIGHT       3
#define TUI_TOAST_MARGIN       1

typedef struct TuiToastManager TuiToastManager;

/**
 * @brief Toast severity levels with distinct colors.
 */
typedef enum {
    TUI_TOAST_INFO    = 0,
    TUI_TOAST_SUCCESS = 1,
    TUI_TOAST_WARNING = 2,
    TUI_TOAST_ERROR   = 3
} TuiToastSeverity;

/**
 * @brief Toast animation state.
 */
typedef enum {
    TUI_TOAST_FADE_IN  = 0,
    TUI_TOAST_VISIBLE  = 1,
    TUI_TOAST_FADE_OUT = 2,
    TUI_TOAST_DISMISSED = 3
} TuiToastState;

typedef void (*TuiToastDismissFn)(int toast_id, void *user_data);

/**
 * @brief A single toast notification instance.
 */
typedef struct {
    int              id;
    TuiToastSeverity severity;
    char             message[TUI_TOAST_MAX_MESSAGE];
    double           duration_ms;
    double           elapsed_ms;
    double           anim_ms;
    TuiToastState    state;
    int              width;
    int              row;
    int              col;
    TuiToastDismissFn on_dismiss;
    void            *user_data;
} TuiToast;

struct TuiToastManager {
    TuiToast toasts[TUI_TOAST_MAX_COUNT];
    int       count;
    int       next_id;
};

/**
 * @brief Initialize the toast manager.
 *
 * @param mgr  Toast manager struct to initialize.
 * @return TUI_OK on success.
 */
TuiResult tui_toast_manager_init(TuiToastManager *mgr);

/**
 * @brief Free the toast manager resources.
 *
 * @param mgr  Toast manager to free.
 */
void tui_toast_manager_free(TuiToastManager *mgr);

/**
 * @brief Show a toast notification.
 *
 * @param mgr           Toast manager.
 * @param severity      Severity level.
 * @param message       Message text (copied, max 127 chars).
 * @param duration_ms   Auto-dismiss time in ms (0 = use default 3000ms).
 * @return Toast ID (>= 0), or -1 on failure.
 */
int tui_toast_show(TuiToastManager *mgr, TuiToastSeverity severity,
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
int tui_toast_show_cb(TuiToastManager *mgr, TuiToastSeverity severity,
                      const char *message, double duration_ms,
                      TuiToastDismissFn on_dismiss, void *user_data);

/**
 * @brief Dismiss a specific toast by ID.
 *
 * Triggers the fade-out animation. The toast is removed once the
 * animation completes.
 *
 * @param mgr  Toast manager.
 * @param id   Toast ID returned by tui_toast_show().
 */
void tui_toast_dismiss(TuiToastManager *mgr, int id);

/**
 * @brief Dismiss all active toasts.
 *
 * @param mgr  Toast manager.
 */
void tui_toast_dismiss_all(TuiToastManager *mgr);

/**
 * @brief Update toast animations and auto-dismiss timers.
 *
 * Call once per frame with the elapsed time in ms.
 *
 * @param mgr       Toast manager.
 * @param delta_ms  Elapsed time since last update.
 */
void tui_toast_update(TuiToastManager *mgr, double delta_ms);

/**
 * @brief Render all active toasts.
 *
 * Call after all other rendering (including overlays) so toasts
 * appear on top of everything.
 *
 * @param ctx        TuiContext for rendering.
 * @param mgr        Toast manager.
 * @param screen_rows Terminal rows (for positioning).
 * @param screen_cols Terminal cols (for positioning).
 */
void tui_toast_render(TuiContext *ctx, TuiToastManager *mgr, int screen_rows, int screen_cols);

/**
 * @brief Check if any toast is currently visible.
 *
 * @param mgr  Toast manager.
 * @return 1 if at least one toast is visible, 0 otherwise.
 */
int tui_toast_has_active(const TuiToastManager *mgr);

#endif
