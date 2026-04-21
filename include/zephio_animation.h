/**
 * @file zephio_animation.h
 * @brief Core animation types, easing functions, and animation state.
 *
 * Provides time-based animations with configurable easing curves.
 * Each animation has a duration, easing function, and user callbacks
 * for per-frame updates and completion.
 *
 * Usage:
 *   1. Create via zephio_animator_create() on a ZephioAnimator.
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
    ZEPHIO_EASE_LINEAR,
    ZEPHIO_EASE_IN_QUAD,
    ZEPHIO_EASE_OUT_QUAD,
    ZEPHIO_EASE_IN_OUT_QUAD,
    ZEPHIO_EASE_IN_CUBIC,
    ZEPHIO_EASE_OUT_CUBIC,
    ZEPHIO_EASE_IN_OUT_CUBIC,
    ZEPHIO_EASE_IN_SINE,
    ZEPHIO_EASE_OUT_SINE,
    ZEPHIO_EASE_IN_OUT_SINE,
    ZEPHIO_EASE_IN_EXPO,
    ZEPHIO_EASE_OUT_EXPO,
    ZEPHIO_EASE_IN_OUT_EXPO
} ZephioEasing;

/**
 * @brief Animation playback state.
 */
typedef enum {
    ZEPHIO_ANIM_IDLE,
    ZEPHIO_ANIM_PLAYING,
    ZEPHIO_ANIM_PAUSED,
    ZEPHIO_ANIM_COMPLETED
} ZephioAnimState;

/**
 * @brief Called every tick with the current eased progress.
 *
 * @param progress   Eased progress in [0.0, 1.0].
 * @param user_data  User-provided context.
 */
typedef void (*ZephioAnimUpdateFn)(double progress, void *user_data);

/**
 * @brief Called once when the animation completes (all loops finished).
 *
 * @param user_data  User-provided context.
 */
typedef void (*ZephioAnimCompleteFn)(void *user_data);

/**
 * @brief A single animation instance.
 *
 * Managed by ZephioAnimator. Create via zephio_animator_create(), control
 * via play/pause/stop.
 */
typedef struct ZephioAnimation {
    int              id;
    ZephioAnimState     state;
    ZephioEasing        easing;
    double           duration_ms;
    double           elapsed_ms;
    int              loops;
    int              loops_remaining;
    ZephioAnimUpdateFn  on_update;
    ZephioAnimCompleteFn on_complete;
    void            *user_data;
} ZephioAnimation;

/**
 * @brief Apply an easing function to a normalized time value.
 *
 * @param easing  The easing curve to apply.
 * @param t       Normalized time in [0.0, 1.0]. Values outside this range
 *                are clamped.
 * @return The eased progress value in [0.0, 1.0].
 */
double zephio_ease(ZephioEasing easing, double t);

/**
 * @brief Initialize an animation struct.
 *
 * Typically called internally by ZephioAnimator. Can be used directly
 * for standalone animations.
 *
 * @param anim         Animation to initialize.
 * @param duration_ms  Duration in milliseconds (> 0).
 * @param easing       Easing curve.
 * @param on_update    Per-frame callback (may be NULL).
 * @param on_complete  Completion callback (may be NULL).
 * @param user_data    User context passed to callbacks.
 */
void zephio_animation_init(ZephioAnimation *anim, double duration_ms, ZephioEasing easing,
                        ZephioAnimUpdateFn on_update, ZephioAnimCompleteFn on_complete,
                        void *user_data);

/**
 * @brief Set the number of loops.
 *
 * @param anim   Animation.
 * @param loops  0 = play once (no repeat), >0 = loop that many additional
 *               times, -1 = loop forever.
 */
void zephio_animation_set_loops(ZephioAnimation *anim, int loops);

/**
 * @brief Get the current eased progress of an animation.
 *
 * @param anim  Animation.
 * @return Eased progress in [0.0, 1.0], or 0.0 if not playing.
 */
double zephio_animation_get_progress(const ZephioAnimation *anim);

/**
 * @brief Start or resume an animation.
 */
void zephio_animation_play(ZephioAnimation *anim);

/**
 * @brief Pause a playing animation.
 */
void zephio_animation_pause(ZephioAnimation *anim);

/**
 * @brief Stop and reset an animation to the beginning.
 */
void zephio_animation_stop(ZephioAnimation *anim);

/**
 * @brief Advance an animation by delta milliseconds.
 *
 * Called internally by ZephioAnimator. Calls on_update with the current
 * eased progress. Handles looping and completion.
 *
 * @param anim       Animation.
 * @param delta_ms   Elapsed time since last update in milliseconds.
 */
void zephio_animation_update(ZephioAnimation *anim, double delta_ms);

#endif
