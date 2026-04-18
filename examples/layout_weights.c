#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "tui_input.h"
#include "tui_screen.h"
#include "tui_terminal.h"
#include "tui_layout.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    TuiColor fg;
    TuiColor bg;
} PanelColor;

static TuiLayout root;
static TuiWidget panels[5];
static PanelColor colors[5];

static void draw_panel(TuiContext *ctx, TuiWidget *w, const char *label, PanelColor c)
{
    if (!w->visible || w->width <= 0 || w->height <= 0) return;

    tui_screen_fill(ctx, w->abs_y, w->abs_x, w->width, w->height,
                    " ", c.fg, c.bg, TUI_ATTR_NONE);

    if (w->width > 2) {
        tui_screen_write(ctx, w->abs_y, w->abs_x + 1, label,
                         c.fg, c.bg, TUI_ATTR_BOLD);
    }

    char dim[32];
    int dim_len = snprintf(dim, sizeof(dim), "%dx%d", w->width, w->height);
    if (w->height >= 2 && w->width > 2) {
        tui_screen_write(ctx, w->abs_y + 1, w->abs_x + 1, dim,
                         c.fg, c.bg, TUI_ATTR_DIM);
    } else if (dim_len < w->width - 2) {
        tui_screen_write(ctx, w->abs_y, w->abs_x + w->width - dim_len - 1,
                         dim, c.fg, c.bg, TUI_ATTR_DIM);
    }
}

static void draw_frame(TuiContext *ctx, int rows, int cols)
{
    tui_screen_clear(ctx);

    tui_screen_fill(ctx, 0, 0, cols, 1, " ", TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4), TUI_ATTR_BOLD);
    tui_screen_write(ctx, 0, 2, "Phase 6 - Weighted Fill Layouts (2:1, 1:1:1)",
                     TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4), TUI_ATTR_BOLD);

    char info[64];
    int info_len = snprintf(info, sizeof(info), "%dx%d  |  Press 'q' to quit",
                            cols, rows);
    tui_screen_write(ctx, 0, cols - info_len - 1, info, TUI_COLOR_INDEX(15), TUI_COLOR_INDEX(4), TUI_ATTR_BOLD);

    tui_widget_render(&root.base);

    const char *labels[5] = {
        "2:1 ratio [weight=2]",
        "2:1 ratio [weight=1]",
        "1:1:1 equal [weight=1]",
        "1:1:1 equal [weight=1]",
        "1:1:1 equal [weight=1]"
    };

    for (int i = 0; i < 5; i++) {
        draw_panel(ctx, &panels[i], labels[i], colors[i]);
    }

    tui_screen_write(ctx, rows - 1, 1,
        "Resize terminal to see panels redistribute space proportionally",
        TUI_COLOR_INDEX(8), TUI_COLOR_INDEX(0), TUI_ATTR_DIM);

    tui_screen_render(ctx);
}

static int input_callback(const TuiEvent *event, void *user_data)
{
    TuiContext *ctx = (TuiContext *)user_data;

    if (event->key == TUI_EVENT_RESIZE) {
        tui_screen_resize(ctx, event->size.rows, event->size.cols);
        tui_widget_resize(&root.base, event->size.cols, event->size.rows);
        draw_frame(ctx, event->size.rows, event->size.cols);
        return 0;
    }

    if (event->key == TUI_KEY_ESCAPE || event->key == TUI_KEY_CTRL_C) return 1;
    if (event->codepoint == 'q' && event->modifiers == TUI_MOD_NONE) return 1;

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

    static const int bg_vals[5] = {21, 54, 97, 132, 165};
    for (int i = 0; i < 5; i++) {
        colors[i].fg = TUI_COLOR_INDEX(15);
        colors[i].bg = TUI_COLOR_INDEX(bg_vals[i]);
    }

    TuiSize size = tui_screen_size(&ctx);

    tui_layout_init_ctx(&root, &ctx, TUI_LAYOUT_VERTICAL, 0, 0, size.cols, size.rows);
    tui_layout_set_margin(&root, 1, 0, 1, 0);
    tui_layout_set_padding(&root, 1);

    static TuiLayout top_row;
    tui_layout_init_ctx(&top_row, &ctx, TUI_LAYOUT_HORIZONTAL, 0, 0, 1, 1);
    tui_layout_set_padding(&top_row, 1);
    tui_layout_add(&root, &top_row.base, TUI_LAYOUT_FILL_WEIGHT(2));

    static TuiLayout bot_row;
    tui_layout_init_ctx(&bot_row, &ctx, TUI_LAYOUT_HORIZONTAL, 0, 0, 1, 1);
    tui_layout_set_padding(&bot_row, 1);
    tui_layout_add(&root, &bot_row.base, TUI_LAYOUT_FILL_WEIGHT(3));

    for (int i = 0; i < 5; i++) {
        tui_widget_init_ctx(&panels[i], 0, 0, 1, 1, NULL, &ctx, NULL);
    }

    tui_layout_add(&top_row, &panels[0], TUI_LAYOUT_FILL_WEIGHT(2));
    tui_layout_add(&top_row, &panels[1], TUI_LAYOUT_FILL);

    tui_layout_add(&bot_row, &panels[2], TUI_LAYOUT_FILL);
    tui_layout_add(&bot_row, &panels[3], TUI_LAYOUT_FILL);
    tui_layout_add(&bot_row, &panels[4], TUI_LAYOUT_FILL);

    tui_widget_resize(&root.base, size.cols, size.rows);

    draw_frame(&ctx, size.rows, size.cols);

    tui_input_loop(&ctx, input_callback, &ctx);

    tui_layout_remove_all(&top_row);
    tui_layout_remove_all(&bot_row);
    tui_layout_remove_all(&root);

    tui_input_shutdown(&ctx);
    tui_shutdown(&ctx);
    return 0;
}
