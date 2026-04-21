/**
 * @file toast_demo.c
 * @brief Toast notification demonstration.
 *
 * Demonstrates:
 *   - ZephioToastManager: posting toasts with 4 severity levels
 *   - Auto-dismiss with configurable duration
 *   - Fade-in / fade-out animation
 *   - zephio_app_toast() convenience function
 *
 * Press:
 *   i  - info toast
 *   s  - success toast
 *   w  - warning toast
 *   e  - error toast
 *   b  - burst (one of each)
 *   d  - dismiss all
 *   q/Esc - quit
 */

#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "zephio_app.h"
#include "zephio_button.h"
#include "zephio_container.h"
#include "zephio_label.h"
#include "zephio_list.h"
#include "zephio_screen.h"
#include "zephio_separator.h"
#include "zephio_toast.h"
#include "zephio_widget.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    ZephioWidget root;
    ZephioLabel  title;
    ZephioLabel  hint;
    ZephioSeparator sep;
    ZephioButton btn_info;
    ZephioButton btn_success;
    ZephioButton btn_warning;
    ZephioButton btn_error;
    ZephioButton btn_burst;
    ZephioButton btn_dismiss;
    ZephioSeparator sep2;
    ZephioLabel  status;
    ZephioList   menu;
    char      status_text[120];
} AppWidgets;

static ZephioApp *g_app = NULL;

static void update_status(AppWidgets *w, const char *msg)
{
    snprintf(w->status_text, sizeof(w->status_text), " %s", msg);
    zephio_label_set_text(&w->status, w->status_text);
    zephio_label_set_colors(&w->status, ZEPHIO_COLOR_INDEX(15),
                         ZEPHIO_COLOR_INDEX(236));
}

static void on_info_click(ZephioWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;
    zephio_app_toast(g_app, ZEPHIO_TOAST_INFO, "This is an informational message.", 3000);
    update_status(w, "Info toast posted");
}

static void on_success_click(ZephioWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;
    zephio_app_toast(g_app, ZEPHIO_TOAST_SUCCESS, "Operation completed successfully!", 3000);
    update_status(w, "Success toast posted");
}

static void on_warning_click(ZephioWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;
    zephio_app_toast(g_app, ZEPHIO_TOAST_WARNING, "Warning: check your input.", 4000);
    update_status(w, "Warning toast posted");
}

static void on_error_click(ZephioWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;
    zephio_app_toast(g_app, ZEPHIO_TOAST_ERROR, "Error: something went wrong!", 5000);
    update_status(w, "Error toast posted");
}

static void on_burst_click(ZephioWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;
    zephio_app_toast(g_app, ZEPHIO_TOAST_INFO,    "Burst: info message",    4000);
    zephio_app_toast(g_app, ZEPHIO_TOAST_SUCCESS, "Burst: success message", 4000);
    zephio_app_toast(g_app, ZEPHIO_TOAST_WARNING, "Burst: warning message", 4000);
    zephio_app_toast(g_app, ZEPHIO_TOAST_ERROR,   "Burst: error message",   4000);
    update_status(w, "Burst: 4 toasts posted");
}

static void on_dismiss_click(ZephioWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;
    zephio_toast_dismiss_all(zephio_app_get_toasts(g_app));
    update_status(w, "All toasts dismissed");
}

static void on_list_select(ZephioWidget *widget, int index, const char *item,
                           void *user_data)
{
    (void)widget;
    (void)index;
    AppWidgets *w = (AppWidgets *)user_data;
    snprintf(w->status_text, sizeof(w->status_text), " Selected: %s", item);
    zephio_label_set_text(&w->status, w->status_text);
    zephio_label_set_colors(&w->status, ZEPHIO_COLOR_INDEX(14),
                         ZEPHIO_COLOR_INDEX(236));
}

static void build_widgets(AppWidgets *w, int rows, int cols, ZephioContext *ctx)
{
    int uw = cols > 72 ? 68 : cols - 4;
    int ux = (cols - uw) / 2;
    if (ux < 1) ux = 1;
    int y = 2;

    zephio_widget_init_ctx(&w->root, 0, 0, cols, rows, NULL, ctx, NULL);

    zephio_label_init_ctx(&w->title, ctx, ux, y, uw, 1,
                   "Toast Notification Demo  |  i: info  s: success  w: warning  e: error");
    zephio_label_set_colors(&w->title, ZEPHIO_COLOR_INDEX(14), ZEPHIO_COLOR_INDEX(0));
    zephio_label_set_attr(&w->title, ZEPHIO_ATTR_BOLD);
    zephio_widget_add_child(&w->root, &w->title.base);
    y += 2;

    zephio_label_init_ctx(&w->hint, ctx, ux, y, uw, 1,
                   "Click buttons or press i/s/w/e keys. b = burst, d = dismiss all.");
    zephio_label_set_colors(&w->hint, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&w->root, &w->hint.base);
    y += 2;

    zephio_separator_init_h_ctx(&w->sep, ctx, ux, y, uw);
    zephio_separator_set_colors(&w->sep, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&w->root, &w->sep.base);
    y += 1;

    {
        int bw = (uw - 4) / 3;
        if (bw < 12) bw = 12;
        int col = ux;

        zephio_button_init_ctx(&w->btn_info, ctx, col, y, bw, 1, "Info");
        zephio_button_set_colors(&w->btn_info,
                              ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4),
                              ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(12));
        zephio_button_set_on_click(&w->btn_info, on_info_click, w);
        w->btn_info.base.focusable = 1;
        zephio_widget_add_child(&w->root, &w->btn_info.base);
        col += bw + 2;

        zephio_button_init_ctx(&w->btn_success, ctx, col, y, bw, 1, "Success");
        zephio_button_set_colors(&w->btn_success,
                              ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(2),
                              ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(10));
        zephio_button_set_on_click(&w->btn_success, on_success_click, w);
        w->btn_success.base.focusable = 1;
        zephio_widget_add_child(&w->root, &w->btn_success.base);
        col += bw + 2;

        zephio_button_init_ctx(&w->btn_warning, ctx, col, y, bw, 1, "Warning");
        zephio_button_set_colors(&w->btn_warning,
                              ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(3),
                              ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(11));
        zephio_button_set_on_click(&w->btn_warning, on_warning_click, w);
        w->btn_warning.base.focusable = 1;
        zephio_widget_add_child(&w->root, &w->btn_warning.base);
    }
    y += 2;

    {
        int bw = (uw - 4) / 3;
        if (bw < 12) bw = 12;
        int col = ux;

        zephio_button_init_ctx(&w->btn_error, ctx, col, y, bw, 1, "Error");
        zephio_button_set_colors(&w->btn_error,
                              ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(1),
                              ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(9));
        zephio_button_set_on_click(&w->btn_error, on_error_click, w);
        w->btn_error.base.focusable = 1;
        zephio_widget_add_child(&w->root, &w->btn_error.base);
        col += bw + 2;

        zephio_button_init_ctx(&w->btn_burst, ctx, col, y, bw, 1, "Burst All");
        zephio_button_set_colors(&w->btn_burst,
                              ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(5),
                              ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(13));
        zephio_button_set_on_click(&w->btn_burst, on_burst_click, w);
        w->btn_burst.base.focusable = 1;
        zephio_widget_add_child(&w->root, &w->btn_burst.base);
        col += bw + 2;

        zephio_button_init_ctx(&w->btn_dismiss, ctx, col, y, bw, 1, "Dismiss All");
        zephio_button_set_colors(&w->btn_dismiss,
                              ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(8),
                              ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(7));
        zephio_button_set_on_click(&w->btn_dismiss, on_dismiss_click, w);
        w->btn_dismiss.base.focusable = 1;
        zephio_widget_add_child(&w->root, &w->btn_dismiss.base);
    }
    y += 2;

    zephio_separator_init_h_ctx(&w->sep2, ctx, ux, y, uw);
    zephio_separator_set_colors(&w->sep2, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&w->root, &w->sep2.base);
    y += 1;

    {
        int list_h = rows - y - 3;
        if (list_h < 3) list_h = 3;
        if (list_h > 6) list_h = 6;

        zephio_list_init_ctx(&w->menu, ctx, ux, y, uw, list_h);
        zephio_list_set_colors(&w->menu, ZEPHIO_COLOR_INDEX(15),
                            ZEPHIO_COLOR_INDEX(234), ZEPHIO_COLOR_INDEX(0),
                            ZEPHIO_COLOR_INDEX(12));
        zephio_list_set_on_select(&w->menu, on_list_select, w);
        w->menu.base.focusable = 1;
        zephio_list_add_item(&w->menu, "Show info toast (i)");
        zephio_list_add_item(&w->menu, "Show success toast (s)");
        zephio_list_add_item(&w->menu, "Show warning toast (w)");
        zephio_list_add_item(&w->menu, "Show error toast (e)");
        zephio_list_add_item(&w->menu, "Burst: all 4 at once (b)");
        zephio_list_add_item(&w->menu, "Dismiss all toasts (d)");
        zephio_widget_add_child(&w->root, &w->menu.base);
    }

    zephio_label_init_ctx(&w->status, ctx, 1, rows - 1, cols - 2, 1,
                   " Press i/s/w/e or click buttons  |  b: burst  d: dismiss  q: quit");
    zephio_label_set_colors(&w->status, ZEPHIO_COLOR_INDEX(15),
                         ZEPHIO_COLOR_INDEX(236));
    zephio_widget_add_child(&w->root, &w->status.base);
}

static void destroy_widgets(AppWidgets *w)
{
    for (int i = w->root.child_count - 1; i >= 0; i--) {
        zephio_widget_destroy(w->root.children[i]);
    }
    zephio_widget_remove_all_children(&w->root);
}

static int on_init(ZephioApp *app, void *user_data)
{
    g_app = app;
    AppWidgets *w = (AppWidgets *)user_data;
    memset(w, 0, sizeof(*w));

    ZephioSize size = zephio_screen_size(app->ctx);
    build_widgets(w, size.rows, size.cols, app->ctx);
    return 0;
}

static int on_resize(ZephioApp *app, int rows, int cols, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;
    destroy_widgets(w);
    build_widgets(w, rows, cols, app->ctx);
    return 0;
}

static int on_render(ZephioApp *app, void *user_data)
{
    AppWidgets *w = (AppWidgets *)user_data;
    ZephioSize size = zephio_screen_size(app->ctx);

    zephio_screen_clear(app->ctx);
    zephio_screen_fill(app->ctx, 0, 0, size.cols, 1, " ",
                    ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    zephio_screen_write(app->ctx, 0, 2,
                     "Toast Demo  |  Notification Overlays",
                     ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    zephio_screen_fill(app->ctx, size.rows - 1, 0, size.cols, 1, " ",
                    ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236), ZEPHIO_ATTR_NONE);

    zephio_widget_render(&w->root);
    zephio_app_render_overlays(app);
    zephio_app_render_toasts(app);
    zephio_screen_render(app->ctx);
    return 0;
}

static int on_input(ZephioApp *app, const ZephioEvent *event, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;

    if (event->key == ZEPHIO_KEY_ESCAPE || event->key == ZEPHIO_KEY_CTRL_C) {
        return 1;
    }

    if (event->key == ZEPHIO_KEY_TAB) {
        if (event->modifiers & ZEPHIO_MOD_SHIFT) {
            zephio_widget_focus_prev(&w->root);
        } else {
            zephio_widget_focus_next(&w->root);
        }
        zephio_widget_mark_dirty_recursive(&w->root);
        return 0;
    }

    if (event->codepoint == 'i') {
        on_info_click(NULL, w);
        return 0;
    }
    if (event->codepoint == 's') {
        on_success_click(NULL, w);
        return 0;
    }
    if (event->codepoint == 'w') {
        on_warning_click(NULL, w);
        return 0;
    }
    if (event->codepoint == 'e') {
        on_error_click(NULL, w);
        return 0;
    }
    if (event->codepoint == 'b') {
        on_burst_click(NULL, w);
        return 0;
    }
    if (event->codepoint == 'd') {
        on_dismiss_click(NULL, w);
        return 0;
    }
    if (event->codepoint == 'q' && event->modifiers == ZEPHIO_MOD_NONE) {
        return 1;
    }

    ZephioWidget *focused = zephio_widget_get_focused(&w->root);
    if (focused) {
        zephio_widget_handle_input(focused, event);
    }

    return 0;
}

static int on_mouse(ZephioApp *app, const ZephioMouseEvent *mouse, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;

    if (mouse->action == ZEPHIO_MOUSE_PRESS && mouse->button == ZEPHIO_MOUSE_BTN_LEFT) {
        ZephioWidget *target = zephio_widget_find_at(&w->root, mouse->row,
                                               mouse->col);
        if (target && target->focusable) {
            zephio_widget_focus(target);
        }
    }

    zephio_widget_handle_mouse(&w->root, mouse);
    zephio_widget_mark_dirty_recursive(&w->root);
    return 0;
}

static void on_shutdown(ZephioApp *app, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;
    destroy_widgets(w);
}

int main(void)
{
    AppWidgets widgets;

    ZephioContext ctx;

    ZephioAppConfig config = {
        .on_init     = on_init,
        .on_resize   = on_resize,
        .on_render   = on_render,
        .on_input    = on_input,
        .on_mouse    = on_mouse,
        .on_shutdown = on_shutdown,
        .user_data   = &widgets,
        .tick_rate_ms = 50
    };

    ZephioApp *app = zephio_app_new(&ctx, &config);
    if (!app) {
        fprintf(stderr, "Failed to create app\n");
        return 1;
    }

    int ret = zephio_app_run(app);
    zephio_app_free(app);
    return ret;
}
