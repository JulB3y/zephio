#include "util.h"
#include "tui_app.h"
#include "tui_animator.h"
#include "tui_widget.h"

static int g_app_mock_render_count = 0;
static int g_app_config_init_called = 0;

static void app_mock_render(TuiWidget *w) { (void)w; g_app_mock_render_count++; }
static int  app_mock_init(TuiApp *a, void *ud) { (void)a; (void)ud; g_app_config_init_called = 1; return 0; }

static TuiWidgetVTable g_app_render_vt = { app_mock_render, NULL, NULL, NULL, NULL, NULL, NULL };

/* ── app_new / free ─────────────────────────────────────────────── */

TEST_BEGIN(app_new_basic)
{
    TuiApp *app = tui_app_new(NULL);
    TEST_ASSERT(app != NULL);
    TEST_EQ(app->running, 0);
    TEST_EQ(app->exit_code, 0);
    TEST_EQ(app->overlay_count, 0);
    TEST_ASSERT(app->animator != NULL);
    tui_app_free(app);
}

TEST_BEGIN(app_new_with_config)
{
    TuiAppConfig config = {0};
    config.tick_rate_ms = 50;

    TuiApp *app = tui_app_new(&config);
    TEST_ASSERT(app != NULL);
    TEST_EQ(app->config.tick_rate_ms, 50);

    tui_app_free(app);
}

TEST_BEGIN(app_new_null_config)
{
    TuiApp *app = tui_app_new(NULL);
    TEST_ASSERT(app != NULL);
    TEST_EQ(app->config.tick_rate_ms, 0);
    tui_app_free(app);
}

TEST_BEGIN(app_free_null)
{
    tui_app_free(NULL);
}

/* ── app stop via struct ────────────────────────────────────────── */

TEST_BEGIN(app_stop_via_struct)
{
    TuiApp *app = tui_app_new(NULL);
    app->running = 1;

    app->running = 0;
    app->exit_code = 0;
    TEST_EQ(app->running, 0);
    TEST_EQ(app->exit_code, 0);

    tui_app_free(app);
}

/* ── app animator ───────────────────────────────────────────────── */

TEST_BEGIN(app_animator_created)
{
    TuiApp *app = tui_app_new(NULL);
    TEST_ASSERT(app->animator != NULL);

    TuiAnimator *anim = tui_app_get_animator(app);
    TEST_ASSERT(anim == app->animator);

    tui_app_free(app);
}

TEST_BEGIN(app_get_animator_null)
{
    TEST_ASSERT(tui_app_get_animator(NULL) == NULL);
}

/* ── app overlays ───────────────────────────────────────────────── */

TEST_BEGIN(app_overlay_push_pop)
{
    TuiApp *app = tui_app_new(NULL);
    TuiWidget a, b;
    tui_widget_init(&a, 0, 0, 40, 10, NULL, NULL);
    tui_widget_init(&b, 0, 0, 40, 10, NULL, NULL);

    TEST_EQ(app->overlay_count, 0);

    TuiResult r1 = tui_app_push_overlay(app, &a);
    TEST_EQ(r1, TUI_OK);
    TEST_EQ(app->overlay_count, 1);

    TuiResult r2 = tui_app_push_overlay(app, &b);
    TEST_EQ(r2, TUI_OK);
    TEST_EQ(app->overlay_count, 2);

    TEST_ASSERT(app->overlays[app->overlay_count - 1] == &b);

    TuiWidget *popped = tui_app_pop_overlay(app);
    TEST_ASSERT(popped == &b);
    TEST_EQ(app->overlay_count, 1);
    TEST_ASSERT(app->overlays[app->overlay_count - 1] == &a);

    tui_app_pop_overlay(app);
    TEST_EQ(app->overlay_count, 0);

    tui_app_free(app);
    tui_widget_destroy(&a);
    tui_widget_destroy(&b);
}

TEST_BEGIN(app_overlay_push_null)
{
    TuiApp *app = tui_app_new(NULL);
    TEST_NE(tui_app_push_overlay(NULL, NULL), TUI_OK);
    TEST_NE(tui_app_push_overlay(app, NULL), TUI_OK);
    TEST_EQ(app->overlay_count, 0);
    tui_app_free(app);
}

TEST_BEGIN(app_overlay_pop_empty)
{
    TuiApp *app = tui_app_new(NULL);
    TuiWidget *w = tui_app_pop_overlay(app);
    TEST_ASSERT(w == NULL);
    TEST_EQ(app->overlay_count, 0);
    tui_app_free(app);
}

TEST_BEGIN(app_overlay_max)
{
    TuiApp *app = tui_app_new(NULL);
    TuiWidget widgets[TUI_APP_MAX_OVERLAYS + 1];
    for (int i = 0; i < TUI_APP_MAX_OVERLAYS + 1; i++) {
        tui_widget_init(&widgets[i], 0, 0, 10, 5, NULL, NULL);
    }

    for (int i = 0; i < TUI_APP_MAX_OVERLAYS; i++) {
        TuiResult r = tui_app_push_overlay(app, &widgets[i]);
        TEST_EQ(r, TUI_OK);
    }

    TuiResult r = tui_app_push_overlay(app, &widgets[TUI_APP_MAX_OVERLAYS]);
    TEST_NE(r, TUI_OK);
    TEST_EQ(app->overlay_count, TUI_APP_MAX_OVERLAYS);

    tui_app_free(app);
    for (int i = 0; i < TUI_APP_MAX_OVERLAYS + 1; i++) {
        tui_widget_destroy(&widgets[i]);
    }
}

TEST_BEGIN(app_overlay_push_focuses)
{
    TuiApp *app = tui_app_new(NULL);
    TuiWidget w;
    tui_widget_init(&w, 0, 0, 40, 10, NULL, NULL);
    w.focusable = 1;

    tui_app_push_overlay(app, &w);
    TEST_EQ(w.focused, 1);

    tui_app_pop_overlay(app);
    TEST_EQ(w.focused, 0);

    tui_app_free(app);
    tui_widget_destroy(&w);
}

TEST_BEGIN(app_overlay_render_overlays)
{
    g_app_mock_render_count = 0;

    TuiApp *app = tui_app_new(NULL);
    TuiWidget a, b;
    tui_widget_init(&a, 0, 0, 40, 10, &g_app_render_vt, NULL);
    tui_widget_init(&b, 0, 0, 40, 10, &g_app_render_vt, NULL);
    a.visible = 1;
    b.visible = 1;

    tui_app_push_overlay(app, &a);
    tui_app_push_overlay(app, &b);

    tui_app_render_overlays(app);
    TEST_EQ(g_app_mock_render_count, 2);

    tui_app_render_overlays(NULL);

    tui_app_free(app);
    tui_widget_destroy(&a);
    tui_widget_destroy(&b);
}

/* ── app toasts ─────────────────────────────────────────────────── */

TEST_BEGIN(app_get_toasts)
{
    TuiApp *app = tui_app_new(NULL);
    TuiToastManager *tm = tui_app_get_toasts(app);
    TEST_ASSERT(tm != NULL);
    tui_app_free(app);
}

TEST_BEGIN(app_get_toasts_null)
{
    TEST_ASSERT(tui_app_get_toasts(NULL) == NULL);
}

TEST_BEGIN(app_toast_basic)
{
    TuiApp *app = tui_app_new(NULL);
    int id = tui_app_toast(app, TUI_TOAST_INFO, "Hello", 0);
    TEST_ASSERT(id >= 0);
    tui_app_free(app);
}

TEST_BEGIN(app_toast_null)
{
    TEST_EQ(tui_app_toast(NULL, TUI_TOAST_INFO, "X", 0), -1);
}

/* ── Config validation ──────────────────────────────────────────── */

TEST_BEGIN(app_config_copied)
{
    g_app_config_init_called = 0;

    TuiAppConfig config = {0};
    config.on_init = app_mock_init;
    config.tick_rate_ms = 100;

    TuiApp *app = tui_app_new(&config);
    TEST_ASSERT(app->config.on_init == app_mock_init);
    TEST_EQ(app->config.tick_rate_ms, 100);
    (void)g_app_config_init_called;

    tui_app_free(app);
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
