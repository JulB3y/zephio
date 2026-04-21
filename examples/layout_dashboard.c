#define _POSIX_C_SOURCE 200809L

#include "tui.h"
#include "zephio_input.h"
#include "zephio_screen.h"
#include "zephio_terminal.h"
#include "zephio_layout.h"

#include <stdio.h>
#include <string.h>

static ZephioLayout root;
static ZephioWidget titlebar;

static ZephioLayout main_row;
static ZephioWidget nav;
static ZephioLayout center_col;
static ZephioWidget toolbar;
static ZephioLayout content_row;
static ZephioWidget editor;
static ZephioWidget preview;
static ZephioWidget statusbar;

static void draw_panel(ZephioContext *ctx, ZephioWidget *w, const char *label,
                       ZephioColor fg, ZephioColor bg)
{
    if (!w->visible || w->width <= 0 || w->height <= 0) return;

    zephio_screen_box_single(ctx, w->abs_y, w->abs_x, w->width, w->height,
                          fg, bg, ZEPHIO_ATTR_NONE);

    if (w->width > 2) {
        zephio_screen_write(ctx, w->abs_y, w->abs_x + 1, label,
                         fg, bg, ZEPHIO_ATTR_BOLD);
    }

    if (w->height >= 3) {
        for (int r = 1; r < w->height - 1; r++) {
            char line[128];
            snprintf(line, sizeof(line),
                "row %d: x=%d y=%d w=%d h=%d",
                r, w->abs_x, w->abs_y, w->width, w->height);
            zephio_screen_write(ctx, w->abs_y + r, w->abs_x + 1, line,
                             fg, bg, ZEPHIO_ATTR_DIM);
        }
    }

    char dim[32];
    int dim_len = snprintf(dim, sizeof(dim), "%dx%d", w->width, w->height);
    if (w->height >= 2 && w->width > 2 && dim_len < w->width - 2) {
        zephio_screen_write(ctx, w->abs_y, w->abs_x + w->width - dim_len - 1,
                         dim, fg, bg, ZEPHIO_ATTR_DIM);
    }
}

static void draw_bar(ZephioContext *ctx, ZephioWidget *w, const char *text,
                     ZephioColor fg, ZephioColor bg)
{
    if (!w->visible || w->width <= 0 || w->height <= 0) return;
    zephio_screen_fill(ctx, w->abs_y, w->abs_x, w->width, w->height,
                    " ", fg, bg, ZEPHIO_ATTR_BOLD);
    if (w->width > 2) {
        zephio_screen_write(ctx, w->abs_y, w->abs_x + 1, text, fg, bg, ZEPHIO_ATTR_BOLD);
    }
}

static void draw_frame(ZephioContext *ctx, int rows, int cols)
{
    zephio_screen_clear(ctx);

    zephio_widget_render(&root.base);

    draw_bar(ctx, &titlebar,
             "Phase 6 - Dashboard Layout  |  Nested H+V Layouts  |  Resize to test",
             ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4));

    draw_bar(ctx, &toolbar, "  File  Edit  View  Tools  Help", ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(252));

    draw_panel(ctx, &nav, "Navigation", ZEPHIO_COLOR_INDEX(7), ZEPHIO_COLOR_INDEX(194));
    draw_panel(ctx, &editor, "Editor [Fill w=2]", ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(234));
    draw_panel(ctx, &preview, "Preview [Fill w=1]", ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(230));
    draw_bar(ctx, &statusbar,
             "  Layout: nested V>H>V  |  Press 'q' or Escape to quit",
             ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236));

    zephio_screen_render(ctx);
    (void)rows;
    (void)cols;
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

    ZephioSize size = zephio_screen_size(&ctx);

    zephio_layout_init_ctx(&root, &ctx, ZEPHIO_LAYOUT_VERTICAL, 0, 0, size.cols, size.rows);
    zephio_layout_set_padding(&root, 0);

    zephio_widget_init_ctx(&titlebar, 0, 0, 1, 1, NULL, &ctx, NULL);
    zephio_layout_add(&root, &titlebar, ZEPHIO_LAYOUT_FIXED(1));

    zephio_layout_init_ctx(&main_row, &ctx, ZEPHIO_LAYOUT_HORIZONTAL, 0, 0, 1, 1);
    zephio_layout_set_padding(&main_row, 1);
    zephio_layout_add(&root, &main_row.base, ZEPHIO_LAYOUT_FILL);

    zephio_widget_init_ctx(&nav, 0, 0, 18, 1, NULL, &ctx, NULL);
    zephio_layout_add(&main_row, &nav, ZEPHIO_LAYOUT_FIXED(18));

    zephio_layout_init_ctx(&center_col, &ctx, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 1, 1);
    zephio_layout_set_padding(&center_col, 1);
    zephio_layout_add(&main_row, &center_col.base, ZEPHIO_LAYOUT_FILL);

    zephio_widget_init_ctx(&toolbar, 0, 0, 1, 1, NULL, &ctx, NULL);
    zephio_layout_add(&center_col, &toolbar, ZEPHIO_LAYOUT_FIXED(1));

    zephio_layout_init_ctx(&content_row, &ctx, ZEPHIO_LAYOUT_HORIZONTAL, 0, 0, 1, 1);
    zephio_layout_set_padding(&content_row, 1);
    zephio_layout_add(&center_col, &content_row.base, ZEPHIO_LAYOUT_FILL);

    zephio_widget_init_ctx(&editor, 0, 0, 1, 1, NULL, &ctx, NULL);
    zephio_layout_add(&content_row, &editor, ZEPHIO_LAYOUT_FILL_WEIGHT(2));

    zephio_widget_init_ctx(&preview, 0, 0, 1, 1, NULL, &ctx, NULL);
    zephio_layout_add(&content_row, &preview, ZEPHIO_LAYOUT_FILL);

    zephio_widget_init_ctx(&statusbar, 0, 0, 1, 1, NULL, &ctx, NULL);
    zephio_layout_add(&root, &statusbar, ZEPHIO_LAYOUT_FIXED(1));

    zephio_widget_resize(&root.base, size.cols, size.rows);

    draw_frame(&ctx, size.rows, size.cols);

    zephio_input_loop(&ctx, input_callback, &ctx);

    zephio_layout_remove_all(&content_row);
    zephio_layout_remove_all(&center_col);
    zephio_layout_remove_all(&main_row);
    zephio_layout_remove_all(&root);

    zephio_input_shutdown(&ctx);
    zephio_shutdown(&ctx);
    return 0;
}
