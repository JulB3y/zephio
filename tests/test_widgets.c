#include "util.h"
#include "tui_progress.h"
#include "tui_checkbox.h"
#include "tui_radio.h"
#include "tui_label.h"
#include "tui_button.h"
#include "tui_box.h"
#include "tui_list.h"
#include "tui_input_field.h"
#include "tui_screen.h"

#include <string.h>

static int g_btn_clicked = 0;
static int g_list_selected_idx = -1;
static int g_field_submitted = 0;
static int g_field_change_count = 0;

static void stub_btn_on_click(TuiWidget *w, void *ud) { (void)w; (void)ud; g_btn_clicked = 1; }
static void stub_list_on_select(TuiWidget *w, int idx, const char *item, void *ud) { (void)w; (void)item; (void)ud; g_list_selected_idx = idx; }
static void stub_field_on_submit(TuiWidget *w, const char *text, void *ud) { (void)w; (void)text; (void)ud; g_field_submitted = 1; }
static void stub_field_on_change(TuiWidget *w, const char *text, void *ud) { (void)w; (void)text; (void)ud; g_field_change_count++; }

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

/* ── Label ──────────────────────────────────────────────────────── */

TEST_BEGIN(label_init)
{
    TuiLabel l;
    TuiResult res = tui_label_init(&l, 2, 1, 20, 1, "Hello");
    TEST_EQ(res, TUI_OK);
    TEST_EQ(l.base.focusable, 0);
    TEST_EQ(l.base.visible, 1);
    TEST_ASSERT(l.text != NULL);
    TEST_STR_EQ(l.text, "Hello");
    tui_widget_destroy(&l.base);
}

TEST_BEGIN(label_init_null_widget)
{
    TEST_NE(tui_label_init(NULL, 0, 0, 20, 1, "X"), TUI_OK);
}

TEST_BEGIN(label_init_null_text)
{
    TuiLabel l;
    TuiResult res = tui_label_init(&l, 0, 0, 20, 1, NULL);
    TEST_EQ(res, TUI_OK);
    TEST_ASSERT(l.text == NULL);
    tui_widget_destroy(&l.base);
}

TEST_BEGIN(label_set_text)
{
    TuiLabel l;
    tui_label_init(&l, 0, 0, 20, 1, NULL);

    tui_label_set_text(&l, "World");
    TEST_ASSERT(l.text != NULL);
    TEST_STR_EQ(l.text, "World");
    TEST_EQ(l.base.dirty, 1);

    tui_label_set_text(&l, NULL);
    TEST_ASSERT(l.text == NULL);

    tui_widget_destroy(&l.base);
}

TEST_BEGIN(label_set_colors)
{
    TuiLabel l;
    tui_label_init(&l, 0, 0, 20, 1, NULL);

    TuiColor fg = TUI_COLOR_INDEX(10);
    TuiColor bg = TUI_COLOR_INDEX(2);
    tui_label_set_colors(&l, fg, bg);
    TEST_ASSERT(tui_color_eq(l.fg, fg));
    TEST_ASSERT(tui_color_eq(l.bg, bg));
    TEST_EQ(l.base.dirty, 1);

    tui_widget_destroy(&l.base);
}

TEST_BEGIN(label_set_attr)
{
    TuiLabel l;
    tui_label_init(&l, 0, 0, 20, 1, NULL);

    tui_label_set_attr(&l, TUI_ATTR_BOLD);
    TEST_EQ(l.attr, TUI_ATTR_BOLD);

    tui_widget_destroy(&l.base);
}

TEST_BEGIN(label_render)
{
    tui_screen_init(24, 80);
    TuiLabel l;
    tui_label_init(&l, 5, 2, 10, 1, "Hi");
    l.base.dirty = 1;

    tui_widget_render(&l.base);
    TEST_EQ(l.base.dirty, 0);
    TEST_EQ(g_screen.back[2 * 80 + 5].ch[0], 'H');
    TEST_EQ(g_screen.back[2 * 80 + 6].ch[0], 'i');

    tui_widget_destroy(&l.base);
    tui_screen_free();
}

TEST_BEGIN(label_render_null_text)
{
    tui_screen_init(24, 80);
    TuiLabel l;
    tui_label_init(&l, 0, 0, 10, 1, NULL);
    l.base.dirty = 1;

    tui_widget_render(&l.base);
    TEST_EQ(l.base.dirty, 0);

    tui_widget_destroy(&l.base);
    tui_screen_free();
}

/* ── Button ─────────────────────────────────────────────────────── */

TEST_BEGIN(button_init)
{
    TuiButton b;
    TuiResult res = tui_button_init(&b, 0, 0, 10, 3, "OK");
    TEST_EQ(res, TUI_OK);
    TEST_EQ(b.base.focusable, 1);
    TEST_ASSERT(b.text != NULL);
    TEST_STR_EQ(b.text, "OK");
    tui_widget_destroy(&b.base);
}

TEST_BEGIN(button_init_null_widget)
{
    TEST_NE(tui_button_init(NULL, 0, 0, 10, 3, "X"), TUI_OK);
}

TEST_BEGIN(button_set_text)
{
    TuiButton b;
    tui_button_init(&b, 0, 0, 10, 3, NULL);

    tui_button_set_text(&b, "Cancel");
    TEST_ASSERT(b.text != NULL);
    TEST_STR_EQ(b.text, "Cancel");
    TEST_EQ(b.base.dirty, 1);

    tui_button_set_text(&b, NULL);
    TEST_ASSERT(b.text == NULL);

    tui_widget_destroy(&b.base);
}

TEST_BEGIN(button_set_colors)
{
    TuiButton b;
    tui_button_init(&b, 0, 0, 10, 3, NULL);

    TuiColor fg = TUI_COLOR_INDEX(15);
    TuiColor bg = TUI_COLOR_INDEX(1);
    TuiColor fgf = TUI_COLOR_INDEX(0);
    TuiColor bgf = TUI_COLOR_INDEX(12);
    tui_button_set_colors(&b, fg, bg, fgf, bgf);
    TEST_ASSERT(tui_color_eq(b.fg, fg));
    TEST_ASSERT(tui_color_eq(b.bg, bg));
    TEST_ASSERT(tui_color_eq(b.fg_focused, fgf));
    TEST_ASSERT(tui_color_eq(b.bg_focused, bgf));

    tui_widget_destroy(&b.base);
}

TEST_BEGIN(button_on_click)
{
    g_btn_clicked = 0;

    TuiButton b;
    tui_button_init(&b, 0, 0, 10, 3, "Go");
    tui_button_set_on_click(&b, stub_btn_on_click, NULL);

    TuiEvent event = {0};
    event.key = TUI_KEY_ENTER;
    int handled = tui_widget_handle_input(&b.base, &event);
    TEST_EQ(handled, 1);
    TEST_EQ(g_btn_clicked, 1);

    tui_widget_destroy(&b.base);
}

TEST_BEGIN(button_on_click_space)
{
    g_btn_clicked = 0;

    TuiButton b;
    tui_button_init(&b, 0, 0, 10, 3, "Go");
    tui_button_set_on_click(&b, stub_btn_on_click, NULL);

    TuiEvent event = {0};
    event.codepoint = ' ';
    int handled = tui_widget_handle_input(&b.base, &event);
    TEST_EQ(handled, 1);
    TEST_EQ(g_btn_clicked, 1);

    tui_widget_destroy(&b.base);
}

TEST_BEGIN(button_mouse_click)
{
    g_btn_clicked = 0;

    TuiButton b;
    tui_button_init(&b, 0, 0, 10, 3, "Go");
    b.base.abs_x = 0;
    b.base.abs_y = 0;
    tui_button_set_on_click(&b, stub_btn_on_click, NULL);

    TuiMouseEvent mouse = { 0, 5, TUI_MOUSE_BTN_LEFT, TUI_MOUSE_PRESS, 0 };
    int handled = tui_widget_handle_mouse(&b.base, &mouse);
    TEST_EQ(handled, 1);
    TEST_EQ(g_btn_clicked, 1);

    tui_widget_destroy(&b.base);
}

TEST_BEGIN(button_render)
{
    tui_screen_init(24, 80);
    TuiButton b;
    tui_button_init(&b, 3, 5, 8, 1, "OK");
    b.base.dirty = 1;

    tui_widget_render(&b.base);
    TEST_EQ(b.base.dirty, 0);

    tui_widget_destroy(&b.base);
    tui_screen_free();
}

/* ── Box ────────────────────────────────────────────────────────── */

TEST_BEGIN(box_init)
{
    TuiBox box;
    TuiResult res = tui_box_init(&box, 0, 0, 30, 15, TUI_BOX_SINGLE);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(box.base.focusable, 0);
    TEST_EQ(box.border_style, TUI_BOX_SINGLE);
    TEST_EQ(box.padding, 0);
    TEST_ASSERT(box.title == NULL);
    tui_widget_destroy(&box.base);
}

TEST_BEGIN(box_init_null)
{
    TEST_NE(tui_box_init(NULL, 0, 0, 30, 15, TUI_BOX_SINGLE), TUI_OK);
}

TEST_BEGIN(box_set_title)
{
    TuiBox box;
    tui_box_init(&box, 0, 0, 30, 15, TUI_BOX_SINGLE);

    tui_box_set_title(&box, "Settings");
    TEST_ASSERT(box.title != NULL);
    TEST_STR_EQ(box.title, "Settings");
    TEST_EQ(box.base.dirty, 1);

    tui_box_set_title(&box, NULL);
    TEST_ASSERT(box.title == NULL);

    tui_widget_destroy(&box.base);
}

TEST_BEGIN(box_set_colors)
{
    TuiBox box;
    tui_box_init(&box, 0, 0, 30, 15, TUI_BOX_SINGLE);

    TuiColor fg = TUI_COLOR_INDEX(14);
    TuiColor bg = TUI_COLOR_INDEX(0);
    tui_box_set_colors(&box, fg, bg);
    TEST_ASSERT(tui_color_eq(box.fg, fg));
    TEST_ASSERT(tui_color_eq(box.bg, bg));

    tui_widget_destroy(&box.base);
}

TEST_BEGIN(box_set_attr)
{
    TuiBox box;
    tui_box_init(&box, 0, 0, 30, 15, TUI_BOX_SINGLE);

    tui_box_set_attr(&box, TUI_ATTR_BOLD);
    TEST_EQ(box.attr, TUI_ATTR_BOLD);

    tui_widget_destroy(&box.base);
}

TEST_BEGIN(box_set_padding)
{
    TuiBox box;
    tui_box_init(&box, 0, 0, 30, 15, TUI_BOX_SINGLE);

    tui_box_set_padding(&box, 2);
    TEST_EQ(box.padding, 2);
    TEST_EQ(box.base.dirty, 1);

    tui_widget_destroy(&box.base);
}

TEST_BEGIN(box_render_single)
{
    tui_screen_init(24, 80);
    TuiBox box;
    tui_box_init(&box, 0, 0, 10, 5, TUI_BOX_SINGLE);
    box.base.dirty = 1;

    tui_widget_render(&box.base);
    TEST_EQ(box.base.dirty, 0);
    TEST_ASSERT(memcmp(g_screen.back[0].ch, "\xe2\x94\x8c", 3) == 0);

    tui_widget_destroy(&box.base);
    tui_screen_free();
}

TEST_BEGIN(box_render_double)
{
    tui_screen_init(24, 80);
    TuiBox box;
    tui_box_init(&box, 0, 0, 10, 5, TUI_BOX_DOUBLE);
    box.base.dirty = 1;

    tui_widget_render(&box.base);
    TEST_EQ(box.base.dirty, 0);
    TEST_ASSERT(memcmp(g_screen.back[0].ch, "\xe2\x95\x94", 3) == 0);

    tui_widget_destroy(&box.base);
    tui_screen_free();
}

TEST_BEGIN(box_render_too_small)
{
    tui_screen_init(24, 80);
    TuiBox box;
    tui_box_init(&box, 0, 0, 1, 1, TUI_BOX_SINGLE);
    box.base.dirty = 1;

    tui_widget_render(&box.base);
    TEST_EQ(box.base.dirty, 0);

    tui_widget_destroy(&box.base);
    tui_screen_free();
}

TEST_BEGIN(box_render_with_title)
{
    tui_screen_init(24, 80);
    TuiBox box;
    tui_box_init(&box, 0, 0, 20, 5, TUI_BOX_SINGLE);
    tui_box_set_title(&box, "Test");
    box.base.dirty = 1;

    tui_widget_render(&box.base);
    TEST_EQ(g_screen.back[0 * 80 + 2].ch[0], 'T');
    TEST_EQ(g_screen.back[0 * 80 + 3].ch[0], 'e');
    TEST_EQ(g_screen.back[0 * 80 + 4].ch[0], 's');
    TEST_EQ(g_screen.back[0 * 80 + 5].ch[0], 't');

    tui_widget_destroy(&box.base);
    tui_screen_free();
}

/* ── List ───────────────────────────────────────────────────────── */

TEST_BEGIN(list_init)
{
    TuiList list;
    TuiResult res = tui_list_init(&list, 0, 0, 30, 10);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(list.base.focusable, 1);
    TEST_EQ(list.item_count, 0);
    TEST_EQ(list.selected, 0);
    TEST_EQ(list.scroll_offset, 0);
    tui_widget_destroy(&list.base);
}

TEST_BEGIN(list_init_null)
{
    TEST_NE(tui_list_init(NULL, 0, 0, 30, 10), TUI_OK);
}

TEST_BEGIN(list_add_item)
{
    TuiList list;
    tui_list_init(&list, 0, 0, 30, 10);

    tui_list_add_item(&list, "Item A");
    tui_list_add_item(&list, "Item B");
    tui_list_add_item(&list, "Item C");

    TEST_EQ(list.item_count, 3);
    TEST_STR_EQ(list.items[0], "Item A");
    TEST_STR_EQ(list.items[1], "Item B");
    TEST_STR_EQ(list.items[2], "Item C");

    tui_widget_destroy(&list.base);
}

TEST_BEGIN(list_add_null_item)
{
    TuiList list;
    tui_list_init(&list, 0, 0, 30, 10);

    TuiResult res = tui_list_add_item(&list, NULL);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(list.item_count, 1);
    TEST_ASSERT(list.items[0] == NULL);

    tui_widget_destroy(&list.base);
}

TEST_BEGIN(list_remove_item)
{
    TuiList list;
    tui_list_init(&list, 0, 0, 30, 10);
    tui_list_add_item(&list, "A");
    tui_list_add_item(&list, "B");
    tui_list_add_item(&list, "C");

    tui_list_remove_item(&list, 1);
    TEST_EQ(list.item_count, 2);
    TEST_STR_EQ(list.items[0], "A");
    TEST_STR_EQ(list.items[1], "C");

    tui_widget_destroy(&list.base);
}

TEST_BEGIN(list_remove_out_of_bounds)
{
    TuiList list;
    tui_list_init(&list, 0, 0, 30, 10);
    tui_list_add_item(&list, "A");

    tui_list_remove_item(&list, -1);
    TEST_EQ(list.item_count, 1);

    tui_list_remove_item(&list, 5);
    TEST_EQ(list.item_count, 1);

    tui_widget_destroy(&list.base);
}

TEST_BEGIN(list_clear)
{
    TuiList list;
    tui_list_init(&list, 0, 0, 30, 10);
    tui_list_add_item(&list, "A");
    tui_list_add_item(&list, "B");

    tui_list_clear(&list);
    TEST_EQ(list.item_count, 0);
    TEST_EQ(list.selected, 0);
    TEST_EQ(list.scroll_offset, 0);
    TEST_EQ(list.base.dirty, 1);

    tui_widget_destroy(&list.base);
}

TEST_BEGIN(list_get_selected)
{
    TuiList list;
    tui_list_init(&list, 0, 0, 30, 10);
    tui_list_add_item(&list, "A");
    tui_list_add_item(&list, "B");

    TEST_EQ(tui_list_get_selected(&list), 0);
    TEST_STR_EQ(tui_list_get_selected_item(&list), "A");

    list.selected = 1;
    TEST_EQ(tui_list_get_selected(&list), 1);
    TEST_STR_EQ(tui_list_get_selected_item(&list), "B");

    tui_widget_destroy(&list.base);
}

TEST_BEGIN(list_get_selected_null)
{
    TEST_EQ(tui_list_get_selected(NULL), -1);
    TEST_ASSERT(tui_list_get_selected_item(NULL) == NULL);
}

TEST_BEGIN(list_get_selected_empty)
{
    TuiList list;
    tui_list_init(&list, 0, 0, 30, 10);
    TEST_ASSERT(tui_list_get_selected_item(&list) == NULL);
    tui_widget_destroy(&list.base);
}

TEST_BEGIN(list_set_colors)
{
    TuiList list;
    tui_list_init(&list, 0, 0, 30, 10);

    TuiColor fg = TUI_COLOR_INDEX(15);
    TuiColor bg = TUI_COLOR_INDEX(0);
    TuiColor fgs = TUI_COLOR_INDEX(0);
    TuiColor bgs = TUI_COLOR_INDEX(12);
    tui_list_set_colors(&list, fg, bg, fgs, bgs);
    TEST_ASSERT(tui_color_eq(list.fg, fg));
    TEST_ASSERT(tui_color_eq(list.bg, bg));
    TEST_ASSERT(tui_color_eq(list.fg_selected, fgs));
    TEST_ASSERT(tui_color_eq(list.bg_selected, bgs));

    tui_widget_destroy(&list.base);
}

TEST_BEGIN(list_input_navigation)
{
    TuiList list;
    tui_list_init(&list, 0, 0, 30, 5);
    tui_list_add_item(&list, "A");
    tui_list_add_item(&list, "B");
    tui_list_add_item(&list, "C");

    TuiEvent down = {0};
    down.key = TUI_KEY_DOWN;
    tui_widget_handle_input(&list.base, &down);
    TEST_EQ(list.selected, 1);

    tui_widget_handle_input(&list.base, &down);
    TEST_EQ(list.selected, 2);

    TuiEvent up = {0};
    up.key = TUI_KEY_UP;
    tui_widget_handle_input(&list.base, &up);
    TEST_EQ(list.selected, 1);

    tui_widget_destroy(&list.base);
}

TEST_BEGIN(list_input_select)
{
    g_list_selected_idx = -1;

    TuiList list;
    tui_list_init(&list, 0, 0, 30, 5);
    tui_list_add_item(&list, "A");
    tui_list_add_item(&list, "B");
    tui_list_set_on_select(&list, stub_list_on_select, NULL);

    TuiEvent enter = {0};
    enter.key = TUI_KEY_ENTER;
    tui_widget_handle_input(&list.base, &enter);
    TEST_EQ(g_list_selected_idx, 0);

    tui_widget_destroy(&list.base);
}

TEST_BEGIN(list_render)
{
    tui_screen_init(24, 80);
    TuiList list;
    tui_list_init(&list, 0, 0, 20, 5);
    tui_list_add_item(&list, "Hello");
    list.base.dirty = 1;
    list.base.focused = 1;

    tui_widget_render(&list.base);
    TEST_EQ(list.base.dirty, 0);
    TEST_EQ(g_screen.back[0].ch[0], '>');
    TEST_EQ(g_screen.back[1].ch[0], 'H');

    tui_widget_destroy(&list.base);
    tui_screen_free();
}

/* ── InputField ─────────────────────────────────────────────────── */

TEST_BEGIN(input_field_init)
{
    TuiInputField f;
    TuiResult res = tui_input_field_init(&f, 0, 0, 30, 256);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(f.base.focusable, 1);
    TEST_EQ(f.text_capacity, 256);
    TEST_EQ(f.cursor_pos, 0);
    TEST_EQ(f.scroll_offset, 0);
    TEST_ASSERT(f.text == NULL);
    tui_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_init_null)
{
    TEST_NE(tui_input_field_init(NULL, 0, 0, 30, 256), TUI_OK);
}

TEST_BEGIN(input_field_set_text)
{
    TuiInputField f;
    tui_input_field_init(&f, 0, 0, 30, 256);

    tui_input_field_set_text(&f, "Hello");
    TEST_ASSERT(f.text != NULL);
    TEST_STR_EQ(f.text, "Hello");
    TEST_EQ(f.cursor_pos, 5);
    TEST_EQ(f.base.dirty, 1);

    tui_input_field_set_text(&f, NULL);
    TEST_ASSERT(f.text == NULL);
    TEST_EQ(f.cursor_pos, 0);

    tui_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_get_text)
{
    TuiInputField f;
    tui_input_field_init(&f, 0, 0, 30, 256);

    TEST_STR_EQ(tui_input_field_get_text(&f), "");

    tui_input_field_set_text(&f, "Test");
    TEST_STR_EQ(tui_input_field_get_text(&f), "Test");

    TEST_ASSERT(tui_input_field_get_text(NULL) == NULL);

    tui_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_set_colors)
{
    TuiInputField f;
    tui_input_field_init(&f, 0, 0, 30, 256);

    TuiColor fg = TUI_COLOR_INDEX(15);
    TuiColor bg = TUI_COLOR_INDEX(234);
    TuiColor cfg = TUI_COLOR_INDEX(0);
    TuiColor cbg = TUI_COLOR_INDEX(15);
    tui_input_field_set_colors(&f, fg, bg, cfg, cbg);
    TEST_ASSERT(tui_color_eq(f.fg, fg));
    TEST_ASSERT(tui_color_eq(f.bg, bg));
    TEST_ASSERT(tui_color_eq(f.cursor_fg, cfg));
    TEST_ASSERT(tui_color_eq(f.cursor_bg, cbg));

    tui_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_typing)
{
    TuiInputField f;
    tui_input_field_init(&f, 0, 0, 30, 256);

    TuiEvent e = {0};
    e.key = TUI_KEY_UNKNOWN;

    e.codepoint = 'A';
    tui_widget_handle_input(&f.base, &e);
    TEST_STR_EQ(f.text, "A");
    TEST_EQ(f.cursor_pos, 1);

    e.codepoint = 'B';
    tui_widget_handle_input(&f.base, &e);
    TEST_STR_EQ(f.text, "AB");
    TEST_EQ(f.cursor_pos, 2);

    tui_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_backspace)
{
    TuiInputField f;
    tui_input_field_init(&f, 0, 0, 30, 256);
    tui_input_field_set_text(&f, "ABC");
    f.cursor_pos = 3;

    TuiEvent e = {0};
    e.key = TUI_KEY_BACKSPACE;
    tui_widget_handle_input(&f.base, &e);
    TEST_STR_EQ(f.text, "AB");
    TEST_EQ(f.cursor_pos, 2);

    tui_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_delete)
{
    TuiInputField f;
    tui_input_field_init(&f, 0, 0, 30, 256);
    tui_input_field_set_text(&f, "ABC");
    f.cursor_pos = 1;

    TuiEvent e = {0};
    e.key = TUI_KEY_DELETE;
    tui_widget_handle_input(&f.base, &e);
    TEST_STR_EQ(f.text, "AC");
    TEST_EQ(f.cursor_pos, 1);

    tui_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_cursor_movement)
{
    TuiInputField f;
    tui_input_field_init(&f, 0, 0, 30, 256);
    tui_input_field_set_text(&f, "ABCDE");

    TuiEvent left = {0};
    left.key = TUI_KEY_LEFT;

    TuiEvent right = {0};
    right.key = TUI_KEY_RIGHT;

    tui_widget_handle_input(&f.base, &left);
    TEST_EQ(f.cursor_pos, 4);

    tui_widget_handle_input(&f.base, &left);
    TEST_EQ(f.cursor_pos, 3);

    tui_widget_handle_input(&f.base, &right);
    TEST_EQ(f.cursor_pos, 4);

    tui_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_home_end)
{
    TuiInputField f;
    tui_input_field_init(&f, 0, 0, 30, 256);
    tui_input_field_set_text(&f, "ABCDE");

    TuiEvent home = {0};
    home.key = TUI_KEY_HOME;
    tui_widget_handle_input(&f.base, &home);
    TEST_EQ(f.cursor_pos, 0);

    TuiEvent end = {0};
    end.key = TUI_KEY_END;
    tui_widget_handle_input(&f.base, &end);
    TEST_EQ(f.cursor_pos, 5);

    tui_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_submit)
{
    g_field_submitted = 0;

    TuiInputField f;
    tui_input_field_init(&f, 0, 0, 30, 256);
    tui_input_field_set_on_submit(&f, stub_field_on_submit, NULL);

    TuiEvent enter = {0};
    enter.key = TUI_KEY_ENTER;
    tui_widget_handle_input(&f.base, &enter);
    TEST_EQ(g_field_submitted, 1);

    tui_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_on_change)
{
    g_field_change_count = 0;

    TuiInputField f;
    tui_input_field_init(&f, 0, 0, 30, 256);
    tui_input_field_set_on_change(&f, stub_field_on_change, NULL);

    TuiEvent e = {0};
    e.key = TUI_KEY_UNKNOWN;
    e.codepoint = 'X';
    tui_widget_handle_input(&f.base, &e);
    TEST_EQ(g_field_change_count, 1);

    tui_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_render)
{
    tui_screen_init(24, 80);
    TuiInputField f;
    tui_input_field_init(&f, 0, 0, 20, 256);
    tui_input_field_set_text(&f, "Hi");
    f.base.dirty = 1;
    f.base.focused = 1;

    tui_widget_render(&f.base);
    TEST_EQ(f.base.dirty, 0);
    TEST_EQ(g_screen.back[0].ch[0], 'H');
    TEST_EQ(g_screen.back[1].ch[0], 'i');

    tui_widget_destroy(&f.base);
    tui_screen_free();
}

TEST_BEGIN(input_field_capacity_truncation)
{
    TuiInputField f;
    tui_input_field_init(&f, 0, 0, 20, 5);

    tui_input_field_set_text(&f, "ABCDE");
    TEST_STR_EQ(f.text, "ABCD");

    tui_widget_destroy(&f.base);
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

    TEST_RUN(label_init);
    TEST_RUN(label_init_null_widget);
    TEST_RUN(label_init_null_text);
    TEST_RUN(label_set_text);
    TEST_RUN(label_set_colors);
    TEST_RUN(label_set_attr);
    TEST_RUN(label_render);
    TEST_RUN(label_render_null_text);

    TEST_RUN(button_init);
    TEST_RUN(button_init_null_widget);
    TEST_RUN(button_set_text);
    TEST_RUN(button_set_colors);
    TEST_RUN(button_on_click);
    TEST_RUN(button_on_click_space);
    TEST_RUN(button_mouse_click);
    TEST_RUN(button_render);

    TEST_RUN(box_init);
    TEST_RUN(box_init_null);
    TEST_RUN(box_set_title);
    TEST_RUN(box_set_colors);
    TEST_RUN(box_set_attr);
    TEST_RUN(box_set_padding);
    TEST_RUN(box_render_single);
    TEST_RUN(box_render_double);
    TEST_RUN(box_render_too_small);
    TEST_RUN(box_render_with_title);

    TEST_RUN(list_init);
    TEST_RUN(list_init_null);
    TEST_RUN(list_add_item);
    TEST_RUN(list_add_null_item);
    TEST_RUN(list_remove_item);
    TEST_RUN(list_remove_out_of_bounds);
    TEST_RUN(list_clear);
    TEST_RUN(list_get_selected);
    TEST_RUN(list_get_selected_null);
    TEST_RUN(list_get_selected_empty);
    TEST_RUN(list_set_colors);
    TEST_RUN(list_input_navigation);
    TEST_RUN(list_input_select);
    TEST_RUN(list_render);

    TEST_RUN(input_field_init);
    TEST_RUN(input_field_init_null);
    TEST_RUN(input_field_set_text);
    TEST_RUN(input_field_get_text);
    TEST_RUN(input_field_set_colors);
    TEST_RUN(input_field_typing);
    TEST_RUN(input_field_backspace);
    TEST_RUN(input_field_delete);
    TEST_RUN(input_field_cursor_movement);
    TEST_RUN(input_field_home_end);
    TEST_RUN(input_field_submit);
    TEST_RUN(input_field_on_change);
    TEST_RUN(input_field_render);
    TEST_RUN(input_field_capacity_truncation);

    TEST_SUMMARY();
}
