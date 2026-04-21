#include "util.h"
#include "zephio_animation.h"
#include "zephio_animator.h"

#include <math.h>

static int g_anim_update_count = 0;
static int g_anim_complete_count = 0;
static double g_last_progress = -1.0;

static void stub_on_update(double progress, void *ud) {
    (void)ud;
    g_anim_update_count++;
    g_last_progress = progress;
}

static void stub_on_complete(void *ud) {
    (void)ud;
    g_anim_complete_count++;
}

/* ── easing functions ───────────────────────────────────────────── */

TEST_BEGIN(ease_linear)
{
    TEST_EQ((int)zephio_ease(ZEPHIO_EASE_LINEAR, 0.0), 0);
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_LINEAR, 1.0) - 1.0) < 0.001);
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_LINEAR, 0.5) - 0.5) < 0.001);
}

TEST_BEGIN(ease_in_quad)
{
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_IN_QUAD, 0.0) - 0.0) < 0.001);
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_IN_QUAD, 1.0) - 1.0) < 0.001);
    TEST_ASSERT(zephio_ease(ZEPHIO_EASE_IN_QUAD, 0.5) < 0.5);
}

TEST_BEGIN(ease_out_quad)
{
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_OUT_QUAD, 0.0) - 0.0) < 0.001);
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_OUT_QUAD, 1.0) - 1.0) < 0.001);
    TEST_ASSERT(zephio_ease(ZEPHIO_EASE_OUT_QUAD, 0.5) > 0.5);
}

TEST_BEGIN(ease_in_out_quad)
{
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_IN_OUT_QUAD, 0.0)) < 0.001);
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_IN_OUT_QUAD, 1.0) - 1.0) < 0.001);
}

TEST_BEGIN(ease_clamps)
{
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_LINEAR, -0.5)) < 0.001);
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_LINEAR, 1.5) - 1.0) < 0.001);
}

TEST_BEGIN(ease_in_cubic)
{
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_IN_CUBIC, 0.0)) < 0.001);
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_IN_CUBIC, 1.0) - 1.0) < 0.001);
}

TEST_BEGIN(ease_out_cubic)
{
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_OUT_CUBIC, 1.0) - 1.0) < 0.001);
}

TEST_BEGIN(ease_in_sine)
{
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_IN_SINE, 0.0)) < 0.001);
}

TEST_BEGIN(ease_out_sine)
{
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_OUT_SINE, 1.0) - 1.0) < 0.001);
}

TEST_BEGIN(ease_in_expo)
{
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_IN_EXPO, 0.0)) < 0.001);
}

TEST_BEGIN(ease_out_expo)
{
    TEST_ASSERT(fabs(zephio_ease(ZEPHIO_EASE_OUT_EXPO, 1.0) - 1.0) < 0.001);
}

/* ── animation init ─────────────────────────────────────────────── */

TEST_BEGIN(animation_init)
{
    ZephioAnimation anim;
    zephio_animation_init(&anim, 1000, ZEPHIO_EASE_LINEAR,
                       stub_on_update, stub_on_complete, NULL);
    TEST_EQ(anim.state, ZEPHIO_ANIM_IDLE);
    TEST_EQ(anim.easing, ZEPHIO_EASE_LINEAR);
    TEST_ASSERT(fabs(anim.duration_ms - 1000.0) < 0.001);
    TEST_EQ(anim.elapsed_ms, 0);
    TEST_EQ(anim.loops, 0);
    TEST_EQ(anim.loops_remaining, 0);
}

TEST_BEGIN(animation_set_loops)
{
    ZephioAnimation anim;
    zephio_animation_init(&anim, 1000, ZEPHIO_EASE_LINEAR, NULL, NULL, NULL);
    zephio_animation_set_loops(&anim, 3);
    TEST_EQ(anim.loops, 3);
    zephio_animation_set_loops(&anim, -1);
    TEST_EQ(anim.loops, -1);
}

TEST_BEGIN(animation_get_progress_idle)
{
    ZephioAnimation anim;
    zephio_animation_init(&anim, 1000, ZEPHIO_EASE_LINEAR, NULL, NULL, NULL);
    TEST_ASSERT(fabs(zephio_animation_get_progress(&anim)) < 0.001);
}

/* ── animation play/pause/stop ──────────────────────────────────── */

TEST_BEGIN(animation_play)
{
    ZephioAnimation anim;
    zephio_animation_init(&anim, 1000, ZEPHIO_EASE_LINEAR, NULL, NULL, NULL);
    zephio_animation_play(&anim);
    TEST_EQ(anim.state, ZEPHIO_ANIM_PLAYING);
}

TEST_BEGIN(animation_pause)
{
    ZephioAnimation anim;
    zephio_animation_init(&anim, 1000, ZEPHIO_EASE_LINEAR, NULL, NULL, NULL);
    zephio_animation_play(&anim);
    zephio_animation_pause(&anim);
    TEST_EQ(anim.state, ZEPHIO_ANIM_PAUSED);
}

TEST_BEGIN(animation_stop)
{
    ZephioAnimation anim;
    zephio_animation_init(&anim, 1000, ZEPHIO_EASE_LINEAR, NULL, NULL, NULL);
    zephio_animation_play(&anim);
    zephio_animation_stop(&anim);
    TEST_EQ(anim.state, ZEPHIO_ANIM_IDLE);
    TEST_EQ(anim.elapsed_ms, 0);
}

/* ── animation update ───────────────────────────────────────────── */

TEST_BEGIN(animation_update_progress)
{
    g_anim_update_count = 0;
    g_last_progress = -1.0;

    ZephioAnimation anim;
    zephio_animation_init(&anim, 1000, ZEPHIO_EASE_LINEAR,
                       stub_on_update, NULL, NULL);
    zephio_animation_play(&anim);
    zephio_animation_update(&anim, 500);
    TEST_EQ(g_anim_update_count, 1);
    TEST_ASSERT(fabs(g_last_progress - 0.5) < 0.01);
}

TEST_BEGIN(animation_update_complete)
{
    g_anim_complete_count = 0;

    ZephioAnimation anim;
    zephio_animation_init(&anim, 100, ZEPHIO_EASE_LINEAR,
                       NULL, stub_on_complete, NULL);
    zephio_animation_play(&anim);
    zephio_animation_update(&anim, 200);
    TEST_EQ(anim.state, ZEPHIO_ANIM_COMPLETED);
    TEST_EQ(g_anim_complete_count, 1);
}

TEST_BEGIN(animation_update_loop)
{
    g_anim_complete_count = 0;

    ZephioAnimation anim;
    zephio_animation_init(&anim, 100, ZEPHIO_EASE_LINEAR,
                       NULL, stub_on_complete, NULL);
    zephio_animation_set_loops(&anim, 2);
    zephio_animation_play(&anim);
    zephio_animation_update(&anim, 100);
    TEST_EQ(anim.state, ZEPHIO_ANIM_PLAYING);
    zephio_animation_update(&anim, 100);
    TEST_EQ(anim.state, ZEPHIO_ANIM_PLAYING);
    zephio_animation_update(&anim, 100);
    TEST_EQ(anim.state, ZEPHIO_ANIM_COMPLETED);
    TEST_EQ(g_anim_complete_count, 1);
}

TEST_BEGIN(animation_update_idle_noop)
{
    g_anim_update_count = 0;
    ZephioAnimation anim;
    zephio_animation_init(&anim, 100, ZEPHIO_EASE_LINEAR,
                       stub_on_update, NULL, NULL);
    zephio_animation_update(&anim, 50);
    TEST_EQ(g_anim_update_count, 0);
}

/* ── animator new/free ──────────────────────────────────────────── */

TEST_BEGIN(animator_new)
{
    ZephioAnimator *a = zephio_animator_new();
    TEST_ASSERT(a != NULL);
    TEST_EQ(a->count, 0);
    TEST_EQ(a->active_count, 0);
    zephio_animator_free(a);
}

TEST_BEGIN(animator_free_null)
{
    zephio_animator_free(NULL);
}

/* ── animator create/remove ─────────────────────────────────────── */

TEST_BEGIN(animator_create)
{
    ZephioAnimator *a = zephio_animator_new();
    int id = zephio_animator_create(a, 500, ZEPHIO_EASE_OUT_QUAD,
                                  NULL, NULL, NULL);
    TEST_ASSERT(id >= 0);
    TEST_EQ(a->count, 1);
    zephio_animator_free(a);
}

TEST_BEGIN(animator_remove)
{
    ZephioAnimator *a = zephio_animator_new();
    int id = zephio_animator_create(a, 500, ZEPHIO_EASE_LINEAR, NULL, NULL, NULL);
    zephio_animator_remove(a, id);
    zephio_animator_remove(a, -999);
    zephio_animator_free(a);
}

TEST_BEGIN(animator_play_pause_stop)
{
    ZephioAnimator *a = zephio_animator_new();
    int id = zephio_animator_create(a, 500, ZEPHIO_EASE_LINEAR, NULL, NULL, NULL);
    zephio_animator_play(a, id);
    ZephioAnimation *anim = zephio_animator_get(a, id);
    TEST_ASSERT(anim != NULL);
    TEST_EQ(anim->state, ZEPHIO_ANIM_PLAYING);

    zephio_animator_pause(a, id);
    TEST_EQ(anim->state, ZEPHIO_ANIM_PAUSED);

    zephio_animator_stop(a, id);
    TEST_EQ(anim->state, ZEPHIO_ANIM_IDLE);
    zephio_animator_free(a);
}

TEST_BEGIN(animator_update)
{
    g_anim_update_count = 0;

    ZephioAnimator *a = zephio_animator_new();
    int id = zephio_animator_create(a, 1000, ZEPHIO_EASE_LINEAR,
                                  stub_on_update, NULL, NULL);
    zephio_animator_play(a, id);
    zephio_animator_update(a, 250);
    TEST_EQ(g_anim_update_count, 1);
    zephio_animator_free(a);
}

TEST_BEGIN(animator_is_active)
{
    ZephioAnimator *a = zephio_animator_new();
    TEST_EQ(zephio_animator_is_active(a), 0);
    int id = zephio_animator_create(a, 1000, ZEPHIO_EASE_LINEAR, NULL, NULL, NULL);
    zephio_animator_play(a, id);
    TEST_EQ(zephio_animator_is_active(a), 1);
    zephio_animator_free(a);
}

TEST_BEGIN(animator_get)
{
    ZephioAnimator *a = zephio_animator_new();
    TEST_ASSERT(zephio_animator_get(a, 0) == NULL);
    int id = zephio_animator_create(a, 500, ZEPHIO_EASE_LINEAR, NULL, NULL, NULL);
    TEST_ASSERT(zephio_animator_get(a, id) != NULL);
    zephio_animator_free(a);
}

TEST_BEGIN(animator_get_null)
{
    TEST_ASSERT(zephio_animator_get(NULL, 0) == NULL);
}

TEST_BEGIN(animator_stop_all)
{
    ZephioAnimator *a = zephio_animator_new();
    int id1 = zephio_animator_create(a, 500, ZEPHIO_EASE_LINEAR, NULL, NULL, NULL);
    int id2 = zephio_animator_create(a, 500, ZEPHIO_EASE_LINEAR, NULL, NULL, NULL);
    zephio_animator_play(a, id1);
    zephio_animator_play(a, id2);
    TEST_EQ(zephio_animator_is_active(a), 1);
    zephio_animator_stop_all(a);
    TEST_EQ(zephio_animator_is_active(a), 0);
    zephio_animator_free(a);
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void)
{
    fprintf(stderr, "Running animation tests...\n\n");

    TEST_RUN(ease_linear);
    TEST_RUN(ease_in_quad);
    TEST_RUN(ease_out_quad);
    TEST_RUN(ease_in_out_quad);
    TEST_RUN(ease_clamps);
    TEST_RUN(ease_in_cubic);
    TEST_RUN(ease_out_cubic);
    TEST_RUN(ease_in_sine);
    TEST_RUN(ease_out_sine);
    TEST_RUN(ease_in_expo);
    TEST_RUN(ease_out_expo);

    TEST_RUN(animation_init);
    TEST_RUN(animation_set_loops);
    TEST_RUN(animation_get_progress_idle);

    TEST_RUN(animation_play);
    TEST_RUN(animation_pause);
    TEST_RUN(animation_stop);

    TEST_RUN(animation_update_progress);
    TEST_RUN(animation_update_complete);
    TEST_RUN(animation_update_loop);
    TEST_RUN(animation_update_idle_noop);

    TEST_RUN(animator_new);
    TEST_RUN(animator_free_null);
    TEST_RUN(animator_create);
    TEST_RUN(animator_remove);
    TEST_RUN(animator_play_pause_stop);
    TEST_RUN(animator_update);
    TEST_RUN(animator_is_active);
    TEST_RUN(animator_get);
    TEST_RUN(animator_get_null);
    TEST_RUN(animator_stop_all);

    TEST_SUMMARY();
}
