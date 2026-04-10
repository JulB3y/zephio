#define _POSIX_C_SOURCE 200809L

#include "tui_app.h"
#include "tui_terminal.h"
#include "tui_screen.h"
#include "tui_mouse.h"

#include <poll.h>
#include <stdlib.h>
#include <string.h>

TuiApp *tui_app_new(const TuiAppConfig *config)
{
    TuiApp *app = (TuiApp *)calloc(1, sizeof(TuiApp));
    if (!app) return NULL;

    if (config) {
        app->config = *config;
    }

    app->running       = 0;
    app->exit_code     = 0;
    app->overlay_count = 0;

    return app;
}

void tui_app_free(TuiApp *app)
{
    if (app) free(app);
}

void tui_app_stop(TuiApp *app)
{
    if (app) {
        app->running   = 0;
        app->exit_code = 0;
    }
}

static int app_poll_event(TuiApp *app, TuiEvent *event)
{
    if (app->config.tick_rate_ms > 0) {
        struct pollfd pfd = { g_terminal.fd, POLLIN, 0 };
        int ret = poll(&pfd, 1, app->config.tick_rate_ms);

        if (ret == 0) {
            memset(event, 0, sizeof(*event));
            return 0;
        }

        if (ret < 0) {
            TuiResult res = tui_input_poll(event);
            return (res == TUI_OK) ? 1 : -1;
        }
    }

    TuiResult res = tui_input_poll(event);
    if (res != TUI_OK) return -1;
    return 1;
}

int tui_app_run(TuiApp *app)
{
    if (!app) return 1;

    TuiResult res = tui_init();
    if (res != TUI_OK) return 1;

    tui_input_init();
    tui_mouse_enable();

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

    while (app->running) {
        TuiEvent event;
        int has_event = app_poll_event(app, &event);

        if (has_event < 0) {
            continue;
        }

        if (has_event == 0) {
            if (app->config.on_render) {
                app->config.on_render(app, app->config.user_data);
            }
            continue;
        }

        if (event.key == TUI_EVENT_RESIZE) {
            tui_screen_resize(event.size.rows, event.size.cols);

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

        if (event.key == TUI_EVENT_MOUSE) {
            if (app->overlay_count > 0) {
                tui_app_handle_overlay_mouse(app, &event.mouse);
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
            tui_app_handle_overlay_input(app, &event);
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

    tui_input_shutdown();
    tui_shutdown();

    return app->exit_code;
}

TuiResult tui_app_push_overlay(TuiApp *app, TuiWidget *widget)
{
    if (!app || !widget) return TUI_ERR_MEMORY;
    if (app->overlay_count >= TUI_APP_MAX_OVERLAYS) return TUI_ERR_MEMORY;

    app->overlays[app->overlay_count++] = widget;
    widget->dirty = 1;

    if (widget->focusable) {
        tui_widget_focus(widget);
    }

    return TUI_OK;
}

TuiWidget *tui_app_pop_overlay(TuiApp *app)
{
    if (!app || app->overlay_count == 0) return NULL;

    TuiWidget *widget = app->overlays[--app->overlay_count];
    app->overlays[app->overlay_count] = NULL;

    if (widget->focused) {
        tui_widget_blur(widget);
    }

    return widget;
}

TuiWidget *tui_app_top_overlay(TuiApp *app)
{
    if (!app || app->overlay_count == 0) return NULL;
    return app->overlays[app->overlay_count - 1];
}

void tui_app_render_overlays(TuiApp *app)
{
    if (!app) return;
    for (int i = 0; i < app->overlay_count; i++) {
        tui_widget_render(app->overlays[i]);
    }
}

int tui_app_handle_overlay_input(TuiApp *app, const TuiEvent *event)
{
    if (!app || app->overlay_count == 0) return 0;

    TuiWidget *top = app->overlays[app->overlay_count - 1];
    if (!top) return 0;

    if (top->focused || top->focusable) {
        if (!top->focused) tui_widget_focus(top);
        return tui_widget_handle_input(top, event);
    }

    return 0;
}

int tui_app_handle_overlay_mouse(TuiApp *app, const TuiMouseEvent *mouse)
{
    if (!app || app->overlay_count == 0) return 0;

    TuiWidget *top = app->overlays[app->overlay_count - 1];
    if (!top) return 0;

    return tui_widget_handle_mouse(top, mouse);
}
