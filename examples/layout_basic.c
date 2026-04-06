#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "tui_input.h"
#include "tui_screen.h"
#include "tui_terminal.h"
#include "tui_layout.h"

#include <stdio.h>
#include <string.h>

static TuiLayout root_layout;
static TuiWidget header;
static TuiLayout body_layout;
static TuiWidget sidebar;
static TuiWidget content;
static TuiWidget footer;

static void draw_panel(TuiWidget *w, const char *label,
                       uint8_t fg, uint8_t bg, TuiAttr attr)
{
    if (!w->visible || w->width <= 0 || w->height <= 0) return;

    tui_screen_fill(w->abs_y, w->abs_x, w->width, w->height, " ", fg, bg, attr);

    if (w->width > 2) {
        tui_screen_write(w->abs_y, w->abs_x + 1, label, fg, bg, TUI_ATTR_BOLD);
    }

    char dim[32];
    int dim_len = snprintf(dim, sizeof(dim), "%dx%d", w->width, w->height);
    if (dim_len < w->width - 2) {
        tui_screen_write(w->abs_y, w->abs_x + w->width - dim_len - 1,
                         dim, fg, bg, TUI_ATTR_DIM);
    }
}

static void draw_frame(int rows, int cols)
{
    tui_screen_clear();

    tui_screen_fill(0, 0, cols, 1, " ", 15, 4, TUI_ATTR_BOLD);
    tui_screen_write(0, 2, "Phase 6 - Basic Layout (Fixed / Fill / Auto)",
                     15, 4, TUI_ATTR_BOLD);

    char info[64];
    int info_len = snprintf(info, sizeof(info), "%dx%d", cols, rows);
    tui_screen_write(0, cols - info_len - 1, info, 15, 4, TUI_ATTR_BOLD);

    tui_widget_render(&root_layout.base);

    draw_panel(&header, "Header [Fixed h=1]", 15, 236, TUI_ATTR_BOLD);
    draw_panel(&sidebar, "Sidebar [Fixed w=20]", 0, 24, TUI_ATTR_NONE);
    draw_panel(&content, "Content [Fill]", 0, 240, TUI_ATTR_NONE);
    draw_panel(&footer, "Footer [Fixed h=1]  |  Press 'q' or Escape to quit",
               15, 236, TUI_ATTR_BOLD);

    tui_screen_render();
}

static int input_callback(const TuiEvent *event, void *user_data)
{
    (void)user_data;

    if (event->key == TUI_EVENT_RESIZE) {
        tui_screen_resize(event->size.rows, event->size.cols);
        tui_widget_resize(&root_layout.base, event->size.cols, event->size.rows);
        draw_frame(event->size.rows, event->size.cols);
        return 0;
    }

    if (event->key == TUI_KEY_ESCAPE || event->key == TUI_KEY_CTRL_C) return 1;
    if (event->codepoint == 'q' && event->modifiers == TUI_MOD_NONE) return 1;

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

    TuiSize size = tui_screen_size();

    tui_layout_init(&root_layout, TUI_LAYOUT_VERTICAL,
                    0, 0, size.cols, size.rows);
    tui_layout_set_padding(&root_layout, 1);

    tui_widget_init(&header, 0, 0, 1, 1, NULL, NULL);
    tui_layout_add(&root_layout, &header, TUI_LAYOUT_FIXED(1));

    tui_layout_init(&body_layout, TUI_LAYOUT_HORIZONTAL,
                    0, 0, 1, 1);
    tui_layout_set_padding(&body_layout, 1);
    tui_layout_add(&root_layout, &body_layout.base, TUI_LAYOUT_FILL);

    tui_widget_init(&sidebar, 0, 0, 20, 1, NULL, NULL);
    tui_layout_add(&body_layout, &sidebar, TUI_LAYOUT_FIXED(20));

    tui_widget_init(&content, 0, 0, 1, 1, NULL, NULL);
    tui_layout_add(&body_layout, &content, TUI_LAYOUT_FILL);

    tui_widget_init(&footer, 0, 0, 1, 1, NULL, NULL);
    tui_layout_add(&root_layout, &footer, TUI_LAYOUT_FIXED(1));

    tui_widget_resize(&root_layout.base, size.cols, size.rows);

    draw_frame(size.rows, size.cols);

    tui_input_loop(input_callback, NULL);

    tui_layout_remove_all(&body_layout);
    tui_layout_remove_all(&root_layout);

    tui_input_shutdown();
    tui_shutdown();
    return 0;
}
