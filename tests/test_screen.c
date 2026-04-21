#include "util.h"
#include "zephio_screen.h"
#include "zephio_context.h"

static ZephioContext g_test_ctx;

/* ── screen_init / free ─────────────────────────────────────────── */

TEST_BEGIN(screen_init_basic)
{
    ZephioResult res = zephio_screen_init(&g_test_ctx, 24, 80);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(g_test_ctx.screen.rows, 24);
    TEST_EQ(g_test_ctx.screen.cols, 80);
    TEST_ASSERT(g_test_ctx.screen.front != NULL);
    TEST_ASSERT(g_test_ctx.screen.back != NULL);
    TEST_EQ(g_test_ctx.screen.initialized, 1);
    zephio_screen_free(&g_test_ctx);
}

TEST_BEGIN(screen_init_invalid)
{
    TEST_NE(zephio_screen_init(&g_test_ctx, 0, 80), ZEPHIO_OK);
    TEST_NE(zephio_screen_init(&g_test_ctx, 24, 0), ZEPHIO_OK);
    TEST_NE(zephio_screen_init(&g_test_ctx, -1, 80), ZEPHIO_OK);
    TEST_NE(zephio_screen_init(&g_test_ctx, 24, -1), ZEPHIO_OK);
}

TEST_BEGIN(screen_free_uninit)
{
    g_test_ctx.screen.initialized = 0;
    g_test_ctx.screen.front = NULL;
    g_test_ctx.screen.back = NULL;
    zephio_screen_free(&g_test_ctx);
    TEST_EQ(g_test_ctx.screen.initialized, 0);
}

TEST_BEGIN(screen_double_init)
{
    zephio_screen_init(&g_test_ctx, 10, 10);
    zephio_screen_set_cell(&g_test_ctx, 0, 0, "X", ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE, 0);
    ZephioResult res = zephio_screen_init(&g_test_ctx, 20, 20);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(g_test_ctx.screen.rows, 20);
    TEST_EQ(g_test_ctx.screen.cols, 20);
    TEST_EQ(g_test_ctx.screen.back[0].ch[0], ' ');
    zephio_screen_free(&g_test_ctx);
}

/* ── screen_size ────────────────────────────────────────────────── */

TEST_BEGIN(screen_size)
{
    zephio_screen_init(&g_test_ctx, 30, 60);
    ZephioSize s = zephio_screen_size(&g_test_ctx);
    TEST_EQ(s.rows, 30);
    TEST_EQ(s.cols, 60);
    zephio_screen_free(&g_test_ctx);
}

/* ── screen_set_cell ────────────────────────────────────────────── */

TEST_BEGIN(screen_set_cell_basic)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(7);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    zephio_screen_set_cell(&g_test_ctx, 5, 10, "A", fg, bg, ZEPHIO_ATTR_BOLD);

    ZephioCell *cell = &g_test_ctx.screen.back[5 * 80 + 10];
    TEST_EQ(cell->ch[0], 'A');
    TEST_ASSERT(zephio_color_eq(cell->fg, fg));
    TEST_ASSERT(zephio_color_eq(cell->bg, bg));
    TEST_EQ(cell->attr, ZEPHIO_ATTR_BOLD);
    zephio_screen_free(&g_test_ctx);
}

TEST_BEGIN(screen_set_cell_utf8)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    zephio_screen_set_cell(&g_test_ctx, 0, 0, "\xC3\xBC", ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE, 0);

    ZephioCell *cell = &g_test_ctx.screen.back[0];
    TEST_EQ((unsigned char)cell->ch[0], 0xC3);
    TEST_EQ((unsigned char)cell->ch[1], 0xBC);
    TEST_EQ(cell->ch[2], 0);
    TEST_EQ(cell->ch[3], 0);
    zephio_screen_free(&g_test_ctx);
}

TEST_BEGIN(screen_set_cell_out_of_bounds)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    zephio_screen_set_cell(&g_test_ctx, -1, 0, "X", ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE, 0);
    zephio_screen_set_cell(&g_test_ctx, 24, 0, "X", ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE, 0);
    zephio_screen_set_cell(&g_test_ctx, 0, -1, "X", ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE, 0);
    zephio_screen_set_cell(&g_test_ctx, 0, 80, "X", ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE, 0);
    zephio_screen_free(&g_test_ctx);
}

/* ── screen_write ───────────────────────────────────────────────── */

TEST_BEGIN(screen_write_basic)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    zephio_screen_write(&g_test_ctx, 0, 0, "Hi", ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE, 0);

    TEST_EQ(g_test_ctx.screen.back[0].ch[0], 'H');
    TEST_EQ(g_test_ctx.screen.back[1].ch[0], 'i');
    zephio_screen_free(&g_test_ctx);
}

TEST_BEGIN(screen_write_clips)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    zephio_screen_write(&g_test_ctx, 0, 79, "AB", ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE, 0);

    TEST_EQ(g_test_ctx.screen.back[79].ch[0], 'A');
    zephio_screen_free(&g_test_ctx);
}

TEST_BEGIN(screen_write_null)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    zephio_screen_write(&g_test_ctx, 0, 0, NULL, ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE, 0);
    zephio_screen_free(&g_test_ctx);
}

/* ── screen_fill ────────────────────────────────────────────────── */

TEST_BEGIN(screen_fill_basic)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    zephio_screen_fill(&g_test_ctx, 2, 3, 2, 2, "#", ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE, 0);

    TEST_EQ(g_test_ctx.screen.back[2 * 80 + 3].ch[0], '#');
    TEST_EQ(g_test_ctx.screen.back[2 * 80 + 4].ch[0], '#');
    TEST_EQ(g_test_ctx.screen.back[3 * 80 + 3].ch[0], '#');
    TEST_EQ(g_test_ctx.screen.back[3 * 80 + 4].ch[0], '#');
    zephio_screen_free(&g_test_ctx);
}

/* ── screen_clear ───────────────────────────────────────────────── */

TEST_BEGIN(screen_clear)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    zephio_screen_set_cell(&g_test_ctx, 0, 0, "X", ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE, 0);
    zephio_screen_clear(&g_test_ctx);
    TEST_EQ(g_test_ctx.screen.back[0].ch[0], ' ');
    zephio_screen_free(&g_test_ctx);
}

/* ── screen_resize ──────────────────────────────────────────────── */

TEST_BEGIN(screen_resize_larger)
{
    zephio_screen_init(&g_test_ctx, 10, 10);
    zephio_screen_set_cell(&g_test_ctx, 0, 0, "A", ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE, 0);

    ZephioResult res = zephio_screen_resize(&g_test_ctx, 20, 20);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(g_test_ctx.screen.rows, 20);
    TEST_EQ(g_test_ctx.screen.cols, 20);
    TEST_EQ(g_test_ctx.screen.back[0].ch[0], 'A');
    zephio_screen_free(&g_test_ctx);
}

TEST_BEGIN(screen_resize_smaller)
{
    zephio_screen_init(&g_test_ctx, 10, 10);
    zephio_screen_set_cell(&g_test_ctx, 3, 3, "B", ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE, 0);

    ZephioResult res = zephio_screen_resize(&g_test_ctx, 5, 5);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(g_test_ctx.screen.rows, 5);
    TEST_EQ(g_test_ctx.screen.cols, 5);
    TEST_EQ(g_test_ctx.screen.back[3 * 5 + 3].ch[0], 'B');
    zephio_screen_free(&g_test_ctx);
}

TEST_BEGIN(screen_resize_same)
{
    zephio_screen_init(&g_test_ctx, 10, 10);
    ZephioResult res = zephio_screen_resize(&g_test_ctx, 10, 10);
    TEST_EQ(res, ZEPHIO_OK);
    zephio_screen_free(&g_test_ctx);
}

TEST_BEGIN(screen_resize_invalid)
{
    zephio_screen_init(&g_test_ctx, 10, 10);
    TEST_NE(zephio_screen_resize(&g_test_ctx, 0, 10), ZEPHIO_OK);
    TEST_EQ(g_test_ctx.screen.rows, 10);
    zephio_screen_free(&g_test_ctx);
}

/* ── screen_box_single ──────────────────────────────────────────── */

TEST_BEGIN(screen_box_single_basic)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(7);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    zephio_screen_box_single(&g_test_ctx, 0, 0, 4, 3, fg, bg, 0);

    ZephioCell *tl = &g_test_ctx.screen.back[0];
    ZephioCell *tr = &g_test_ctx.screen.back[3];
    ZephioCell *bl = &g_test_ctx.screen.back[2 * 80];
    ZephioCell *br = &g_test_ctx.screen.back[2 * 80 + 3];
    ZephioCell *top = &g_test_ctx.screen.back[1];
    ZephioCell *side = &g_test_ctx.screen.back[80];

    TEST_ASSERT(memcmp(tl->ch, "\xe2\x94\x8c", 3) == 0);
    TEST_ASSERT(memcmp(tr->ch, "\xe2\x94\x90", 3) == 0);
    TEST_ASSERT(memcmp(bl->ch, "\xe2\x94\x94", 3) == 0);
    TEST_ASSERT(memcmp(br->ch, "\xe2\x94\x98", 3) == 0);
    TEST_ASSERT(memcmp(top->ch, "\xe2\x94\x80", 3) == 0);
    TEST_ASSERT(memcmp(side->ch, "\xe2\x94\x82", 3) == 0);
    zephio_screen_free(&g_test_ctx);
}

/* ── screen_box_double ──────────────────────────────────────────── */

TEST_BEGIN(screen_box_double_basic)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(7);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    zephio_screen_box_double(&g_test_ctx, 0, 0, 4, 3, fg, bg, 0);

    ZephioCell *tl = &g_test_ctx.screen.back[0];
    ZephioCell *tr = &g_test_ctx.screen.back[3];
    ZephioCell *bl = &g_test_ctx.screen.back[2 * 80];
    ZephioCell *br = &g_test_ctx.screen.back[2 * 80 + 3];

    TEST_ASSERT(memcmp(tl->ch, "\xe2\x95\x94", 3) == 0);
    TEST_ASSERT(memcmp(tr->ch, "\xe2\x95\x97", 3) == 0);
    TEST_ASSERT(memcmp(bl->ch, "\xe2\x95\x9a", 3) == 0);
    TEST_ASSERT(memcmp(br->ch, "\xe2\x95\x9d", 3) == 0);
    zephio_screen_free(&g_test_ctx);
}

TEST_BEGIN(screen_box_too_small)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    zephio_screen_box_single(&g_test_ctx, 0, 0, 1, 1, ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE, 0);
    zephio_screen_box_double(&g_test_ctx, 0, 0, 1, 1, ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE, 0);
    zephio_screen_free(&g_test_ctx);
}

/* ── screen_invert_cell ─────────────────────────────────────────── */

TEST_BEGIN(screen_invert_cell)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(7);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    zephio_screen_set_cell(&g_test_ctx, 5, 5, "X", fg, bg, 0);

    zephio_screen_invert_cell(&g_test_ctx, 5, 5);

    ZephioCell *cell = &g_test_ctx.screen.back[5 * 80 + 5];
    TEST_ASSERT(zephio_color_eq(cell->fg, bg));
    TEST_ASSERT(zephio_color_eq(cell->bg, fg));
    zephio_screen_free(&g_test_ctx);
}

TEST_BEGIN(screen_invert_cell_out_of_bounds)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    zephio_screen_invert_cell(&g_test_ctx, -1, 0);
    zephio_screen_invert_cell(&g_test_ctx, 24, 0);
    zephio_screen_invert_cell(&g_test_ctx, 0, -1);
    zephio_screen_invert_cell(&g_test_ctx, 0, 80);
    zephio_screen_free(&g_test_ctx);
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void)
{
    fprintf(stderr, "Running screen tests...\n\n");

    TEST_RUN(screen_init_basic);
    TEST_RUN(screen_init_invalid);
    TEST_RUN(screen_free_uninit);
    TEST_RUN(screen_double_init);

    TEST_RUN(screen_size);

    TEST_RUN(screen_set_cell_basic);
    TEST_RUN(screen_set_cell_utf8);
    TEST_RUN(screen_set_cell_out_of_bounds);

    TEST_RUN(screen_write_basic);
    TEST_RUN(screen_write_clips);
    TEST_RUN(screen_write_null);

    TEST_RUN(screen_fill_basic);

    TEST_RUN(screen_clear);

    TEST_RUN(screen_resize_larger);
    TEST_RUN(screen_resize_smaller);
    TEST_RUN(screen_resize_same);
    TEST_RUN(screen_resize_invalid);

    TEST_RUN(screen_box_single_basic);
    TEST_RUN(screen_box_double_basic);
    TEST_RUN(screen_box_too_small);

    TEST_RUN(screen_invert_cell);
    TEST_RUN(screen_invert_cell_out_of_bounds);

    TEST_SUMMARY();
}
