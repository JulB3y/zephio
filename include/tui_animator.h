/**
 * @file tui_animator.h
 * @brief Animation manager that drives TuiAnimation instances.
 *
 * TuiAnimator maintains a pool of animations and advances them each
 * tick with delta-time. Integrate into the app loop via
 * tui_app_get_animator() and the animator is auto-updated.
 *
 * Usage:
 *   1. TuiAnimator *a = tui_app_get_animator(app);
 *   2. int id = tui_animator_create(a, 300, TUI_EASE_OUT_QUAD,
 *                                    my_update, my_done, ptr);
 *   3. tui_animator_play(a, id);
 *   4. The app loop calls tui_animator_update() automatically.
 */

#ifndef TUI_ANIMATOR_H
#define TUI_ANIMATOR_H

#include "tui_animation.h"

#define TUI_ANIMATOR_MAX_ANIMATIONS 64
#define TUI_ANIMATOR_INVALID_ID     (-1)

/**
 * @brief Animation pool manager.
 *
 * Stores up to TUI_ANIMATOR_MAX_ANIMATIONS active animations.
 * Create with tui_animator_new(), destroy with tui_animator_free().
 */
typedef struct TuiAnimator {
    TuiAnimation animations[TUI_ANIMATOR_MAX_ANIMATIONS];
    int          count;
    int          next_id;
    int          active_count;
} TuiAnimator;

/**
 * @brief Allocate and initialize a TuiAnimator.
 *
 * @return Heap-allocated animator, or NULL on failure.
 */
TuiAnimator *tui_animator_new(void);

/**
 * @brief Free a TuiAnimator allocated by tui_animator_new().
 */
void tui_animator_free(TuiAnimator *animator);

/**
 * @brief Create a new animation and add it to the pool.
 *
 * The animation starts in TUI_ANIM_IDLE state. Call
 * tui_animator_play() to start it.
 *
 * @param animator    Animator instance.
 * @param duration_ms Duration in milliseconds (> 0).
 * @param easing      Easing curve.
 * @param on_update   Per-frame callback (may be NULL).
 * @param on_complete Completion callback (may be NULL).
 * @param user_data   User context for callbacks.
 * @return Animation ID, or TUI_ANIMATOR_INVALID_ID on failure.
 */
int tui_animator_create(TuiAnimator *animator, double duration_ms,
                        TuiEasing easing, TuiAnimUpdateFn on_update,
                        TuiAnimCompleteFn on_complete, void *user_data);

/**
 * @brief Remove an animation from the pool by ID.
 *
 * Safe to call with an invalid ID (no-op).
 *
 * @param animator  Animator instance.
 * @param id        Animation ID returned by tui_animator_create().
 */
void tui_animator_remove(TuiAnimator *animator, int id);

/**
 * @brief Start or resume an animation.
 */
void tui_animator_play(TuiAnimator *animator, int id);

/**
 * @brief Pause a playing animation.
 */
void tui_animator_pause(TuiAnimator *animator, int id);

/**
 * @brief Stop and reset an animation to the beginning.
 */
void tui_animator_stop(TuiAnimator *animator, int id);

/**
 * @brief Advance all active animations by delta milliseconds.
 *
 * Called automatically by the app loop. Can be called manually for
 * standalone use.
 *
 * @param animator  Animator instance.
 * @param delta_ms  Elapsed time in milliseconds since last update.
 */
void tui_animator_update(TuiAnimator *animator, double delta_ms);

/**
 * @brief Check if any animation is currently playing.
 *
 * @return 1 if at least one animation is playing, 0 otherwise.
 */
int tui_animator_is_active(const TuiAnimator *animator);

/**
 * @brief Look up an animation by ID.
 *
 * @param animator  Animator instance.
 * @param id        Animation ID.
 * @return Pointer to the animation, or NULL if not found.
 */
TuiAnimation *tui_animator_get(TuiAnimator *animator, int id);

/**
 * @brief Stop and remove all animations.
 */
void tui_animator_stop_all(TuiAnimator *animator);

#endif
