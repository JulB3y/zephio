#define _POSIX_C_SOURCE 200809L

#include "zephio_app.h"
#include "zephio_animator.h"
#include "zephio_terminal.h"
#include "zephio_screen.h"
#include "zephio_mouse.h"
#include "zephio_context.h"

#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void zephio_app_stop(ZephioApp *app);
static ZephioWidget *zephio_app_top_overlay(ZephioApp *app);
static int zephio_app_handle_overlay_input(ZephioApp *app, const ZephioEvent *event);
static int zephio_app_handle_overlay_mouse(ZephioApp *app, const ZephioMouseEvent *mouse);

static double time_now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

ZephioApp *zephio_app_new(ZephioContext *ctx, const ZephioAppConfig *config)
{
    ZephioApp *app = (ZephioApp *)calloc(1, sizeof(ZephioApp));
    if (!app) return NULL;

    app->ctx = ctx;

    if (config) {
        app->config = *config;
    }

    app->running       = 0;
    app->exit_code     = 0;
    app->overlay_count = 0;
    app->animator      = zephio_animator_new();
    app->last_tick_ms  = 0.0;

    zephio_toast_manager_init(&app->toasts);

    return app;
}

void zephio_app_free(ZephioApp *app)
{
    if (app) {
        zephio_toast_manager_free(&app->toasts);
        zephio_animator_free(app->animator);
        free(app);
    }
}

static void __attribute__((unused)) zephio_app_stop(ZephioApp *app)
{
    if (app) {
        app->running   = 0;
        app->exit_code = 0;
    }
}

static int app_poll_event(ZephioApp *app, ZephioEvent *event)
{
    if (app->config.tick_rate_ms > 0) {
        struct pollfd pfd = { app->ctx->terminal.fd, POLLIN, 0 };
        int ret = poll(&pfd, 1, app->config.tick_rate_ms);

        if (ret == 0) {
            memset(event, 0, sizeof(*event));
            return 0;
        }

        if (ret < 0) {
            ZephioResult res = zephio_input_poll(app->ctx, event);
            return (res == ZEPHIO_OK) ? 1 : -1;
        }
    }

    ZephioResult res = zephio_input_poll(app->ctx, event);
    if (res != ZEPHIO_OK) return -1;
    return 1;
}

int zephio_app_run(ZephioApp *app)
{
    if (!app) return 1;

    ZephioResult res = zephio_init(app->ctx);
    if (res != ZEPHIO_OK) return 1;

    zephio_input_init(app->ctx);
    zephio_mouse_enable(app->ctx);

    app->running   = 1;
    app->exit_code = 0;

    if (app->config.on_init) {
        int ret = app->config.on_init(app, app->config.user_data);
        if (ret != 0) {
            app->exit_code = ret;
            goto cleanup;
        }
    }

    if (app->config.on_render) {
        app->config.on_render(app, app->config.user_data);
    }

    app->last_tick_ms = time_now_ms();

    while (app->running) {
        ZephioEvent event;
        int has_event = app_poll_event(app, &event);

        {
            double now = time_now_ms();
            double delta = now - app->last_tick_ms;
            app->last_tick_ms = now;
            if (delta < 0.0 || delta > 1000.0) delta = 16.0;

            if (app->animator)
                zephio_animator_update(app->animator, delta);
            zephio_toast_update(&app->toasts, delta);
        }

        if (has_event < 0) {
            continue;
        }

        if (has_event == 0) {
            if (app->config.on_render) {
                app->config.on_render(app, app->config.user_data);
            }
            continue;
        }

        if (event.key == ZEPHIO_EVENT_RESIZE) {
            zephio_screen_resize(app->ctx, event.size.rows, event.size.cols);

            if (app->config.on_resize) {
                int ret = app->config.on_resize(
                    app, event.size.rows, event.size.cols,
                    app->config.user_data);
                if (ret != 0) {
                    app->exit_code = ret;
                    break;
                }
            }

            if (app->config.on_render) {
                app->config.on_render(app, app->config.user_data);
            }
            continue;
        }

        if (event.key == ZEPHIO_EVENT_MOUSE) {
            if (app->overlay_count > 0) {
                zephio_app_handle_overlay_mouse(app, &event.mouse);
            } else if (app->config.on_mouse) {
                int ret = app->config.on_mouse(app, &event.mouse,
                                               app->config.user_data);
                if (ret != 0) {
                    app->exit_code = ret;
                    break;
                }
            }

            if (app->config.on_render) {
                app->config.on_render(app, app->config.user_data);
            }
            continue;
        }

        if (app->overlay_count > 0) {
            zephio_app_handle_overlay_input(app, &event);
        } else if (app->config.on_input) {
            int ret = app->config.on_input(app, &event, app->config.user_data);
            if (ret != 0) {
                app->exit_code = ret;
                break;
            }
        }

        if (app->config.on_render) {
            app->config.on_render(app, app->config.user_data);
        }
    }

cleanup:
    if (app->config.on_shutdown) {
        app->config.on_shutdown(app, app->config.user_data);
    }

    zephio_input_shutdown(app->ctx);
    zephio_shutdown(app->ctx);

    return app->exit_code;
}

ZephioResult zephio_app_push_overlay(ZephioApp *app, ZephioWidget *widget)
{
    if (!app || !widget) return TUI_ERR_MEMORY;
    if (app->overlay_count >= ZEPHIO_APP_MAX_OVERLAYS) return TUI_ERR_MEMORY;

    app->overlays[app->overlay_count++] = widget;
    widget->dirty = 1;

    if (widget->focusable) {
        zephio_widget_focus(widget);
    }

    return ZEPHIO_OK;
}

ZephioWidget *zephio_app_pop_overlay(ZephioApp *app)
{
    if (!app || app->overlay_count == 0) return NULL;

    ZephioWidget *widget = app->overlays[--app->overlay_count];
    app->overlays[app->overlay_count] = NULL;

    if (widget->focused) {
        zephio_widget_blur(widget);
    }

    return widget;
}

static __attribute__((unused)) ZephioWidget *zephio_app_top_overlay(ZephioApp *app)
{
    if (!app || app->overlay_count == 0) return NULL;
    return app->overlays[app->overlay_count - 1];
}

void zephio_app_render_overlays(ZephioApp *app)
{
    if (!app) return;
    for (int i = 0; i < app->overlay_count; i++) {
        zephio_widget_render(app->overlays[i]);
    }
}

static int zephio_app_handle_overlay_input(ZephioApp *app, const ZephioEvent *event)
{
    if (!app || app->overlay_count == 0) return 0;

    ZephioWidget *top = app->overlays[app->overlay_count - 1];
    if (!top) return 0;

    if (top->focused || top->focusable) {
        if (!top->focused) zephio_widget_focus(top);
        return zephio_widget_handle_input(top, event);
    }

    return 0;
}

static int zephio_app_handle_overlay_mouse(ZephioApp *app, const ZephioMouseEvent *mouse)
{
    if (!app || app->overlay_count == 0) return 0;

    ZephioWidget *top = app->overlays[app->overlay_count - 1];
    if (!top) return 0;

    return zephio_widget_handle_mouse(top, mouse);
}

ZephioAnimator *zephio_app_get_animator(ZephioApp *app)
{
    if (!app) return NULL;
    return app->animator;
}

ZephioToastManager *zephio_app_get_toasts(ZephioApp *app)
{
    if (!app) return NULL;
    return &app->toasts;
}

int zephio_app_toast(ZephioApp *app, ZephioToastSeverity severity,
                  const char *message, double duration_ms)
{
    if (!app) return -1;
    return zephio_toast_show(&app->toasts, severity, message, duration_ms);
}

void zephio_app_render_toasts(ZephioApp *app)
{
    if (!app) return;
    ZephioSize size = zephio_screen_size(app->ctx);
    zephio_toast_render(app->ctx, &app->toasts, size.rows, size.cols);
}
