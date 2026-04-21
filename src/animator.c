#define _POSIX_C_SOURCE 200809L

#include "zephio_animator.h"

#include <stdlib.h>
#include <string.h>

ZephioAnimator *zephio_animator_new(void)
{
    ZephioAnimator *a = (ZephioAnimator *)calloc(1, sizeof(ZephioAnimator));
    if (!a) return NULL;
    a->count        = 0;
    a->next_id      = 1;
    a->active_count = 0;
    return a;
}

void zephio_animator_free(ZephioAnimator *animator)
{
    if (!animator) return;
    zephio_animator_stop_all(animator);
    free(animator);
}

static int find_slot(const ZephioAnimator *animator, int id)
{
    for (int i = 0; i < animator->count; i++) {
        if (animator->animations[i].id == id)
            return i;
    }
    return -1;
}

int zephio_animator_create(ZephioAnimator *animator, double duration_ms,
                        ZephioEasing easing, ZephioAnimUpdateFn on_update,
                        ZephioAnimCompleteFn on_complete, void *user_data)
{
    if (!animator) return ZEPHIO_ANIMATOR_INVALID_ID;
    if (animator->count >= ZEPHIO_ANIMATOR_MAX_ANIMATIONS)
        return ZEPHIO_ANIMATOR_INVALID_ID;

    int slot = animator->count;
    int id   = animator->next_id++;

    ZephioAnimation *anim = &animator->animations[slot];
    zephio_animation_init(anim, duration_ms, easing, on_update, on_complete, user_data);
    anim->id = id;

    animator->count++;
    return id;
}

void zephio_animator_remove(ZephioAnimator *animator, int id)
{
    if (!animator) return;
    int slot = find_slot(animator, id);
    if (slot < 0) return;

    ZephioAnimation *anim = &animator->animations[slot];
    if (anim->state == ZEPHIO_ANIM_PLAYING)
        animator->active_count--;

    int last = animator->count - 1;
    if (slot < last)
        memmove(&animator->animations[slot], &animator->animations[slot + 1],
                (last - slot) * sizeof(ZephioAnimation));
    animator->count--;
}

void zephio_animator_play(ZephioAnimator *animator, int id)
{
    if (!animator) return;
    ZephioAnimation *anim = zephio_animator_get(animator, id);
    if (!anim) return;

    ZephioAnimState prev = anim->state;
    zephio_animation_play(anim);
    if (prev != ZEPHIO_ANIM_PLAYING && anim->state == ZEPHIO_ANIM_PLAYING)
        animator->active_count++;
}

void zephio_animator_pause(ZephioAnimator *animator, int id)
{
    if (!animator) return;
    ZephioAnimation *anim = zephio_animator_get(animator, id);
    if (!anim) return;

    ZephioAnimState prev = anim->state;
    zephio_animation_pause(anim);
    if (prev == ZEPHIO_ANIM_PLAYING && anim->state == ZEPHIO_ANIM_PAUSED)
        animator->active_count--;
}

void zephio_animator_stop(ZephioAnimator *animator, int id)
{
    if (!animator) return;
    ZephioAnimation *anim = zephio_animator_get(animator, id);
    if (!anim) return;

    if (anim->state == ZEPHIO_ANIM_PLAYING)
        animator->active_count--;
    zephio_animation_stop(anim);
}

void zephio_animator_update(ZephioAnimator *animator, double delta_ms)
{
    if (!animator || delta_ms <= 0.0) return;

    for (int i = 0; i < animator->count; i++) {
        ZephioAnimation *anim = &animator->animations[i];
        ZephioAnimState prev = anim->state;
        zephio_animation_update(anim, delta_ms);
        if (prev == ZEPHIO_ANIM_PLAYING && anim->state == ZEPHIO_ANIM_COMPLETED)
            animator->active_count--;
    }
}

int zephio_animator_is_active(const ZephioAnimator *animator)
{
    if (!animator) return 0;
    return animator->active_count > 0 ? 1 : 0;
}

ZephioAnimation *zephio_animator_get(ZephioAnimator *animator, int id)
{
    if (!animator) return NULL;
    int slot = find_slot(animator, id);
    if (slot < 0) return NULL;
    return &animator->animations[slot];
}

void zephio_animator_stop_all(ZephioAnimator *animator)
{
    if (!animator) return;
    for (int i = 0; i < animator->count; i++) {
        zephio_animation_stop(&animator->animations[i]);
    }
    animator->active_count = 0;
    animator->count = 0;
}
