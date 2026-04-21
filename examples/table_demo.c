/**
 * @file table_demo.c
 * @brief Table widget demonstration.
 *
 * Demonstrates:
 *   - ZephioTable: column headers, row selection, sorting
 *   - Column click to cycle sort order (none -> asc -> desc)
 *   - Keyboard navigation (arrows, PageUp/Down, Home/End, Enter)
 *   - Horizontal scrolling when columns exceed widget width
 *   - Mouse: header click to sort, row click to select, wheel to scroll
 *
 * Press q/Esc to quit.
 */

#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "zephio_app.h"
#include "zephio_label.h"
#include "zephio_screen.h"
#include "zephio_separator.h"
#include "zephio_table.h"
#include "zephio_widget.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    ZephioWidget  root;
    ZephioLabel   title;
    ZephioLabel   hint;
    ZephioSeparator sep;
    ZephioTable   table;
    ZephioSeparator sep2;
    ZephioLabel   status;
    char       status_text[256];
} AppWidgets;

static void update_status(AppWidgets *w, const char *msg)
{
    snprintf(w->status_text, sizeof(w->status_text), " %s", msg);
    zephio_label_set_text(&w->status, w->status_text);
}

static void on_row_select(ZephioWidget *widget, int row, void *user_data)
{
    (void)widget;
    AppWidgets *w = (AppWidgets *)user_data;
    ZephioTable *t = &w->table;
    if (row < 0 || row >= t->row_count) return;

    int mapped = row;
    if (t->sort_mapping && row >= 0 && row < t->row_count)
        mapped = t->sort_mapping[row];

    char buf[160];
    if (t->rows[mapped] && t->col_count > 0 && t->rows[mapped][0]) {
        snprintf(buf, sizeof(buf), "Selected row %d: %s", row,
                 t->rows[mapped][0]);
    } else {
        snprintf(buf, sizeof(buf), "Selected row %d", row);
    }
    update_status(w, buf);
}

static void populate_table(ZephioTable *t)
{
    zephio_table_add_column(t, "Name", 20);
    zephio_table_add_column(t, "Type", 10);
    zephio_table_add_column(t, "Size", 10);
    zephio_table_add_column(t, "Modified", 18);
    zephio_table_add_column(t, "Permissions", 12);

    const char *rows[][5] = {
        { "src/main.c",        "C Source",  "12.4 KB", "2026-04-10 14:30", "rw-r--r--" },
        { "src/widget.c",      "C Source",  "28.1 KB", "2026-04-09 09:15", "rw-r--r--" },
        { "src/table.c",       "C Source",  "18.7 KB", "2026-04-10 16:00", "rw-r--r--" },
        { "src/tree_view.c",   "C Source",  "15.3 KB", "2026-04-10 11:45", "rw-r--r--" },
        { "include/tui.h",     "Header",    "2.1 KB",  "2026-04-01 10:00", "rw-r--r--" },
        { "include/zephio_app.h", "Header",    "4.8 KB",  "2026-04-05 13:20", "rw-r--r--" },
        { "include/zephio_table.h","Header",   "3.2 KB",  "2026-04-10 15:00", "rw-r--r--" },
        { "Makefile",          "Makefile",  "1.5 KB",  "2026-04-08 08:00", "rw-r--r--" },
        { "README.md",         "Markdown",  "8.9 KB",  "2026-04-10 12:00", "rw-r--r--" },
        { "roadmap.md",        "Markdown",  "6.4 KB",  "2026-04-10 09:30", "rw-r--r--" },
        { "lib/libzephio.a",      "Archive",   "342 KB",  "2026-04-10 16:05", "rw-r--r--" },
        { "examples/hello.c",  "C Source",  "0.8 KB",  "2026-03-15 10:00", "rw-r--r--" },
        { "examples/demo.c",   "C Source",  "5.2 KB",  "2026-04-09 14:00", "rw-r--r--" },
        { "tests/test_text.c", "C Source",  "3.7 KB",  "2026-04-06 16:30", "rw-r--r--" },
        { ".gitignore",        "Git",       "0.2 KB",  "2026-03-01 09:00", "rw-r--r--" },
    };

    for (int i = 0; i < (int)(sizeof(rows) / sizeof(rows[0])); i++)
        zephio_table_add_row(t, rows[i], 5);
}

static void build_widgets(AppWidgets *w, int rows, int cols, ZephioContext *ctx)
{
    int uw = cols > 80 ? 76 : cols - 4;
    if (uw < 30) uw = 30;
    int ux = (cols - uw) / 2;
    if (ux < 1) ux = 1;
    int y = 2;

    zephio_widget_init_ctx(&w->root, 0, 0, cols, rows, NULL, ctx, NULL);

    zephio_label_init_ctx(&w->title, ctx, ux, y, uw, 1,
                   "Table Demo  |  Click headers to sort  |  Arrows to navigate");
    zephio_label_set_colors(&w->title, ZEPHIO_COLOR_INDEX(14), ZEPHIO_COLOR_INDEX(0));
    zephio_label_set_attr(&w->title, ZEPHIO_ATTR_BOLD);
    zephio_widget_add_child(&w->root, &w->title.base);
    y += 2;

    zephio_label_init_ctx(&w->hint, ctx, ux, y, uw, 1,
                   "Enter: select  |  Click header: sort  |  Left/Right: scroll");
    zephio_label_set_colors(&w->hint, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&w->root, &w->hint.base);
    y += 1;

    zephio_separator_init_h_ctx(&w->sep, ctx, ux, y, uw);
    zephio_separator_set_colors(&w->sep, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&w->root, &w->sep.base);
    y += 1;

    {
        int table_h = rows - y - 3;
        if (table_h < 5) table_h = 5;
        if (table_h > 20) table_h = 20;

        zephio_table_init_ctx(&w->table, ctx, ux, y, uw, table_h);
        zephio_table_set_colors(&w->table,
                             ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(234),
                             ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(240),
                             ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(12));
        zephio_table_set_on_select(&w->table, on_row_select, w);
        populate_table(&w->table);
        zephio_widget_add_child(&w->root, &w->table.base);
        y += table_h;
    }

    zephio_separator_init_h_ctx(&w->sep2, ctx, ux, y, uw);
    zephio_separator_set_colors(&w->sep2, ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0));
    zephio_widget_add_child(&w->root, &w->sep2.base);
    y += 1;

    zephio_label_init_ctx(&w->status, ctx, 1, rows - 1, cols - 2, 1,
                   " Use arrow keys, Enter, or click to interact  |  q/Esc: quit");
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
                     "Table Demo  |  Sortable columns + row selection",
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
