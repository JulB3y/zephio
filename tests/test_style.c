#include "util.h"
#include "zephio_style.h"

/* ── color equality ─────────────────────────────────────────────── */

TEST_BEGIN(color_eq_same_index)
{
    ZephioColor a = ZEPHIO_COLOR_INDEX(5);
    ZephioColor b = ZEPHIO_COLOR_INDEX(5);
    TEST_ASSERT(zephio_color_eq(a, b));
}

TEST_BEGIN(color_eq_diff_index)
{
    ZephioColor a = ZEPHIO_COLOR_INDEX(5);
    ZephioColor b = ZEPHIO_COLOR_INDEX(7);
    TEST_ASSERT(!zephio_color_eq(a, b));
}

TEST_BEGIN(color_eq_same_rgb)
{
    ZephioColor a = ZEPHIO_COLOR_RGB(100, 150, 200);
    ZephioColor b = ZEPHIO_COLOR_RGB(100, 150, 200);
    TEST_ASSERT(zephio_color_eq(a, b));
}

TEST_BEGIN(color_eq_diff_rgb)
{
    ZephioColor a = ZEPHIO_COLOR_RGB(100, 150, 200);
    ZephioColor b = ZEPHIO_COLOR_RGB(100, 150, 201);
    TEST_ASSERT(!zephio_color_eq(a, b));
}

TEST_BEGIN(color_eq_diff_type)
{
    ZephioColor a = ZEPHIO_COLOR_INDEX(5);
    ZephioColor b = ZEPHIO_COLOR_RGB(5, 0, 0);
    TEST_ASSERT(!zephio_color_eq(a, b));
}

TEST_BEGIN(color_eq_both_none)
{
    TEST_ASSERT(zephio_color_eq(ZEPHIO_COLOR_NONE, ZEPHIO_COLOR_NONE));
}

/* ── style_make ─────────────────────────────────────────────────── */

TEST_BEGIN(style_make)
{
    ZephioColor fg = ZEPHIO_COLOR_INDEX(15);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    ZephioStyle s = zephio_style_make(fg, bg, ZEPHIO_ATTR_BOLD);
    TEST_ASSERT(zephio_color_eq(s.fg, fg));
    TEST_ASSERT(zephio_color_eq(s.bg, bg));
    TEST_EQ(s.attr, ZEPHIO_ATTR_BOLD);
}

TEST_BEGIN(style_macro)
{
    ZephioStyle s = TUI_STYLE(ZEPHIO_COLOR_WHITE, ZEPHIO_COLOR_BLACK, ZEPHIO_ATTR_NONE);
    TEST_ASSERT(zephio_color_eq(s.fg, ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_WHITE)));
    TEST_ASSERT(zephio_color_eq(s.bg, ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BLACK)));
    TEST_EQ(s.attr, ZEPHIO_ATTR_NONE);
}

TEST_BEGIN(style_rgb_macro)
{
    ZephioStyle s = TUI_STYLE_RGB(10, 20, 30, 40, 50, 60, ZEPHIO_ATTR_ITALIC);
    TEST_ASSERT(zephio_color_eq(s.fg, ZEPHIO_COLOR_RGB(10, 20, 30)));
    TEST_ASSERT(zephio_color_eq(s.bg, ZEPHIO_COLOR_RGB(40, 50, 60)));
    TEST_EQ(s.attr, ZEPHIO_ATTR_ITALIC);
}

/* ── theme_default ──────────────────────────────────────────────── */

TEST_BEGIN(theme_default)
{
    ZephioTheme theme = zephio_theme_default();

    TEST_EQ(theme.styles[ZEPHIO_STATE_NORMAL].attr, ZEPHIO_ATTR_NONE);
    TEST_EQ(theme.styles[ZEPHIO_STATE_FOCUSED].attr, ZEPHIO_ATTR_BOLD);
    TEST_EQ(theme.styles[ZEPHIO_STATE_DISABLED].attr, ZEPHIO_ATTR_DIM);
    TEST_EQ(theme.styles[ZEPHIO_STATE_ACTIVE].attr, ZEPHIO_ATTR_BOLD);
    TEST_EQ(theme.styles[ZEPHIO_STATE_HOVER].attr, ZEPHIO_ATTR_NONE);
}

/* ── theme_create ───────────────────────────────────────────────── */

TEST_BEGIN(theme_create_full)
{
    ZephioStyle normal = TUI_STYLE(7, 0, 0);
    ZephioStyle focused = TUI_STYLE(0, 14, ZEPHIO_ATTR_BOLD);
    ZephioStyle disabled = TUI_STYLE(8, 0, ZEPHIO_ATTR_DIM);
    ZephioStyle active = TUI_STYLE(15, 12, ZEPHIO_ATTR_BOLD);

    ZephioTheme theme = zephio_theme_create(&normal, &focused, &disabled, &active);

    TEST_ASSERT(zephio_color_eq(theme.styles[ZEPHIO_STATE_NORMAL].fg, ZEPHIO_COLOR_INDEX(7)));
    TEST_EQ(theme.styles[ZEPHIO_STATE_NORMAL].attr, 0);
    TEST_ASSERT(zephio_color_eq(theme.styles[ZEPHIO_STATE_FOCUSED].fg, ZEPHIO_COLOR_INDEX(0)));
    TEST_EQ(theme.styles[ZEPHIO_STATE_FOCUSED].attr, ZEPHIO_ATTR_BOLD);
}

TEST_BEGIN(theme_create_null)
{
    ZephioTheme theme = zephio_theme_create(NULL, NULL, NULL, NULL);

    TEST_EQ(theme.styles[ZEPHIO_STATE_NORMAL].attr, ZEPHIO_ATTR_NONE);
    TEST_EQ(theme.styles[ZEPHIO_STATE_FOCUSED].attr, ZEPHIO_ATTR_NONE);
    TEST_EQ(theme.styles[ZEPHIO_STATE_DISABLED].attr, ZEPHIO_ATTR_NONE);
    TEST_EQ(theme.styles[ZEPHIO_STATE_ACTIVE].attr, ZEPHIO_ATTR_NONE);
}

/* ── style set_cell / write / fill ──────────────────────────────── */

TEST_BEGIN(style_set_cell)
{
    zephio_screen_init(24, 80);
    ZephioStyle s = TUI_STYLE(7, 0, ZEPHIO_ATTR_BOLD);
    zephio_style_set_cell(5, 10, "Z", &s);

    ZephioCell *cell = &g_screen.back[5 * 80 + 10];
    TEST_EQ(cell->ch[0], 'Z');
    TEST_ASSERT(zephio_color_eq(cell->fg, ZEPHIO_COLOR_INDEX(7)));
    TEST_ASSERT(zephio_color_eq(cell->bg, ZEPHIO_COLOR_INDEX(0)));
    TEST_EQ(cell->attr, ZEPHIO_ATTR_BOLD);
    zephio_screen_free();
}

TEST_BEGIN(style_write)
{
    zephio_screen_init(24, 80);
    ZephioStyle s = TUI_STYLE(3, 1, ZEPHIO_ATTR_UNDERLINE);
    zephio_style_write(0, 0, "AB", &s);

    TEST_EQ(g_screen.back[0].ch[0], 'A');
    TEST_EQ(g_screen.back[1].ch[0], 'B');
    TEST_ASSERT(zephio_color_eq(g_screen.back[0].fg, ZEPHIO_COLOR_INDEX(3)));
    TEST_EQ(g_screen.back[0].attr, ZEPHIO_ATTR_UNDERLINE);
    zephio_screen_free();
}

TEST_BEGIN(style_fill)
{
    zephio_screen_init(24, 80);
    ZephioStyle s = TUI_STYLE(2, 4, 0);
    zephio_style_fill(0, 0, 3, 2, ".", &s);

    TEST_EQ(g_screen.back[0].ch[0], '.');
    TEST_EQ(g_screen.back[1].ch[0], '.');
    TEST_EQ(g_screen.back[2].ch[0], '.');
    TEST_EQ(g_screen.back[80].ch[0], '.');
    TEST_EQ(g_screen.back[81].ch[0], '.');
    TEST_EQ(g_screen.back[82].ch[0], '.');
    zephio_screen_free();
}

TEST_BEGIN(style_null_guard)
{
    zephio_screen_init(24, 80);
    zephio_style_set_cell(0, 0, "X", NULL);
    zephio_style_write(0, 0, "X", NULL);
    zephio_style_fill(0, 0, 1, 1, "X", NULL);
    zephio_screen_free();
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void)
{
    fprintf(stderr, "Running style tests...\n\n");

    TEST_RUN(color_eq_same_index);
    TEST_RUN(color_eq_diff_index);
    TEST_RUN(color_eq_same_rgb);
    TEST_RUN(color_eq_diff_rgb);
    TEST_RUN(color_eq_diff_type);
    TEST_RUN(color_eq_both_none);

    TEST_RUN(style_make);
    TEST_RUN(style_macro);
    TEST_RUN(style_rgb_macro);

    TEST_RUN(theme_default);
    TEST_RUN(theme_create_full);
    TEST_RUN(theme_create_null);

    TEST_RUN(style_set_cell);
    TEST_RUN(style_write);
    TEST_RUN(style_fill);
    TEST_RUN(style_null_guard);

    TEST_SUMMARY();
}
