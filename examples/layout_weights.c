#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "zephio_input.h"
#include "zephio_screen.h"
#include "zephio_terminal.h"
#include "zephio_layout.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    ZephioColor fg;
    ZephioColor bg;
} PanelColor;

static ZephioLayout root;
static ZephioWidget panels[5];
static PanelColor colors[5];

static void draw_panel(ZephioContext *ctx, ZephioWidget *w, const char *label, PanelColor c)
{
    if (!w->visible || w->width <= 0 || w->height <= 0) return;

    zephio_screen_fill(ctx, w->abs_y, w->abs_x, w->width, w->height,
                    " ", c.fg, c.bg, ZEPHIO_ATTR_NONE);

    if (w->width > 2) {
        zephio_screen_write(ctx, w->abs_y, w->abs_x + 1, label,
                         c.fg, c.bg, ZEPHIO_ATTR_BOLD);
    }

    char dim[32];
    int dim_len = snprintf(dim, sizeof(dim), "%dx%d", w->width, w->height);
    if (w->height >= 2 && w->width > 2) {
        zephio_screen_write(ctx, w->abs_y + 1, w->abs_x + 1, dim,
                         c.fg, c.bg, ZEPHIO_ATTR_DIM);
    } else if (dim_len < w->width - 2) {
        zephio_screen_write(ctx, w->abs_y, w->abs_x + w->width - dim_len - 1,
                         dim, c.fg, c.bg, ZEPHIO_ATTR_DIM);
    }
}

static void draw_frame(ZephioContext *ctx, int rows, int cols)
{
    zephio_screen_clear(ctx);

    zephio_screen_fill(ctx, 0, 0, cols, 1, " ", ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    zephio_screen_write(ctx, 0, 2, "Phase 6 - Weighted Fill Layouts (2:1, 1:1:1)",
                     ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);

    char info[64];
    int info_len = snprintf(info, sizeof(info), "%dx%d  |  Press 'q' to quit",
                            cols, rows);
    zephio_screen_write(ctx, 0, cols - info_len - 1, info, ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);

    zephio_widget_render(&root.base);

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

    zephio_screen_write(ctx, rows - 1, 1,
        "Resize terminal to see panels redistribute space proportionally",
        ZEPHIO_COLOR_INDEX(8), ZEPHIO_COLOR_INDEX(0), ZEPHIO_ATTR_DIM);

    zephio_screen_render(ctx);
}

static int input_callback(const ZephioEvent *event, void *user_data)
{
    ZephioContext *ctx = (ZephioContext *)user_data;

    if (event->key == ZEPHIO_EVENT_RESIZE) {
        zephio_screen_resize(ctx, event->size.rows, event->size.cols);
        zephio_widget_resize(&root.base, event->size.cols, event->size.rows);
        draw_frame(ctx, event->size.rows, event->size.cols);
        return 0;
    }

    if (event->key == ZEPHIO_KEY_ESCAPE || event->key == ZEPHIO_KEY_CTRL_C) return 1;
    if (event->codepoint == 'q' && event->modifiers == ZEPHIO_MOD_NONE) return 1;

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

    static const int bg_vals[5] = {21, 54, 97, 132, 165};
    for (int i = 0; i < 5; i++) {
        colors[i].fg = ZEPHIO_COLOR_INDEX(15);
        colors[i].bg = ZEPHIO_COLOR_INDEX(bg_vals[i]);
    }

    ZephioSize size = zephio_screen_size(&ctx);

    zephio_layout_init_ctx(&root, &ctx, ZEPHIO_LAYOUT_VERTICAL, 0, 0, size.cols, size.rows);
    zephio_layout_set_margin(&root, 1, 0, 1, 0);
    zephio_layout_set_padding(&root, 1);

    static ZephioLayout top_row;
    zephio_layout_init_ctx(&top_row, &ctx, ZEPHIO_LAYOUT_HORIZONTAL, 0, 0, 1, 1);
    zephio_layout_set_padding(&top_row, 1);
    zephio_layout_add(&root, &top_row.base, ZEPHIO_LAYOUT_FILL_WEIGHT(2));

    static ZephioLayout bot_row;
    zephio_layout_init_ctx(&bot_row, &ctx, ZEPHIO_LAYOUT_HORIZONTAL, 0, 0, 1, 1);
    zephio_layout_set_padding(&bot_row, 1);
    zephio_layout_add(&root, &bot_row.base, ZEPHIO_LAYOUT_FILL_WEIGHT(3));

    for (int i = 0; i < 5; i++) {
        zephio_widget_init_ctx(&panels[i], 0, 0, 1, 1, NULL, &ctx, NULL);
    }

    zephio_layout_add(&top_row, &panels[0], ZEPHIO_LAYOUT_FILL_WEIGHT(2));
    zephio_layout_add(&top_row, &panels[1], ZEPHIO_LAYOUT_FILL);

    zephio_layout_add(&bot_row, &panels[2], ZEPHIO_LAYOUT_FILL);
    zephio_layout_add(&bot_row, &panels[3], ZEPHIO_LAYOUT_FILL);
    zephio_layout_add(&bot_row, &panels[4], ZEPHIO_LAYOUT_FILL);

    zephio_widget_resize(&root.base, size.cols, size.rows);

    draw_frame(&ctx, size.rows, size.cols);

    zephio_input_loop(&ctx, input_callback, &ctx);

    zephio_layout_remove_all(&top_row);
    zephio_layout_remove_all(&bot_row);
    zephio_layout_remove_all(&root);

    zephio_input_shutdown(&ctx);
    zephio_shutdown(&ctx);
    return 0;
}
