#define _POSIX_C_SOURCE 200809L

#include "zephio_anim_effects.h"
#include "zephio_context.h"
#include "zephio_container.h"
#include "zephio_label.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t lerp_u8(uint8_t a, uint8_t b, double t)
{
    double v = a + (b - a) * t;
    if (v < 0.0) v = 0.0;
    if (v > 255.0) v = 255.0;
    return (uint8_t)(v + 0.5);
}

ZephioColor zephio_color_lerp(ZephioColor from, ZephioColor to, double t)
{
    if (t <= 0.0) return from;
    if (t >= 1.0) return to;

    if (from.type == ZEPHIO_COLOR_TYPE_RGB && to.type == ZEPHIO_COLOR_TYPE_RGB) {
        return ZEPHIO_COLOR_RGB(
            lerp_u8(from.rgb.r, to.rgb.r, t),
            lerp_u8(from.rgb.g, to.rgb.g, t),
            lerp_u8(from.rgb.b, to.rgb.b, t)
        );
    }

    if (from.type == ZEPHIO_COLOR_TYPE_INDEX && to.type == ZEPHIO_COLOR_TYPE_INDEX) {
        double idx = from.index + (to.index - from.index) * t;
        if (idx < 0.0) idx = 0.0;
        if (idx > 255.0) idx = 255.0;
        return ZEPHIO_COLOR_INDEX((uint8_t)(idx + 0.5));
    }

    return to;
}

typedef struct {
    ZephioWidget *widget;
    int        orig_x;
    int        orig_y;
    int        start_x;
    int        start_y;
} SlideData;

static void slide_in_update(double progress, void *user_data)
{
    SlideData *sd = (SlideData *)user_data;
    if (!sd || !sd->widget) return;

    int x = sd->start_x + (int)((sd->orig_x - sd->start_x) * progress);
    int y = sd->start_y + (int)((sd->orig_y - sd->start_y) * progress);
    zephio_widget_set_position(sd->widget, x, y);
}

static void slide_free_complete(void *user_data)
{
    free(user_data);
}

int zephio_effect_slide_in(ZephioAnimator *animator, ZephioWidget *widget,
                        ZephioSlideDirection dir, double duration_ms)
{
    if (!animator || !widget) return ZEPHIO_ANIMATOR_INVALID_ID;
    if (duration_ms <= 0.0) duration_ms = 300.0;

    SlideData *sd = (SlideData *)calloc(1, sizeof(SlideData));
    if (!sd) return ZEPHIO_ANIMATOR_INVALID_ID;

    sd->widget  = widget;
    sd->orig_x  = widget->x;
    sd->orig_y  = widget->y;

    switch (dir) {
    case TUI_SLIDE_FROM_LEFT:
        sd->start_x = -widget->width;
        sd->start_y = widget->y;
        break;
    case TUI_SLIDE_FROM_RIGHT:
        sd->start_x = widget->parent ? widget->parent->width + widget->width : 200;
        sd->start_y = widget->y;
        break;
    case TUI_SLIDE_FROM_TOP:
        sd->start_x = widget->x;
        sd->start_y = -widget->height;
        break;
    case TUI_SLIDE_FROM_BOTTOM:
        sd->start_y = widget->parent ? widget->parent->height + widget->height : 60;
        sd->start_x = widget->x;
        break;
    }

    zephio_widget_set_position(widget, sd->start_x, sd->start_y);

    int id = zephio_animator_create(animator, duration_ms, ZEPHIO_EASE_OUT_QUAD,
                                 slide_in_update, slide_free_complete, sd);
    if (id == ZEPHIO_ANIMATOR_INVALID_ID) {
        free(sd);
        return ZEPHIO_ANIMATOR_INVALID_ID;
    }

    zephio_animator_play(animator, id);
    return id;
}

typedef struct {
    ZephioWidget      *widget;
    int             orig_x;
    int             orig_y;
    int             target_x;
    int             target_y;
    ZephioEffectDoneFn on_done;
    void           *on_done_data;
} SlideOutData;

static void slide_out_update(double progress, void *user_data)
{
    SlideOutData *sd = (SlideOutData *)user_data;
    if (!sd || !sd->widget) return;

    int x = sd->orig_x + (int)((sd->target_x - sd->orig_x) * progress);
    int y = sd->orig_y + (int)((sd->target_y - sd->orig_y) * progress);
    zephio_widget_set_position(sd->widget, x, y);
}

static void slide_out_complete(void *user_data)
{
    SlideOutData *sd = (SlideOutData *)user_data;
    if (!sd) return;

    if (sd->on_done) {
        sd->on_done(sd->widget, sd->on_done_data);
    }
    free(sd);
}

int zephio_effect_slide_out(ZephioAnimator *animator, ZephioWidget *widget,
                         ZephioSlideDirection dir, double duration_ms,
                         ZephioEffectDoneFn on_done, void *user_data)
{
    if (!animator || !widget) return ZEPHIO_ANIMATOR_INVALID_ID;
    if (duration_ms <= 0.0) duration_ms = 300.0;

    SlideOutData *sd = (SlideOutData *)calloc(1, sizeof(SlideOutData));
    if (!sd) return ZEPHIO_ANIMATOR_INVALID_ID;

    sd->widget       = widget;
    sd->orig_x       = widget->x;
    sd->orig_y       = widget->y;
    sd->on_done      = on_done;
    sd->on_done_data = user_data;

    switch (dir) {
    case TUI_SLIDE_FROM_LEFT:
        sd->target_x = -widget->width;
        sd->target_y = widget->y;
        break;
    case TUI_SLIDE_FROM_RIGHT:
        sd->target_x = widget->parent ? widget->parent->width + widget->width : 200;
        sd->target_y = widget->y;
        break;
    case TUI_SLIDE_FROM_TOP:
        sd->target_x = widget->x;
        sd->target_y = -widget->height;
        break;
    case TUI_SLIDE_FROM_BOTTOM:
        sd->target_x = widget->x;
        sd->target_y = widget->parent ? widget->parent->height + widget->height : 60;
        break;
    }

    int id = zephio_animator_create(animator, duration_ms, ZEPHIO_EASE_IN_QUAD,
                                 slide_out_update, slide_out_complete, sd);
    if (id == ZEPHIO_ANIMATOR_INVALID_ID) {
        free(sd);
        return ZEPHIO_ANIMATOR_INVALID_ID;
    }

    zephio_animator_play(animator, id);
    return id;
}

typedef struct {
    ZephioWidget *widget;
    ZephioColor   target_bg;
    ZephioColor   black;
} FadeData;

static void fade_free_complete(void *user_data)
{
    free(user_data);
}

static void fade_in_update(double progress, void *user_data)
{
    FadeData *fd = (FadeData *)user_data;
    if (!fd || !fd->widget) return;

    ZephioColor bg = zephio_color_lerp(fd->black, fd->target_bg, progress);
    zephio_container_set_bg((ZephioContainer *)fd->widget, bg);
}

int zephio_effect_fade_in(ZephioAnimator *animator, ZephioWidget *widget,
                       double duration_ms)
{
    if (!animator || !widget) return ZEPHIO_ANIMATOR_INVALID_ID;
    if (duration_ms <= 0.0) duration_ms = 250.0;

    FadeData *fd = (FadeData *)calloc(1, sizeof(FadeData));
    if (!fd) return ZEPHIO_ANIMATOR_INVALID_ID;

    fd->widget    = widget;
    fd->target_bg = ZEPHIO_COLOR_RGB(30, 30, 46);
    fd->black     = ZEPHIO_COLOR_RGB(0, 0, 0);

    int id = zephio_animator_create(animator, duration_ms, ZEPHIO_EASE_OUT_SINE,
                                 fade_in_update,
                                 fade_free_complete, fd);
    if (id == ZEPHIO_ANIMATOR_INVALID_ID) {
        free(fd);
        return ZEPHIO_ANIMATOR_INVALID_ID;
    }

    zephio_animator_play(animator, id);
    return id;
}

typedef struct {
    ZephioWidget      *widget;
    ZephioColor        from_bg;
    ZephioColor        black;
    ZephioEffectDoneFn on_done;
    void           *on_done_data;
} FadeOutData;

static void fade_out_update(double progress, void *user_data)
{
    FadeOutData *fd = (FadeOutData *)user_data;
    if (!fd || !fd->widget) return;

    ZephioColor bg = zephio_color_lerp(fd->from_bg, fd->black, progress);
    zephio_container_set_bg((ZephioContainer *)fd->widget, bg);
}

static void fade_out_complete(void *user_data)
{
    FadeOutData *fd = (FadeOutData *)user_data;
    if (!fd) return;
    if (fd->on_done) {
        fd->on_done(fd->widget, fd->on_done_data);
    }
    free(fd);
}

int zephio_effect_fade_out(ZephioAnimator *animator, ZephioWidget *widget,
                        double duration_ms,
                        ZephioEffectDoneFn on_done, void *user_data)
{
    if (!animator || !widget) return ZEPHIO_ANIMATOR_INVALID_ID;
    if (duration_ms <= 0.0) duration_ms = 250.0;

    FadeOutData *fd = (FadeOutData *)calloc(1, sizeof(FadeOutData));
    if (!fd) return ZEPHIO_ANIMATOR_INVALID_ID;

    fd->widget       = widget;
    fd->from_bg      = ZEPHIO_COLOR_RGB(30, 30, 46);
    fd->black        = ZEPHIO_COLOR_RGB(0, 0, 0);
    fd->on_done      = on_done;
    fd->on_done_data = user_data;

    int id = zephio_animator_create(animator, duration_ms, ZEPHIO_EASE_IN_SINE,
                                 fade_out_update, fade_out_complete, fd);
    if (id == ZEPHIO_ANIMATOR_INVALID_ID) {
        free(fd);
        return ZEPHIO_ANIMATOR_INVALID_ID;
    }

    zephio_animator_play(animator, id);
    return id;
}

static const char SPINNER_CHARS[] = { '|', '/', '-', '\\' };

static void spinner_update(double progress, void *user_data)
{
    (void)progress;
    ZephioSpinner *spinner = (ZephioSpinner *)user_data;
    if (!spinner || !spinner->widget) return;

    spinner->frame = (spinner->frame + 1) % 4;
    char text[80];
    snprintf(text, sizeof(text), "%s%c", spinner->base_text,
             SPINNER_CHARS[spinner->frame]);
    zephio_label_set_text((ZephioLabel *)spinner->widget, text);
    zephio_label_set_colors((ZephioLabel *)spinner->widget, spinner->fg, spinner->bg);
}

void zephio_spinner_init(ZephioSpinner *spinner, ZephioWidget *widget,
                      const char *base_text, ZephioColor fg, ZephioColor bg)
{
    if (!spinner) return;
    spinner->widget  = widget;
    spinner->anim_id = ZEPHIO_ANIMATOR_INVALID_ID;
    spinner->frame   = 0;
    spinner->fg      = fg;
    spinner->bg      = bg;
    snprintf(spinner->base_text, sizeof(spinner->base_text), "%s",
             base_text ? base_text : "");
}

void zephio_spinner_start(ZephioAnimator *animator, ZephioSpinner *spinner,
                       double interval_ms)
{
    if (!animator || !spinner) return;
    if (interval_ms <= 0.0) interval_ms = 120.0;

    if (spinner->anim_id != ZEPHIO_ANIMATOR_INVALID_ID) {
        zephio_animator_remove(animator, spinner->anim_id);
    }

    spinner->anim_id = zephio_animator_create(animator, interval_ms,
                                           ZEPHIO_EASE_LINEAR,
                                           spinner_update, NULL, spinner);
    if (spinner->anim_id != ZEPHIO_ANIMATOR_INVALID_ID) {
        ZephioAnimation *anim = zephio_animator_get(animator, spinner->anim_id);
        if (anim) zephio_animation_set_loops(anim, -1);
        zephio_animator_play(animator, spinner->anim_id);
    }
}

void zephio_spinner_stop(ZephioAnimator *animator, ZephioSpinner *spinner)
{
    if (!animator || !spinner) return;
    if (spinner->anim_id != ZEPHIO_ANIMATOR_INVALID_ID) {
        zephio_animator_remove(animator, spinner->anim_id);
        spinner->anim_id = ZEPHIO_ANIMATOR_INVALID_ID;
    }
}

static void pulse_update(double progress, void *user_data)
{
    (void)progress;
    ZephioPulse *pulse = (ZephioPulse *)user_data;
    if (!pulse) return;

    pulse->position += pulse->speed * 0.03;
    if (pulse->position > 2.0) pulse->position = 0.0;
}

void zephio_pulse_render(ZephioPulse *pulse)
{
    if (!pulse || !pulse->widget) return;

    int abs_x = pulse->widget->abs_x;
    int abs_y = pulse->widget->abs_y;
    int bw    = pulse->bar_width;

    double pos = pulse->position <= 1.0 ? pulse->position : 2.0 - pulse->position;
    int head = (int)(pos * bw);
    int block_w = bw / 4;
    if (block_w < 2) block_w = 2;
    int tail = head - block_w;
    if (tail < 0) tail = 0;

    for (int i = 0; i < bw; i++) {
        ZephioColor fg, bg;
        if (i >= tail && i < head) {
            fg = pulse->fill_fg;
            bg = pulse->fill_bg;
        } else {
            fg = pulse->empty_fg;
            bg = pulse->empty_bg;
        }
        zephio_screen_set_cell(pulse->widget->ctx, abs_y, abs_x + i, " ", fg, bg, ZEPHIO_ATTR_NONE);
    }
}

void zephio_pulse_init(ZephioPulse *pulse, ZephioWidget *widget, int width,
                    ZephioColor fill_fg, ZephioColor fill_bg,
                    ZephioColor empty_fg, ZephioColor empty_bg)
{
    if (!pulse) return;
    pulse->widget   = widget;
    pulse->anim_id  = ZEPHIO_ANIMATOR_INVALID_ID;
    pulse->position = 0.0;
    pulse->speed    = 1.0;
    pulse->fill_fg  = fill_fg;
    pulse->fill_bg  = fill_bg;
    pulse->empty_fg = empty_fg;
    pulse->empty_bg = empty_bg;
    pulse->bar_width = width;
}

void zephio_pulse_start(ZephioAnimator *animator, ZephioPulse *pulse, double speed)
{
    if (!animator || !pulse) return;
    if (speed <= 0.0) speed = 1.0;

    pulse->speed = speed;

    if (pulse->anim_id != ZEPHIO_ANIMATOR_INVALID_ID) {
        zephio_animator_remove(animator, pulse->anim_id);
    }

    pulse->anim_id = zephio_animator_create(animator, 30.0,
                                         ZEPHIO_EASE_LINEAR,
                                         pulse_update, NULL, pulse);
    if (pulse->anim_id != ZEPHIO_ANIMATOR_INVALID_ID) {
        ZephioAnimation *anim = zephio_animator_get(animator, pulse->anim_id);
        if (anim) zephio_animation_set_loops(anim, -1);
        zephio_animator_play(animator, pulse->anim_id);
    }
}

void zephio_pulse_stop(ZephioAnimator *animator, ZephioPulse *pulse)
{
    if (!animator || !pulse) return;
    if (pulse->anim_id != ZEPHIO_ANIMATOR_INVALID_ID) {
        zephio_animator_remove(animator, pulse->anim_id);
        pulse->anim_id = ZEPHIO_ANIMATOR_INVALID_ID;
    }
}

typedef struct {
    ZephioWidget   *widget;
    ZephioColorFade *fade;
} ColorFadeData;

static void color_fade_update(double progress, void *user_data)
{
    ColorFadeData *cd = (ColorFadeData *)user_data;
    if (!cd || !cd->fade) return;

    ZephioColorFade *fade = cd->fade;
    ZephioColor fg = zephio_color_lerp(fade->from_fg, fade->to_fg, progress);
    ZephioColor bg = zephio_color_lerp(fade->from_bg, fade->to_bg, progress);

    ZephioLabel *label = (ZephioLabel *)fade->widget;
    zephio_label_set_colors(label, fg, bg);
}

static void color_fade_complete(void *user_data)
{
    free(user_data);
}

int zephio_effect_color_fade(ZephioAnimator *animator, ZephioColorFade *fade,
                          ZephioWidget *widget,
                          ZephioColor from_fg, ZephioColor from_bg,
                          ZephioColor to_fg, ZephioColor to_bg,
                          double duration_ms)
{
    if (!animator || !fade || !widget) return ZEPHIO_ANIMATOR_INVALID_ID;
    if (duration_ms <= 0.0) duration_ms = 400.0;

    fade->widget  = widget;
    fade->from_fg = from_fg;
    fade->from_bg = from_bg;
    fade->to_fg   = to_fg;
    fade->to_bg   = to_bg;

    ColorFadeData *cd = (ColorFadeData *)calloc(1, sizeof(ColorFadeData));
    if (!cd) return ZEPHIO_ANIMATOR_INVALID_ID;
    cd->widget = widget;
    cd->fade   = fade;

    int id = zephio_animator_create(animator, duration_ms, ZEPHIO_EASE_IN_OUT_SINE,
                                 color_fade_update, color_fade_complete, cd);
    if (id == ZEPHIO_ANIMATOR_INVALID_ID) {
        free(cd);
        fade->anim_id = ZEPHIO_ANIMATOR_INVALID_ID;
        return ZEPHIO_ANIMATOR_INVALID_ID;
    }

    zephio_label_set_colors((ZephioLabel *)widget, from_fg, from_bg);

    fade->anim_id = id;
    zephio_animator_play(animator, id);
    return id;
}
