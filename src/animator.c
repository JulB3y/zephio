#define _POSIX_C_SOURCE 200809L

#include "tui_animator.h"

#include <stdlib.h>
#include <string.h>

TuiAnimator *tui_animator_new(void)
{
    TuiAnimator *a = (TuiAnimator *)calloc(1, sizeof(TuiAnimator));
    if (!a) return NULL;
    a->count        = 0;
    a->next_id      = 1;
    a->active_count = 0;
    return a;
}

void tui_animator_free(TuiAnimator *animator)
{
    if (!animator) return;
    tui_animator_stop_all(animator);
    free(animator);
}

static int find_slot(const TuiAnimator *animator, int id)
{
    for (int i = 0; i < animator->count; i++) {
        if (animator->animations[i].id == id)
            return i;
    }
    return -1;
}

int tui_animator_create(TuiAnimator *animator, double duration_ms,
                        TuiEasing easing, TuiAnimUpdateFn on_update,
                        TuiAnimCompleteFn on_complete, void *user_data)
{
    if (!animator) return TUI_ANIMATOR_INVALID_ID;
    if (animator->count >= TUI_ANIMATOR_MAX_ANIMATIONS)
        return TUI_ANIMATOR_INVALID_ID;

    int slot = animator->count;
    int id   = animator->next_id++;

    TuiAnimation *anim = &animator->animations[slot];
    tui_animation_init(anim, duration_ms, easing, on_update, on_complete, user_data);
    anim->id = id;

    animator->count++;
    return id;
}

void tui_animator_remove(TuiAnimator *animator, int id)
{
    if (!animator) return;
    int slot = find_slot(animator, id);
    if (slot < 0) return;

    TuiAnimation *anim = &animator->animations[slot];
    if (anim->state == TUI_ANIM_PLAYING)
        animator->active_count--;

    int last = animator->count - 1;
    if (slot < last)
        memmove(&animator->animations[slot], &animator->animations[slot + 1],
                (last - slot) * sizeof(TuiAnimation));
    animator->count--;
}

void tui_animator_play(TuiAnimator *animator, int id)
{
    if (!animator) return;
    TuiAnimation *anim = tui_animator_get(animator, id);
    if (!anim) return;

    TuiAnimState prev = anim->state;
    tui_animation_play(anim);
    if (prev != TUI_ANIM_PLAYING && anim->state == TUI_ANIM_PLAYING)
        animator->active_count++;
}

void tui_animator_pause(TuiAnimator *animator, int id)
{
    if (!animator) return;
    TuiAnimation *anim = tui_animator_get(animator, id);
    if (!anim) return;

    TuiAnimState prev = anim->state;
    tui_animation_pause(anim);
    if (prev == TUI_ANIM_PLAYING && anim->state == TUI_ANIM_PAUSED)
        animator->active_count--;
}

void tui_animator_stop(TuiAnimator *animator, int id)
{
    if (!animator) return;
    TuiAnimation *anim = tui_animator_get(animator, id);
    if (!anim) return;

    if (anim->state == TUI_ANIM_PLAYING)
        animator->active_count--;
    tui_animation_stop(anim);
}

void tui_animator_update(TuiAnimator *animator, double delta_ms)
{
    if (!animator || delta_ms <= 0.0) return;

    for (int i = 0; i < animator->count; i++) {
        TuiAnimation *anim = &animator->animations[i];
        TuiAnimState prev = anim->state;
        tui_animation_update(anim, delta_ms);
        if (prev == TUI_ANIM_PLAYING && anim->state == TUI_ANIM_COMPLETED)
            animator->active_count--;
    }
}

int tui_animator_is_active(const TuiAnimator *animator)
{
    if (!animator) return 0;
    return animator->active_count > 0 ? 1 : 0;
}

TuiAnimation *tui_animator_get(TuiAnimator *animator, int id)
{
    if (!animator) return NULL;
    int slot = find_slot(animator, id);
    if (slot < 0) return NULL;
    return &animator->animations[slot];
}

void tui_animator_stop_all(TuiAnimator *animator)
{
    if (!animator) return;
    for (int i = 0; i < animator->count; i++) {
        tui_animation_stop(&animator->animations[i]);
    }
    animator->active_count = 0;
    animator->count = 0;
}
