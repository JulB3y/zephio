#define _POSIX_C_SOURCE 200809L

#include "zephio_animation.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double zephio_ease(ZephioEasing easing, double t)
{
    if (t <= 0.0) return 0.0;
    if (t >= 1.0) return 1.0;

    switch (easing) {
    case ZEPHIO_EASE_LINEAR:
        return t;

    case ZEPHIO_EASE_IN_QUAD:
        return t * t;

    case ZEPHIO_EASE_OUT_QUAD:
        return t * (2.0 - t);

    case ZEPHIO_EASE_IN_OUT_QUAD:
        if (t < 0.5)
            return 2.0 * t * t;
        return -1.0 + (4.0 - 2.0 * t) * t;

    case ZEPHIO_EASE_IN_CUBIC:
        return t * t * t;

    case ZEPHIO_EASE_OUT_CUBIC: {
        double t1 = t - 1.0;
        return t1 * t1 * t1 + 1.0;
    }

    case ZEPHIO_EASE_IN_OUT_CUBIC:
        if (t < 0.5)
            return 4.0 * t * t * t;
        else {
            double t1 = t - 1.0;
            return 1.0 + 4.0 * t1 * t1 * t1;
        }

    case ZEPHIO_EASE_IN_SINE:
        return 1.0 - cos(t * M_PI / 2.0);

    case ZEPHIO_EASE_OUT_SINE:
        return sin(t * M_PI / 2.0);

    case ZEPHIO_EASE_IN_OUT_SINE:
        return -(cos(M_PI * t) - 1.0) / 2.0;

    case ZEPHIO_EASE_IN_EXPO:
        if (t == 0.0) return 0.0;
        return pow(2.0, 10.0 * (t - 1.0));

    case ZEPHIO_EASE_OUT_EXPO:
        if (t >= 1.0) return 1.0;
        return 1.0 - pow(2.0, -10.0 * t);

    case ZEPHIO_EASE_IN_OUT_EXPO:
        if (t == 0.0) return 0.0;
        if (t >= 1.0) return 1.0;
        if (t < 0.5)
            return pow(2.0, 20.0 * t - 10.0) / 2.0;
        return (2.0 - pow(2.0, -20.0 * t + 10.0)) / 2.0;
    }

    return t;
}

void zephio_animation_init(ZephioAnimation *anim, double duration_ms, ZephioEasing easing,
                        ZephioAnimUpdateFn on_update, ZephioAnimCompleteFn on_complete,
                        void *user_data)
{
    if (!anim) return;
    if (duration_ms <= 0.0) duration_ms = 1.0;

    anim->id              = 0;
    anim->state           = ZEPHIO_ANIM_IDLE;
    anim->easing          = easing;
    anim->duration_ms     = duration_ms;
    anim->elapsed_ms      = 0.0;
    anim->loops           = 0;
    anim->loops_remaining = 0;
    anim->on_update       = on_update;
    anim->on_complete     = on_complete;
    anim->user_data       = user_data;
}

void zephio_animation_set_loops(ZephioAnimation *anim, int loops)
{
    if (!anim) return;
    anim->loops           = loops;
    anim->loops_remaining = loops;
}

double zephio_animation_get_progress(const ZephioAnimation *anim)
{
    if (!anim || anim->duration_ms <= 0.0) return 0.0;

    double raw = anim->elapsed_ms / anim->duration_ms;
    if (raw > 1.0) raw = 1.0;
    return zephio_ease(anim->easing, raw);
}

void zephio_animation_play(ZephioAnimation *anim)
{
    if (!anim) return;
    if (anim->state == ZEPHIO_ANIM_COMPLETED) {
        anim->elapsed_ms      = 0.0;
        anim->loops_remaining = anim->loops;
    }
    anim->state = ZEPHIO_ANIM_PLAYING;
}

void zephio_animation_pause(ZephioAnimation *anim)
{
    if (!anim) return;
    if (anim->state == ZEPHIO_ANIM_PLAYING)
        anim->state = ZEPHIO_ANIM_PAUSED;
}

void zephio_animation_stop(ZephioAnimation *anim)
{
    if (!anim) return;
    anim->state           = ZEPHIO_ANIM_IDLE;
    anim->elapsed_ms      = 0.0;
    anim->loops_remaining = anim->loops;
}

void zephio_animation_update(ZephioAnimation *anim, double delta_ms)
{
    if (!anim || anim->state != ZEPHIO_ANIM_PLAYING) return;

    anim->elapsed_ms += delta_ms;

    if (anim->on_update) {
        anim->on_update(zephio_animation_get_progress(anim), anim->user_data);
    }

    if (anim->elapsed_ms >= anim->duration_ms) {
        if (anim->loops == -1) {
            anim->elapsed_ms -= anim->duration_ms;
        } else if (anim->loops_remaining > 0) {
            anim->loops_remaining--;
            anim->elapsed_ms -= anim->duration_ms;
        } else {
            anim->elapsed_ms = anim->duration_ms;
            anim->state = ZEPHIO_ANIM_COMPLETED;
            if (anim->on_complete)
                anim->on_complete(anim->user_data);
        }
    }
}
