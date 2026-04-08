#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "tui_input.h"
#include "tui_screen.h"
#include "tui_terminal.h"
#include "tui_layout.h"

#include <stdio.h>
#include <string.h>

static TuiLayout root;
static TuiWidget titlebar;

static TuiLayout main_row;
static TuiWidget nav;
static TuiLayout center_col;
static TuiWidget toolbar;
static TuiLayout content_row;
static TuiWidget editor;
static TuiWidget preview;
static TuiWidget statusbar;

static void draw_panel(TuiWidget *w, const char *label,
                       TuiColor fg, TuiColor bg)
{
    if (!w->visible || w->width <= 0 || w->height <= 0) return;

    tui_screen_box_single(w->abs_y, w->abs_x, w->width, w->height,
                          fg, bg, TUI_ATTR_NONE);

    if (w->width > 2) {
        tui_screen_write(w->abs_y, w->abs_x + 1, label,
                         fg, bg, TUI_ATTR_BOLD);
    }

    if (w->height >= 3) {
        for (int r = 1; r < w->height - 1; r++) {
            char line[128];
            snprintf(line, sizeof(line),
                "row %d: x=%d y=%d w=%d h=%d",
                r, w->abs_x, w->abs_y, w->width, w->height);
            tui_screen_write(w->abs_y + r, w->abs_x + 1, line,
                             fg, bg, TUI_ATTR_DIM);
        }
    }

    char dim[32];
    int dim_len = snprintf(dim, sizeof(dim), "%dx%d", w->width, w->height);
    if (w->height >= 2 && w->width > 2 && dim_len < w->width - 2) {
        tui_screen_write(w->abs_y, w->abs_x + w->width - dim_len - 1,
                         dim, fg, bg, TUI_ATTR_DIM);
    }
}

static void draw_bar(TuiWidget *w, const char *text,
                     TuiColor fg, TuiColor bg)
{
    if (!w->visible || w->width <= 0 || w->height <= 0) return;
    tui_screen_fill(w->abs_y, w->abs_x, w->width, w->height,
                    " ", fg, bg, TUI_ATTR_BOLD);
    if (w->width > 2) {
        tui_screen_write(w->abs_y, w->abs_x + 1, text, fg, bg, TUI_ATTR_BOLD);
    }
}

static void draw_frame(int rows, int cols)
{
    tui_screen_clear();

    tui_widget_render(&root.base);

    draw_bar(&titlebar,
             "Phase 6 - Dashboard Layout  |  Nested H+V Layouts  |  Resize to test",
             TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4));

    draw_bar(&toolbar, "  File  Edit  View  Tools  Help", TUI_COLOR_INDEX(0), TUI_COLOR_INDEX(252));

    draw_panel(&nav, "Navigation", TUI_COLOR_INDEX(7), TUI_COLOR_INDEX(194));
    draw_panel(&editor, "Editor [Fill w=2]", TUI_COLOR_INDEX(0), TUI_COLOR_INDEX(234));
    draw_panel(&preview, "Preview [Fill w=1]", TUI_COLOR_INDEX(0), TUI_COLOR_INDEX(230));
    draw_bar(&statusbar,
             "  Layout: nested V>H>V  |  Press 'q' or Escape to quit",
             TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(236));

    tui_screen_render();
    (void)rows;
    (void)cols;
}

static int input_callback(const TuiEvent *event, void *user_data)
{
    (void)user_data;

    if (event->key == TUI_EVENT_RESIZE) {
        tui_screen_resize(event->size.rows, event->size.cols);
        tui_widget_resize(&root.base, event->size.cols, event->size.rows);
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

    tui_layout_init(&root, TUI_LAYOUT_VERTICAL, 0, 0, size.cols, size.rows);
    tui_layout_set_padding(&root, 0);

    tui_widget_init(&titlebar, 0, 0, 1, 1, NULL, NULL);
    tui_layout_add(&root, &titlebar, TUI_LAYOUT_FIXED(1));

    tui_layout_init(&main_row, TUI_LAYOUT_HORIZONTAL, 0, 0, 1, 1);
    tui_layout_set_padding(&main_row, 1);
    tui_layout_add(&root, &main_row.base, TUI_LAYOUT_FILL);

    tui_widget_init(&nav, 0, 0, 18, 1, NULL, NULL);
    tui_layout_add(&main_row, &nav, TUI_LAYOUT_FIXED(18));

    tui_layout_init(&center_col, TUI_LAYOUT_VERTICAL, 0, 0, 1, 1);
    tui_layout_set_padding(&center_col, 1);
    tui_layout_add(&main_row, &center_col.base, TUI_LAYOUT_FILL);

    tui_widget_init(&toolbar, 0, 0, 1, 1, NULL, NULL);
    tui_layout_add(&center_col, &toolbar, TUI_LAYOUT_FIXED(1));

    tui_layout_init(&content_row, TUI_LAYOUT_HORIZONTAL, 0, 0, 1, 1);
    tui_layout_set_padding(&content_row, 1);
    tui_layout_add(&center_col, &content_row.base, TUI_LAYOUT_FILL);

    tui_widget_init(&editor, 0, 0, 1, 1, NULL, NULL);
    tui_layout_add(&content_row, &editor, TUI_LAYOUT_FILL_WEIGHT(2));

    tui_widget_init(&preview, 0, 0, 1, 1, NULL, NULL);
    tui_layout_add(&content_row, &preview, TUI_LAYOUT_FILL);

    tui_widget_init(&statusbar, 0, 0, 1, 1, NULL, NULL);
    tui_layout_add(&root, &statusbar, TUI_LAYOUT_FIXED(1));

    tui_widget_resize(&root.base, size.cols, size.rows);

    draw_frame(size.rows, size.cols);

    tui_input_loop(input_callback, NULL);

    tui_layout_remove_all(&content_row);
    tui_layout_remove_all(&center_col);
    tui_layout_remove_all(&main_row);
    tui_layout_remove_all(&root);

    tui_input_shutdown();
    tui_shutdown();
    return 0;
}
