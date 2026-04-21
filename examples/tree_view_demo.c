/**
 * @file tree_view_demo.c
 * @brief TreeView widget demonstration.
 *
 * Demonstrates:
 *   - ZephioTreeView: hierarchical data with expand/collapse
 *   - Unicode box-drawing connectors
 *   - Keyboard navigation (Up/Down, Left=collapse, Right/Enter=expand)
 *   - Mouse: click to select/toggle, wheel to scroll
 *   - expand_all / collapse_all via keyboard shortcuts
 *
 * Press q/Esc to quit.  e=expand all, c=collapse all.
 */

#define _POSIX_C_SOURCE 200809L

#include "zephio.h"
#include "zephio_app.h"
#include "zephio_label.h"
#include "zephio_screen.h"
#include "zephio_separator.h"
#include "zephio_tree_view.h"
#include "zephio_widget.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    ZephioWidget  root;
    ZephioLabel   title;
    ZephioLabel   hint;
    ZephioSeparator sep;
    ZephioTreeView tree;
    ZephioSeparator sep2;
    ZephioLabel   status;
    char       status_text[256];
} AppWidgets;

static ZephioTreeNode *build_tree(void)
{
    ZephioTreeNode *root = zephio_tree_node_create("project", NULL);

    ZephioTreeNode *src = zephio_tree_node_create("src", NULL);
    zephio_tree_node_add_child(root, src);

    ZephioTreeNode *core = zephio_tree_node_create("core", NULL);
    zephio_tree_node_add_child(src, core);
    zephio_tree_node_add_child(core, zephio_tree_node_create("terminal.c", NULL));
    zephio_tree_node_add_child(core, zephio_tree_node_create("screen.c", NULL));
    zephio_tree_node_add_child(core, zephio_tree_node_create("input.c", NULL));
    zephio_tree_node_add_child(core, zephio_tree_node_create("app.c", NULL));

    ZephioTreeNode *widgets = zephio_tree_node_create("widgets", NULL);
    zephio_tree_node_add_child(src, widgets);
    zephio_tree_node_add_child(widgets, zephio_tree_node_create("label.c", NULL));
    zephio_tree_node_add_child(widgets, zephio_tree_node_create("button.c", NULL));
    zephio_tree_node_add_child(widgets, zephio_tree_node_create("input_field.c", NULL));
    zephio_tree_node_add_child(widgets, zephio_tree_node_create("list.c", NULL));
    zephio_tree_node_add_child(widgets, zephio_tree_node_create("table.c", NULL));
    zephio_tree_node_add_child(widgets, zephio_tree_node_create("tree_view.c", NULL));

    ZephioTreeNode *layout = zephio_tree_node_create("layout", NULL);
    zephio_tree_node_add_child(src, layout);
    zephio_tree_node_add_child(layout, zephio_tree_node_create("layout.c", NULL));
    zephio_tree_node_add_child(layout, zephio_tree_node_create("container.c", NULL));
    zephio_tree_node_add_child(layout, zephio_tree_node_create("box.c", NULL));

    ZephioTreeNode *inc = zephio_tree_node_create("include", NULL);
    zephio_tree_node_add_child(root, inc);
    zephio_tree_node_add_child(inc, zephio_tree_node_create("zephio.h", NULL));
    zephio_tree_node_add_child(inc, zephio_tree_node_create("zephio_app.h", NULL));
    zephio_tree_node_add_child(inc, zephio_tree_node_create("zephio_widget.h", NULL));
    zephio_tree_node_add_child(inc, zephio_tree_node_create("zephio_table.h", NULL));
    zephio_tree_node_add_child(inc, zephio_tree_node_create("zephio_tree_view.h", NULL));

    ZephioTreeNode *examples = zephio_tree_node_create("examples", NULL);
    zephio_tree_node_add_child(root, examples);
    zephio_tree_node_add_child(examples, zephio_tree_node_create("hello.c", NULL));
    zephio_tree_node_add_child(examples, zephio_tree_node_create("widgets_demo.c", NULL));
    zephio_tree_node_add_child(examples, zephio_tree_node_create("table_demo.c", NULL));
    zephio_tree_node_add_child(examples, zephio_tree_node_create("tree_view_demo.c", NULL));

    ZephioTreeNode *tests = zephio_tree_node_create("tests", NULL);
    zephio_tree_node_add_child(root, tests);
    zephio_tree_node_add_child(tests, zephio_tree_node_create("test_text.c", NULL));

    zephio_tree_node_add_child(root, zephio_tree_node_create("Makefile", NULL));
    zephio_tree_node_add_child(root, zephio_tree_node_create("README.md", NULL));
    zephio_tree_node_add_child(root, zephio_tree_node_create("roadmap.md", NULL));
    zephio_tree_node_add_child(root, zephio_tree_node_create(".gitignore", NULL));

    return root;
}

static void update_status(AppWidgets *w, const char *msg)
{
    snprintf(w->status_text, sizeof(w->status_text), " %s", msg);
    zephio_label_set_text(&w->status, w->status_text);
}

static void on_node_select(ZephioWidget *widget, ZephioTreeNode *node, void *user_data)
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

static void build_widgets(AppWidgets *w, int rows, int cols, ZephioContext *ctx)
{
    int uw = cols > 60 ? 56 : cols - 4;
    if (uw < 20) uw = 20;
    int ux = (cols - uw) / 2;
    if (ux < 1) ux = 1;
    int y = 2;

    zephio_widget_init_ctx(&w->root, 0, 0, cols, rows, NULL, ctx, NULL);

    zephio_label_init_ctx(&w->title, ctx, ux, y, uw, 1,
                   "TreeView Demo  |  Navigate with arrow keys");
    zephio_label_set_colors(&w->title, ZEPHIO_COLOR_INDEX(14), ZEPHIO_COLOR_INDEX(0));
    zephio_label_set_attr(&w->title, ZEPHIO_ATTR_BOLD);
    zephio_widget_add_child(&w->root, &w->title.base);
    y += 2;

    zephio_label_init_ctx(&w->hint, ctx, ux, y, uw, 1,
                   "Enter/Right: expand  Left: collapse  e: all  c: none");
    zephio_label_set_colors(&w->hint, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&w->root, &w->hint.base);
    y += 1;

    zephio_separator_init_h_ctx(&w->sep, ctx, ux, y, uw);
    zephio_separator_set_colors(&w->sep, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&w->root, &w->sep.base);
    y += 1;

    {
        int tree_h = rows - y - 3;
        if (tree_h < 5) tree_h = 5;
        if (tree_h > 20) tree_h = 20;

        zephio_tree_view_init_ctx(&w->tree, ctx, ux, y, uw, tree_h);
        zephio_tree_view_set_colors(&w->tree,
                                 ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(234),
                                 ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(12),
                                 ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_BLACK));
        zephio_tree_view_set_bg_empty(&w->tree, ZEPHIO_COLOR_INDEX(236));
        zephio_tree_view_set_on_select(&w->tree, on_node_select, w);

        zephio_tree_view_set_root(&w->tree, build_tree());
        zephio_widget_add_child(&w->root, &w->tree.base);
        y += tree_h;
    }

    zephio_separator_init_h_ctx(&w->sep2, ctx, ux, y, uw);
    zephio_separator_set_colors(&w->sep2, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&w->root, &w->sep2.base);
    y += 1;

    zephio_label_init_ctx(&w->status, ctx, 1, rows - 1, cols - 2, 1,
                   " Click nodes or use keyboard  |  e: expand all  c: collapse all  |  q/Esc: quit");
    zephio_label_set_colors(&w->status, ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236));
    zephio_widget_add_child(&w->root, &w->status.base);
}

static void destroy_widgets(AppWidgets *w)
{
    for (int i = w->root.child_count - 1; i >= 0; i--)
        zephio_widget_destroy(w->root.children[i]);
    zephio_widget_remove_all_children(&w->root);
}

static int on_init(ZephioApp *app, void *user_data)
{
    (void)app;
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
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;
    ZephioSize size = zephio_screen_size(app->ctx);

    zephio_screen_clear(app->ctx);
    zephio_screen_fill(app->ctx, 0, 0, size.cols, 1, " ",
                    ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    zephio_screen_write(app->ctx, 0, 2,
                     "TreeView Demo  |  Expand/collapse hierarchical data",
                     ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    zephio_screen_fill(app->ctx, size.rows - 1, 0, size.cols, 1, " ",
                    ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236), ZEPHIO_ATTR_NONE);

    zephio_widget_render(&w->root);
    zephio_screen_render(app->ctx);
    return 0;
}

static int on_input(ZephioApp *app, const ZephioEvent *event, void *user_data)
{
    (void)app;
    AppWidgets *w = (AppWidgets *)user_data;

    if (event->key == ZEPHIO_KEY_ESCAPE || event->key == ZEPHIO_KEY_CTRL_C)
        return 1;

    if (event->codepoint == 'q' && event->modifiers == ZEPHIO_MOD_NONE) {
        ZephioWidget *focused = zephio_widget_get_focused(&w->root);
        if (!focused || focused == &w->root)
            return 1;
    }

    if (event->codepoint == 'e' && event->modifiers == ZEPHIO_MOD_NONE) {
        zephio_tree_view_expand_all(&w->tree);
        update_status(w, "Expanded all nodes");
        return 0;
    }

    if (event->codepoint == 'c' && event->modifiers == ZEPHIO_MOD_NONE) {
        zephio_tree_view_collapse_all(&w->tree);
        update_status(w, "Collapsed all nodes");
        return 0;
    }

    if (event->key == ZEPHIO_KEY_TAB) {
        if (event->modifiers & ZEPHIO_MOD_SHIFT)
            zephio_widget_focus_prev(&w->root);
        else
            zephio_widget_focus_next(&w->root);
        zephio_widget_mark_dirty_recursive(&w->root);
        return 0;
    }

    ZephioWidget *focused = zephio_widget_get_focused(&w->root);
    if (focused) {
        zephio_widget_handle_input(focused, event);
        zephio_widget_mark_dirty_recursive(&w->root);
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
        if (target && target->focusable)
            zephio_widget_focus(target);
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
        .on_shutdown = on_shutdown,
        .on_input    = on_input,
        .on_mouse    = on_mouse,
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
