/**
 * @file animation_demo.c
 * @brief Animation effects showcase — slide, fade, spinner, pulse.
 *
 * Demonstrates:
 *   - Slide-in / Slide-out panels from all 4 directions
 *   - Fade-in / Fade-out overlay
 *   - Spinner (loading animation)
 *   - Pulse bar (indeterminate progress)
 *   - Color fade on a label
 *
 * Keys:
 *   1 — Slide panel from left
 *   2 — Slide panel from right
 *   3 — Slide panel from top
 *   4 — Slide panel from bottom
 *   5 — Fade overlay in/out
 *   s — Toggle spinner
 *   p — Toggle pulse bar
 *   c — Trigger color fade on status label
 *   r — Reset all
 *   q/Esc — quit
 */

#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "tui_anim_effects.h"
#include "tui_animator.h"
#include "tui_app.h"
#include "tui_button.h"
#include "tui_container.h"
#include "tui_label.h"
#include "tui_screen.h"
#include "tui_widget.h"
#include "tui_context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    TuiWidget  root;

    TuiLabel   title;
    TuiLabel   section_slide;
    TuiLabel   section_loading;
    TuiLabel   hint;
    TuiLabel   status;

    TuiLabel   spinner_label;
    TuiLabel   pulse_label;
    TuiLabel   pulse_bar;

    TuiContainer slide_panel;
    TuiLabel     slide_panel_title;
    TuiLabel     slide_panel_hint;

    TuiContainer fade_panel;

    TuiContainer color_block;
    TuiLabel     color_block_text;

    TuiSpinner spinner;
    TuiPulse   pulse;

    int  color_fade_anim_id;

    int  spinner_active;
    int  pulse_active;
    int  slide_anim_id;
    int  fade_anim_id;
    int  panel_visible;
    int  fade_panel_visible;
    int  color_fade_state;

    int  panel_home_x;
    int  panel_home_y;

    char status_text[128];
} AppWidgets;

static TuiApp *g_app = NULL;

static void update_status(AppWidgets *w, const char *msg)
{
    snprintf(w->status_text, sizeof(w->status_text), " %s", msg);
    tui_label_set_text(&w->status, w->status_text);
    tui_label_set_colors(&w->status,
                         ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236));
}

static void build_widgets(AppWidgets *w, TuiContext *ctx, int rows, int cols)
{
    tui_widget_init_ctx(&w->root, 0, 0, cols, rows, NULL, ctx, NULL);

    tui_label_init_ctx(&w->title, ctx, 2, 1, cols - 4, 1,
                   "Animation Demo  |  1-4: slide  5: fade  s: spinner  p: pulse  c: color");
    tui_label_set_colors(&w->title, ZEPHIO_COLOR_INDEX(14), ZEPHIO_COLOR_INDEX(0));
    tui_label_set_attr(&w->title, ZEPHIO_ATTR_BOLD);
    tui_widget_add_child(&w->root, &w->title.base);

    tui_label_init_ctx(&w->section_slide, ctx, 2, 3, 30, 1,
                       "Slide Effects (keys 1-4):");
    tui_label_set_colors(&w->section_slide,
                         ZEPHIO_COLOR_INDEX(12), ZEPHIO_COLOR_INDEX(0));
    tui_label_set_attr(&w->section_slide, ZEPHIO_ATTR_BOLD);
    tui_widget_add_child(&w->root, &w->section_slide.base);

    tui_label_init_ctx(&w->section_loading, ctx, 2, 5, 30, 1,
                        "Loading Animations (s/p/c):");
    tui_label_set_colors(&w->section_loading,
                         ZEPHIO_COLOR_INDEX(12), ZEPHIO_COLOR_INDEX(0));
    tui_label_set_attr(&w->section_loading, ZEPHIO_ATTR_BOLD);
    tui_widget_add_child(&w->root, &w->section_loading.base);

    tui_label_init_ctx(&w->spinner_label, ctx, 4, 6, 20, 1, "Spinner: stopped");
    tui_label_set_colors(&w->spinner_label,
                         ZEPHIO_COLOR_INDEX(7), ZEPHIO_COLOR_INDEX(0));
    tui_widget_add_child(&w->root, &w->spinner_label.base);

    tui_label_init_ctx(&w->pulse_label, ctx, 4, 7, 20, 1, "Pulse:   stopped");
    tui_label_set_colors(&w->pulse_label,
                         ZEPHIO_COLOR_INDEX(7), ZEPHIO_COLOR_INDEX(0));
    tui_widget_add_child(&w->root, &w->pulse_label.base);

    tui_label_init_ctx(&w->pulse_bar, ctx, 4, 8, 20, 1, "");
    tui_label_set_colors(&w->pulse_bar,
                         ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(234));
    tui_widget_add_child(&w->root, &w->pulse_bar.base);

    tui_label_init_ctx(&w->hint, ctx, 2, 10, cols - 4, 1,
                   "Press 1/2/3/4 to slide a panel in, it slides back out automatically.");
    tui_label_set_colors(&w->hint, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    tui_widget_add_child(&w->root, &w->hint.base);

    int panel_w = 30;
    int panel_h = 5;
    int panel_x = (cols - panel_w) / 2;
    int panel_y = (rows - panel_h) / 2;

    tui_container_init_ctx(&w->slide_panel, ctx, panel_x, panel_y, panel_w, panel_h);
    tui_container_set_bg(&w->slide_panel, ZEPHIO_COLOR_INDEX(236));
    w->slide_panel.base.visible = 0;
    w->panel_home_x = panel_x;
    w->panel_home_y = panel_y;
    tui_widget_add_child(&w->root, &w->slide_panel.base);

    tui_label_init_ctx(&w->slide_panel_title, ctx, 2, 1, panel_w - 4, 1,
                       "Sliding Panel — auto-dismiss");
    tui_label_set_colors(&w->slide_panel_title,
                         ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236));
    tui_label_set_attr(&w->slide_panel_title, ZEPHIO_ATTR_BOLD);
    tui_widget_add_child(&w->slide_panel.base, &w->slide_panel_title.base);

    tui_label_init_ctx(&w->slide_panel_hint, ctx, 2, 3, panel_w - 4, 1,
                       "Wait for auto-dismiss...");
    tui_label_set_colors(&w->slide_panel_hint,
                         ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(236));
    tui_widget_add_child(&w->slide_panel.base, &w->slide_panel_hint.base);

    int fade_w = 28;
    int fade_h = 4;
    int fade_x = (cols - fade_w) / 2;
    int fade_y = (rows - fade_h) / 2;

    tui_container_init_ctx(&w->fade_panel, ctx, fade_x, fade_y, fade_w, fade_h);
    tui_container_set_bg(&w->fade_panel, ZEPHIO_COLOR_INDEX(236));
    w->fade_panel.base.visible = 0;
    tui_widget_add_child(&w->root, &w->fade_panel.base);

    {
        int cb_w = 40;
        int cb_h = 3;
        int cb_x = (cols - cb_w) / 2;
        int cb_y = (rows - cb_h) / 2;
        if (cb_x < 1) cb_x = 1;

        tui_container_init_ctx(&w->color_block, ctx, cb_x, cb_y, cb_w, cb_h);
        tui_container_set_bg(&w->color_block, ZEPHIO_COLOR_RGB(28, 28, 28));
        w->color_block.base.visible = 0;
        tui_widget_add_child(&w->root, &w->color_block.base);

        tui_label_init_ctx(&w->color_block_text, ctx, 2, 1, cb_w - 4, 1,
                           "Press 'c' to animate this block");
        tui_label_set_colors(&w->color_block_text,
                             ZEPHIO_COLOR_RGB(210, 210, 210), ZEPHIO_COLOR_RGB(28, 28, 28));
        tui_widget_add_child(&w->color_block.base, &w->color_block_text.base);
    }

    tui_label_init_ctx(&w->status, ctx, 1, rows - 1, cols - 2, 1,
                       " Press a key to trigger an animation  |  q/Esc: quit");
    tui_label_set_colors(&w->status,
                         ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236));
    tui_widget_add_child(&w->root, &w->status.base);

    tui_spinner_init(&w->spinner, &w->spinner_label.base,
                     "Spinner: ", ZEPHIO_COLOR_INDEX(14), ZEPHIO_COLOR_INDEX(0));

    tui_pulse_init(&w->pulse, &w->pulse_bar.base, 20,
                   ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(12),
                   ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(234));

    w->color_fade_anim_id = ZEPHIO_ANIMATOR_INVALID_ID;
}

static void trigger_slide(AppWidgets *w, TuiSlideDirection dir, const char *name)
{
    TuiAnimator *animator = tui_app_get_animator(g_app);
    if (!animator) return;

    if (w->slide_anim_id != ZEPHIO_ANIMATOR_INVALID_ID) {
        tui_animator_remove(animator, w->slide_anim_id);
        w->slide_anim_id = ZEPHIO_ANIMATOR_INVALID_ID;
    }

    w->slide_panel.base.visible = 1;
    w->panel_visible = 1;

    tui_widget_set_position(&w->slide_panel.base,
                            w->panel_home_x, w->panel_home_y);

    char msg[80];
    snprintf(msg, sizeof(msg), "Sliding panel in from %s", name);
    update_status(w, msg);

    w->slide_anim_id = tui_effect_slide_in(animator,
                                           &w->slide_panel.base,
                                           dir, 400);
}

static void trigger_fade(AppWidgets *w)
{
    TuiAnimator *animator = tui_app_get_animator(g_app);
    if (!animator) return;

    if (w->fade_anim_id != ZEPHIO_ANIMATOR_INVALID_ID) {
        tui_animator_remove(animator, w->fade_anim_id);
        w->fade_anim_id = ZEPHIO_ANIMATOR_INVALID_ID;
    }

    if (!w->fade_panel_visible) {
        w->fade_panel.base.visible = 1;
        w->fade_panel_visible = 1;
        update_status(w, "Fading overlay in...");
        w->fade_anim_id = tui_effect_fade_in(animator,
                                             &w->fade_panel.base, 400);
    } else {
        update_status(w, "Fading overlay out...");
        w->fade_anim_id = tui_effect_fade_out(animator,
                                              &w->fade_panel.base, 300,
                                              NULL, NULL);
        w->fade_panel_visible = 0;
    }
}

static void toggle_spinner(AppWidgets *w)
{
    TuiAnimator *animator = tui_app_get_animator(g_app);
    if (!animator) return;

    if (w->spinner_active) {
        tui_spinner_stop(animator, &w->spinner);
        w->spinner_active = 0;
        tui_label_set_text(&w->spinner_label, "Spinner: stopped");
        tui_label_set_colors(&w->spinner_label,
                             ZEPHIO_COLOR_INDEX(7), ZEPHIO_COLOR_INDEX(0));
        update_status(w, "Spinner stopped");
    } else {
        tui_spinner_start(animator, &w->spinner, 120);
        w->spinner_active = 1;
        tui_label_set_colors(&w->spinner_label,
                             ZEPHIO_COLOR_INDEX(14), ZEPHIO_COLOR_INDEX(0));
        update_status(w, "Spinner started");
    }
}

static void toggle_pulse(AppWidgets *w)
{
    TuiAnimator *animator = tui_app_get_animator(g_app);
    if (!animator) return;

    if (w->pulse_active) {
        tui_pulse_stop(animator, &w->pulse);
        w->pulse_active = 0;
        tui_label_set_text(&w->pulse_label, "Pulse:   stopped");
        tui_label_set_colors(&w->pulse_label,
                             ZEPHIO_COLOR_INDEX(7), ZEPHIO_COLOR_INDEX(0));
        update_status(w, "Pulse stopped");
    } else {
        tui_pulse_start(animator, &w->pulse, 1.0);
        w->pulse_active = 1;
        tui_label_set_text(&w->pulse_label, "Pulse:   ");
        tui_label_set_colors(&w->pulse_label,
                             ZEPHIO_COLOR_INDEX(12), ZEPHIO_COLOR_INDEX(0));
        update_status(w, "Pulse started");
    }
}

typedef struct {
    TuiContainer *block;
    TuiLabel     *label;
    TuiColor      from_bg;
    TuiColor      to_bg;
    TuiColor      from_fg;
    TuiColor      to_fg;
    int           hide_on_done;
} ColorBlockFade;

static void color_block_fade_update(double progress, void *user_data)
{
    ColorBlockFade *fd = (ColorBlockFade *)user_data;
    if (!fd) return;

    TuiColor bg = tui_color_lerp(fd->from_bg, fd->to_bg, progress);
    TuiColor fg = tui_color_lerp(fd->from_fg, fd->to_fg, progress);
    tui_container_set_bg(fd->block, bg);
    tui_label_set_colors(fd->label, fg, bg);
}

static void color_block_fade_complete(void *user_data)
{
    ColorBlockFade *fd = (ColorBlockFade *)user_data;
    if (!fd) return;
    if (fd->hide_on_done) {
        fd->block->base.visible = 0;
    }
    free(fd);
}

static void trigger_color_fade(AppWidgets *w)
{
    TuiAnimator *animator = tui_app_get_animator(g_app);
    if (!animator) return;

    if (w->color_fade_anim_id != ZEPHIO_ANIMATOR_INVALID_ID) {
        tui_animator_remove(animator, w->color_fade_anim_id);
        w->color_fade_anim_id = ZEPHIO_ANIMATOR_INVALID_ID;
    }

    w->color_block.base.visible = 1;

    if (w->color_fade_state == 0) {
        update_status(w, "Color fade: dark -> bright");
        ColorBlockFade *fd = (ColorBlockFade *)calloc(1, sizeof(ColorBlockFade));
        if (!fd) return;
        fd->block         = &w->color_block;
        fd->label         = &w->color_block_text;
        fd->from_bg       = ZEPHIO_COLOR_RGB(28, 28, 28);
        fd->to_bg         = ZEPHIO_COLOR_RGB(180, 40, 40);
        fd->from_fg       = ZEPHIO_COLOR_RGB(210, 210, 210);
        fd->to_fg         = ZEPHIO_COLOR_RGB(255, 255, 200);
        fd->hide_on_done  = 0;

        w->color_fade_anim_id = tui_animator_create(
            animator, 800, TUI_EASE_IN_OUT_SINE,
            color_block_fade_update, color_block_fade_complete, fd);
        if (w->color_fade_anim_id != ZEPHIO_ANIMATOR_INVALID_ID)
            tui_animator_play(animator, w->color_fade_anim_id);
        w->color_fade_state = 1;
    } else {
        update_status(w, "Color fade: bright -> dark");
        ColorBlockFade *fd = (ColorBlockFade *)calloc(1, sizeof(ColorBlockFade));
        if (!fd) return;
        fd->block         = &w->color_block;
        fd->label         = &w->color_block_text;
        fd->from_bg       = ZEPHIO_COLOR_RGB(180, 40, 40);
        fd->to_bg         = ZEPHIO_COLOR_RGB(28, 28, 28);
        fd->from_fg       = ZEPHIO_COLOR_RGB(255, 255, 200);
        fd->to_fg         = ZEPHIO_COLOR_RGB(210, 210, 210);
        fd->hide_on_done  = 0;

        w->color_fade_anim_id = tui_animator_create(
            animator, 800, TUI_EASE_IN_OUT_SINE,
            color_block_fade_update, color_block_fade_complete, fd);
        if (w->color_fade_anim_id != ZEPHIO_ANIMATOR_INVALID_ID)
            tui_animator_play(animator, w->color_fade_anim_id);
        w->color_fade_state = 0;
    }
}

static int on_init(TuiApp *app, void *user_data)
{
    g_app = app;
    AppWidgets *w = (AppWidgets *)user_data;
    memset(w, 0, sizeof(*w));

    w->slide_anim_id      = ZEPHIO_ANIMATOR_INVALID_ID;
    w->fade_anim_id       = ZEPHIO_ANIMATOR_INVALID_ID;
    w->color_fade_anim_id = ZEPHIO_ANIMATOR_INVALID_ID;

    TuiSize size = tui_screen_size(app->ctx);
    build_widgets(w, app->ctx, size.rows, size.cols);
    return 0;
}

static int on_resize(TuiApp *app, int rows, int cols, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;

    for (int i = w->root.child_count - 1; i >= 0; i--) {
        tui_widget_destroy(w->root.children[i]);
    }
    tui_widget_remove_all_children(&w->root);

    w->slide_anim_id      = ZEPHIO_ANIMATOR_INVALID_ID;
    w->fade_anim_id       = ZEPHIO_ANIMATOR_INVALID_ID;
    w->color_fade_anim_id = ZEPHIO_ANIMATOR_INVALID_ID;
    w->panel_visible        = 0;
    w->fade_panel_visible   = 0;
        w->spinner_active       = 0;
        w->pulse_active         = 0;

    build_widgets(w, app->ctx, rows, cols);
    return 0;
}

static int on_render(TuiApp *app, void *user_data)
{
    AppWidgets *w = (AppWidgets *)user_data;
    TuiSize size = tui_screen_size(app->ctx);

    tui_screen_clear(app->ctx);
    tui_screen_fill(app->ctx, 0, 0, size.cols, 1, " ",
                    ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    tui_screen_write(app->ctx, 0, 2,
                     "Animation Demo  |  Phase 21 Effects",
                     ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    tui_screen_fill(app->ctx, size.rows - 1, 0, size.cols, 1, " ",
                    ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236), ZEPHIO_ATTR_NONE);

    tui_widget_render(&w->root);
    tui_app_render_overlays(app);

    if (w->pulse_active) {
        tui_pulse_render(&w->pulse);
    }

    tui_screen_render(app->ctx);
    return 0;
}

static int on_input(TuiApp *app, const TuiEvent *event, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;

    if (event->key == TUI_KEY_ESCAPE || event->key == TUI_KEY_CTRL_C) {
        return 1;
    }

    if (event->codepoint == 'q' && event->modifiers == TUI_MOD_NONE) {
        return 1;
    }

    if (event->codepoint == '1') {
        trigger_slide(w, TUI_SLIDE_FROM_LEFT, "left");
        return 0;
    }
    if (event->codepoint == '2') {
        trigger_slide(w, TUI_SLIDE_FROM_RIGHT, "right");
        return 0;
    }
    if (event->codepoint == '3') {
        trigger_slide(w, TUI_SLIDE_FROM_TOP, "top");
        return 0;
    }
    if (event->codepoint == '4') {
        trigger_slide(w, TUI_SLIDE_FROM_BOTTOM, "bottom");
        return 0;
    }
    if (event->codepoint == '5') {
        trigger_fade(w);
        return 0;
    }
    if (event->codepoint == 's') {
        toggle_spinner(w);
        return 0;
    }
    if (event->codepoint == 'p') {
        toggle_pulse(w);
        return 0;
    }
    if (event->codepoint == 'c') {
        trigger_color_fade(w);
        return 0;
    }
    if (event->codepoint == 'r') {
        TuiAnimator *animator = tui_app_get_animator(g_app);
        if (animator) {
            tui_animator_stop_all(animator);
        }
        w->slide_anim_id      = ZEPHIO_ANIMATOR_INVALID_ID;
        w->fade_anim_id       = ZEPHIO_ANIMATOR_INVALID_ID;
        w->color_fade_anim_id = ZEPHIO_ANIMATOR_INVALID_ID;
        w->panel_visible        = 0;
        w->fade_panel_visible   = 0;
        w->spinner_active       = 0;
        w->pulse_active         = 0;
        w->color_block.base.visible = 0;
        update_status(w, "All animations reset");
        return 0;
    }

    return 0;
}

static void on_shutdown(TuiApp *app, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;
    for (int i = w->root.child_count - 1; i >= 0; i--) {
        tui_widget_destroy(w->root.children[i]);
    }
    tui_widget_remove_all_children(&w->root);
}

int main(void)
{
    TuiContext ctx;
    AppWidgets widgets;
    memset(&widgets, 0, sizeof(widgets));

    TuiAppConfig config = {
        .on_init     = on_init,
        .on_resize   = on_resize,
        .on_render   = on_render,
        .on_input    = on_input,
        .on_shutdown = on_shutdown,
        .user_data   = &widgets,
        .tick_rate_ms = 30
    };

    TuiApp *app = tui_app_new(&ctx, &config);
    if (!app) {
        fprintf(stderr, "Failed to create app\n");
        return 1;
    }

    int ret = tui_app_run(app);
    tui_app_free(app);
    return ret;
}
