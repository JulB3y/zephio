/**
 * @file dialog_demo.c
 * @brief Dialog/Modal and clipboard demonstration.
 *
 * Demonstrates:
 *   - ZephioDialog: modal popup with buttons (OK/Cancel, Yes/No)
 *   - Overlay stack: zephio_app_push_overlay / zephio_app_pop_overlay
 *   - Clipboard: zephio_clipboard_copy (OSC 52)
 *   - Input blocking: main widgets are inert while a dialog is open
 *
 * Press:
 *   d  - open a confirm dialog
 *   a  - open an alert dialog
 *   c  - copy status text to clipboard
 *   q/Esc - quit
 */

#define _POSIX_C_SOURCE 200809L

#include "zephio.h"
#include "zephio_app.h"
#include "zephio_button.h"
#include "zephio_clipboard.h"
#include "zephio_container.h"
#include "zephio_dialog.h"
#include "zephio_label.h"
#include "zephio_list.h"
#include "zephio_screen.h"
#include "zephio_separator.h"
#include "zephio_widget.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    ZephioWidget root;
    ZephioLabel title;
    ZephioLabel hint;
    ZephioSeparator sep;
    ZephioButton btn_dialog;
    ZephioButton btn_alert;
    ZephioButton btn_clipboard;
    ZephioSeparator sep2;
    ZephioLabel status;
    ZephioList menu;
    char status_text[120];
} AppWidgets;

static ZephioDialog g_confirm_dialog;
static ZephioDialog g_alert_dialog;
static ZephioApp *g_app = NULL;
static int g_dialog_active = 0;
static char g_clipboard_buf[256];

static void update_status(AppWidgets *w, const char *msg)
{
    snprintf(w->status_text, sizeof(w->status_text), " %s", msg);
    zephio_label_set_text(&w->status, w->status_text);
    zephio_label_set_colors(&w->status, ZEPHIO_COLOR_INDEX(15),
                         ZEPHIO_COLOR_INDEX(236));
}

static void on_confirm_response(ZephioDialog *dialog, int button_index,
                                void *user_data)
{
    (void)dialog;
    AppWidgets *w = (AppWidgets *)user_data;

    if (button_index < 0) {
        update_status(w, "Confirm cancelled (Escape)");
    } else if (button_index == 0) {
        update_status(w, "Confirmed: Yes");
    } else {
        update_status(w, "Confirmed: No");
    }

    zephio_app_pop_overlay(g_app);
    g_dialog_active = 0;
}

static void on_alert_response(ZephioDialog *dialog, int button_index,
                              void *user_data)
{
    (void)dialog;
    (void)button_index;
    AppWidgets *w = (AppWidgets *)user_data;
    update_status(w, "Alert dismissed");
    zephio_app_pop_overlay(g_app);
    g_dialog_active = 0;
}

static void on_dialog_click(ZephioWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;

    if (g_dialog_active) return;

    zephio_dialog_init_ctx(&g_confirm_dialog, g_app->ctx, "Confirm",
                        "Are you sure you want to proceed?\n"
                        "This action cannot be undone.");
    zephio_dialog_add_button(&g_confirm_dialog, "Yes");
    zephio_dialog_add_button(&g_confirm_dialog, "No");
    zephio_dialog_set_on_button(&g_confirm_dialog, on_confirm_response, w);
    zephio_dialog_center(g_app->ctx, &g_confirm_dialog);

    g_dialog_active = 1;
    zephio_app_push_overlay(g_app, &g_confirm_dialog.base);
}

static void on_alert_click(ZephioWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;

    if (g_dialog_active) return;

    zephio_dialog_init_ctx(&g_alert_dialog, g_app->ctx, "Information",
                        "This is a modal alert dialog.\n"
                        "Press Enter or click OK to dismiss.\n"
                        "Press Escape to cancel.");
    zephio_dialog_add_button(&g_alert_dialog, "OK");
    zephio_dialog_set_on_button(&g_alert_dialog, on_alert_response, w);
    zephio_dialog_center(g_app->ctx, &g_alert_dialog);

    g_dialog_active = 1;
    zephio_app_push_overlay(g_app, &g_alert_dialog.base);
}

static void on_clipboard_click(ZephioWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;

    snprintf(g_clipboard_buf, sizeof(g_clipboard_buf),
             "Dialog Demo - Status: %s", w->status_text);

    int ret = zephio_clipboard_copy(g_app->ctx, g_clipboard_buf);
    if (ret == 0) {
        update_status(w, "Copied to clipboard via OSC 52");
    } else {
        update_status(w, "Clipboard copy failed");
    }
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
    int uw = cols > 64 ? 60 : cols - 4;
    int ux = (cols - uw) / 2;
    if (ux < 1) ux = 1;
    int y = 2;

    zephio_widget_init_ctx(&w->root, 0, 0, cols, rows, NULL, ctx, NULL);

    zephio_label_init_ctx(&w->title, ctx, ux, y, uw, 1,
                   "Dialog / Modal Demo  |  d: confirm  a: alert  c: clipboard");
    zephio_label_set_colors(&w->title, ZEPHIO_COLOR_INDEX(14), ZEPHIO_COLOR_INDEX(0));
    zephio_label_set_attr(&w->title, ZEPHIO_ATTR_BOLD);
    zephio_widget_add_child(&w->root, &w->title.base);
    y += 2;

    zephio_label_init_ctx(&w->hint, ctx, ux, y, uw, 1,
                   "Click buttons or press d/a/c keys. Tab cycles focus.");
    zephio_label_set_colors(&w->hint, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&w->root, &w->hint.base);
    y += 2;

    zephio_separator_init_h_ctx(&w->sep, ctx, ux, y, uw);
    zephio_separator_set_colors(&w->sep, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&w->root, &w->sep.base);
    y += 1;

    {
        int bw = (uw - 4) / 3;
        if (bw < 10) bw = 10;

        zephio_button_init_ctx(&w->btn_dialog, ctx, ux, y, bw, 1, "Confirm");
        zephio_button_set_colors(&w->btn_dialog,
                              ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(2),
                              ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(10));
        zephio_button_set_on_click(&w->btn_dialog, on_dialog_click, w);
        w->btn_dialog.base.focusable = 1;
        zephio_widget_add_child(&w->root, &w->btn_dialog.base);

        zephio_button_init_ctx(&w->btn_alert, ctx, ux + bw + 2, y, bw, 1, "Alert");
        zephio_button_set_colors(&w->btn_alert,
                              ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(4),
                              ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(12));
        zephio_button_set_on_click(&w->btn_alert, on_alert_click, w);
        w->btn_alert.base.focusable = 1;
        zephio_widget_add_child(&w->root, &w->btn_alert.base);

        zephio_button_init_ctx(&w->btn_clipboard, ctx, ux + 2 * (bw + 2), y, bw, 1,
                             "Clipboard");
        zephio_button_set_colors(&w->btn_clipboard,
                              ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(5),
                              ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(13));
        zephio_button_set_on_click(&w->btn_clipboard, on_clipboard_click, w);
        w->btn_clipboard.base.focusable = 1;
        zephio_widget_add_child(&w->root, &w->btn_clipboard.base);
    }
    y += 2;

    zephio_separator_init_h_ctx(&w->sep2, ctx, ux, y, uw);
    zephio_separator_set_colors(&w->sep2, ZEPHIO_COLOR_INDEX(8),
                             ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&w->root, &w->sep2.base);
    y += 1;

    {
        int list_h = rows - y - 3;
        if (list_h < 3) list_h = 3;
        if (list_h > 8) list_h = 8;

        zephio_list_init_ctx(&w->menu, ctx, ux, y, uw, list_h);
        zephio_list_set_colors(&w->menu, ZEPHIO_COLOR_INDEX(15),
                            ZEPHIO_COLOR_INDEX(234), ZEPHIO_COLOR_INDEX(0),
                            ZEPHIO_COLOR_INDEX(12));
        zephio_list_set_on_select(&w->menu, on_list_select, w);
        w->menu.base.focusable = 1;
        zephio_list_add_item(&w->menu, "Open confirm dialog (d)");
        zephio_list_add_item(&w->menu, "Show alert dialog (a)");
        zephio_list_add_item(&w->menu, "Copy to clipboard (c)");
        zephio_list_add_item(&w->menu, "About this demo");
        zephio_widget_add_child(&w->root, &w->menu.base);
    }

    zephio_label_init_ctx(&w->status, ctx, 1, rows - 1, cols - 2, 1,
                   " Press d/a/c or click buttons  |  q/Esc to quit");
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

    ZephioSize size = zephio_screen_size(g_app->ctx);
    build_widgets(w, size.rows, size.cols, g_app->ctx);
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
    ZephioSize size = zephio_screen_size(g_app->ctx);

    zephio_screen_clear(g_app->ctx);
    zephio_screen_fill(g_app->ctx, 0, 0, size.cols, 1, " ",
                    ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    zephio_screen_write(g_app->ctx, 0, 2,
                     "Dialog Demo  |  Modal overlays + Clipboard",
                     ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    zephio_screen_fill(g_app->ctx, size.rows - 1, 0, size.cols, 1, " ",
                    ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236), ZEPHIO_ATTR_NONE);

    zephio_widget_render(&w->root);
    zephio_app_render_overlays(app);
    zephio_screen_render(g_app->ctx);
    return 0;
}

static int on_input(ZephioApp *app, const ZephioEvent *event, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;

    if (event->key == ZEPHIO_KEY_ESCAPE || event->key == ZEPHIO_KEY_CTRL_C) {
        if (g_dialog_active) {
            return 0;
        }
        return 1;
    }

    if (g_dialog_active) return 0;

    if (event->key == ZEPHIO_KEY_TAB) {
        if (event->modifiers & ZEPHIO_MOD_SHIFT) {
            zephio_widget_focus_prev(&w->root);
        } else {
            zephio_widget_focus_next(&w->root);
        }
        zephio_widget_mark_dirty_recursive(&w->root);
        return 0;
    }

    if (event->codepoint == 'd') {
        on_dialog_click(NULL, w);
        return 0;
    }

    if (event->codepoint == 'a') {
        on_alert_click(NULL, w);
        return 0;
    }

    if (event->codepoint == 'c') {
        on_clipboard_click(NULL, w);
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

    if (g_dialog_active) {
        zephio_dialog_init_ctx(&g_confirm_dialog, g_app->ctx, NULL, NULL);
        zephio_dialog_init_ctx(&g_alert_dialog, g_app->ctx, NULL, NULL);
    }
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
