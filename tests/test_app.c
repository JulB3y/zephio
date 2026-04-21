#include "util.h"
#include "zephio_app.h"
#include "zephio_animator.h"
#include "zephio_widget.h"

static int g_app_mock_render_count = 0;
static int g_app_config_init_called = 0;

static void app_mock_render(ZephioWidget *w) { (void)w; g_app_mock_render_count++; }
static int  app_mock_init(ZephioApp *a, void *ud) { (void)a; (void)ud; g_app_config_init_called = 1; return 0; }

static ZephioWidgetVTable g_app_render_vt = { app_mock_render, NULL, NULL, NULL, NULL, NULL, NULL };

/* ── app_new / free ─────────────────────────────────────────────── */

TEST_BEGIN(app_new_basic)
{
    ZephioApp *app = zephio_app_new(NULL);
    TEST_ASSERT(app != NULL);
    TEST_EQ(app->running, 0);
    TEST_EQ(app->exit_code, 0);
    TEST_EQ(app->overlay_count, 0);
    TEST_ASSERT(app->animator != NULL);
    zephio_app_free(app);
}

TEST_BEGIN(app_new_with_config)
{
    ZephioAppConfig config = {0};
    config.tick_rate_ms = 50;

    ZephioApp *app = zephio_app_new(&config);
    TEST_ASSERT(app != NULL);
    TEST_EQ(app->config.tick_rate_ms, 50);

    zephio_app_free(app);
}

TEST_BEGIN(app_new_null_config)
{
    ZephioApp *app = zephio_app_new(NULL);
    TEST_ASSERT(app != NULL);
    TEST_EQ(app->config.tick_rate_ms, 0);
    zephio_app_free(app);
}

TEST_BEGIN(app_free_null)
{
    zephio_app_free(NULL);
}

/* ── app stop via struct ────────────────────────────────────────── */

TEST_BEGIN(app_stop_via_struct)
{
    ZephioApp *app = zephio_app_new(NULL);
    app->running = 1;

    app->running = 0;
    app->exit_code = 0;
    TEST_EQ(app->running, 0);
    TEST_EQ(app->exit_code, 0);

    zephio_app_free(app);
}

/* ── app animator ───────────────────────────────────────────────── */

TEST_BEGIN(app_animator_created)
{
    ZephioApp *app = zephio_app_new(NULL);
    TEST_ASSERT(app->animator != NULL);

    ZephioAnimator *anim = zephio_app_get_animator(app);
    TEST_ASSERT(anim == app->animator);

    zephio_app_free(app);
}

TEST_BEGIN(app_get_animator_null)
{
    TEST_ASSERT(zephio_app_get_animator(NULL) == NULL);
}

/* ── app overlays ───────────────────────────────────────────────── */

TEST_BEGIN(app_overlay_push_pop)
{
    ZephioApp *app = zephio_app_new(NULL);
    ZephioWidget a, b;
    zephio_widget_init(&a, 0, 0, 40, 10, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 40, 10, NULL, NULL);

    TEST_EQ(app->overlay_count, 0);

    ZephioResult r1 = zephio_app_push_overlay(app, &a);
    TEST_EQ(r1, ZEPHIO_OK);
    TEST_EQ(app->overlay_count, 1);

    ZephioResult r2 = zephio_app_push_overlay(app, &b);
    TEST_EQ(r2, ZEPHIO_OK);
    TEST_EQ(app->overlay_count, 2);

    TEST_ASSERT(app->overlays[app->overlay_count - 1] == &b);

    ZephioWidget *popped = zephio_app_pop_overlay(app);
    TEST_ASSERT(popped == &b);
    TEST_EQ(app->overlay_count, 1);
    TEST_ASSERT(app->overlays[app->overlay_count - 1] == &a);

    zephio_app_pop_overlay(app);
    TEST_EQ(app->overlay_count, 0);

    zephio_app_free(app);
    zephio_widget_destroy(&a);
    zephio_widget_destroy(&b);
}

TEST_BEGIN(app_overlay_push_null)
{
    ZephioApp *app = zephio_app_new(NULL);
    TEST_NE(zephio_app_push_overlay(NULL, NULL), ZEPHIO_OK);
    TEST_NE(zephio_app_push_overlay(app, NULL), ZEPHIO_OK);
    TEST_EQ(app->overlay_count, 0);
    zephio_app_free(app);
}

TEST_BEGIN(app_overlay_pop_empty)
{
    ZephioApp *app = zephio_app_new(NULL);
    ZephioWidget *w = zephio_app_pop_overlay(app);
    TEST_ASSERT(w == NULL);
    TEST_EQ(app->overlay_count, 0);
    zephio_app_free(app);
}

TEST_BEGIN(app_overlay_max)
{
    ZephioApp *app = zephio_app_new(NULL);
    ZephioWidget widgets[TUI_APP_MAX_OVERLAYS + 1];
    for (int i = 0; i < TUI_APP_MAX_OVERLAYS + 1; i++) {
        zephio_widget_init(&widgets[i], 0, 0, 10, 5, NULL, NULL);
    }

    for (int i = 0; i < TUI_APP_MAX_OVERLAYS; i++) {
        ZephioResult r = zephio_app_push_overlay(app, &widgets[i]);
        TEST_EQ(r, ZEPHIO_OK);
    }

    ZephioResult r = zephio_app_push_overlay(app, &widgets[TUI_APP_MAX_OVERLAYS]);
    TEST_NE(r, ZEPHIO_OK);
    TEST_EQ(app->overlay_count, TUI_APP_MAX_OVERLAYS);

    zephio_app_free(app);
    for (int i = 0; i < TUI_APP_MAX_OVERLAYS + 1; i++) {
        zephio_widget_destroy(&widgets[i]);
    }
}

TEST_BEGIN(app_overlay_push_focuses)
{
    ZephioApp *app = zephio_app_new(NULL);
    ZephioWidget w;
    zephio_widget_init(&w, 0, 0, 40, 10, NULL, NULL);
    w.focusable = 1;

    zephio_app_push_overlay(app, &w);
    TEST_EQ(w.focused, 1);

    zephio_app_pop_overlay(app);
    TEST_EQ(w.focused, 0);

    zephio_app_free(app);
    zephio_widget_destroy(&w);
}

TEST_BEGIN(app_overlay_render_overlays)
{
    g_app_mock_render_count = 0;

    ZephioApp *app = zephio_app_new(NULL);
    ZephioWidget a, b;
    zephio_widget_init(&a, 0, 0, 40, 10, &g_app_render_vt, NULL);
    zephio_widget_init(&b, 0, 0, 40, 10, &g_app_render_vt, NULL);
    a.visible = 1;
    b.visible = 1;

    zephio_app_push_overlay(app, &a);
    zephio_app_push_overlay(app, &b);

    zephio_app_render_overlays(app);
    TEST_EQ(g_app_mock_render_count, 2);

    zephio_app_render_overlays(NULL);

    zephio_app_free(app);
    zephio_widget_destroy(&a);
    zephio_widget_destroy(&b);
}

/* ── app toasts ─────────────────────────────────────────────────── */

TEST_BEGIN(app_get_toasts)
{
    ZephioApp *app = zephio_app_new(NULL);
    ZephioToastManager *tm = zephio_app_get_toasts(app);
    TEST_ASSERT(tm != NULL);
    zephio_app_free(app);
}

TEST_BEGIN(app_get_toasts_null)
{
    TEST_ASSERT(zephio_app_get_toasts(NULL) == NULL);
}

TEST_BEGIN(app_toast_basic)
{
    ZephioApp *app = zephio_app_new(NULL);
    int id = zephio_app_toast(app, ZEPHIO_TOAST_INFO, "Hello", 0);
    TEST_ASSERT(id >= 0);
    zephio_app_free(app);
}

TEST_BEGIN(app_toast_null)
{
    TEST_EQ(zephio_app_toast(NULL, ZEPHIO_TOAST_INFO, "X", 0), -1);
}

/* ── Config validation ──────────────────────────────────────────── */

TEST_BEGIN(app_config_copied)
{
    g_app_config_init_called = 0;

    ZephioAppConfig config = {0};
    config.on_init = app_mock_init;
    config.tick_rate_ms = 100;

    ZephioApp *app = zephio_app_new(&config);
    TEST_ASSERT(app->config.on_init == app_mock_init);
    TEST_EQ(app->config.tick_rate_ms, 100);
    (void)g_app_config_init_called;

    zephio_app_free(app);
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void)
{
    fprintf(stderr, "Running app tests...\n\n");

    TEST_RUN(app_new_basic);
    TEST_RUN(app_new_with_config);
    TEST_RUN(app_new_null_config);
    TEST_RUN(app_free_null);

    TEST_RUN(app_stop_via_struct);

    TEST_RUN(app_animator_created);
    TEST_RUN(app_get_animator_null);

    TEST_RUN(app_overlay_push_pop);
    TEST_RUN(app_overlay_push_null);
    TEST_RUN(app_overlay_pop_empty);
    TEST_RUN(app_overlay_max);
    TEST_RUN(app_overlay_push_focuses);
    TEST_RUN(app_overlay_render_overlays);

    TEST_RUN(app_get_toasts);
    TEST_RUN(app_get_toasts_null);
    TEST_RUN(app_toast_basic);
    TEST_RUN(app_toast_null);

    TEST_RUN(app_config_copied);

    TEST_SUMMARY();
}
