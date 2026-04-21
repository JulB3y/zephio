#define _POSIX_C_SOURCE 200809L

#include "zephio.h"
#include "zephio_input.h"
#include "zephio_screen.h"
#include "zephio_terminal.h"
#include "zephio_layout.h"

#include <stdio.h>
#include <string.h>

static ZephioLayout root_layout;
static ZephioWidget header;
static ZephioLayout body_layout;
static ZephioWidget sidebar;
static ZephioWidget content;
static ZephioWidget footer;

static void draw_panel(ZephioContext *ctx, ZephioWidget *w, const char *label,
                       ZephioColor fg, ZephioColor bg, ZephioAttr attr)
{
    if (!w->visible || w->width <= 0 || w->height <= 0) return;

    zephio_screen_fill(ctx, w->abs_y, w->abs_x, w->width, w->height, " ", fg, bg, attr);

    if (w->width > 2) {
        zephio_screen_write(ctx, w->abs_y, w->abs_x + 1, label, fg, bg, ZEPHIO_ATTR_BOLD);
    }

    char dim[32];
    int dim_len = snprintf(dim, sizeof(dim), "%dx%d", w->width, w->height);
    if (dim_len < w->width - 2) {
        zephio_screen_write(ctx, w->abs_y, w->abs_x + w->width - dim_len - 1,
                         dim, fg, bg, ZEPHIO_ATTR_DIM);
    }
}

static void draw_frame(ZephioContext *ctx, int rows, int cols)
{
    zephio_screen_clear(ctx);

    zephio_screen_fill(ctx, 0, 0, cols, 1, " ", ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);
    zephio_screen_write(ctx, 0, 2, "Phase 6 - Basic Layout (Fixed / Fill / Auto)",
                     ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);

    char info[64];
    int info_len = snprintf(info, sizeof(info), "%dx%d", cols, rows);
    zephio_screen_write(ctx, 0, cols - info_len - 1, info, ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(4), ZEPHIO_ATTR_BOLD);

    zephio_widget_render(&root_layout.base);

    draw_panel(ctx, &header, "Header [Fixed h=1]", ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236), ZEPHIO_ATTR_BOLD);
    draw_panel(ctx, &sidebar, "Sidebar [Fixed w=20]", ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(24), ZEPHIO_ATTR_NONE);
    draw_panel(ctx, &content, "Content [Fill]", ZEPHIO_COLOR_INDEX(0), ZEPHIO_COLOR_INDEX(240), ZEPHIO_ATTR_NONE);
    draw_panel(ctx, &footer, "Footer [Fixed h=1]  |  Press 'q' or Escape to quit",
               ZEPHIO_COLOR_INDEX(15), ZEPHIO_COLOR_INDEX(236), ZEPHIO_ATTR_BOLD);

    zephio_screen_render(ctx);
}

static int input_callback(const ZephioEvent *event, void *user_data)
{
    ZephioContext *ctx = (ZephioContext *)user_data;

    if (event->key == ZEPHIO_EVENT_RESIZE) {
        zephio_screen_resize(ctx, event->size.rows, event->size.cols);
        zephio_widget_resize(&root_layout.base, event->size.cols, event->size.rows);
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

    zephio_layout_init_ctx(&root_layout, &ctx, ZEPHIO_LAYOUT_VERTICAL,
                    0, 0, size.cols, size.rows);
    zephio_layout_set_padding(&root_layout, 1);

    zephio_widget_init_ctx(&header, 0, 0, 1, 1, NULL, &ctx, NULL);
    zephio_layout_add(&root_layout, &header, ZEPHIO_LAYOUT_FIXED(1));

    zephio_layout_init_ctx(&body_layout, &ctx, ZEPHIO_LAYOUT_HORIZONTAL,
                    0, 0, 1, 1);
    zephio_layout_set_padding(&body_layout, 1);
    zephio_layout_add(&root_layout, &body_layout.base, ZEPHIO_LAYOUT_FILL);

    zephio_widget_init_ctx(&sidebar, 0, 0, 20, 1, NULL, &ctx, NULL);
    zephio_layout_add(&body_layout, &sidebar, ZEPHIO_LAYOUT_FIXED(20));

    zephio_widget_init_ctx(&content, 0, 0, 1, 1, NULL, &ctx, NULL);
    zephio_layout_add(&body_layout, &content, ZEPHIO_LAYOUT_FILL);

    zephio_widget_init_ctx(&footer, 0, 0, 1, 1, NULL, &ctx, NULL);
    zephio_layout_add(&root_layout, &footer, ZEPHIO_LAYOUT_FIXED(1));

    zephio_widget_resize(&root_layout.base, size.cols, size.rows);

    draw_frame(&ctx, size.rows, size.cols);

    zephio_input_loop(&ctx, input_callback, &ctx);

    zephio_layout_remove_all(&body_layout);
    zephio_layout_remove_all(&root_layout);

    zephio_input_shutdown(&ctx);
    zephio_shutdown(&ctx);
    return 0;
}
