/**
 * @file toast_demo.c
 * @brief Toast notification demonstration.
 *
 * Demonstrates:
 *   - TuiToastManager: posting toasts with 4 severity levels
 *   - Auto-dismiss with configurable duration
 *   - Fade-in / fade-out animation
 *   - tui_app_toast() convenience function
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
#include "tui_app.h"
#include "tui_button.h"
#include "tui_container.h"
#include "tui_label.h"
#include "tui_list.h"
#include "tui_screen.h"
#include "tui_separator.h"
#include "tui_toast.h"
#include "tui_widget.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    TuiWidget root;
    TuiLabel  title;
    TuiLabel  hint;
    TuiSeparator sep;
    TuiButton btn_info;
    TuiButton btn_success;
    TuiButton btn_warning;
    TuiButton btn_error;
    TuiButton btn_burst;
    TuiButton btn_dismiss;
    TuiSeparator sep2;
    TuiLabel  status;
    TuiList   menu;
    char      status_text[120];
} AppWidgets;

static TuiApp *g_app = NULL;

static void update_status(AppWidgets *w, const char *msg)
{
    snprintf(w->status_text, sizeof(w->status_text), " %s", msg);
    tui_label_set_text(&w->status, w->status_text);
    tui_label_set_colors(&w->status, TUI_COLOR_INDEX(15),
                         TUI_COLOR_INDEX(236));
}

static void on_info_click(TuiWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;
    tui_app_toast(g_app, TUI_TOAST_INFO, "This is an informational message.", 3000);
    update_status(w, "Info toast posted");
}

static void on_success_click(TuiWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;
    tui_app_toast(g_app, TUI_TOAST_SUCCESS, "Operation completed successfully!", 3000);
    update_status(w, "Success toast posted");
}

static void on_warning_click(TuiWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;
    tui_app_toast(g_app, TUI_TOAST_WARNING, "Warning: check your input.", 4000);
    update_status(w, "Warning toast posted");
}

static void on_error_click(TuiWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;
    tui_app_toast(g_app, TUI_TOAST_ERROR, "Error: something went wrong!", 5000);
    update_status(w, "Error toast posted");
}

static void on_burst_click(TuiWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;
    tui_app_toast(g_app, TUI_TOAST_INFO,    "Burst: info message",    4000);
    tui_app_toast(g_app, TUI_TOAST_SUCCESS, "Burst: success message", 4000);
    tui_app_toast(g_app, TUI_TOAST_WARNING, "Burst: warning message", 4000);
    tui_app_toast(g_app, TUI_TOAST_ERROR,   "Burst: error message",   4000);
    update_status(w, "Burst: 4 toasts posted");
}

static void on_dismiss_click(TuiWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;
    tui_toast_dismiss_all(tui_app_get_toasts(g_app));
    update_status(w, "All toasts dismissed");
}

static void on_list_select(TuiWidget *widget, int index, const char *item,
                           void *user_data)
{
    (void)widget;
    (void)index;
    AppWidgets *w = (AppWidgets *)user_data;
    snprintf(w->status_text, sizeof(w->status_text), " Selected: %s", item);
    tui_label_set_text(&w->status, w->status_text);
    tui_label_set_colors(&w->status, TUI_COLOR_INDEX(14),
                         TUI_COLOR_INDEX(236));
}

static void build_widgets(AppWidgets *w, int rows, int cols, TuiContext *ctx)
{
    int uw = cols > 72 ? 68 : cols - 4;
    int ux = (cols - uw) / 2;
    if (ux < 1) ux = 1;
    int y = 2;

    tui_widget_init_ctx(&w->root, 0, 0, cols, rows, NULL, ctx, NULL);

    tui_label_init_ctx(&w->title, ctx, ux, y, uw, 1,
                   "Toast Notification Demo  |  i: info  s: success  w: warning  e: error");
    tui_label_set_colors(&w->title, TUI_COLOR_INDEX(14), TUI_COLOR_INDEX(0));
    tui_label_set_attr(&w->title, TUI_ATTR_BOLD);
    tui_widget_add_child(&w->root, &w->title.base);
    y += 2;

    tui_label_init_ctx(&w->hint, ctx, ux, y, uw, 1,
                   "Click buttons or press i/s/w/e keys. b = burst, d = dismiss all.");
    tui_label_set_colors(&w->hint, TUI_COLOR_INDEX(8), TUI_COLOR_INDEX(0));
    tui_widget_add_child(&w->root, &w->hint.base);
    y += 2;

    tui_separator_init_h_ctx(&w->sep, ctx, ux, y, uw);
    tui_separator_set_colors(&w->sep, TUI_COLOR_INDEX(8), TUI_COLOR_INDEX(0));
    tui_widget_add_child(&w->root, &w->sep.base);
    y += 1;

    {
        int bw = (uw - 4) / 3;
        if (bw < 12) bw = 12;
        int col = ux;

        tui_button_init_ctx(&w->btn_info, ctx, col, y, bw, 1, "Info");
        tui_button_set_colors(&w->btn_info,
                              TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4),
                              TUI_COLOR_INDEX(0), TUI_COLOR_INDEX(12));
        tui_button_set_on_click(&w->btn_info, on_info_click, w);
        w->btn_info.base.focusable = 1;
        tui_widget_add_child(&w->root, &w->btn_info.base);
        col += bw + 2;

        tui_button_init_ctx(&w->btn_success, ctx, col, y, bw, 1, "Success");
        tui_button_set_colors(&w->btn_success,
                              TUI_COLOR_INDEX(0), TUI_COLOR_INDEX(2),
                              TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(10));
        tui_button_set_on_click(&w->btn_success, on_success_click, w);
        w->btn_success.base.focusable = 1;
        tui_widget_add_child(&w->root, &w->btn_success.base);
        col += bw + 2;

        tui_button_init_ctx(&w->btn_warning, ctx, col, y, bw, 1, "Warning");
        tui_button_set_colors(&w->btn_warning,
                              TUI_COLOR_INDEX(0), TUI_COLOR_INDEX(3),
                              TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(11));
        tui_button_set_on_click(&w->btn_warning, on_warning_click, w);
        w->btn_warning.base.focusable = 1;
        tui_widget_add_child(&w->root, &w->btn_warning.base);
    }
    y += 2;

    {
        int bw = (uw - 4) / 3;
        if (bw < 12) bw = 12;
        int col = ux;

        tui_button_init_ctx(&w->btn_error, ctx, col, y, bw, 1, "Error");
        tui_button_set_colors(&w->btn_error,
                              TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(1),
                              TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(9));
        tui_button_set_on_click(&w->btn_error, on_error_click, w);
        w->btn_error.base.focusable = 1;
        tui_widget_add_child(&w->root, &w->btn_error.base);
        col += bw + 2;

        tui_button_init_ctx(&w->btn_burst, ctx, col, y, bw, 1, "Burst All");
        tui_button_set_colors(&w->btn_burst,
                              TUI_COLOR_INDEX(0), TUI_COLOR_INDEX(5),
                              TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(13));
        tui_button_set_on_click(&w->btn_burst, on_burst_click, w);
        w->btn_burst.base.focusable = 1;
        tui_widget_add_child(&w->root, &w->btn_burst.base);
        col += bw + 2;

        tui_button_init_ctx(&w->btn_dismiss, ctx, col, y, bw, 1, "Dismiss All");
        tui_button_set_colors(&w->btn_dismiss,
                              TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(8),
                              TUI_COLOR_INDEX(0), TUI_COLOR_INDEX(7));
        tui_button_set_on_click(&w->btn_dismiss, on_dismiss_click, w);
        w->btn_dismiss.base.focusable = 1;
        tui_widget_add_child(&w->root, &w->btn_dismiss.base);
    }
    y += 2;

    tui_separator_init_h_ctx(&w->sep2, ctx, ux, y, uw);
    tui_separator_set_colors(&w->sep2, TUI_COLOR_INDEX(8), TUI_COLOR_INDEX(0));
    tui_widget_add_child(&w->root, &w->sep2.base);
    y += 1;

    {
        int list_h = rows - y - 3;
        if (list_h < 3) list_h = 3;
        if (list_h > 6) list_h = 6;

        tui_list_init_ctx(&w->menu, ctx, ux, y, uw, list_h);
        tui_list_set_colors(&w->menu, TUI_COLOR_INDEX(15),
                            TUI_COLOR_INDEX(234), TUI_COLOR_INDEX(0),
                            TUI_COLOR_INDEX(12));
        tui_list_set_on_select(&w->menu, on_list_select, w);
        w->menu.base.focusable = 1;
        tui_list_add_item(&w->menu, "Show info toast (i)");
        tui_list_add_item(&w->menu, "Show success toast (s)");
        tui_list_add_item(&w->menu, "Show warning toast (w)");
        tui_list_add_item(&w->menu, "Show error toast (e)");
        tui_list_add_item(&w->menu, "Burst: all 4 at once (b)");
        tui_list_add_item(&w->menu, "Dismiss all toasts (d)");
        tui_widget_add_child(&w->root, &w->menu.base);
    }

    tui_label_init_ctx(&w->status, ctx, 1, rows - 1, cols - 2, 1,
                   " Press i/s/w/e or click buttons  |  b: burst  d: dismiss  q: quit");
    tui_label_set_colors(&w->status, TUI_COLOR_INDEX(15),
                         TUI_COLOR_INDEX(236));
    tui_widget_add_child(&w->root, &w->status.base);
}

static void destroy_widgets(AppWidgets *w)
{
    for (int i = w->root.child_count - 1; i >= 0; i--) {
        tui_widget_destroy(w->root.children[i]);
    }
    tui_widget_remove_all_children(&w->root);
}

static int on_init(TuiApp *app, void *user_data)
{
    g_app = app;
    AppWidgets *w = (AppWidgets *)user_data;
    memset(w, 0, sizeof(*w));

    TuiSize size = tui_screen_size(app->ctx);
    build_widgets(w, size.rows, size.cols, app->ctx);
    return 0;
}

static int on_resize(TuiApp *app, int rows, int cols, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;
    destroy_widgets(w);
    build_widgets(w, rows, cols, app->ctx);
    return 0;
}

static int on_render(TuiApp *app, void *user_data)
{
    AppWidgets *w = (AppWidgets *)user_data;
    TuiSize size = tui_screen_size(app->ctx);

    tui_screen_clear(app->ctx);
    tui_screen_fill(app->ctx, 0, 0, size.cols, 1, " ",
                    TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4), TUI_ATTR_BOLD);
    tui_screen_write(app->ctx, 0, 2,
                     "Toast Demo  |  Notification Overlays",
                     TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4), TUI_ATTR_BOLD);
    tui_screen_fill(app->ctx, size.rows - 1, 0, size.cols, 1, " ",
                    TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(236), TUI_ATTR_NONE);

    tui_widget_render(&w->root);
    tui_app_render_overlays(app);
    tui_app_render_toasts(app);
    tui_screen_render(app->ctx);
    return 0;
}

static int on_input(TuiApp *app, const TuiEvent *event, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;

    if (event->key == TUI_KEY_ESCAPE || event->key == TUI_KEY_CTRL_C) {
        return 1;
    }

    if (event->key == TUI_KEY_TAB) {
        if (event->modifiers & TUI_MOD_SHIFT) {
            tui_widget_focus_prev(&w->root);
        } else {
            tui_widget_focus_next(&w->root);
        }
        tui_widget_mark_dirty_recursive(&w->root);
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
    if (event->codepoint == 'q' && event->modifiers == TUI_MOD_NONE) {
        return 1;
    }

    TuiWidget *focused = tui_widget_get_focused(&w->root);
    if (focused) {
        tui_widget_handle_input(focused, event);
    }

    return 0;
}

static int on_mouse(TuiApp *app, const TuiMouseEvent *mouse, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;

    if (mouse->action == TUI_MOUSE_PRESS && mouse->button == TUI_MOUSE_BTN_LEFT) {
        TuiWidget *target = tui_widget_find_at(&w->root, mouse->row,
                                               mouse->col);
        if (target && target->focusable) {
            tui_widget_focus(target);
        }
    }

    tui_widget_handle_mouse(&w->root, mouse);
    tui_widget_mark_dirty_recursive(&w->root);
    return 0;
}

static void on_shutdown(TuiApp *app, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;
    destroy_widgets(w);
}

int main(void)
{
    AppWidgets widgets;

    TuiContext ctx;

    TuiAppConfig config = {
        .on_init     = on_init,
        .on_resize   = on_resize,
        .on_render   = on_render,
        .on_input    = on_input,
        .on_mouse    = on_mouse,
        .on_shutdown = on_shutdown,
        .user_data   = &widgets,
        .tick_rate_ms = 50
    };

    TuiApp *app = tui_app_new(&ctx, &config);
    if (!app) {
        fprintf(stderr, "Failed to create app\n");
        return 1;
    }

    int ret = tui_app_run(app);
    tui_app_free(app);
    return ret;
}
