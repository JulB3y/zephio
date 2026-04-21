/**
 * @file tree_view_demo.c
 * @brief TreeView widget demonstration.
 *
 * Demonstrates:
 *   - TuiTreeView: hierarchical data with expand/collapse
 *   - Unicode box-drawing connectors
 *   - Keyboard navigation (Up/Down, Left=collapse, Right/Enter=expand)
 *   - Mouse: click to select/toggle, wheel to scroll
 *   - expand_all / collapse_all via keyboard shortcuts
 *
 * Press q/Esc to quit.  e=expand all, c=collapse all.
 */

#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "tui_app.h"
#include "tui_label.h"
#include "tui_screen.h"
#include "tui_separator.h"
#include "tui_tree_view.h"
#include "tui_widget.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    TuiWidget  root;
    TuiLabel   title;
    TuiLabel   hint;
    TuiSeparator sep;
    TuiTreeView tree;
    TuiSeparator sep2;
    TuiLabel   status;
    char       status_text[256];
} AppWidgets;

static TuiTreeNode *build_tree(void)
{
    TuiTreeNode *root = tui_tree_node_create("project", NULL);

    TuiTreeNode *src = tui_tree_node_create("src", NULL);
    tui_tree_node_add_child(root, src);

    TuiTreeNode *core = tui_tree_node_create("core", NULL);
    tui_tree_node_add_child(src, core);
    tui_tree_node_add_child(core, tui_tree_node_create("terminal.c", NULL));
    tui_tree_node_add_child(core, tui_tree_node_create("screen.c", NULL));
    tui_tree_node_add_child(core, tui_tree_node_create("input.c", NULL));
    tui_tree_node_add_child(core, tui_tree_node_create("app.c", NULL));

    TuiTreeNode *widgets = tui_tree_node_create("widgets", NULL);
    tui_tree_node_add_child(src, widgets);
    tui_tree_node_add_child(widgets, tui_tree_node_create("label.c", NULL));
    tui_tree_node_add_child(widgets, tui_tree_node_create("button.c", NULL));
    tui_tree_node_add_child(widgets, tui_tree_node_create("input_field.c", NULL));
    tui_tree_node_add_child(widgets, tui_tree_node_create("list.c", NULL));
    tui_tree_node_add_child(widgets, tui_tree_node_create("table.c", NULL));
    tui_tree_node_add_child(widgets, tui_tree_node_create("tree_view.c", NULL));

    TuiTreeNode *layout = tui_tree_node_create("layout", NULL);
    tui_tree_node_add_child(src, layout);
    tui_tree_node_add_child(layout, tui_tree_node_create("layout.c", NULL));
    tui_tree_node_add_child(layout, tui_tree_node_create("container.c", NULL));
    tui_tree_node_add_child(layout, tui_tree_node_create("box.c", NULL));

    TuiTreeNode *inc = tui_tree_node_create("include", NULL);
    tui_tree_node_add_child(root, inc);
    tui_tree_node_add_child(inc, tui_tree_node_create("tui.h", NULL));
    tui_tree_node_add_child(inc, tui_tree_node_create("tui_app.h", NULL));
    tui_tree_node_add_child(inc, tui_tree_node_create("tui_widget.h", NULL));
    tui_tree_node_add_child(inc, tui_tree_node_create("tui_table.h", NULL));
    tui_tree_node_add_child(inc, tui_tree_node_create("tui_tree_view.h", NULL));

    TuiTreeNode *examples = tui_tree_node_create("examples", NULL);
    tui_tree_node_add_child(root, examples);
    tui_tree_node_add_child(examples, tui_tree_node_create("hello.c", NULL));
    tui_tree_node_add_child(examples, tui_tree_node_create("widgets_demo.c", NULL));
    tui_tree_node_add_child(examples, tui_tree_node_create("table_demo.c", NULL));
    tui_tree_node_add_child(examples, tui_tree_node_create("tree_view_demo.c", NULL));

    TuiTreeNode *tests = tui_tree_node_create("tests", NULL);
    tui_tree_node_add_child(root, tests);
    tui_tree_node_add_child(tests, tui_tree_node_create("test_text.c", NULL));

    tui_tree_node_add_child(root, tui_tree_node_create("Makefile", NULL));
    tui_tree_node_add_child(root, tui_tree_node_create("README.md", NULL));
    tui_tree_node_add_child(root, tui_tree_node_create("roadmap.md", NULL));
    tui_tree_node_add_child(root, tui_tree_node_create(".gitignore", NULL));

    return root;
}

static void update_status(AppWidgets *w, const char *msg)
{
    snprintf(w->status_text, sizeof(w->status_text), " %s", msg);
    tui_label_set_text(&w->status, w->status_text);
}

static void on_node_select(TuiWidget *widget, TuiTreeNode *node, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;
    char buf[160];
    if (node->text) {
        const char *kind = node->child_count > 0 ? "directory" : "file";
        snprintf(buf, sizeof(buf), "%s: %s (%d children)",
                 kind, node->text, node->child_count);
    } else {
        snprintf(buf, sizeof(buf), "(unnamed node)");
    }
    update_status(w, buf);
}

static void build_widgets(AppWidgets *w, int rows, int cols, TuiContext *ctx)
{
    int uw = cols > 60 ? 56 : cols - 4;
    if (uw < 20) uw = 20;
    int ux = (cols - uw) / 2;
    if (ux < 1) ux = 1;
    int y = 2;

    tui_widget_init_ctx(&w->root, 0, 0, cols, rows, NULL, ctx, NULL);

    tui_label_init_ctx(&w->title, ctx, ux, y, uw, 1,
                   "TreeView Demo  |  Navigate with arrow keys");
    tui_label_set_colors(&w->title, ZEPHIO_COLOR_INDEX(14), ZEPHIO_COLOR_INDEX(0));
    tui_label_set_attr(&w->title, ZEPHIO_ATTR_BOLD);
    tui_widget_add_child(&w->root, &w->title.base);
    y += 2;

    tui_label_init_ctx(&w->hint, ctx, ux, y, uw, 1,
                   "Enter/Right: expand  Left: collapse  e: all  c: none");
    tui_label_set_colors(&w->hint, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    tui_widget_add_child(&w->root, &w->hint.base);
    y += 1;

    tui_separator_init_h_ctx(&w->sep, ctx, ux, y, uw);
    tui_separator_set_colors(&w->sep, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    tui_widget_add_child(&w->root, &w->sep.base);
    y += 1;

    {
        int tree_h = rows - y - 3;
        if (tree_h < 5) tree_h = 5;
        if (tree_h > 20) tree_h = 20;

        tui_tree_view_init_ctx(&w->tree, ctx, ux, y, uw, tree_h);
        tui_tree_view_set_colors(&w->tree,
                                 ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(234),
                                 ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(12),
                                 ZEPHIO_COLOR_INDEX(TUI_COLOR_BRIGHT_BLACK));
        tui_tree_view_set_bg_empty(&w->tree, ZEPHIO_COLOR_INDEX(236));
        tui_tree_view_set_on_select(&w->tree, on_node_select, w);

        tui_tree_view_set_root(&w->tree, build_tree());
        tui_widget_add_child(&w->root, &w->tree.base);
        y += tree_h;
    }

    tui_separator_init_h_ctx(&w->sep2, ctx, ux, y, uw);
    tui_separator_set_colors(&w->sep2, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    tui_widget_add_child(&w->root, &w->sep2.base);
    y += 1;

    tui_label_init_ctx(&w->status, ctx, 1, rows - 1, cols - 2, 1,
                   " Click nodes or use keyboard  |  e: expand all  c: collapse all  |  q/Esc: quit");
    tui_label_set_colors(&w->status, ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236));
    tui_widget_add_child(&w->root, &w->status.base);
}

static void destroy_widgets(AppWidgets *w)
{
    for (int i = w->root.child_count - 1; i >= 0; i--)
        tui_widget_destroy(w->root.children[i]);
    tui_widget_remove_all_children(&w->root);
}

static int on_init(TuiApp *app, void *user_data)
{
    (void)app;
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
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;
    TuiSize size = tui_screen_size(app->ctx);

    tui_screen_clear(app->ctx);
    tui_screen_fill(app->ctx, 0, 0, size.cols, 1, " ",
                    ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    tui_screen_write(app->ctx, 0, 2,
                     "TreeView Demo  |  Expand/collapse hierarchical data",
                     ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    tui_screen_fill(app->ctx, size.rows - 1, 0, size.cols, 1, " ",
                    ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236), ZEPHIO_ATTR_NONE);

    tui_widget_render(&w->root);
    tui_screen_render(app->ctx);
    return 0;
}

static int on_input(TuiApp *app, const TuiEvent *event, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;

    if (event->key == TUI_KEY_ESCAPE || event->key == TUI_KEY_CTRL_C)
        return 1;

    if (event->codepoint == 'q' && event->modifiers == TUI_MOD_NONE) {
        TuiWidget *focused = tui_widget_get_focused(&w->root);
        if (!focused || focused == &w->root)
            return 1;
    }

    if (event->codepoint == 'e' && event->modifiers == TUI_MOD_NONE) {
        tui_tree_view_expand_all(&w->tree);
        update_status(w, "Expanded all nodes");
        return 0;
    }

    if (event->codepoint == 'c' && event->modifiers == TUI_MOD_NONE) {
        tui_tree_view_collapse_all(&w->tree);
        update_status(w, "Collapsed all nodes");
        return 0;
    }

    if (event->key == TUI_KEY_TAB) {
        if (event->modifiers & TUI_MOD_SHIFT)
            tui_widget_focus_prev(&w->root);
        else
            tui_widget_focus_next(&w->root);
        tui_widget_mark_dirty_recursive(&w->root);
        return 0;
    }

    TuiWidget *focused = tui_widget_get_focused(&w->root);
    if (focused) {
        tui_widget_handle_input(focused, event);
        tui_widget_mark_dirty_recursive(&w->root);
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
        if (target && target->focusable)
            tui_widget_focus(target);
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
        .on_shutdown = on_shutdown,
        .on_input    = on_input,
        .on_mouse    = on_mouse,
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
