#include "util.h"
#include "tui_progress.h"
#include "tui_checkbox.h"
#include "tui_radio.h"

#include <string.h>

/* ── Progress Bar ────────────────────────────────────────────────── */

TEST_BEGIN(progress_init)
{
    TuiProgress p;
    TuiResult res = tui_progress_init(&p, 0, 0, 20, 1);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(p.value, 0);
    TEST_EQ(p.show_percent, 1);
    TEST_EQ(p.base.focusable, 0);
    TEST_EQ(p.base.visible, 1);
    tui_widget_destroy(&p.base);
}

TEST_BEGIN(progress_init_null)
{
    TEST_NE(tui_progress_init(NULL, 0, 0, 20, 1), TUI_OK);
}

TEST_BEGIN(progress_set_value)
{
    TuiProgress p;
    tui_progress_init(&p, 0, 0, 20, 1);

    tui_progress_set_value(&p, 50);
    TEST_EQ(tui_progress_get_value(&p), 50);

    tui_progress_set_value(&p, 100);
    TEST_EQ(tui_progress_get_value(&p), 100);

    tui_progress_set_value(&p, 0);
    TEST_EQ(tui_progress_get_value(&p), 0);

    tui_widget_destroy(&p.base);
}

TEST_BEGIN(progress_value_clamped)
{
    TuiProgress p;
    tui_progress_init(&p, 0, 0, 20, 1);

    tui_progress_set_value(&p, -10);
    TEST_EQ(tui_progress_get_value(&p), 0);

    tui_progress_set_value(&p, 150);
    TEST_EQ(tui_progress_get_value(&p), 100);

    tui_widget_destroy(&p.base);
}

TEST_BEGIN(progress_get_value_null)
{
    TEST_EQ(tui_progress_get_value(NULL), 0);
}

TEST_BEGIN(progress_set_label)
{
    TuiProgress p;
    tui_progress_init(&p, 0, 0, 30, 1);

    tui_progress_set_label(&p, "Loading");
    TEST_ASSERT(p.label != NULL);
    TEST_STR_EQ(p.label, "Loading");

    tui_progress_set_label(&p, NULL);
    TEST_ASSERT(p.label == NULL);

    tui_widget_destroy(&p.base);
}

TEST_BEGIN(progress_set_chars)
{
    TuiProgress p;
    tui_progress_init(&p, 0, 0, 20, 1);

    tui_progress_set_chars(&p, "#", "-");
    TEST_STR_EQ(p.fill_char, "#");
    TEST_STR_EQ(p.empty_char, "-");

    tui_progress_set_chars(&p, NULL, NULL);
    TEST_STR_EQ(p.fill_char, "#");
    TEST_STR_EQ(p.empty_char, "-");

    tui_widget_destroy(&p.base);
}

TEST_BEGIN(progress_show_percent)
{
    TuiProgress p;
    tui_progress_init(&p, 0, 0, 20, 1);

    TEST_EQ(p.show_percent, 1);
    tui_progress_set_show_percent(&p, 0);
    TEST_EQ(p.show_percent, 0);
    tui_progress_set_show_percent(&p, 1);
    TEST_EQ(p.show_percent, 1);

    tui_widget_destroy(&p.base);
}

TEST_BEGIN(progress_set_colors)
{
    TuiProgress p;
    tui_progress_init(&p, 0, 0, 20, 1);

    TuiColor fg = TUI_COLOR_INDEX(2);
    TuiColor bg = TUI_COLOR_INDEX(4);
    tui_progress_set_colors(&p, fg, bg, fg, bg);
    TEST_ASSERT(tui_color_eq(p.fg_fill, fg));
    TEST_ASSERT(tui_color_eq(p.bg_fill, bg));

    tui_widget_destroy(&p.base);
}

TEST_BEGIN(progress_set_attr)
{
    TuiProgress p;
    tui_progress_init(&p, 0, 0, 20, 1);

    tui_progress_set_attr(&p, TUI_ATTR_BOLD);
    TEST_EQ(p.attr, TUI_ATTR_BOLD);

    tui_widget_destroy(&p.base);
}

/* ── Checkbox ────────────────────────────────────────────────────── */

TEST_BEGIN(checkbox_init)
{
    TuiCheckbox cb;
    TuiResult res = tui_checkbox_init(&cb, 0, 0, 20, 1, "Enable");
    TEST_EQ(res, TUI_OK);
    TEST_EQ(cb.state, TUI_CHECK_UNCHECKED);
    TEST_EQ(cb.tristate, 0);
    TEST_EQ(cb.base.focusable, 1);
    TEST_ASSERT(cb.label != NULL);
    TEST_STR_EQ(cb.label, "Enable");
    tui_widget_destroy(&cb.base);
}

TEST_BEGIN(checkbox_init_null)
{
    TEST_NE(tui_checkbox_init(NULL, 0, 0, 20, 1, "X"), TUI_OK);
}

TEST_BEGIN(checkbox_init_no_label)
{
    TuiCheckbox cb;
    TuiResult res = tui_checkbox_init(&cb, 0, 0, 10, 1, NULL);
    TEST_EQ(res, TUI_OK);
    TEST_ASSERT(cb.label == NULL);
    tui_widget_destroy(&cb.base);
}

TEST_BEGIN(checkbox_set_state)
{
    TuiCheckbox cb;
    tui_checkbox_init(&cb, 0, 0, 20, 1, NULL);

    tui_checkbox_set_state(&cb, TUI_CHECK_CHECKED);
    TEST_EQ(tui_checkbox_get_state(&cb), TUI_CHECK_CHECKED);

    tui_checkbox_set_state(&cb, TUI_CHECK_INDETERMINATE);
    TEST_EQ(tui_checkbox_get_state(&cb), TUI_CHECK_INDETERMINATE);

    tui_checkbox_set_state(&cb, TUI_CHECK_UNCHECKED);
    TEST_EQ(tui_checkbox_get_state(&cb), TUI_CHECK_UNCHECKED);

    tui_widget_destroy(&cb.base);
}

TEST_BEGIN(checkbox_get_state_null)
{
    TEST_EQ(tui_checkbox_get_state(NULL), TUI_CHECK_UNCHECKED);
}

TEST_BEGIN(checkbox_tristate)
{
    TuiCheckbox cb;
    tui_checkbox_init(&cb, 0, 0, 20, 1, NULL);
    tui_checkbox_set_tristate(&cb, 1);

    TEST_EQ(cb.state, TUI_CHECK_UNCHECKED);

    cb.state = TUI_CHECK_CHECKED;
    TEST_EQ(cb.state, TUI_CHECK_CHECKED);

    cb.state = TUI_CHECK_INDETERMINATE;
    TEST_EQ(cb.state, TUI_CHECK_INDETERMINATE);

    cb.state = TUI_CHECK_UNCHECKED;
    TEST_EQ(cb.state, TUI_CHECK_UNCHECKED);

    TEST_EQ(cb.tristate, 1);

    tui_widget_destroy(&cb.base);
}

TEST_BEGIN(checkbox_set_label)
{
    TuiCheckbox cb;
    tui_checkbox_init(&cb, 0, 0, 20, 1, NULL);

    tui_checkbox_set_label(&cb, "New label");
    TEST_ASSERT(cb.label != NULL);
    TEST_STR_EQ(cb.label, "New label");

    tui_checkbox_set_label(&cb, NULL);
    TEST_ASSERT(cb.label == NULL);

    tui_widget_destroy(&cb.base);
}

TEST_BEGIN(checkbox_set_colors)
{
    TuiCheckbox cb;
    tui_checkbox_init(&cb, 0, 0, 20, 1, NULL);

    TuiColor fg = TUI_COLOR_INDEX(10);
    TuiColor bg = TUI_COLOR_INDEX(5);
    TuiColor fgf = TUI_COLOR_INDEX(1);
    TuiColor bgf = TUI_COLOR_INDEX(2);
    tui_checkbox_set_colors(&cb, fg, bg, fgf, bgf);
    TEST_ASSERT(tui_color_eq(cb.fg, fg));
    TEST_ASSERT(tui_color_eq(cb.bg, bg));
    TEST_ASSERT(tui_color_eq(cb.fg_focused, fgf));
    TEST_ASSERT(tui_color_eq(cb.bg_focused, bgf));

    tui_widget_destroy(&cb.base);
}

/* ── Radio Group ─────────────────────────────────────────────────── */

TEST_BEGIN(radio_init)
{
    TuiRadio r;
    TuiResult res = tui_radio_init(&r, 0, 0, 20, 5);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(r.option_count, 0);
    TEST_EQ(r.selected, 0);
    TEST_EQ(r.base.focusable, 1);
    tui_widget_destroy(&r.base);
}

TEST_BEGIN(radio_init_null)
{
    TEST_NE(tui_radio_init(NULL, 0, 0, 20, 5), TUI_OK);
}

TEST_BEGIN(radio_add_option)
{
    TuiRadio r;
    tui_radio_init(&r, 0, 0, 20, 5);

    TuiResult res;
    res = tui_radio_add_option(&r, "Option A");
    TEST_EQ(res, TUI_OK);
    TEST_EQ(r.option_count, 1);

    res = tui_radio_add_option(&r, "Option B");
    TEST_EQ(res, TUI_OK);
    TEST_EQ(r.option_count, 2);

    res = tui_radio_add_option(&r, "Option C");
    TEST_EQ(res, TUI_OK);
    TEST_EQ(r.option_count, 3);

    tui_widget_destroy(&r.base);
}

TEST_BEGIN(radio_add_null_option)
{
    TuiRadio r;
    tui_radio_init(&r, 0, 0, 20, 5);

    TuiResult res = tui_radio_add_option(&r, NULL);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(r.option_count, 1);
    TEST_ASSERT(r.options[0] == NULL);

    tui_widget_destroy(&r.base);
}

TEST_BEGIN(radio_remove_option)
{
    TuiRadio r;
    tui_radio_init(&r, 0, 0, 20, 5);
    tui_radio_add_option(&r, "A");
    tui_radio_add_option(&r, "B");
    tui_radio_add_option(&r, "C");

    tui_radio_remove_option(&r, 1);
    TEST_EQ(r.option_count, 2);
    TEST_STR_EQ(r.options[0], "A");
    TEST_STR_EQ(r.options[1], "C");

    tui_widget_destroy(&r.base);
}

TEST_BEGIN(radio_remove_out_of_bounds)
{
    TuiRadio r;
    tui_radio_init(&r, 0, 0, 20, 5);
    tui_radio_add_option(&r, "A");

    tui_radio_remove_option(&r, -1);
    TEST_EQ(r.option_count, 1);

    tui_radio_remove_option(&r, 5);
    TEST_EQ(r.option_count, 1);

    tui_widget_destroy(&r.base);
}

TEST_BEGIN(radio_clear)
{
    TuiRadio r;
    tui_radio_init(&r, 0, 0, 20, 5);
    tui_radio_add_option(&r, "A");
    tui_radio_add_option(&r, "B");
    tui_radio_add_option(&r, "C");

    tui_radio_clear(&r);
    TEST_EQ(r.option_count, 0);
    TEST_EQ(r.selected, 0);

    tui_widget_destroy(&r.base);
}

TEST_BEGIN(radio_selected)
{
    TuiRadio r;
    tui_radio_init(&r, 0, 0, 20, 5);
    tui_radio_add_option(&r, "A");
    tui_radio_add_option(&r, "B");
    tui_radio_add_option(&r, "C");

    TEST_EQ(tui_radio_get_selected(&r), 0);
    TEST_STR_EQ(tui_radio_get_selected_option(&r), "A");

    tui_radio_set_selected(&r, 2);
    TEST_EQ(tui_radio_get_selected(&r), 2);
    TEST_STR_EQ(tui_radio_get_selected_option(&r), "C");

    tui_widget_destroy(&r.base);
}

TEST_BEGIN(radio_selected_null)
{
    TEST_EQ(tui_radio_get_selected(NULL), -1);
    TEST_ASSERT(tui_radio_get_selected_option(NULL) == NULL);
}

TEST_BEGIN(radio_set_selected_bounds)
{
    TuiRadio r;
    tui_radio_init(&r, 0, 0, 20, 5);
    tui_radio_add_option(&r, "A");
    tui_radio_add_option(&r, "B");

    tui_radio_set_selected(&r, -1);
    TEST_EQ(r.selected, 0);

    tui_radio_set_selected(&r, 10);
    TEST_EQ(r.selected, 1);

    tui_widget_destroy(&r.base);
}

TEST_BEGIN(radio_set_colors)
{
    TuiRadio r;
    tui_radio_init(&r, 0, 0, 20, 5);

    TuiColor fg = TUI_COLOR_INDEX(15);
    TuiColor bg = TUI_COLOR_INDEX(0);
    TuiColor fgs = TUI_COLOR_INDEX(0);
    TuiColor bgs = TUI_COLOR_INDEX(12);
    tui_radio_set_colors(&r, fg, bg, fgs, bgs);
    TEST_ASSERT(tui_color_eq(r.fg, fg));
    TEST_ASSERT(tui_color_eq(r.bg, bg));
    TEST_ASSERT(tui_color_eq(r.fg_selected, fgs));
    TEST_ASSERT(tui_color_eq(r.bg_selected, bgs));

    tui_widget_destroy(&r.base);
}

TEST_BEGIN(radio_remove_adjusts_selected)
{
    TuiRadio r;
    tui_radio_init(&r, 0, 0, 20, 5);
    tui_radio_add_option(&r, "A");
    tui_radio_add_option(&r, "B");
    tui_radio_add_option(&r, "C");

    tui_radio_set_selected(&r, 2);
    TEST_EQ(r.selected, 2);

    tui_radio_remove_option(&r, 2);
    TEST_EQ(r.option_count, 2);
    TEST_EQ(r.selected, 1);

    tui_widget_destroy(&r.base);
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void)
{
    fprintf(stderr, "Running widget tests...\n\n");

    TEST_RUN(progress_init);
    TEST_RUN(progress_init_null);
    TEST_RUN(progress_set_value);
    TEST_RUN(progress_value_clamped);
    TEST_RUN(progress_get_value_null);
    TEST_RUN(progress_set_label);
    TEST_RUN(progress_set_chars);
    TEST_RUN(progress_show_percent);
    TEST_RUN(progress_set_colors);
    TEST_RUN(progress_set_attr);

    TEST_RUN(checkbox_init);
    TEST_RUN(checkbox_init_null);
    TEST_RUN(checkbox_init_no_label);
    TEST_RUN(checkbox_set_state);
    TEST_RUN(checkbox_get_state_null);
    TEST_RUN(checkbox_tristate);
    TEST_RUN(checkbox_set_label);
    TEST_RUN(checkbox_set_colors);

    TEST_RUN(radio_init);
    TEST_RUN(radio_init_null);
    TEST_RUN(radio_add_option);
    TEST_RUN(radio_add_null_option);
    TEST_RUN(radio_remove_option);
    TEST_RUN(radio_remove_out_of_bounds);
    TEST_RUN(radio_clear);
    TEST_RUN(radio_selected);
    TEST_RUN(radio_selected_null);
    TEST_RUN(radio_set_selected_bounds);
    TEST_RUN(radio_set_colors);
    TEST_RUN(radio_remove_adjusts_selected);

    TEST_SUMMARY();
}
