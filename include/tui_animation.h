/**
 * @file tui_animation.h
 * @brief Core animation types, easing functions, and animation state.
 *
 * Provides time-based animations with configurable easing curves.
 * Each animation has a duration, easing function, and user callbacks
 * for per-frame updates and completion.
 *
 * Usage:
 *   1. Create via tui_animator_create() on a TuiAnimator.
 *   2. The animator calls on_update(progress) each tick with the eased
 *      progress value (0.0 to 1.0).
 *   3. on_complete() is called when the animation finishes.
 *   4. Control playback with play/pause/stop.
 */

#ifndef ZEPHIO_ANIMATION_H
#define ZEPHIO_ANIMATION_H

#include <stdint.h>

/**
 * @brief Easing curve types.
 *
 * Each easing function maps a normalized time t in [0,1] to a
 * progress value in [0,1] with different acceleration profiles.
 */
typedef enum {
    TUI_EASE_LINEAR,
    TUI_EASE_IN_QUAD,
    TUI_EASE_OUT_QUAD,
    TUI_EASE_IN_OUT_QUAD,
    TUI_EASE_IN_CUBIC,
    TUI_EASE_OUT_CUBIC,
    TUI_EASE_IN_OUT_CUBIC,
    TUI_EASE_IN_SINE,
    TUI_EASE_OUT_SINE,
    TUI_EASE_IN_OUT_SINE,
    TUI_EASE_IN_EXPO,
    TUI_EASE_OUT_EXPO,
    TUI_EASE_IN_OUT_EXPO
} TuiEasing;

/**
 * @brief Animation playback state.
 */
typedef enum {
    TUI_ANIM_IDLE,
    TUI_ANIM_PLAYING,
    TUI_ANIM_PAUSED,
    TUI_ANIM_COMPLETED
} TuiAnimState;

/**
 * @brief Called every tick with the current eased progress.
 *
 * @param progress   Eased progress in [0.0, 1.0].
 * @param user_data  User-provided context.
 */
typedef void (*TuiAnimUpdateFn)(double progress, void *user_data);

/**
 * @brief Called once when the animation completes (all loops finished).
 *
 * @param user_data  User-provided context.
 */
typedef void (*TuiAnimCompleteFn)(void *user_data);

/**
 * @brief A single animation instance.
 *
 * Managed by TuiAnimator. Create via tui_animator_create(), control
 * via play/pause/stop.
 */
typedef struct TuiAnimation {
    int              id;
    TuiAnimState     state;
    TuiEasing        easing;
    double           duration_ms;
    double           elapsed_ms;
    int              loops;
    int              loops_remaining;
    TuiAnimUpdateFn  on_update;
    TuiAnimCompleteFn on_complete;
    void            *user_data;
} TuiAnimation;

/**
 * @brief Apply an easing function to a normalized time value.
 *
 * @param easing  The easing curve to apply.
 * @param t       Normalized time in [0.0, 1.0]. Values outside this range
 *                are clamped.
 * @return The eased progress value in [0.0, 1.0].
 */
double tui_ease(TuiEasing easing, double t);

/**
 * @brief Initialize an animation struct.
 *
 * Typically called internally by TuiAnimator. Can be used directly
 * for standalone animations.
 *
 * @param anim         Animation to initialize.
 * @param duration_ms  Duration in milliseconds (> 0).
 * @param easing       Easing curve.
 * @param on_update    Per-frame callback (may be NULL).
 * @param on_complete  Completion callback (may be NULL).
 * @param user_data    User context passed to callbacks.
 */
void tui_animation_init(TuiAnimation *anim, double duration_ms, TuiEasing easing,
                        TuiAnimUpdateFn on_update, TuiAnimCompleteFn on_complete,
                        void *user_data);

/**
 * @brief Set the number of loops.
 *
 * @param anim   Animation.
 * @param loops  0 = play once (no repeat), >0 = loop that many additional
 *               times, -1 = loop forever.
 */
void tui_animation_set_loops(TuiAnimation *anim, int loops);

/**
 * @brief Get the current eased progress of an animation.
 *
 * @param anim  Animation.
 * @return Eased progress in [0.0, 1.0], or 0.0 if not playing.
 */
double tui_animation_get_progress(const TuiAnimation *anim);

/**
 * @brief Start or resume an animation.
 */
void tui_animation_play(TuiAnimation *anim);

/**
 * @brief Pause a playing animation.
 */
void tui_animation_pause(TuiAnimation *anim);

/**
 * @brief Stop and reset an animation to the beginning.
 */
void tui_animation_stop(TuiAnimation *anim);

/**
 * @brief Advance an animation by delta milliseconds.
 *
 * Called internally by TuiAnimator. Calls on_update with the current
 * eased progress. Handles looping and completion.
 *
 * @param anim       Animation.
 * @param delta_ms   Elapsed time since last update in milliseconds.
 */
void tui_animation_update(TuiAnimation *anim, double delta_ms);

#endif
