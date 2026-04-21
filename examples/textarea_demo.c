/**
 * @file textarea_demo.c
 * @brief TextArea widget demo — multi-line text editing.
 *
 * Controls:
 *   Arrows / PageUp / PageDown / Home / End — cursor navigation
 *   Mouse click                             — position cursor
 *   Mouse wheel                             — scroll
 *   Printable keys                          — insert text
 *   Enter                                   — new line
 *   Backspace / Delete                      — remove text
 *   q / Escape                              — quit (when textarea not focused)
 */

#define _POSIX_C_SOURCE 200809L

#include "zephio.h"
#include "zephio_input.h"
#include "zephio_label.h"
#include "zephio_mouse.h"
#include "zephio_screen.h"
#include "zephio_textarea.h"
#include "zephio_widget.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    ZephioWidget   root;
    ZephioLabel    header;
    ZephioTextArea editor;
    ZephioLabel    status;
    char        status_text[256];
    int         initialized;
} Demo;

static Demo g_demo;

static const char *g_initial_text =
    "Welcome to the TextArea Demo!\n"
    "\n"
    "This is a multi-line text editor widget.\n"
    "You can type, delete, and navigate freely.\n"
    "\n"
    "Features:\n"
    "  - Arrow keys for cursor navigation\n"
    "  - Home/End to jump to line start/end\n"
    "  - PageUp/PageDown to scroll by page\n"
    "  - Enter to create new lines\n"
    "  - Backspace/Delete to remove characters\n"
    "  - Horizontal scrolling for long lines\n"
    "  - Mouse click to position cursor\n"
    "  - Mouse wheel to scroll\n"
    "\n"
    "Try editing this text!\n"
    "\n"
    "Here is a long line to test horizontal scrolling: "
    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
    "\n"
    "End of sample text.";

static void update_status(void)
{
    int crow = zephio_textarea_get_cursor_row(&g_demo.editor);
    int ccol = zephio_textarea_get_cursor_col(&g_demo.editor);
    int sy   = zephio_textarea_get_scroll_y(&g_demo.editor);
    int lc   = zephio_textarea_get_line_count(&g_demo.editor);

    snprintf(g_demo.status_text, sizeof(g_demo.status_text),
             " Ln %d, Col %d | Lines: %d | Scroll: %d | "
             "Arrows/Home/End/PgUp/PgDn: Navigate | Enter: New Line | Esc: Quit",
             crow + 1, ccol + 1, lc, sy);
    zephio_label_set_text(&g_demo.status, g_demo.status_text);
}

static void build_ui(int rows, int cols, ZephioContext *ctx)
{
    zephio_widget_init_ctx(&g_demo.root, 0, 0, cols, rows, NULL, ctx, NULL);

    zephio_label_init_ctx(&g_demo.header, ctx, 0, 0, cols, 1,
                   " TextArea Demo  |  Phase 14  |  Multi-line text editor");
    zephio_label_set_colors(&g_demo.header,
                           ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4));
    zephio_label_set_attr(&g_demo.header, ZEPHIO_ATTR_BOLD);
    zephio_widget_add_child(&g_demo.root, &g_demo.header.base);

    int editor_h = rows - 2;
    if (editor_h < 3) editor_h = 3;

    zephio_textarea_init_ctx(&g_demo.editor, ctx, 1, 1, cols - 2, editor_h);
    zephio_textarea_set_colors(&g_demo.editor,
                             ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(234),
                             ZEPHIO_ATTR_NONE);
    g_demo.editor.base.focusable = 1;
    zephio_textarea_set_text(&g_demo.editor, g_initial_text);
    zephio_widget_add_child(&g_demo.root, &g_demo.editor.base);

    zephio_label_init_ctx(&g_demo.status, ctx, 0, rows - 1, cols, 1, "");
    zephio_label_set_colors(&g_demo.status,
                           ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236));
    zephio_widget_add_child(&g_demo.root, &g_demo.status.base);

    update_status();
    g_demo.initialized = 1;
}

static void destroy_ui(void)
{
    if (!g_demo.initialized) return;
    for (int i = g_demo.root.child_count - 1; i >= 0; i--) {
        zephio_widget_destroy(g_demo.root.children[i]);
    }
    zephio_widget_remove_all_children(&g_demo.root);
    g_demo.initialized = 0;
}

static void draw_frame(ZephioContext *ctx)
{
    zephio_screen_clear(ctx);
    zephio_widget_render(&g_demo.root);
    zephio_screen_render(ctx);
}

static int input_callback(const ZephioEvent *event, void *user_data)
{
    ZephioContext *ctx = (ZephioContext *)user_data;

    if (event->key == ZEPHIO_EVENT_RESIZE) {
        zephio_screen_resize(ctx, event->size.rows, event->size.cols);
        destroy_ui();
        build_ui( event->size.rows, event->size.cols, ctx);
        zephio_widget_focus_next(&g_demo.root);
        draw_frame(ctx);
        return 0;
    }

    if (event->key == ZEPHIO_EVENT_MOUSE) {
        zephio_widget_handle_mouse(&g_demo.root, &event->mouse);
        update_status();
        draw_frame(ctx);
        return 0;
    }

    if (event->key == ZEPHIO_KEY_ESCAPE || event->key == ZEPHIO_KEY_CTRL_C) {
        return 1;
    }

    if (event->key == ZEPHIO_KEY_TAB) {
        if (event->modifiers & ZEPHIO_MOD_SHIFT) {
            zephio_widget_focus_prev(&g_demo.root);
        } else {
            zephio_widget_focus_next(&g_demo.root);
        }
        zephio_widget_mark_dirty_recursive(&g_demo.root);
        update_status();
        draw_frame(ctx);
        return 0;
    }

    ZephioWidget *focused = zephio_widget_get_focused(&g_demo.root);
    if (focused) {
        zephio_widget_handle_input(focused, event);
    }

    update_status();
    draw_frame(ctx);

    return 0;
}

int main(void)
{
    ZephioContext ctx;

    ZephioResult res = zephio_init(&ctx);
    if (res != ZEPHIO_OK) {
        fprintf(stderr, "zephio_init failed: %d\n", res);
        return 1;
    }

    zephio_input_init(&ctx);
    zephio_mouse_enable(&ctx);

    ZephioSize size = zephio_screen_size(&ctx);
    build_ui( size.rows, size.cols, &ctx);
    zephio_widget_focus_next(&g_demo.root);
    draw_frame(&ctx);

    zephio_input_loop(&ctx, input_callback, &ctx);

    destroy_ui();
    zephio_mouse_disable(&ctx);
    zephio_input_shutdown(&ctx);
    zephio_shutdown(&ctx);
    return 0;
}
