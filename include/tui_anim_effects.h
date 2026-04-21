/**
 * @file tui_anim_effects.h
 * @brief Reusable animation effects: slide, fade, spinner, pulse.
 *
 * Provides convenience functions that create TuiAnimation instances
 * via the TuiAnimator. Each effect sets up the appropriate easing,
 * duration, and update callback to animate widget properties such as
 * position (slide), opacity/color (fade), or content (spinner/pulse).
 *
 * Usage:
 *   TuiAnimator *anim = tui_app_get_animator(app);
 *   int id = tui_effect_fade_in(anim, &my_widget, 300);
 *   tui_animator_play(anim, id);
 */

#ifndef ZEPHIO_ANIM_EFFECTS_H
#define ZEPHIO_ANIM_EFFECTS_H

#include "tui_animator.h"
#include "tui_widget.h"
#include "tui_screen.h"

/**
 * @brief Direction for slide animations.
 */
typedef enum {
    TUI_SLIDE_FROM_LEFT,
    TUI_SLIDE_FROM_RIGHT,
    TUI_SLIDE_FROM_TOP,
    TUI_SLIDE_FROM_BOTTOM
} TuiSlideDirection;

/**
 * @brief Callback type for fade completion.
 *
 * @param widget   The animated widget.
 * @param user_data  User context.
 */
typedef void (*TuiEffectDoneFn)(TuiWidget *widget, void *user_data);

/**
 * @brief Slide a widget in from a direction.
 *
 * Moves the widget from off-screen to its current (x, y) position
 * using TUI_EASE_OUT_QUAD easing.
 *
 * @param animator  Animator instance.
 * @param widget    Widget to animate (its x/y must be the final position).
 * @param dir       Slide direction.
 * @param duration_ms  Animation duration (ms). 0 defaults to 300.
 * @return Animation ID, or TUI_ANIMATOR_INVALID_ID on failure.
 */
int tui_effect_slide_in(TuiAnimator *animator, TuiWidget *widget,
                        TuiSlideDirection dir, double duration_ms);

/**
 * @brief Slide a widget out towards a direction.
 *
 * Moves the widget from its current (x, y) position to off-screen
 * using TUI_EASE_IN_QUAD easing.
 *
 * @param animator  Animator instance.
 * @param widget    Widget to animate.
 * @param dir       Slide direction.
 * @param duration_ms  Animation duration (ms). 0 defaults to 300.
 * @param on_done   Optional callback when complete.
 * @param user_data  User context for on_done.
 * @return Animation ID, or TUI_ANIMATOR_INVALID_ID on failure.
 */
int tui_effect_slide_out(TuiAnimator *animator, TuiWidget *widget,
                         TuiSlideDirection dir, double duration_ms,
                         TuiEffectDoneFn on_done, void *user_data);

/**
 * @brief Fade a widget in by interpolating background color alpha.
 *
 * Uses TUI_EASE_OUT_SINE easing. The widget becomes progressively
 * more visible as the animation progresses.
 *
 * @param animator  Animator instance.
 * @param widget    Widget to animate.
 * @param duration_ms  Animation duration (ms). 0 defaults to 250.
 * @return Animation ID, or TUI_ANIMATOR_INVALID_ID on failure.
 */
int tui_effect_fade_in(TuiAnimator *animator, TuiWidget *widget,
                       double duration_ms);

/**
 * @brief Fade a widget out.
 *
 * Uses TUI_EASE_IN_SINE easing. The widget becomes progressively
 * less visible.
 *
 * @param animator  Animator instance.
 * @param widget    Widget to animate.
 * @param duration_ms  Animation duration (ms). 0 defaults to 250.
 * @param on_done   Optional callback when complete.
 * @param user_data  User context for on_done.
 * @return Animation ID, or TUI_ANIMATOR_INVALID_ID on failure.
 */
int tui_effect_fade_out(TuiAnimator *animator, TuiWidget *widget,
                        double duration_ms,
                        TuiEffectDoneFn on_done, void *user_data);

/**
 * @brief Spinner state — rotating character animation.
 *
 * Attach to a label or render manually. The spinner cycles through
 * characters: | / - \
 */
typedef struct {
    TuiWidget *widget;
    int        anim_id;
    int        frame;
    TuiColor   fg;
    TuiColor   bg;
    char       base_text[64];
} TuiSpinner;

/**
 * @brief Initialize a spinner attached to a widget.
 *
 * @param spinner   Spinner struct to initialize.
 * @param widget    Widget (typically a TuiLabel) to update each frame.
 * @param base_text Prefix text shown before the spinner character (e.g. "Loading ").
 * @param fg        Foreground color for the spinner character.
 * @param bg        Background color.
 */
void tui_spinner_init(TuiSpinner *spinner, TuiWidget *widget,
                      const char *base_text, TuiColor fg, TuiColor bg);

/**
 * @brief Start the spinner animation.
 *
 * @param animator  Animator instance.
 * @param spinner   Spinner to start.
 * @param interval_ms  Time per frame (ms). 0 defaults to 120.
 */
void tui_spinner_start(TuiAnimator *animator, TuiSpinner *spinner,
                       double interval_ms);

/**
 * @brief Stop the spinner animation.
 *
 * @param animator  Animator instance.
 * @param spinner   Spinner to stop.
 */
void tui_spinner_stop(TuiAnimator *animator, TuiSpinner *spinner);

/**
 * @brief Pulse state — oscillating progress bar animation.
 */
typedef struct {
    TuiWidget *widget;
    int        anim_id;
    double     position;
    double     speed;
    TuiColor   fill_fg;
    TuiColor   fill_bg;
    TuiColor   empty_fg;
    TuiColor   empty_bg;
    int        bar_width;
} TuiPulse;

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
void tui_pulse_init(TuiPulse *pulse, TuiWidget *widget, int width,
                    TuiColor fill_fg, TuiColor fill_bg,
                    TuiColor empty_fg, TuiColor empty_bg);

/**
 * @brief Start the pulse animation.
 *
 * The bar oscillates back and forth continuously.
 *
 * @param animator  Animator instance.
 * @param pulse     Pulse to start.
 * @param speed     Speed factor (1.0 = normal). 0 defaults to 1.0.
 */
void tui_pulse_start(TuiAnimator *animator, TuiPulse *pulse, double speed);

/**
 * @brief Stop the pulse animation.
 *
 * @param animator  Animator instance.
 * @param pulse     Pulse to stop.
 */
void tui_pulse_stop(TuiAnimator *animator, TuiPulse *pulse);

/**
 * @brief Render the pulse bar to the screen.
 *
 * Must be called AFTER tui_widget_render and BEFORE tui_screen_render
 * in the render callback. Draws the pulse bar at the widget's absolute
 * position.
 *
 * @param pulse  Pulse to render.
 */
void tui_pulse_render(TuiPulse *pulse);

/**
 * @brief Color fade state — interpolates between two colors.
 */
typedef struct {
    TuiWidget *widget;
    int        anim_id;
    TuiColor   from_fg;
    TuiColor   from_bg;
    TuiColor   to_fg;
    TuiColor   to_bg;
} TuiColorFade;

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
int tui_effect_color_fade(TuiAnimator *animator, TuiColorFade *fade,
                          TuiWidget *widget,
                          TuiColor from_fg, TuiColor from_bg,
                          TuiColor to_fg, TuiColor to_bg,
                          double duration_ms);

/**
 * @brief Interpolate between two TuiColor values.
 *
 * Only works with TUI_COLOR_TYPE_RGB colors. Falls back to `to`
 * for indexed colors.
 *
 * @param from  Start color.
 * @param to    End color.
 * @param t     Interpolation factor [0.0, 1.0].
 * @return Interpolated color.
 */
TuiColor tui_color_lerp(TuiColor from, TuiColor to, double t);

#endif
