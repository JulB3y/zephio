/**
 * @file split_pane_demo.c
 * @brief Split pane widget demonstration.
 *
 * Demonstrates:
 *   - TuiSplitPane: horizontal and vertical splits
 *   - Mouse drag on separator to resize panes
 *   - Keyboard resize (Ctrl+Arrow keys)
 *   - Nested split panes (a vertical split inside the left pane)
 *
 * Press:
 *   Ctrl+Left/Right - resize horizontal split
 *   Ctrl+Up/Down    - resize vertical split (in left pane)
 *   Mouse drag on the separator bars to resize
 *   q/Esc - quit
 */

#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "tui_app.h"
#include "tui_box.h"
#include "tui_container.h"
#include "tui_label.h"
#include "tui_list.h"
#include "tui_screen.h"
#include "tui_separator.h"
#include "tui_split_pane.h"
#include "tui_widget.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    TuiWidget    root;

    TuiSplitPane hsplit;
    TuiSplitPane vsplit;

    TuiBox       top_left;
    TuiList      top_left_list;
    TuiBox       bottom_left;
    TuiLabel     bottom_left_label;

    TuiBox       right_box;
    TuiLabel     right_content;

    TuiLabel     statusbar;
    char         status_text[256];
} AppWidgets;

static TuiApp *g_app = NULL;

static void update_status(AppWidgets *w, const char *msg)
{
    snprintf(w->status_text, sizeof(w->status_text), " %s", msg);
    tui_label_set_text(&w->statusbar, w->status_text);
    tui_label_set_colors(&w->statusbar, TUI_COLOR_INDEX(15),
                         TUI_COLOR_INDEX(236));
}

static void on_list_select(TuiWidget *widget, int index, const char *item,
                           void *user_data)
{
    (void)widget;
    (void)index;
    AppWidgets *w = (AppWidgets *)user_data;

    char buf[256];
    snprintf(buf, sizeof(buf), "Selected: %s", item);
    tui_label_set_text(&w->right_content, buf);

    char status_buf[120];
    snprintf(status_buf, sizeof(status_buf), "Selected: %s  |  Drag separator or Ctrl+Arrows to resize", item);
    update_status(w, status_buf);
}

static void build_widgets(AppWidgets *w, int rows, int cols)
{
    int content_h = rows - 2;

    tui_widget_init(&w->root, 0, 0, cols, rows, NULL, NULL);

    tui_split_pane_init(&w->hsplit, 0, 1, cols, content_h,
                        TUI_SPLIT_HORIZONTAL);
    tui_split_pane_set_separator_style(&w->hsplit,
                                       TUI_COLOR_INDEX(TUI_COLOR_BRIGHT_CYAN),
                                       TUI_COLOR_INDEX(TUI_COLOR_BG_DARK),
                                       TUI_ATTR_BOLD);
    tui_split_pane_set_min_sizes(&w->hsplit, 15, 20);

    tui_split_pane_init(&w->vsplit, 0, 0, cols / 2, content_h,
                        TUI_SPLIT_VERTICAL);
    tui_split_pane_set_separator_style(&w->vsplit,
                                       TUI_COLOR_INDEX(TUI_COLOR_BRIGHT_YELLOW),
                                       TUI_COLOR_INDEX(TUI_COLOR_BG_DARK),
                                       TUI_ATTR_BOLD);
    tui_split_pane_set_min_sizes(&w->vsplit, 5, 5);

    {
        int left_w = tui_split_pane_get_position(&w->hsplit);
        (void)left_w;

        tui_box_init(&w->top_left, 0, 0, 10, 10, TUI_BOX_SINGLE);
        tui_box_set_title(&w->top_left, "Files");
        tui_box_set_colors(&w->top_left,
                           TUI_COLOR_INDEX(TUI_COLOR_BRIGHT_CYAN),
                           TUI_COLOR_INDEX(TUI_COLOR_BG_DARK));

        tui_list_init(&w->top_left_list, 0, 0, 10, 5);
        tui_list_set_colors(&w->top_left_list, TUI_COLOR_INDEX(15),
                            TUI_COLOR_INDEX(TUI_COLOR_BG_DARK),
                            TUI_COLOR_INDEX(0),
                            TUI_COLOR_INDEX(12));
        w->top_left_list.base.focusable = 1;
        tui_list_add_item(&w->top_left_list, "src/main.c");
        tui_list_add_item(&w->top_left_list, "src/widget.c");
        tui_list_add_item(&w->top_left_list, "src/app.c");
        tui_list_add_item(&w->top_left_list, "include/tui.h");
        tui_list_add_item(&w->top_left_list, "Makefile");
        tui_list_add_item(&w->top_left_list, "README.md");
        tui_list_set_on_select(&w->top_left_list, on_list_select, w);
        tui_widget_add_child(&w->top_left.base, &w->top_left_list.base);

        tui_box_init(&w->bottom_left, 0, 0, 10, 5, TUI_BOX_SINGLE);
        tui_box_set_title(&w->bottom_left, "Details");
        tui_box_set_colors(&w->bottom_left,
                           TUI_COLOR_INDEX(TUI_COLOR_BRIGHT_YELLOW),
                           TUI_COLOR_INDEX(TUI_COLOR_BG_DARK));

        tui_label_init(&w->bottom_left_label, 0, 0, 10, 1,
                       "Select a file above");
        tui_label_set_colors(&w->bottom_left_label,
                             TUI_COLOR_INDEX(TUI_COLOR_BRIGHT_WHITE),
                             TUI_COLOR_INDEX(TUI_COLOR_BG_DARK));
        tui_widget_add_child(&w->bottom_left.base, &w->bottom_left_label.base);

        tui_split_pane_set_panes(&w->vsplit, &w->top_left.base,
                                 &w->bottom_left.base);
    }

    {
        tui_box_init(&w->right_box, 0, 0, 20, 10, TUI_BOX_DOUBLE);
        tui_box_set_title(&w->right_box, "Editor");
        tui_box_set_colors(&w->right_box,
                           TUI_COLOR_INDEX(TUI_COLOR_BRIGHT_GREEN),
                           TUI_COLOR_INDEX(TUI_COLOR_BG_DARK));

        tui_label_init(&w->right_content, 0, 0, 20, 1,
                       "Select a file from the left pane");
        tui_label_set_colors(&w->right_content,
                             TUI_COLOR_INDEX(TUI_COLOR_BRIGHT_WHITE),
                             TUI_COLOR_INDEX(TUI_COLOR_BG_DARK));
        tui_widget_add_child(&w->right_box.base, &w->right_content.base);
    }

    tui_split_pane_set_panes(&w->hsplit, &w->vsplit.base, &w->right_box.base);
    tui_widget_add_child(&w->root, &w->hsplit.base);

    tui_label_init(&w->statusbar, 1, rows - 1, cols - 2, 1,
                   " Drag separator bars or Ctrl+Arrows to resize  |  Tab: focus  q: quit");
    tui_label_set_colors(&w->statusbar, TUI_COLOR_INDEX(15),
                         TUI_COLOR_INDEX(236));
    tui_widget_add_child(&w->root, &w->statusbar.base);
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
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;
    TuiSize size = tui_screen_size();

    tui_screen_clear();
    tui_screen_fill(0, 0, size.cols, 1, " ",
                    TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4), TUI_ATTR_BOLD);
    tui_screen_write(0, 2,
                     "Split Pane Demo  |  Nested H+V Split  |  Drag separators to resize",
                     TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4), TUI_ATTR_BOLD);
    tui_screen_fill(size.rows - 1, 0, size.cols, 1, " ",
                    TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(236), TUI_ATTR_NONE);

    tui_widget_render(&w->root);
    tui_app_render_overlays(app);
    tui_app_render_toasts(app);
    tui_screen_render();
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

    if (event->codepoint == 'q' && event->modifiers == TUI_MOD_NONE) {
        return 1;
    }

    if (event->modifiers & TUI_MOD_CTRL) {
        if (event->key == TUI_KEY_LEFT || event->key == TUI_KEY_RIGHT) {
            if (w->hsplit.base.vtable && w->hsplit.base.vtable->handle_input)
                w->hsplit.base.vtable->handle_input(&w->hsplit.base, event);
            return 0;
        }
        if (event->key == TUI_KEY_UP || event->key == TUI_KEY_DOWN) {
            if (w->vsplit.base.vtable && w->vsplit.base.vtable->handle_input)
                w->vsplit.base.vtable->handle_input(&w->vsplit.base, event);
            return 0;
        }
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
