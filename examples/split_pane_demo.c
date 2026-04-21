/**
 * @file split_pane_demo.c
 * @brief Split pane widget demonstration.
 *
 * Demonstrates:
 *   - ZephioSplitPane: horizontal and vertical splits
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

#include "zephio.h"
#include "zephio_app.h"
#include "zephio_box.h"
#include "zephio_container.h"
#include "zephio_label.h"
#include "zephio_list.h"
#include "zephio_screen.h"
#include "zephio_separator.h"
#include "zephio_split_pane.h"
#include "zephio_widget.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    ZephioWidget    root;

    ZephioSplitPane hsplit;
    ZephioSplitPane vsplit;

    ZephioBox       top_left;
    ZephioList      top_left_list;
    ZephioBox       bottom_left;
    ZephioLabel     bottom_left_label;

    ZephioBox       right_box;
    ZephioLabel     right_content;

    ZephioLabel     statusbar;
    char         status_text[256];
} AppWidgets;

static ZephioApp *g_app = NULL;

static void update_status(AppWidgets *w, const char *msg)
{
    snprintf(w->status_text, sizeof(w->status_text), " %s", msg);
    zephio_label_set_text(&w->statusbar, w->status_text);
    zephio_label_set_colors(&w->statusbar, ZEPHIO_COLOR_INDEX(15),
                         ZEPHIO_COLOR_INDEX(236));
}

static void on_list_select(ZephioWidget *widget, int index, const char *item,
                           void *user_data)
{
    (void)widget;
    (void)index;
    AppWidgets *w = (AppWidgets *)user_data;

    char buf[256];
    snprintf(buf, sizeof(buf), "Selected: %s", item);
    zephio_label_set_text(&w->right_content, buf);

    char status_buf[120];
    snprintf(status_buf, sizeof(status_buf), "Selected: %s  |  Drag separator or Ctrl+Arrows to resize", item);
    update_status(w, status_buf);
}

static void build_widgets(AppWidgets *w, int rows, int cols, ZephioContext *ctx)
{
    int content_h = rows - 2;

    zephio_widget_init_ctx(&w->root, 0, 0, cols, rows, NULL, ctx, NULL);

    zephio_split_pane_init_ctx(&w->hsplit, ctx, 0, 1, cols, content_h,
                            ZEPHIO_SPLIT_HORIZONTAL);
    zephio_split_pane_set_separator_style(&w->hsplit,
                                       ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_CYAN),
                                       ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BG_DARK),
                                       ZEPHIO_ATTR_BOLD);
    zephio_split_pane_set_min_sizes(&w->hsplit, 15, 20);

    zephio_split_pane_init_ctx(&w->vsplit, ctx, 0, 0, cols / 2, content_h,
                            ZEPHIO_SPLIT_VERTICAL);
    zephio_split_pane_set_separator_style(&w->vsplit,
                                       ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_YELLOW),
                                       ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BG_DARK),
                                       ZEPHIO_ATTR_BOLD);
    zephio_split_pane_set_min_sizes(&w->vsplit, 5, 5);

    {
        int left_w = zephio_split_pane_get_position(&w->hsplit);
        (void)left_w;

        zephio_box_init_ctx(&w->top_left, ctx, 0, 0, 10, 10, ZEPHIO_BOX_SINGLE);
        zephio_box_set_title(&w->top_left, "Files");
        zephio_box_set_colors(&w->top_left,
                           ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_CYAN),
                           ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BG_DARK));

        zephio_list_init_ctx(&w->top_left_list, ctx, 0, 0, 10, 5);
        zephio_list_set_colors(&w->top_left_list, ZEPHIO_COLOR_INDEX(15),
                            ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BG_DARK),
                            ZEPHIO_COLOR_INDEX(0),
                            ZEPHIO_COLOR_INDEX(12));
        w->top_left_list.base.focusable = 1;
        zephio_list_add_item(&w->top_left_list, "src/main.c");
        zephio_list_add_item(&w->top_left_list, "src/widget.c");
        zephio_list_add_item(&w->top_left_list, "src/app.c");
        zephio_list_add_item(&w->top_left_list, "include/tui.h");
        zephio_list_add_item(&w->top_left_list, "Makefile");
        zephio_list_add_item(&w->top_left_list, "README.md");
        zephio_list_set_on_select(&w->top_left_list, on_list_select, w);
        zephio_widget_add_child(&w->top_left.base, &w->top_left_list.base);

        zephio_box_init_ctx(&w->bottom_left, ctx, 0, 0, 10, 5, ZEPHIO_BOX_SINGLE);
        zephio_box_set_title(&w->bottom_left, "Details");
        zephio_box_set_colors(&w->bottom_left,
                           ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_YELLOW),
                           ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BG_DARK));

        zephio_label_init_ctx(&w->bottom_left_label, ctx, 0, 0, 10, 1,
                          "Select a file above");
        zephio_label_set_colors(&w->bottom_left_label,
                             ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_WHITE),
                             ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BG_DARK));
        zephio_widget_add_child(&w->bottom_left.base, &w->bottom_left_label.base);

        zephio_split_pane_set_panes(&w->vsplit, &w->top_left.base,
                                 &w->bottom_left.base);
    }

    {
        zephio_box_init_ctx(&w->right_box, ctx, 0, 0, 20, 10, ZEPHIO_BOX_DOUBLE);
        zephio_box_set_title(&w->right_box, "Editor");
        zephio_box_set_colors(&w->right_box,
                           ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_GREEN),
                           ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BG_DARK));

        zephio_label_init_ctx(&w->right_content, ctx, 0, 0, 20, 1,
                          "Select a file from the left pane");
        zephio_label_set_colors(&w->right_content,
                             ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_WHITE),
                             ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BG_DARK));
        zephio_widget_add_child(&w->right_box.base, &w->right_content.base);
    }

    zephio_split_pane_set_panes(&w->hsplit, &w->vsplit.base, &w->right_box.base);
    zephio_widget_add_child(&w->root, &w->hsplit.base);

    zephio_label_init_ctx(&w->statusbar, ctx, 1, rows - 1, cols - 2, 1,
                   " Drag separator bars or Ctrl+Arrows to resize  |  Tab: focus  q: quit");
    zephio_label_set_colors(&w->statusbar, ZEPHIO_COLOR_INDEX(15),
                         ZEPHIO_COLOR_INDEX(236));
    zephio_widget_add_child(&w->root, &w->statusbar.base);
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
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;
    ZephioSize size = zephio_screen_size(g_app->ctx);

    zephio_screen_clear(g_app->ctx);
    zephio_screen_fill(g_app->ctx, 0, 0, size.cols, 1, " ",
                    ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    zephio_screen_write(g_app->ctx, 0, 2,
                     "Split Pane Demo  |  Nested H+V Split  |  Drag separators to resize",
                     ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    zephio_screen_fill(g_app->ctx, size.rows - 1, 0, size.cols, 1, " ",
                    ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236), ZEPHIO_ATTR_NONE);

    zephio_widget_render(&w->root);
    zephio_app_render_overlays(app);
    zephio_app_render_toasts(app);
    zephio_screen_render(g_app->ctx);
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

    if (event->codepoint == 'q' && event->modifiers == ZEPHIO_MOD_NONE) {
        return 1;
    }

    if (event->modifiers & ZEPHIO_MOD_CTRL) {
        if (event->key == ZEPHIO_KEY_LEFT || event->key == ZEPHIO_KEY_RIGHT) {
            if (w->hsplit.base.vtable && w->hsplit.base.vtable->handle_input)
                w->hsplit.base.vtable->handle_input(&w->hsplit.base, event);
            return 0;
        }
        if (event->key == ZEPHIO_KEY_UP || event->key == ZEPHIO_KEY_DOWN) {
            if (w->vsplit.base.vtable && w->vsplit.base.vtable->handle_input)
                w->vsplit.base.vtable->handle_input(&w->vsplit.base, event);
            return 0;
        }
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
