#include "util.h"
#include "tui_screen.h"

/* ── screen_init / free ─────────────────────────────────────────── */

TEST_BEGIN(screen_init_basic)
{
    TuiResult res = tui_screen_init(24, 80);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(g_screen.rows, 24);
    TEST_EQ(g_screen.cols, 80);
    TEST_ASSERT(g_screen.front != NULL);
    TEST_ASSERT(g_screen.back != NULL);
    TEST_EQ(g_screen.initialized, 1);
    tui_screen_free();
}

TEST_BEGIN(screen_init_invalid)
{
    TEST_NE(tui_screen_init(0, 80), TUI_OK);
    TEST_NE(tui_screen_init(24, 0), TUI_OK);
    TEST_NE(tui_screen_init(-1, 80), TUI_OK);
    TEST_NE(tui_screen_init(24, -1), TUI_OK);
}

TEST_BEGIN(screen_free_uninit)
{
    g_screen.initialized = 0;
    g_screen.front = NULL;
    g_screen.back = NULL;
    tui_screen_free();
    TEST_EQ(g_screen.initialized, 0);
}

TEST_BEGIN(screen_double_init)
{
    tui_screen_init(10, 10);
    tui_screen_set_cell(0, 0, "X", TUI_COLOR_NONE, TUI_COLOR_NONE, 0);
    TuiResult res = tui_screen_init(20, 20);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(g_screen.rows, 20);
    TEST_EQ(g_screen.cols, 20);
    TEST_EQ(g_screen.back[0].ch[0], ' ');
    tui_screen_free();
}

/* ── screen_size ────────────────────────────────────────────────── */

TEST_BEGIN(screen_size)
{
    tui_screen_init(30, 60);
    TuiSize s = tui_screen_size();
    TEST_EQ(s.rows, 30);
    TEST_EQ(s.cols, 60);
    tui_screen_free();
}

/* ── screen_set_cell ────────────────────────────────────────────── */

TEST_BEGIN(screen_set_cell_basic)
{
    tui_screen_init(24, 80);
    TuiColor fg = TUI_COLOR_INDEX(7);
    TuiColor bg = TUI_COLOR_INDEX(0);
    tui_screen_set_cell(5, 10, "A", fg, bg, TUI_ATTR_BOLD);

    TuiCell *cell = &g_screen.back[5 * 80 + 10];
    TEST_EQ(cell->ch[0], 'A');
    TEST_ASSERT(tui_color_eq(cell->fg, fg));
    TEST_ASSERT(tui_color_eq(cell->bg, bg));
    TEST_EQ(cell->attr, TUI_ATTR_BOLD);
    tui_screen_free();
}

TEST_BEGIN(screen_set_cell_utf8)
{
    tui_screen_init(24, 80);
    tui_screen_set_cell(0, 0, "\xC3\xBC", TUI_COLOR_NONE, TUI_COLOR_NONE, 0);

    TuiCell *cell = &g_screen.back[0];
    TEST_EQ((unsigned char)cell->ch[0], 0xC3);
    TEST_EQ((unsigned char)cell->ch[1], 0xBC);
    TEST_EQ(cell->ch[2], 0);
    TEST_EQ(cell->ch[3], 0);
    tui_screen_free();
}

TEST_BEGIN(screen_set_cell_out_of_bounds)
{
    tui_screen_init(24, 80);
    tui_screen_set_cell(-1, 0, "X", TUI_COLOR_NONE, TUI_COLOR_NONE, 0);
    tui_screen_set_cell(24, 0, "X", TUI_COLOR_NONE, TUI_COLOR_NONE, 0);
    tui_screen_set_cell(0, -1, "X", TUI_COLOR_NONE, TUI_COLOR_NONE, 0);
    tui_screen_set_cell(0, 80, "X", TUI_COLOR_NONE, TUI_COLOR_NONE, 0);
    tui_screen_free();
}

/* ── screen_write ───────────────────────────────────────────────── */

TEST_BEGIN(screen_write_basic)
{
    tui_screen_init(24, 80);
    tui_screen_write(0, 0, "Hi", TUI_COLOR_NONE, TUI_COLOR_NONE, 0);

    TEST_EQ(g_screen.back[0].ch[0], 'H');
    TEST_EQ(g_screen.back[1].ch[0], 'i');
    tui_screen_free();
}

TEST_BEGIN(screen_write_clips)
{
    tui_screen_init(24, 80);
    tui_screen_write(0, 79, "AB", TUI_COLOR_NONE, TUI_COLOR_NONE, 0);

    TEST_EQ(g_screen.back[79].ch[0], 'A');
    tui_screen_free();
}

TEST_BEGIN(screen_write_null)
{
    tui_screen_init(24, 80);
    tui_screen_write(0, 0, NULL, TUI_COLOR_NONE, TUI_COLOR_NONE, 0);
    tui_screen_free();
}

/* ── screen_fill ────────────────────────────────────────────────── */

TEST_BEGIN(screen_fill_basic)
{
    tui_screen_init(24, 80);
    tui_screen_fill(2, 3, 2, 2, "#", TUI_COLOR_NONE, TUI_COLOR_NONE, 0);

    TEST_EQ(g_screen.back[2 * 80 + 3].ch[0], '#');
    TEST_EQ(g_screen.back[2 * 80 + 4].ch[0], '#');
    TEST_EQ(g_screen.back[3 * 80 + 3].ch[0], '#');
    TEST_EQ(g_screen.back[3 * 80 + 4].ch[0], '#');
    tui_screen_free();
}

/* ── screen_clear ───────────────────────────────────────────────── */

TEST_BEGIN(screen_clear)
{
    tui_screen_init(24, 80);
    tui_screen_set_cell(0, 0, "X", TUI_COLOR_NONE, TUI_COLOR_NONE, 0);
    tui_screen_clear();
    TEST_EQ(g_screen.back[0].ch[0], ' ');
    tui_screen_free();
}

/* ── screen_resize ──────────────────────────────────────────────── */

TEST_BEGIN(screen_resize_larger)
{
    tui_screen_init(10, 10);
    tui_screen_set_cell(0, 0, "A", TUI_COLOR_NONE, TUI_COLOR_NONE, 0);

    TuiResult res = tui_screen_resize(20, 20);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(g_screen.rows, 20);
    TEST_EQ(g_screen.cols, 20);
    TEST_EQ(g_screen.back[0].ch[0], 'A');
    tui_screen_free();
}

TEST_BEGIN(screen_resize_smaller)
{
    tui_screen_init(10, 10);
    tui_screen_set_cell(3, 3, "B", TUI_COLOR_NONE, TUI_COLOR_NONE, 0);

    TuiResult res = tui_screen_resize(5, 5);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(g_screen.rows, 5);
    TEST_EQ(g_screen.cols, 5);
    TEST_EQ(g_screen.back[3 * 5 + 3].ch[0], 'B');
    tui_screen_free();
}

TEST_BEGIN(screen_resize_same)
{
    tui_screen_init(10, 10);
    TuiResult res = tui_screen_resize(10, 10);
    TEST_EQ(res, TUI_OK);
    tui_screen_free();
}

TEST_BEGIN(screen_resize_invalid)
{
    tui_screen_init(10, 10);
    TEST_NE(tui_screen_resize(0, 10), TUI_OK);
    TEST_EQ(g_screen.rows, 10);
    tui_screen_free();
}

/* ── screen_box_single ──────────────────────────────────────────── */

TEST_BEGIN(screen_box_single_basic)
{
    tui_screen_init(24, 80);
    TuiColor fg = TUI_COLOR_INDEX(7);
    TuiColor bg = TUI_COLOR_INDEX(0);
    tui_screen_box_single(0, 0, 4, 3, fg, bg, 0);

    TuiCell *tl = &g_screen.back[0];
    TuiCell *tr = &g_screen.back[3];
    TuiCell *bl = &g_screen.back[2 * 80];
    TuiCell *br = &g_screen.back[2 * 80 + 3];
    TuiCell *top = &g_screen.back[1];
    TuiCell *side = &g_screen.back[80];

    TEST_ASSERT(memcmp(tl->ch, "\xe2\x94\x8c", 3) == 0);
    TEST_ASSERT(memcmp(tr->ch, "\xe2\x94\x90", 3) == 0);
    TEST_ASSERT(memcmp(bl->ch, "\xe2\x94\x94", 3) == 0);
    TEST_ASSERT(memcmp(br->ch, "\xe2\x94\x98", 3) == 0);
    TEST_ASSERT(memcmp(top->ch, "\xe2\x94\x80", 3) == 0);
    TEST_ASSERT(memcmp(side->ch, "\xe2\x94\x82", 3) == 0);
    tui_screen_free();
}

/* ── screen_box_double ──────────────────────────────────────────── */

TEST_BEGIN(screen_box_double_basic)
{
    tui_screen_init(24, 80);
    TuiColor fg = TUI_COLOR_INDEX(7);
    TuiColor bg = TUI_COLOR_INDEX(0);
    tui_screen_box_double(0, 0, 4, 3, fg, bg, 0);

    TuiCell *tl = &g_screen.back[0];
    TuiCell *tr = &g_screen.back[3];
    TuiCell *bl = &g_screen.back[2 * 80];
    TuiCell *br = &g_screen.back[2 * 80 + 3];

    TEST_ASSERT(memcmp(tl->ch, "\xe2\x95\x94", 3) == 0);
    TEST_ASSERT(memcmp(tr->ch, "\xe2\x95\x97", 3) == 0);
    TEST_ASSERT(memcmp(bl->ch, "\xe2\x95\x9a", 3) == 0);
    TEST_ASSERT(memcmp(br->ch, "\xe2\x95\x9d", 3) == 0);
    tui_screen_free();
}

TEST_BEGIN(screen_box_too_small)
{
    tui_screen_init(24, 80);
    tui_screen_box_single(0, 0, 1, 1, TUI_COLOR_NONE, TUI_COLOR_NONE, 0);
    tui_screen_box_double(0, 0, 1, 1, TUI_COLOR_NONE, TUI_COLOR_NONE, 0);
    tui_screen_free();
}

/* ── screen_invert_cell ─────────────────────────────────────────── */

TEST_BEGIN(screen_invert_cell)
{
    tui_screen_init(24, 80);
    TuiColor fg = TUI_COLOR_INDEX(7);
    TuiColor bg = TUI_COLOR_INDEX(0);
    tui_screen_set_cell(5, 5, "X", fg, bg, 0);

    tui_screen_invert_cell(5, 5);

    TuiCell *cell = &g_screen.back[5 * 80 + 5];
    TEST_ASSERT(tui_color_eq(cell->fg, bg));
    TEST_ASSERT(tui_color_eq(cell->bg, fg));
    tui_screen_free();
}

TEST_BEGIN(screen_invert_cell_out_of_bounds)
{
    tui_screen_init(24, 80);
    tui_screen_invert_cell(-1, 0);
    tui_screen_invert_cell(24, 0);
    tui_screen_invert_cell(0, -1);
    tui_screen_invert_cell(0, 80);
    tui_screen_free();
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
