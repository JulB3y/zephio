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

#include "tui.h"
#include "tui_input.h"
#include "tui_label.h"
#include "tui_mouse.h"
#include "tui_screen.h"
#include "tui_textarea.h"
#include "tui_widget.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    TuiWidget   root;
    TuiLabel    header;
    TuiTextArea editor;
    TuiLabel    status;
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
    int crow = tui_textarea_get_cursor_row(&g_demo.editor);
    int ccol = tui_textarea_get_cursor_col(&g_demo.editor);
    int sy   = tui_textarea_get_scroll_y(&g_demo.editor);
    int lc   = tui_textarea_get_line_count(&g_demo.editor);

    snprintf(g_demo.status_text, sizeof(g_demo.status_text),
             " Ln %d, Col %d | Lines: %d | Scroll: %d | "
             "Arrows/Home/End/PgUp/PgDn: Navigate | Enter: New Line | Esc: Quit",
             crow + 1, ccol + 1, lc, sy);
    tui_label_set_text(&g_demo.status, g_demo.status_text);
}

static void build_ui(int rows, int cols, TuiContext *ctx)
{
    tui_widget_init_ctx(&g_demo.root, 0, 0, cols, rows, NULL, ctx, NULL);

    tui_label_init_ctx(&g_demo.header, ctx, 0, 0, cols, 1,
                   " TextArea Demo  |  Phase 14  |  Multi-line text editor");
    tui_label_set_colors(&g_demo.header,
                           TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4));
    tui_label_set_attr(&g_demo.header, TUI_ATTR_BOLD);
    tui_widget_add_child(&g_demo.root, &g_demo.header.base);

    int editor_h = rows - 2;
    if (editor_h < 3) editor_h = 3;

    tui_textarea_init_ctx(&g_demo.editor, ctx, 1, 1, cols - 2, editor_h);
    tui_textarea_set_colors(&g_demo.editor,
                             TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(234),
                             TUI_ATTR_NONE);
    g_demo.editor.base.focusable = 1;
    tui_textarea_set_text(&g_demo.editor, g_initial_text);
    tui_widget_add_child(&g_demo.root, &g_demo.editor.base);

    tui_label_init_ctx(&g_demo.status, ctx, 0, rows - 1, cols, 1, "");
    tui_label_set_colors(&g_demo.status,
                           TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(236));
    tui_widget_add_child(&g_demo.root, &g_demo.status.base);

    update_status();
    g_demo.initialized = 1;
}

static void destroy_ui(void)
{
    if (!g_demo.initialized) return;
    for (int i = g_demo.root.child_count - 1; i >= 0; i--) {
        tui_widget_destroy(g_demo.root.children[i]);
    }
    tui_widget_remove_all_children(&g_demo.root);
    g_demo.initialized = 0;
}

static void draw_frame(TuiContext *ctx)
{
    tui_screen_clear(ctx);
    tui_widget_render(&g_demo.root);
    tui_screen_render(ctx);
}

static int input_callback(const TuiEvent *event, void *user_data)
{
    TuiContext *ctx = (TuiContext *)user_data;

    if (event->key == TUI_EVENT_RESIZE) {
        tui_screen_resize(ctx, event->size.rows, event->size.cols);
        destroy_ui();
        build_ui( event->size.rows, event->size.cols, ctx);
        tui_widget_focus_next(&g_demo.root);
        draw_frame(ctx);
        return 0;
    }

    if (event->key == TUI_EVENT_MOUSE) {
        tui_widget_handle_mouse(&g_demo.root, &event->mouse);
        update_status();
        draw_frame(ctx);
        return 0;
    }

    if (event->key == TUI_KEY_ESCAPE || event->key == TUI_KEY_CTRL_C) {
        return 1;
    }

    if (event->key == TUI_KEY_TAB) {
        if (event->modifiers & TUI_MOD_SHIFT) {
            tui_widget_focus_prev(&g_demo.root);
        } else {
            tui_widget_focus_next(&g_demo.root);
        }
        tui_widget_mark_dirty_recursive(&g_demo.root);
        update_status();
        draw_frame(ctx);
        return 0;
    }

    TuiWidget *focused = tui_widget_get_focused(&g_demo.root);
    if (focused) {
        tui_widget_handle_input(focused, event);
    }

    update_status();
    draw_frame(ctx);

    return 0;
}

int main(void)
{
    TuiContext ctx;

    TuiResult res = tui_init(&ctx);
    if (res != TUI_OK) {
        fprintf(stderr, "tui_init failed: %d\n", res);
        return 1;
    }

    tui_input_init(&ctx);
    tui_mouse_enable(&ctx);

    TuiSize size = tui_screen_size(&ctx);
    build_ui( size.rows, size.cols, &ctx);
    tui_widget_focus_next(&g_demo.root);
    draw_frame(&ctx);

    tui_input_loop(&ctx, input_callback, &ctx);

    destroy_ui();
    tui_mouse_disable(&ctx);
    tui_input_shutdown(&ctx);
    tui_shutdown(&ctx);
    return 0;
}
