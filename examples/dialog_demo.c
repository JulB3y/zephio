/**
 * @file dialog_demo.c
 * @brief Dialog/Modal and clipboard demonstration.
 *
 * Demonstrates:
 *   - TuiDialog: modal popup with buttons (OK/Cancel, Yes/No)
 *   - Overlay stack: tui_app_push_overlay / tui_app_pop_overlay
 *   - Clipboard: tui_clipboard_copy (OSC 52)
 *   - Input blocking: main widgets are inert while a dialog is open
 *
 * Press:
 *   d  - open a confirm dialog
 *   a  - open an alert dialog
 *   c  - copy status text to clipboard
 *   q/Esc - quit
 */

#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "tui_app.h"
#include "tui_button.h"
#include "tui_clipboard.h"
#include "tui_container.h"
#include "tui_dialog.h"
#include "tui_label.h"
#include "tui_list.h"
#include "tui_screen.h"
#include "tui_separator.h"
#include "tui_widget.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    TuiWidget root;
    TuiLabel title;
    TuiLabel hint;
    TuiSeparator sep;
    TuiButton btn_dialog;
    TuiButton btn_alert;
    TuiButton btn_clipboard;
    TuiSeparator sep2;
    TuiLabel status;
    TuiList menu;
    char status_text[120];
} AppWidgets;

static TuiDialog g_confirm_dialog;
static TuiDialog g_alert_dialog;
static TuiApp *g_app = NULL;
static int g_dialog_active = 0;
static char g_clipboard_buf[256];

static void update_status(AppWidgets *w, const char *msg)
{
    snprintf(w->status_text, sizeof(w->status_text), " %s", msg);
    tui_label_set_text(&w->status, w->status_text);
    tui_label_set_colors(&w->status, TUI_COLOR_INDEX(15),
                         TUI_COLOR_INDEX(236));
}

static void on_confirm_response(TuiDialog *dialog, int button_index,
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

    tui_app_pop_overlay(g_app);
    g_dialog_active = 0;
}

static void on_alert_response(TuiDialog *dialog, int button_index,
                              void *user_data)
{
    (void)dialog;
    (void)button_index;
    AppWidgets *w = (AppWidgets *)user_data;
    update_status(w, "Alert dismissed");
    tui_app_pop_overlay(g_app);
    g_dialog_active = 0;
}

static void on_dialog_click(TuiWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;

    if (g_dialog_active) return;

    tui_dialog_init(&g_confirm_dialog, "Confirm",
                    "Are you sure you want to proceed?\n"
                    "This action cannot be undone.");
    tui_dialog_add_button(&g_confirm_dialog, "Yes");
    tui_dialog_add_button(&g_confirm_dialog, "No");
    tui_dialog_set_on_button(&g_confirm_dialog, on_confirm_response, w);
    tui_dialog_center(&g_confirm_dialog);

    g_dialog_active = 1;
    tui_app_push_overlay(g_app, &g_confirm_dialog.base);
}

static void on_alert_click(TuiWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;

    if (g_dialog_active) return;

    tui_dialog_init(&g_alert_dialog, "Information",
                    "This is a modal alert dialog.\n"
                    "Press Enter or click OK to dismiss.\n"
                    "Press Escape to cancel.");
    tui_dialog_add_button(&g_alert_dialog, "OK");
    tui_dialog_set_on_button(&g_alert_dialog, on_alert_response, w);
    tui_dialog_center(&g_alert_dialog);

    g_dialog_active = 1;
    tui_app_push_overlay(g_app, &g_alert_dialog.base);
}

static void on_clipboard_click(TuiWidget *widget, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;

    snprintf(g_clipboard_buf, sizeof(g_clipboard_buf),
             "Dialog Demo - Status: %s", w->status_text);

    int ret = tui_clipboard_copy(g_clipboard_buf);
    if (ret == 0) {
        update_status(w, "Copied to clipboard via OSC 52");
    } else {
        update_status(w, "Clipboard copy failed");
    }
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

static void build_widgets(AppWidgets *w, int rows, int cols)
{
    int uw = cols > 64 ? 60 : cols - 4;
    int ux = (cols - uw) / 2;
    if (ux < 1) ux = 1;
    int y = 2;

    tui_widget_init(&w->root, 0, 0, cols, rows, NULL, NULL);

    tui_label_init(&w->title, ux, y, uw, 1,
                   "Dialog / Modal Demo  |  d: confirm  a: alert  c: clipboard");
    tui_label_set_colors(&w->title, TUI_COLOR_INDEX(14), TUI_COLOR_INDEX(0));
    tui_label_set_attr(&w->title, TUI_ATTR_BOLD);
    tui_widget_add_child(&w->root, &w->title.base);
    y += 2;

    tui_label_init(&w->hint, ux, y, uw, 1,
                   "Click buttons or press d/a/c keys. Tab cycles focus.");
    tui_label_set_colors(&w->hint, TUI_COLOR_INDEX(8), TUI_COLOR_INDEX(0));
    tui_widget_add_child(&w->root, &w->hint.base);
    y += 2;

    tui_separator_init_h(&w->sep, ux, y, uw);
    tui_separator_set_colors(&w->sep, TUI_COLOR_INDEX(8), TUI_COLOR_INDEX(0));
    tui_widget_add_child(&w->root, &w->sep.base);
    y += 1;

    {
        int bw = (uw - 4) / 3;
        if (bw < 10) bw = 10;

        tui_button_init(&w->btn_dialog, ux, y, bw, 1, "Confirm");
        tui_button_set_colors(&w->btn_dialog,
                              TUI_COLOR_INDEX(0), TUI_COLOR_INDEX(2),
                              TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(10));
        tui_button_set_on_click(&w->btn_dialog, on_dialog_click, w);
        w->btn_dialog.base.focusable = 1;
        tui_widget_add_child(&w->root, &w->btn_dialog.base);

        tui_button_init(&w->btn_alert, ux + bw + 2, y, bw, 1, "Alert");
        tui_button_set_colors(&w->btn_alert,
                              TUI_COLOR_INDEX(0), TUI_COLOR_INDEX(4),
                              TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(12));
        tui_button_set_on_click(&w->btn_alert, on_alert_click, w);
        w->btn_alert.base.focusable = 1;
        tui_widget_add_child(&w->root, &w->btn_alert.base);

        tui_button_init(&w->btn_clipboard, ux + 2 * (bw + 2), y, bw, 1,
                        "Clipboard");
        tui_button_set_colors(&w->btn_clipboard,
                              TUI_COLOR_INDEX(0), TUI_COLOR_INDEX(5),
                              TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(13));
        tui_button_set_on_click(&w->btn_clipboard, on_clipboard_click, w);
        w->btn_clipboard.base.focusable = 1;
        tui_widget_add_child(&w->root, &w->btn_clipboard.base);
    }
    y += 2;

    tui_separator_init_h(&w->sep2, ux, y, uw);
    tui_separator_set_colors(&w->sep2, TUI_COLOR_INDEX(8),
                             TUI_COLOR_INDEX(0));
    tui_widget_add_child(&w->root, &w->sep2.base);
    y += 1;

    {
        int list_h = rows - y - 3;
        if (list_h < 3) list_h = 3;
        if (list_h > 8) list_h = 8;

        tui_list_init(&w->menu, ux, y, uw, list_h);
        tui_list_set_colors(&w->menu, TUI_COLOR_INDEX(15),
                            TUI_COLOR_INDEX(234), TUI_COLOR_INDEX(0),
                            TUI_COLOR_INDEX(12));
        tui_list_set_on_select(&w->menu, on_list_select, w);
        w->menu.base.focusable = 1;
        tui_list_add_item(&w->menu, "Open confirm dialog (d)");
        tui_list_add_item(&w->menu, "Show alert dialog (a)");
        tui_list_add_item(&w->menu, "Copy to clipboard (c)");
        tui_list_add_item(&w->menu, "About this demo");
        tui_widget_add_child(&w->root, &w->menu.base);
    }

    tui_label_init(&w->status, 1, rows - 1, cols - 2, 1,
                   " Press d/a/c or click buttons  |  q/Esc to quit");
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

    TuiSize size = tui_screen_size();
    build_widgets(w, size.rows, size.cols);
    return 0;
}

static int on_resize(TuiApp *app, int rows, int cols, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;
    destroy_widgets(w);
    build_widgets(w, rows, cols);
    return 0;
}

static int on_render(TuiApp *app, void *user_data)
{
    AppWidgets *w = (AppWidgets *)user_data;
    TuiSize size = tui_screen_size();

    tui_screen_clear();
    tui_screen_fill(0, 0, size.cols, 1, " ",
                    TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4), TUI_ATTR_BOLD);
    tui_screen_write(0, 2,
                     "Dialog Demo  |  Modal overlays + Clipboard",
                     TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4), TUI_ATTR_BOLD);
    tui_screen_fill(size.rows - 1, 0, size.cols, 1, " ",
                    TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(236), TUI_ATTR_NONE);

    tui_widget_render(&w->root);
    tui_app_render_overlays(app);
    tui_screen_render();
    return 0;
}

static int on_input(TuiApp *app, const TuiEvent *event, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;

    if (event->key == TUI_KEY_ESCAPE || event->key == TUI_KEY_CTRL_C) {
        if (g_dialog_active) {
            return 0;
        }
        return 1;
    }

    if (g_dialog_active) return 0;

    if (event->key == TUI_KEY_TAB) {
        if (event->modifiers & TUI_MOD_SHIFT) {
            tui_widget_focus_prev(&w->root);
        } else {
            tui_widget_focus_next(&w->root);
        }
        tui_widget_mark_dirty_recursive(&w->root);
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

    if (g_dialog_active) {
        tui_dialog_init(&g_confirm_dialog, NULL, NULL);
        tui_dialog_init(&g_alert_dialog, NULL, NULL);
    }
}

int main(void)
{
    AppWidgets widgets;

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

    TuiApp *app = tui_app_new(&config);
    if (!app) {
        fprintf(stderr, "Failed to create app\n");
        return 1;
    }

    int ret = tui_app_run(app);
    tui_app_free(app);
    return ret;
}
