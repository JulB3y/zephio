#include "util.h"
#include "tui_style.h"

/* ── color equality ─────────────────────────────────────────────── */

TEST_BEGIN(color_eq_same_index)
{
    TuiColor a = TUI_COLOR_INDEX(5);
    TuiColor b = TUI_COLOR_INDEX(5);
    TEST_ASSERT(tui_color_eq(a, b));
}

TEST_BEGIN(color_eq_diff_index)
{
    TuiColor a = TUI_COLOR_INDEX(5);
    TuiColor b = TUI_COLOR_INDEX(7);
    TEST_ASSERT(!tui_color_eq(a, b));
}

TEST_BEGIN(color_eq_same_rgb)
{
    TuiColor a = TUI_COLOR_RGB(100, 150, 200);
    TuiColor b = TUI_COLOR_RGB(100, 150, 200);
    TEST_ASSERT(tui_color_eq(a, b));
}

TEST_BEGIN(color_eq_diff_rgb)
{
    TuiColor a = TUI_COLOR_RGB(100, 150, 200);
    TuiColor b = TUI_COLOR_RGB(100, 150, 201);
    TEST_ASSERT(!tui_color_eq(a, b));
}

TEST_BEGIN(color_eq_diff_type)
{
    TuiColor a = TUI_COLOR_INDEX(5);
    TuiColor b = TUI_COLOR_RGB(5, 0, 0);
    TEST_ASSERT(!tui_color_eq(a, b));
}

TEST_BEGIN(color_eq_both_none)
{
    TEST_ASSERT(tui_color_eq(TUI_COLOR_NONE, TUI_COLOR_NONE));
}

/* ── style_make ─────────────────────────────────────────────────── */

TEST_BEGIN(style_make)
{
    TuiColor fg = TUI_COLOR_INDEX(15);
    TuiColor bg = TUI_COLOR_INDEX(0);
    TuiStyle s = tui_style_make(fg, bg, TUI_ATTR_BOLD);
    TEST_ASSERT(tui_color_eq(s.fg, fg));
    TEST_ASSERT(tui_color_eq(s.bg, bg));
    TEST_EQ(s.attr, TUI_ATTR_BOLD);
}

TEST_BEGIN(style_macro)
{
    TuiStyle s = TUI_STYLE(TUI_COLOR_WHITE, TUI_COLOR_BLACK, TUI_ATTR_NONE);
    TEST_ASSERT(tui_color_eq(s.fg, TUI_COLOR_INDEX(TUI_COLOR_WHITE)));
    TEST_ASSERT(tui_color_eq(s.bg, TUI_COLOR_INDEX(TUI_COLOR_BLACK)));
    TEST_EQ(s.attr, TUI_ATTR_NONE);
}

TEST_BEGIN(style_rgb_macro)
{
    TuiStyle s = TUI_STYLE_RGB(10, 20, 30, 40, 50, 60, TUI_ATTR_ITALIC);
    TEST_ASSERT(tui_color_eq(s.fg, TUI_COLOR_RGB(10, 20, 30)));
    TEST_ASSERT(tui_color_eq(s.bg, TUI_COLOR_RGB(40, 50, 60)));
    TEST_EQ(s.attr, TUI_ATTR_ITALIC);
}

/* ── theme_default ──────────────────────────────────────────────── */

TEST_BEGIN(theme_default)
{
    TuiTheme theme = tui_theme_default();

    TEST_EQ(theme.styles[TUI_STATE_NORMAL].attr, TUI_ATTR_NONE);
    TEST_EQ(theme.styles[TUI_STATE_FOCUSED].attr, TUI_ATTR_BOLD);
    TEST_EQ(theme.styles[TUI_STATE_DISABLED].attr, TUI_ATTR_DIM);
    TEST_EQ(theme.styles[TUI_STATE_ACTIVE].attr, TUI_ATTR_BOLD);
    TEST_EQ(theme.styles[TUI_STATE_HOVER].attr, TUI_ATTR_NONE);
}

/* ── theme_create ───────────────────────────────────────────────── */

TEST_BEGIN(theme_create_full)
{
    TuiStyle normal = TUI_STYLE(7, 0, 0);
    TuiStyle focused = TUI_STYLE(0, 14, TUI_ATTR_BOLD);
    TuiStyle disabled = TUI_STYLE(8, 0, TUI_ATTR_DIM);
    TuiStyle active = TUI_STYLE(15, 12, TUI_ATTR_BOLD);

    TuiTheme theme = tui_theme_create(&normal, &focused, &disabled, &active);

    TEST_ASSERT(tui_color_eq(theme.styles[TUI_STATE_NORMAL].fg, TUI_COLOR_INDEX(7)));
    TEST_EQ(theme.styles[TUI_STATE_NORMAL].attr, 0);
    TEST_ASSERT(tui_color_eq(theme.styles[TUI_STATE_FOCUSED].fg, TUI_COLOR_INDEX(0)));
    TEST_EQ(theme.styles[TUI_STATE_FOCUSED].attr, TUI_ATTR_BOLD);
}

TEST_BEGIN(theme_create_null)
{
    TuiTheme theme = tui_theme_create(NULL, NULL, NULL, NULL);

    TEST_EQ(theme.styles[TUI_STATE_NORMAL].attr, TUI_ATTR_NONE);
    TEST_EQ(theme.styles[TUI_STATE_FOCUSED].attr, TUI_ATTR_NONE);
    TEST_EQ(theme.styles[TUI_STATE_DISABLED].attr, TUI_ATTR_NONE);
    TEST_EQ(theme.styles[TUI_STATE_ACTIVE].attr, TUI_ATTR_NONE);
}

/* ── style set_cell / write / fill ──────────────────────────────── */

TEST_BEGIN(style_set_cell)
{
    tui_screen_init(24, 80);
    TuiStyle s = TUI_STYLE(7, 0, TUI_ATTR_BOLD);
    tui_style_set_cell(5, 10, "Z", &s);

    TuiCell *cell = &g_screen.back[5 * 80 + 10];
    TEST_EQ(cell->ch[0], 'Z');
    TEST_ASSERT(tui_color_eq(cell->fg, TUI_COLOR_INDEX(7)));
    TEST_ASSERT(tui_color_eq(cell->bg, TUI_COLOR_INDEX(0)));
    TEST_EQ(cell->attr, TUI_ATTR_BOLD);
    tui_screen_free();
}

TEST_BEGIN(style_write)
{
    tui_screen_init(24, 80);
    TuiStyle s = TUI_STYLE(3, 1, TUI_ATTR_UNDERLINE);
    tui_style_write(0, 0, "AB", &s);

    TEST_EQ(g_screen.back[0].ch[0], 'A');
    TEST_EQ(g_screen.back[1].ch[0], 'B');
    TEST_ASSERT(tui_color_eq(g_screen.back[0].fg, TUI_COLOR_INDEX(3)));
    TEST_EQ(g_screen.back[0].attr, TUI_ATTR_UNDERLINE);
    tui_screen_free();
}

TEST_BEGIN(style_fill)
{
    tui_screen_init(24, 80);
    TuiStyle s = TUI_STYLE(2, 4, 0);
    tui_style_fill(0, 0, 3, 2, ".", &s);

    TEST_EQ(g_screen.back[0].ch[0], '.');
    TEST_EQ(g_screen.back[1].ch[0], '.');
    TEST_EQ(g_screen.back[2].ch[0], '.');
    TEST_EQ(g_screen.back[80].ch[0], '.');
    TEST_EQ(g_screen.back[81].ch[0], '.');
    TEST_EQ(g_screen.back[82].ch[0], '.');
    tui_screen_free();
}

TEST_BEGIN(style_null_guard)
{
    tui_screen_init(24, 80);
    tui_style_set_cell(0, 0, "X", NULL);
    tui_style_write(0, 0, "X", NULL);
    tui_style_fill(0, 0, 1, 1, "X", NULL);
    tui_screen_free();
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
