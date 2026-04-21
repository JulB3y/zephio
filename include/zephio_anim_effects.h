/**
 * @file zephio_anim_effects.h
 * @brief Reusable animation effects: slide, fade, spinner, pulse.
 *
 * Provides convenience functions that create ZephioAnimation instances
 * via the ZephioAnimator. Each effect sets up the appropriate easing,
 * duration, and update callback to animate widget properties such as
 * position (slide), opacity/color (fade), or content (spinner/pulse).
 *
 * Usage:
 *   ZephioAnimator *anim = zephio_app_get_animator(app);
 *   int id = zephio_effect_fade_in(anim, &my_widget, 300);
 *   zephio_animator_play(anim, id);
 */

#ifndef ZEPHIO_ANIM_EFFECTS_H
#define ZEPHIO_ANIM_EFFECTS_H

#include "zephio_animator.h"
#include "zephio_widget.h"
#include "zephio_screen.h"

/**
 * @brief Direction for slide animations.
 */
typedef enum {
    TUI_SLIDE_FROM_LEFT,
    TUI_SLIDE_FROM_RIGHT,
    TUI_SLIDE_FROM_TOP,
    TUI_SLIDE_FROM_BOTTOM
} ZephioSlideDirection;

/**
 * @brief Callback type for fade completion.
 *
 * @param widget   The animated widget.
 * @param user_data  User context.
 */
typedef void (*ZephioEffectDoneFn)(ZephioWidget *widget, void *user_data);

/**
 * @brief Slide a widget in from a direction.
 *
 * Moves the widget from off-screen to its current (x, y) position
 * using ZEPHIO_EASE_OUT_QUAD easing.
 *
 * @param animator  Animator instance.
 * @param widget    Widget to animate (its x/y must be the final position).
 * @param dir       Slide direction.
 * @param duration_ms  Animation duration (ms). 0 defaults to 300.
 * @return Animation ID, or TUI_ANIMATOR_INVALID_ID on failure.
 */
int zephio_effect_slide_in(ZephioAnimator *animator, ZephioWidget *widget,
                        ZephioSlideDirection dir, double duration_ms);

/**
 * @brief Slide a widget out towards a direction.
 *
 * Moves the widget from its current (x, y) position to off-screen
 * using ZEPHIO_EASE_IN_QUAD easing.
 *
 * @param animator  Animator instance.
 * @param widget    Widget to animate.
 * @param dir       Slide direction.
 * @param duration_ms  Animation duration (ms). 0 defaults to 300.
 * @param on_done   Optional callback when complete.
 * @param user_data  User context for on_done.
 * @return Animation ID, or TUI_ANIMATOR_INVALID_ID on failure.
 */
int zephio_effect_slide_out(ZephioAnimator *animator, ZephioWidget *widget,
                         ZephioSlideDirection dir, double duration_ms,
                         ZephioEffectDoneFn on_done, void *user_data);

/**
 * @brief Fade a widget in by interpolating background color alpha.
 *
 * Uses ZEPHIO_EASE_OUT_SINE easing. The widget becomes progressively
 * more visible as the animation progresses.
 *
 * @param animator  Animator instance.
 * @param widget    Widget to animate.
 * @param duration_ms  Animation duration (ms). 0 defaults to 250.
 * @return Animation ID, or TUI_ANIMATOR_INVALID_ID on failure.
 */
int zephio_effect_fade_in(ZephioAnimator *animator, ZephioWidget *widget,
                       double duration_ms);

/**
 * @brief Fade a widget out.
 *
 * Uses ZEPHIO_EASE_IN_SINE easing. The widget becomes progressively
 * less visible.
 *
 * @param animator  Animator instance.
 * @param widget    Widget to animate.
 * @param duration_ms  Animation duration (ms). 0 defaults to 250.
 * @param on_done   Optional callback when complete.
 * @param user_data  User context for on_done.
 * @return Animation ID, or TUI_ANIMATOR_INVALID_ID on failure.
 */
int zephio_effect_fade_out(ZephioAnimator *animator, ZephioWidget *widget,
                        double duration_ms,
                        ZephioEffectDoneFn on_done, void *user_data);

/**
 * @brief Spinner state — rotating character animation.
 *
 * Attach to a label or render manually. The spinner cycles through
 * characters: | / - \
 */
typedef struct {
    ZephioWidget *widget;
    int        anim_id;
    int        frame;
    ZephioColor   fg;
    ZephioColor   bg;
    char       base_text[64];
} ZephioSpinner;

/**
 * @brief Initialize a spinner attached to a widget.
 *
 * @param spinner   Spinner struct to initialize.
 * @param widget    Widget (typically a ZephioLabel) to update each frame.
 * @param base_text Prefix text shown before the spinner character (e.g. "Loading ").
 * @param fg        Foreground color for the spinner character.
 * @param bg        Background color.
 */
void zephio_spinner_init(ZephioSpinner *spinner, ZephioWidget *widget,
                      const char *base_text, ZephioColor fg, ZephioColor bg);

/**
 * @brief Start the spinner animation.
 *
 * @param animator  Animator instance.
 * @param spinner   Spinner to start.
 * @param interval_ms  Time per frame (ms). 0 defaults to 120.
 */
void zephio_spinner_start(ZephioAnimator *animator, ZephioSpinner *spinner,
                       double interval_ms);

/**
 * @brief Stop the spinner animation.
 *
 * @param animator  Animator instance.
 * @param spinner   Spinner to stop.
 */
void zephio_spinner_stop(ZephioAnimator *animator, ZephioSpinner *spinner);

/**
 * @brief Pulse state — oscillating progress bar animation.
 */
typedef struct {
    ZephioWidget *widget;
    int        anim_id;
    double     position;
    double     speed;
    ZephioColor   fill_fg;
    ZephioColor   fill_bg;
    ZephioColor   empty_fg;
    ZephioColor   empty_bg;
    int        bar_width;
} ZephioPulse;

/**
 * @brief Initialize a pulse animation.
 *
 * @param pulse    Pulse struct to initialize.
 * @param widget   Widget to render the pulse bar into.
 * @param width    Width of the pulse bar in columns.
 * @param fill_fg  Foreground color for the filled portion.
 * @param fill_bg  Background color for the filled portion.
 * @param empty_fg Foreground color for the empty portion.
 * @param empty_bg Background color for the empty portion.
 */
void zephio_pulse_init(ZephioPulse *pulse, ZephioWidget *widget, int width,
                    ZephioColor fill_fg, ZephioColor fill_bg,
                    ZephioColor empty_fg, ZephioColor empty_bg);

/**
 * @brief Start the pulse animation.
 *
 * The bar oscillates back and forth continuously.
 *
 * @param animator  Animator instance.
 * @param pulse     Pulse to start.
 * @param speed     Speed factor (1.0 = normal). 0 defaults to 1.0.
 */
void zephio_pulse_start(ZephioAnimator *animator, ZephioPulse *pulse, double speed);

/**
 * @brief Stop the pulse animation.
 *
 * @param animator  Animator instance.
 * @param pulse     Pulse to stop.
 */
void zephio_pulse_stop(ZephioAnimator *animator, ZephioPulse *pulse);

/**
 * @brief Render the pulse bar to the screen.
 *
 * Must be called AFTER zephio_widget_render and BEFORE zephio_screen_render
 * in the render callback. Draws the pulse bar at the widget's absolute
 * position.
 *
 * @param pulse  Pulse to render.
 */
void zephio_pulse_render(ZephioPulse *pulse);

/**
 * @brief Color fade state — interpolates between two colors.
 */
typedef struct {
    ZephioWidget *widget;
    int        anim_id;
    ZephioColor   from_fg;
    ZephioColor   from_bg;
    ZephioColor   to_fg;
    ZephioColor   to_bg;
} ZephioColorFade;

/**
 * @brief Create a color-fade animation.
 *
 * Interpolates the widget's visual appearance from (from_fg, from_bg)
 * to (to_fg, to_bg) over the specified duration.
 *
 * @param animator  Animator instance.
 * @param fade      ColorFade struct (caller-managed).
 * @param widget    Widget to animate.
 * @param from_fg   Starting foreground color.
 * @param from_bg   Starting background color.
 * @param to_fg     Ending foreground color.
 * @param to_bg     Ending background color.
 * @param duration_ms  Duration (ms). 0 defaults to 400.
 * @return Animation ID, or TUI_ANIMATOR_INVALID_ID on failure.
 */
int zephio_effect_color_fade(ZephioAnimator *animator, ZephioColorFade *fade,
                          ZephioWidget *widget,
                          ZephioColor from_fg, ZephioColor from_bg,
                          ZephioColor to_fg, ZephioColor to_bg,
                          double duration_ms);

/**
 * @brief Interpolate between two ZephioColor values.
 *
 * Only works with ZEPHIO_COLOR_TYPE_RGB colors. Falls back to `to`
 * for indexed colors.
 *
 * @param from  Start color.
 * @param to    End color.
 * @param t     Interpolation factor [0.0, 1.0].
 * @return Interpolated color.
 */
ZephioColor zephio_color_lerp(ZephioColor from, ZephioColor to, double t);

#endif
