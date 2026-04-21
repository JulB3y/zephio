/**
 * @file zephio_animator.h
 * @brief Animation manager that drives ZephioAnimation instances.
 *
 * ZephioAnimator maintains a pool of animations and advances them each
 * tick with delta-time. Integrate into the app loop via
 * zephio_app_get_animator() and the animator is auto-updated.
 *
 * Usage:
 *   1. ZephioAnimator *a = zephio_app_get_animator(app);
 *   2. int id = zephio_animator_create(a, 300, ZEPHIO_EASE_OUT_QUAD,
 *                                    my_update, my_done, ptr);
 *   3. zephio_animator_play(a, id);
 *   4. The app loop calls zephio_animator_update() automatically.
 */

#ifndef ZEPHIO_ANIMATOR_H
#define ZEPHIO_ANIMATOR_H

#include "zephio_animation.h"

#define ZEPHIO_ANIMATOR_MAX_ANIMATIONS 64
#define ZEPHIO_ANIMATOR_INVALID_ID     (-1)

/**
 * @brief Animation pool manager.
 *
 * Stores up to ZEPHIO_ANIMATOR_MAX_ANIMATIONS active animations.
 * Create with zephio_animator_new(), destroy with zephio_animator_free().
 */
typedef struct ZephioAnimator {
    ZephioAnimation animations[ZEPHIO_ANIMATOR_MAX_ANIMATIONS];
    int          count;
    int          next_id;
    int          active_count;
} ZephioAnimator;

/**
 * @brief Allocate and initialize a ZephioAnimator.
 *
 * @return Heap-allocated animator, or NULL on failure.
 */
ZephioAnimator *zephio_animator_new(void);

/**
 * @brief Free a ZephioAnimator allocated by zephio_animator_new().
 */
void zephio_animator_free(ZephioAnimator *animator);

/**
 * @brief Create a new animation and add it to the pool.
 *
 * The animation starts in ZEPHIO_ANIM_IDLE state. Call
 * zephio_animator_play() to start it.
 *
 * @param animator    Animator instance.
 * @param duration_ms Duration in milliseconds (> 0).
 * @param easing      Easing curve.
 * @param on_update   Per-frame callback (may be NULL).
 * @param on_complete Completion callback (may be NULL).
 * @param user_data   User context for callbacks.
 * @return Animation ID, or TUI_ANIMATOR_INVALID_ID on failure.
 */
int zephio_animator_create(ZephioAnimator *animator, double duration_ms,
                        ZephioEasing easing, ZephioAnimUpdateFn on_update,
                        ZephioAnimCompleteFn on_complete, void *user_data);

/**
 * @brief Remove an animation from the pool by ID.
 *
 * Safe to call with an invalid ID (no-op).
 *
 * @param animator  Animator instance.
 * @param id        Animation ID returned by zephio_animator_create().
 */
void zephio_animator_remove(ZephioAnimator *animator, int id);

/**
 * @brief Start or resume an animation.
 */
void zephio_animator_play(ZephioAnimator *animator, int id);

/**
 * @brief Pause a playing animation.
 */
void zephio_animator_pause(ZephioAnimator *animator, int id);

/**
 * @brief Stop and reset an animation to the beginning.
 */
void zephio_animator_stop(ZephioAnimator *animator, int id);

/**
 * @brief Advance all active animations by delta milliseconds.
 *
 * Called automatically by the app loop. Can be called manually for
 * standalone use.
 *
 * @param animator  Animator instance.
 * @param delta_ms  Elapsed time in milliseconds since last update.
 */
void zephio_animator_update(ZephioAnimator *animator, double delta_ms);

/**
 * @brief Check if any animation is currently playing.
 *
 * @return 1 if at least one animation is playing, 0 otherwise.
 */
int zephio_animator_is_active(const ZephioAnimator *animator);

/**
 * @brief Look up an animation by ID.
 *
 * @param animator  Animator instance.
 * @param id        Animation ID.
 * @return Pointer to the animation, or NULL if not found.
 */
ZephioAnimation *zephio_animator_get(ZephioAnimator *animator, int id);

/**
 * @brief Stop and remove all animations.
 */
void zephio_animator_stop_all(ZephioAnimator *animator);

#endif
