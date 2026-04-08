/**
 * @file text_view_demo.c
 * @brief TextView widget demo — long text, word-wrap toggle, scrolling.
 *
 * Controls:
 *   Arrows / PageUp / PageDown / Home / End — scroll
 *   Mouse wheel                              — scroll
 *   w                                        — toggle word-wrap
 *   q / Escape                               — quit
 */

#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "tui_input.h"
#include "tui_label.h"
#include "tui_mouse.h"
#include "tui_screen.h"
#include "tui_text_view.h"
#include "tui_widget.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    TuiWidget    root;
    TuiLabel     header;
    TuiTextView  content;
    TuiLabel     status;
    char         status_text[256];
    int          initialized;
} Demo;

static Demo g_demo;

static char g_long_text[8192];

static void build_long_text(void)
{
    int pos = 0;
    int remain = (int)sizeof(g_long_text);

    pos += snprintf(g_long_text + pos, (size_t)remain,
        "TUI Framework -- TextView Demo\n"
        "=============================\n"
        "\n"
        "This is a multi-line text view widget with word-wrapping and\n"
        "scroll container integration. It demonstrates the TuiTextView\n"
        "widget which was implemented as Phase 13 of the roadmap.\n"
        "\n"
        "Features\n"
        "--------\n"
        "1. Word-Wrapping: Long lines wrap at word boundaries.\n"
        "2. Scroll Container: Arrow keys, PageUp/Down, Home/End,\n"
        "   mouse wheel, and proportional scrollbar indicators.\n"
        "3. Read-Only: Phase 14 will add editing.\n"
        "\n"
        "Press 'w' to toggle word-wrapping.\n"
        "\n"
        "Lorem Ipsum\n"
        "-----------\n"
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do\n"
        "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut\n"
        "enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
        "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor\n"
        "in reprehenderit in voluptate velit esse cillum dolore eu fugiat\n"
        "nulla pariatur.\n"
        "\n"
        "Curabitur pretium tincidunt lacus. Nulla gravida orci a odio.\n"
        "Nullam varius, turpis et commodo pharetra, est eros bibendum\n"
        "elit, nec luctus magna felis sollicitudin mauris.\n"
        "\n");
    remain = (int)sizeof(g_long_text) - pos;

    pos += snprintf(g_long_text + pos, (size_t)remain,
        "Scroll Test\n"
        "-----------\n"
        "This text is intentionally long to test scrolling:\n"
        "  - Arrow keys: scroll by 1 line/column\n"
        "  - Page Up / Page Down: scroll by viewport height\n"
        "  - Home: scroll to top  |  End: scroll to bottom\n"
        "  - Mouse wheel: scroll by 3 lines per notch\n"
        "\n");
    remain = (int)sizeof(g_long_text) - pos;

    for (int i = 1; i <= 30; i++) {
        pos += snprintf(g_long_text + pos, (size_t)remain,
                        "Line %02d: ---------- scrolling test "
                        "line ----------\n", i);
        remain = (int)sizeof(g_long_text) - pos;
    }

    pos += snprintf(g_long_text + pos, (size_t)remain,
        "\n"
        "Horizontal Scroll Test (no word-wrap mode)\n"
        "-------------------------------------------\n"
        "Press 'w' to disable word-wrap, then Left/Right to scroll:\n");
    remain = (int)sizeof(g_long_text) - pos;

    for (int i = 0; i < 10; i++) {
        pos += snprintf(g_long_text + pos, (size_t)remain,
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        remain = (int)sizeof(g_long_text) - pos;
    }

    pos += snprintf(g_long_text + pos, (size_t)remain,
        "\n\nEnd of demo text.\n");
}

static void update_status(void)
{
    int lines = tui_text_view_get_line_count(&g_demo.content);
    int sy    = tui_text_view_get_scroll_y(&g_demo.content);
    int wrap  = g_demo.content.word_wrap;

    snprintf(g_demo.status_text, sizeof(g_demo.status_text),
             " Lines: %d | Scroll: %d | Wrap: %s | "
             "Arrows/PgUp/PgDn/Home/End: Scroll | w: Toggle Wrap | q/Esc: Quit",
             lines, sy, wrap ? "ON" : "OFF");
    tui_label_set_text(&g_demo.status, g_demo.status_text);
}

static void build_ui(int rows, int cols)
{
    tui_widget_init(&g_demo.root, 0, 0, cols, rows, NULL, NULL);

    tui_label_init(&g_demo.header, 0, 0, cols, 1,
                   " TextView Demo  |  Phase 13  |  Multi-line text with scrolling");
    tui_label_set_colors(&g_demo.header,
                         TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4));
    tui_label_set_attr(&g_demo.header, TUI_ATTR_BOLD);
    tui_widget_add_child(&g_demo.root, &g_demo.header.base);

    int content_h = rows - 2;
    if (content_h < 3) content_h = 3;

    tui_text_view_init(&g_demo.content, 1, 1, cols - 2, content_h);
    tui_text_view_set_colors(&g_demo.content,
                             TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(234));
    g_demo.content.base.base.focusable = 1;
    tui_text_view_set_text(&g_demo.content, g_long_text);
    tui_widget_add_child(&g_demo.root, &g_demo.content.base.base);

    tui_label_init(&g_demo.status, 0, rows - 1, cols, 1, "");
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

static void draw_frame(void)
{
    tui_screen_clear();
    tui_widget_render(&g_demo.root);
    tui_screen_render();
}

static int input_callback(const TuiEvent *event, void *user_data)
{
    (void)user_data;

    if (event->key == TUI_EVENT_RESIZE) {
        tui_screen_resize(event->size.rows, event->size.cols);
        destroy_ui();
        build_ui(event->size.rows, event->size.cols);
        tui_widget_focus_next(&g_demo.root);
        draw_frame();
        return 0;
    }

    if (event->key == TUI_EVENT_MOUSE) {
        tui_widget_handle_mouse(&g_demo.root, &event->mouse);
        update_status();
        draw_frame();
        return 0;
    }

    if (event->key == TUI_KEY_ESCAPE || event->key == TUI_KEY_CTRL_C) {
        return 1;
    }

    if (event->codepoint == 'q' && event->modifiers == TUI_MOD_NONE) {
        TuiWidget *focused = tui_widget_get_focused(&g_demo.root);
        if (!focused || focused == &g_demo.root) {
            return 1;
        }
    }

    if (event->codepoint == 'w' && event->modifiers == TUI_MOD_NONE) {
        TuiWidget *focused = tui_widget_get_focused(&g_demo.root);
        if (focused == &g_demo.content.base.base) {
            int wrap = g_demo.content.word_wrap;
            tui_text_view_set_word_wrap(&g_demo.content, !wrap);
            update_status();
            tui_widget_mark_dirty_recursive(&g_demo.root);
            draw_frame();
            return 0;
        }
    }

    TuiWidget *focused = tui_widget_get_focused(&g_demo.root);
    if (focused) {
        tui_widget_handle_input(focused, event);
    }

    update_status();
    draw_frame();

    return 0;
}

int main(void)
{
    TuiResult res = tui_init();
    if (res != TUI_OK) {
        fprintf(stderr, "tui_init failed: %d\n", res);
        return 1;
    }

    tui_input_init();
    tui_mouse_enable();

    build_long_text();

    TuiSize size = tui_screen_size();
    build_ui(size.rows, size.cols);
    tui_widget_focus_next(&g_demo.root);
    draw_frame();

    tui_input_loop(input_callback, NULL);

    destroy_ui();
    tui_mouse_disable();
    tui_input_shutdown();
    tui_shutdown();
    return 0;
}
