#include "util.h"
#include "zephio_context.h"
static ZephioContext g_test_ctx;
#include "zephio_progress.h"
#include "zephio_checkbox.h"
#include "zephio_radio.h"
#include "zephio_label.h"
#include "zephio_button.h"
#include "zephio_box.h"
#include "zephio_list.h"
#include "zephio_input_field.h"
#include "zephio_screen.h"

#include <string.h>

static int g_btn_clicked = 0;
static int g_list_selected_idx = -1;
static int g_field_submitted = 0;
static int g_field_change_count = 0;

static void stub_btn_on_click(ZephioWidget *w, void *ud) { (void)w; (void)ud; g_btn_clicked = 1; }
static void stub_list_on_select(ZephioWidget *w, int idx, const char *item, void *ud) { (void)w; (void)item; (void)ud; g_list_selected_idx = idx; }
static void stub_field_on_submit(ZephioWidget *w, const char *text, void *ud) { (void)w; (void)text; (void)ud; g_field_submitted = 1; }
static void stub_field_on_change(ZephioWidget *w, const char *text, void *ud) { (void)w; (void)text; (void)ud; g_field_change_count++; }

/* ── Progress Bar ────────────────────────────────────────────────── */

TEST_BEGIN(progress_init)
{
    ZephioProgress p;
    ZephioResult res = zephio_progress_init_ctx(&p, &g_test_ctx, 0, 0, 20, 1);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(p.value, 0);
    TEST_EQ(p.show_percent, 1);
    TEST_EQ(p.base.focusable, 0);
    TEST_EQ(p.base.visible, 1);
    zephio_widget_destroy(&p.base);
}

TEST_BEGIN(progress_init_null)
{
    TEST_NE(zephio_progress_init_ctx(NULL, NULL, 0, 0, 20, 1), ZEPHIO_OK);
}

TEST_BEGIN(progress_set_value)
{
    ZephioProgress p;
    zephio_progress_init_ctx(&p, &g_test_ctx, 0, 0, 20, 1);

    zephio_progress_set_value(&p, 50);
    TEST_EQ(zephio_progress_get_value(&p), 50);

    zephio_progress_set_value(&p, 100);
    TEST_EQ(zephio_progress_get_value(&p), 100);

    zephio_progress_set_value(&p, 0);
    TEST_EQ(zephio_progress_get_value(&p), 0);

    zephio_widget_destroy(&p.base);
}

TEST_BEGIN(progress_value_clamped)
{
    ZephioProgress p;
    zephio_progress_init_ctx(&p, &g_test_ctx, 0, 0, 20, 1);

    zephio_progress_set_value(&p, -10);
    TEST_EQ(zephio_progress_get_value(&p), 0);

    zephio_progress_set_value(&p, 150);
    TEST_EQ(zephio_progress_get_value(&p), 100);

    zephio_widget_destroy(&p.base);
}

TEST_BEGIN(progress_get_value_null)
{
    TEST_EQ(zephio_progress_get_value(NULL), 0);
}

TEST_BEGIN(progress_set_label)
{
    ZephioProgress p;
    zephio_progress_init_ctx(&p, &g_test_ctx, 0, 0, 30, 1);

    zephio_progress_set_label(&p, "Loading");
    TEST_ASSERT(p.label != NULL);
    TEST_STR_EQ(p.label, "Loading");

    zephio_progress_set_label(&p, NULL);
    TEST_ASSERT(p.label == NULL);

    zephio_widget_destroy(&p.base);
}

TEST_BEGIN(progress_set_chars)
{
    ZephioProgress p;
    zephio_progress_init_ctx(&p, &g_test_ctx, 0, 0, 20, 1);

    zephio_progress_set_chars(&p, "#", "-");
    TEST_STR_EQ(p.fill_char, "#");
    TEST_STR_EQ(p.empty_char, "-");

    zephio_progress_set_chars(&p, NULL, NULL);
    TEST_STR_EQ(p.fill_char, "#");
    TEST_STR_EQ(p.empty_char, "-");

    zephio_widget_destroy(&p.base);
}

TEST_BEGIN(progress_show_percent)
{
    ZephioProgress p;
    zephio_progress_init_ctx(&p, &g_test_ctx, 0, 0, 20, 1);

    TEST_EQ(p.show_percent, 1);
    zephio_progress_set_show_percent(&p, 0);
    TEST_EQ(p.show_percent, 0);
    zephio_progress_set_show_percent(&p, 1);
    TEST_EQ(p.show_percent, 1);

    zephio_widget_destroy(&p.base);
}

TEST_BEGIN(progress_set_colors)
{
    ZephioProgress p;
    zephio_progress_init_ctx(&p, &g_test_ctx, 0, 0, 20, 1);

    ZephioColor fg = ZEPHIO_COLOR_INDEX(2);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(4);
    zephio_progress_set_colors(&p, fg, bg, fg, bg);
    TEST_ASSERT(zephio_color_eq(p.fg_fill, fg));
    TEST_ASSERT(zephio_color_eq(p.bg_fill, bg));

    zephio_widget_destroy(&p.base);
}

TEST_BEGIN(progress_set_attr)
{
    ZephioProgress p;
    zephio_progress_init_ctx(&p, &g_test_ctx, 0, 0, 20, 1);

    zephio_progress_set_attr(&p, ZEPHIO_ATTR_BOLD);
    TEST_EQ(p.attr, ZEPHIO_ATTR_BOLD);

    zephio_widget_destroy(&p.base);
}

/* ── Checkbox ────────────────────────────────────────────────────── */

TEST_BEGIN(checkbox_init)
{
    ZephioCheckbox cb;
    ZephioResult res = zephio_checkbox_init_ctx(&cb, &g_test_ctx, 0, 0, 20, 1, "Enable");
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(cb.state, TUI_CHECK_UNCHECKED);
    TEST_EQ(cb.tristate, 0);
    TEST_EQ(cb.base.focusable, 1);
    TEST_ASSERT(cb.label != NULL);
    TEST_STR_EQ(cb.label, "Enable");
    zephio_widget_destroy(&cb.base);
}

TEST_BEGIN(checkbox_init_null)
{
    TEST_NE(zephio_checkbox_init_ctx(NULL, NULL, 0, 0, 20, 1, "X"), ZEPHIO_OK);
}

TEST_BEGIN(checkbox_init_no_label)
{
    ZephioCheckbox cb;
    ZephioResult res = zephio_checkbox_init_ctx(&cb, &g_test_ctx, 0, 0, 10, 1, NULL);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_ASSERT(cb.label == NULL);
    zephio_widget_destroy(&cb.base);
}

TEST_BEGIN(checkbox_set_state)
{
    ZephioCheckbox cb;
    zephio_checkbox_init_ctx(&cb, &g_test_ctx, 0, 0, 20, 1, NULL);

    zephio_checkbox_set_state(&cb, TUI_CHECK_CHECKED);
    TEST_EQ(zephio_checkbox_get_state(&cb), TUI_CHECK_CHECKED);

    zephio_checkbox_set_state(&cb, TUI_CHECK_INDETERMINATE);
    TEST_EQ(zephio_checkbox_get_state(&cb), TUI_CHECK_INDETERMINATE);

    zephio_checkbox_set_state(&cb, TUI_CHECK_UNCHECKED);
    TEST_EQ(zephio_checkbox_get_state(&cb), TUI_CHECK_UNCHECKED);

    zephio_widget_destroy(&cb.base);
}

TEST_BEGIN(checkbox_get_state_null)
{
    TEST_EQ(zephio_checkbox_get_state(NULL), TUI_CHECK_UNCHECKED);
}

TEST_BEGIN(checkbox_tristate)
{
    ZephioCheckbox cb;
    zephio_checkbox_init_ctx(&cb, &g_test_ctx, 0, 0, 20, 1, NULL);
    zephio_checkbox_set_tristate(&cb, 1);

    TEST_EQ(cb.state, TUI_CHECK_UNCHECKED);

    cb.state = TUI_CHECK_CHECKED;
    TEST_EQ(cb.state, TUI_CHECK_CHECKED);

    cb.state = TUI_CHECK_INDETERMINATE;
    TEST_EQ(cb.state, TUI_CHECK_INDETERMINATE);

    cb.state = TUI_CHECK_UNCHECKED;
    TEST_EQ(cb.state, TUI_CHECK_UNCHECKED);

    TEST_EQ(cb.tristate, 1);

    zephio_widget_destroy(&cb.base);
}

TEST_BEGIN(checkbox_set_label)
{
    ZephioCheckbox cb;
    zephio_checkbox_init_ctx(&cb, &g_test_ctx, 0, 0, 20, 1, NULL);

    zephio_checkbox_set_label(&cb, "New label");
    TEST_ASSERT(cb.label != NULL);
    TEST_STR_EQ(cb.label, "New label");

    zephio_checkbox_set_label(&cb, NULL);
    TEST_ASSERT(cb.label == NULL);

    zephio_widget_destroy(&cb.base);
}

TEST_BEGIN(checkbox_set_colors)
{
    ZephioCheckbox cb;
    zephio_checkbox_init_ctx(&cb, &g_test_ctx, 0, 0, 20, 1, NULL);

    ZephioColor fg = ZEPHIO_COLOR_INDEX(10);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(5);
    ZephioColor fgf = ZEPHIO_COLOR_INDEX(1);
    ZephioColor bgf = ZEPHIO_COLOR_INDEX(2);
    zephio_checkbox_set_colors(&cb, fg, bg, fgf, bgf);
    TEST_ASSERT(zephio_color_eq(cb.fg, fg));
    TEST_ASSERT(zephio_color_eq(cb.bg, bg));
    TEST_ASSERT(zephio_color_eq(cb.fg_focused, fgf));
    TEST_ASSERT(zephio_color_eq(cb.bg_focused, bgf));

    zephio_widget_destroy(&cb.base);
}

/* ── Radio Group ─────────────────────────────────────────────────── */

TEST_BEGIN(radio_init)
{
    ZephioRadio r;
    ZephioResult res = zephio_radio_init_ctx(&r, &g_test_ctx, 0, 0, 20, 5);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(r.option_count, 0);
    TEST_EQ(r.selected, 0);
    TEST_EQ(r.base.focusable, 1);
    zephio_widget_destroy(&r.base);
}

TEST_BEGIN(radio_init_null)
{
    TEST_NE(zephio_radio_init_ctx(NULL, NULL, 0, 0, 20, 5), ZEPHIO_OK);
}

TEST_BEGIN(radio_add_option)
{
    ZephioRadio r;
    zephio_radio_init_ctx(&r, &g_test_ctx, 0, 0, 20, 5);

    ZephioResult res;
    res = zephio_radio_add_option(&r, "Option A");
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(r.option_count, 1);

    res = zephio_radio_add_option(&r, "Option B");
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(r.option_count, 2);

    res = zephio_radio_add_option(&r, "Option C");
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(r.option_count, 3);

    zephio_widget_destroy(&r.base);
}

TEST_BEGIN(radio_add_null_option)
{
    ZephioRadio r;
    zephio_radio_init_ctx(&r, &g_test_ctx, 0, 0, 20, 5);

    ZephioResult res = zephio_radio_add_option(&r, NULL);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(r.option_count, 1);
    TEST_ASSERT(r.options[0] == NULL);

    zephio_widget_destroy(&r.base);
}

TEST_BEGIN(radio_remove_option)
{
    ZephioRadio r;
    zephio_radio_init_ctx(&r, &g_test_ctx, 0, 0, 20, 5);
    zephio_radio_add_option(&r, "A");
    zephio_radio_add_option(&r, "B");
    zephio_radio_add_option(&r, "C");

    zephio_radio_remove_option(&r, 1);
    TEST_EQ(r.option_count, 2);
    TEST_STR_EQ(r.options[0], "A");
    TEST_STR_EQ(r.options[1], "C");

    zephio_widget_destroy(&r.base);
}

TEST_BEGIN(radio_remove_out_of_bounds)
{
    ZephioRadio r;
    zephio_radio_init_ctx(&r, &g_test_ctx, 0, 0, 20, 5);
    zephio_radio_add_option(&r, "A");

    zephio_radio_remove_option(&r, -1);
    TEST_EQ(r.option_count, 1);

    zephio_radio_remove_option(&r, 5);
    TEST_EQ(r.option_count, 1);

    zephio_widget_destroy(&r.base);
}

TEST_BEGIN(radio_clear)
{
    ZephioRadio r;
    zephio_radio_init_ctx(&r, &g_test_ctx, 0, 0, 20, 5);
    zephio_radio_add_option(&r, "A");
    zephio_radio_add_option(&r, "B");
    zephio_radio_add_option(&r, "C");

    zephio_radio_clear(&r);
    TEST_EQ(r.option_count, 0);
    TEST_EQ(r.selected, 0);

    zephio_widget_destroy(&r.base);
}

TEST_BEGIN(radio_selected)
{
    ZephioRadio r;
    zephio_radio_init_ctx(&r, &g_test_ctx, 0, 0, 20, 5);
    zephio_radio_add_option(&r, "A");
    zephio_radio_add_option(&r, "B");
    zephio_radio_add_option(&r, "C");

    TEST_EQ(zephio_radio_get_selected(&r), 0);
    TEST_STR_EQ(zephio_radio_get_selected_option(&r), "A");

    zephio_radio_set_selected(&r, 2);
    TEST_EQ(zephio_radio_get_selected(&r), 2);
    TEST_STR_EQ(zephio_radio_get_selected_option(&r), "C");

    zephio_widget_destroy(&r.base);
}

TEST_BEGIN(radio_selected_null)
{
    TEST_EQ(zephio_radio_get_selected(NULL), -1);
    TEST_ASSERT(zephio_radio_get_selected_option(NULL) == NULL);
}

TEST_BEGIN(radio_set_selected_bounds)
{
    ZephioRadio r;
    zephio_radio_init_ctx(&r, &g_test_ctx, 0, 0, 20, 5);
    zephio_radio_add_option(&r, "A");
    zephio_radio_add_option(&r, "B");

    zephio_radio_set_selected(&r, -1);
    TEST_EQ(r.selected, 0);

    zephio_radio_set_selected(&r, 10);
    TEST_EQ(r.selected, 1);

    zephio_widget_destroy(&r.base);
}

TEST_BEGIN(radio_set_colors)
{
    ZephioRadio r;
    zephio_radio_init_ctx(&r, &g_test_ctx, 0, 0, 20, 5);

    ZephioColor fg = ZEPHIO_COLOR_INDEX(15);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    ZephioColor fgs = ZEPHIO_COLOR_INDEX(0);
    ZephioColor bgs = ZEPHIO_COLOR_INDEX(12);
    zephio_radio_set_colors(&r, fg, bg, fgs, bgs);
    TEST_ASSERT(zephio_color_eq(r.fg, fg));
    TEST_ASSERT(zephio_color_eq(r.bg, bg));
    TEST_ASSERT(zephio_color_eq(r.fg_selected, fgs));
    TEST_ASSERT(zephio_color_eq(r.bg_selected, bgs));

    zephio_widget_destroy(&r.base);
}

TEST_BEGIN(radio_remove_adjusts_selected)
{
    ZephioRadio r;
    zephio_radio_init_ctx(&r, &g_test_ctx, 0, 0, 20, 5);
    zephio_radio_add_option(&r, "A");
    zephio_radio_add_option(&r, "B");
    zephio_radio_add_option(&r, "C");

    zephio_radio_set_selected(&r, 2);
    TEST_EQ(r.selected, 2);

    zephio_radio_remove_option(&r, 2);
    TEST_EQ(r.option_count, 2);
    TEST_EQ(r.selected, 1);

    zephio_widget_destroy(&r.base);
}

/* ── Label ──────────────────────────────────────────────────────── */

TEST_BEGIN(label_init)
{
    ZephioLabel l;
    ZephioResult res = zephio_label_init_ctx(&l, &g_test_ctx, 2, 1, 20, 1, "Hello");
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(l.base.focusable, 0);
    TEST_EQ(l.base.visible, 1);
    TEST_ASSERT(l.text != NULL);
    TEST_STR_EQ(l.text, "Hello");
    zephio_widget_destroy(&l.base);
}

TEST_BEGIN(label_init_null_widget)
{
    TEST_NE(zephio_label_init_ctx(NULL, NULL, 0, 0, 20, 1, "X"), ZEPHIO_OK);
}

TEST_BEGIN(label_init_null_text)
{
    ZephioLabel l;
    ZephioResult res = zephio_label_init_ctx(&l, &g_test_ctx, 0, 0, 20, 1, NULL);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_ASSERT(l.text == NULL);
    zephio_widget_destroy(&l.base);
}

TEST_BEGIN(label_set_text)
{
    ZephioLabel l;
    zephio_label_init_ctx(&l, &g_test_ctx, 0, 0, 20, 1, NULL);

    zephio_label_set_text(&l, "World");
    TEST_ASSERT(l.text != NULL);
    TEST_STR_EQ(l.text, "World");
    TEST_EQ(l.base.dirty, 1);

    zephio_label_set_text(&l, NULL);
    TEST_ASSERT(l.text == NULL);

    zephio_widget_destroy(&l.base);
}

TEST_BEGIN(label_set_colors)
{
    ZephioLabel l;
    zephio_label_init_ctx(&l, &g_test_ctx, 0, 0, 20, 1, NULL);

    ZephioColor fg = ZEPHIO_COLOR_INDEX(10);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(2);
    zephio_label_set_colors(&l, fg, bg);
    TEST_ASSERT(zephio_color_eq(l.fg, fg));
    TEST_ASSERT(zephio_color_eq(l.bg, bg));
    TEST_EQ(l.base.dirty, 1);

    zephio_widget_destroy(&l.base);
}

TEST_BEGIN(label_set_attr)
{
    ZephioLabel l;
    zephio_label_init_ctx(&l, &g_test_ctx, 0, 0, 20, 1, NULL);

    zephio_label_set_attr(&l, ZEPHIO_ATTR_BOLD);
    TEST_EQ(l.attr, ZEPHIO_ATTR_BOLD);

    zephio_widget_destroy(&l.base);
}

TEST_BEGIN(label_render)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    ZephioLabel l;
    zephio_label_init_ctx(&l, &g_test_ctx, 5, 2, 10, 1, "Hi");
    l.base.dirty = 1;

    zephio_widget_render(&l.base);
    TEST_EQ(l.base.dirty, 0);
    TEST_EQ(g_test_ctx.screen.back[2 * 80 + 5].ch[0], 'H');
    TEST_EQ(g_test_ctx.screen.back[2 * 80 + 6].ch[0], 'i');

    zephio_widget_destroy(&l.base);
    zephio_screen_free(&g_test_ctx);
}

TEST_BEGIN(label_render_null_text)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    ZephioLabel l;
    zephio_label_init_ctx(&l, &g_test_ctx, 0, 0, 10, 1, NULL);
    l.base.dirty = 1;

    zephio_widget_render(&l.base);
    TEST_EQ(l.base.dirty, 0);

    zephio_widget_destroy(&l.base);
    zephio_screen_free(&g_test_ctx);
}

/* ── Button ─────────────────────────────────────────────────────── */

TEST_BEGIN(button_init)
{
    ZephioButton b;
    ZephioResult res = zephio_button_init_ctx(&b, &g_test_ctx, 0, 0, 10, 3, "OK");
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(b.base.focusable, 1);
    TEST_ASSERT(b.text != NULL);
    TEST_STR_EQ(b.text, "OK");
    zephio_widget_destroy(&b.base);
}

TEST_BEGIN(button_init_null_widget)
{
    TEST_NE(zephio_button_init_ctx(NULL, NULL, 0, 0, 10, 3, "X"), ZEPHIO_OK);
}

TEST_BEGIN(button_set_text)
{
    ZephioButton b;
    zephio_button_init_ctx(&b, &g_test_ctx, 0, 0, 10, 3, NULL);

    zephio_button_set_text(&b, "Cancel");
    TEST_ASSERT(b.text != NULL);
    TEST_STR_EQ(b.text, "Cancel");
    TEST_EQ(b.base.dirty, 1);

    zephio_button_set_text(&b, NULL);
    TEST_ASSERT(b.text == NULL);

    zephio_widget_destroy(&b.base);
}

TEST_BEGIN(button_set_colors)
{
    ZephioButton b;
    zephio_button_init_ctx(&b, &g_test_ctx, 0, 0, 10, 3, NULL);

    ZephioColor fg = ZEPHIO_COLOR_INDEX(15);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(1);
    ZephioColor fgf = ZEPHIO_COLOR_INDEX(0);
    ZephioColor bgf = ZEPHIO_COLOR_INDEX(12);
    zephio_button_set_colors(&b, fg, bg, fgf, bgf);
    TEST_ASSERT(zephio_color_eq(b.fg, fg));
    TEST_ASSERT(zephio_color_eq(b.bg, bg));
    TEST_ASSERT(zephio_color_eq(b.fg_focused, fgf));
    TEST_ASSERT(zephio_color_eq(b.bg_focused, bgf));

    zephio_widget_destroy(&b.base);
}

TEST_BEGIN(button_on_click)
{
    g_btn_clicked = 0;

    ZephioButton b;
    zephio_button_init_ctx(&b, &g_test_ctx, 0, 0, 10, 3, "Go");
    zephio_button_set_on_click(&b, stub_btn_on_click, NULL);

    ZephioEvent event = {0};
    event.key = ZEPHIO_KEY_ENTER;
    int handled = zephio_widget_handle_input(&b.base, &event);
    TEST_EQ(handled, 1);
    TEST_EQ(g_btn_clicked, 1);

    zephio_widget_destroy(&b.base);
}

TEST_BEGIN(button_on_click_space)
{
    g_btn_clicked = 0;

    ZephioButton b;
    zephio_button_init_ctx(&b, &g_test_ctx, 0, 0, 10, 3, "Go");
    zephio_button_set_on_click(&b, stub_btn_on_click, NULL);

    ZephioEvent event = {0};
    event.codepoint = ' ';
    int handled = zephio_widget_handle_input(&b.base, &event);
    TEST_EQ(handled, 1);
    TEST_EQ(g_btn_clicked, 1);

    zephio_widget_destroy(&b.base);
}

TEST_BEGIN(button_mouse_click)
{
    g_btn_clicked = 0;

    ZephioButton b;
    zephio_button_init_ctx(&b, &g_test_ctx, 0, 0, 10, 3, "Go");
    b.base.abs_x = 0;
    b.base.abs_y = 0;
    zephio_button_set_on_click(&b, stub_btn_on_click, NULL);

    ZephioMouseEvent mouse = { 0, 5, ZEPHIO_MOUSE_BTN_LEFT, ZEPHIO_MOUSE_PRESS, 0 };
    int handled = zephio_widget_handle_mouse(&b.base, &mouse);
    TEST_EQ(handled, 1);
    TEST_EQ(g_btn_clicked, 1);

    zephio_widget_destroy(&b.base);
}

TEST_BEGIN(button_render)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    ZephioButton b;
    zephio_button_init_ctx(&b, &g_test_ctx, 3, 5, 8, 1, "OK");
    b.base.dirty = 1;

    zephio_widget_render(&b.base);
    TEST_EQ(b.base.dirty, 0);

    zephio_widget_destroy(&b.base);
    zephio_screen_free(&g_test_ctx);
}

/* ── Box ────────────────────────────────────────────────────────── */

TEST_BEGIN(box_init)
{
    ZephioBox box;
    ZephioResult res = zephio_box_init_ctx(&box, &g_test_ctx, 0, 0, 30, 15, ZEPHIO_BOX_SINGLE);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(box.base.focusable, 0);
    TEST_EQ(box.border_style, ZEPHIO_BOX_SINGLE);
    TEST_EQ(box.padding, 0);
    TEST_ASSERT(box.title == NULL);
    zephio_widget_destroy(&box.base);
}

TEST_BEGIN(box_init_null)
{
    TEST_NE(zephio_box_init_ctx(NULL, NULL, 0, 0, 30, 15, ZEPHIO_BOX_SINGLE), ZEPHIO_OK);
}

TEST_BEGIN(box_set_title)
{
    ZephioBox box;
    zephio_box_init_ctx(&box, &g_test_ctx, 0, 0, 30, 15, ZEPHIO_BOX_SINGLE);

    zephio_box_set_title(&box, "Settings");
    TEST_ASSERT(box.title != NULL);
    TEST_STR_EQ(box.title, "Settings");
    TEST_EQ(box.base.dirty, 1);

    zephio_box_set_title(&box, NULL);
    TEST_ASSERT(box.title == NULL);

    zephio_widget_destroy(&box.base);
}

TEST_BEGIN(box_set_colors)
{
    ZephioBox box;
    zephio_box_init_ctx(&box, &g_test_ctx, 0, 0, 30, 15, ZEPHIO_BOX_SINGLE);

    ZephioColor fg = ZEPHIO_COLOR_INDEX(14);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    zephio_box_set_colors(&box, fg, bg);
    TEST_ASSERT(zephio_color_eq(box.fg, fg));
    TEST_ASSERT(zephio_color_eq(box.bg, bg));

    zephio_widget_destroy(&box.base);
}

TEST_BEGIN(box_set_attr)
{
    ZephioBox box;
    zephio_box_init_ctx(&box, &g_test_ctx, 0, 0, 30, 15, ZEPHIO_BOX_SINGLE);

    zephio_box_set_attr(&box, ZEPHIO_ATTR_BOLD);
    TEST_EQ(box.attr, ZEPHIO_ATTR_BOLD);

    zephio_widget_destroy(&box.base);
}

TEST_BEGIN(box_set_padding)
{
    ZephioBox box;
    zephio_box_init_ctx(&box, &g_test_ctx, 0, 0, 30, 15, ZEPHIO_BOX_SINGLE);

    zephio_box_set_padding(&box, 2);
    TEST_EQ(box.padding, 2);
    TEST_EQ(box.base.dirty, 1);

    zephio_widget_destroy(&box.base);
}

TEST_BEGIN(box_render_single)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    ZephioBox box;
    zephio_box_init_ctx(&box, &g_test_ctx, 0, 0, 10, 5, ZEPHIO_BOX_SINGLE);
    box.base.dirty = 1;

    zephio_widget_render(&box.base);
    TEST_EQ(box.base.dirty, 0);
    TEST_ASSERT(memcmp(g_test_ctx.screen.back[0].ch, "\xe2\x94\x8c", 3) == 0);

    zephio_widget_destroy(&box.base);
    zephio_screen_free(&g_test_ctx);
}

TEST_BEGIN(box_render_double)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    ZephioBox box;
    zephio_box_init_ctx(&box, &g_test_ctx, 0, 0, 10, 5, ZEPHIO_BOX_DOUBLE);
    box.base.dirty = 1;

    zephio_widget_render(&box.base);
    TEST_EQ(box.base.dirty, 0);
    TEST_ASSERT(memcmp(g_test_ctx.screen.back[0].ch, "\xe2\x95\x94", 3) == 0);

    zephio_widget_destroy(&box.base);
    zephio_screen_free(&g_test_ctx);
}

TEST_BEGIN(box_render_too_small)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    ZephioBox box;
    zephio_box_init_ctx(&box, &g_test_ctx, 0, 0, 1, 1, ZEPHIO_BOX_SINGLE);
    box.base.dirty = 1;

    zephio_widget_render(&box.base);
    TEST_EQ(box.base.dirty, 0);

    zephio_widget_destroy(&box.base);
    zephio_screen_free(&g_test_ctx);
}

TEST_BEGIN(box_render_with_title)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    ZephioBox box;
    zephio_box_init_ctx(&box, &g_test_ctx, 0, 0, 20, 5, ZEPHIO_BOX_SINGLE);
    zephio_box_set_title(&box, "Test");
    box.base.dirty = 1;

    zephio_widget_render(&box.base);
    TEST_EQ(g_test_ctx.screen.back[0 * 80 + 2].ch[0], 'T');
    TEST_EQ(g_test_ctx.screen.back[0 * 80 + 3].ch[0], 'e');
    TEST_EQ(g_test_ctx.screen.back[0 * 80 + 4].ch[0], 's');
    TEST_EQ(g_test_ctx.screen.back[0 * 80 + 5].ch[0], 't');

    zephio_widget_destroy(&box.base);
    zephio_screen_free(&g_test_ctx);
}

/* ── List ───────────────────────────────────────────────────────── */

TEST_BEGIN(list_init)
{
    ZephioList list;
    ZephioResult res = zephio_list_init_ctx(&list, &g_test_ctx, 0, 0, 30, 10);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(list.base.focusable, 1);
    TEST_EQ(list.item_count, 0);
    TEST_EQ(list.selected, 0);
    TEST_EQ(list.scroll_offset, 0);
    zephio_widget_destroy(&list.base);
}

TEST_BEGIN(list_init_null)
{
    TEST_NE(zephio_list_init_ctx(NULL, NULL, 0, 0, 30, 10), ZEPHIO_OK);
}

TEST_BEGIN(list_add_item)
{
    ZephioList list;
    zephio_list_init_ctx(&list, &g_test_ctx, 0, 0, 30, 10);

    zephio_list_add_item(&list, "Item A");
    zephio_list_add_item(&list, "Item B");
    zephio_list_add_item(&list, "Item C");

    TEST_EQ(list.item_count, 3);
    TEST_STR_EQ(list.items[0], "Item A");
    TEST_STR_EQ(list.items[1], "Item B");
    TEST_STR_EQ(list.items[2], "Item C");

    zephio_widget_destroy(&list.base);
}

TEST_BEGIN(list_add_null_item)
{
    ZephioList list;
    zephio_list_init_ctx(&list, &g_test_ctx, 0, 0, 30, 10);

    ZephioResult res = zephio_list_add_item(&list, NULL);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(list.item_count, 1);
    TEST_ASSERT(list.items[0] == NULL);

    zephio_widget_destroy(&list.base);
}

TEST_BEGIN(list_remove_item)
{
    ZephioList list;
    zephio_list_init_ctx(&list, &g_test_ctx, 0, 0, 30, 10);
    zephio_list_add_item(&list, "A");
    zephio_list_add_item(&list, "B");
    zephio_list_add_item(&list, "C");

    zephio_list_remove_item(&list, 1);
    TEST_EQ(list.item_count, 2);
    TEST_STR_EQ(list.items[0], "A");
    TEST_STR_EQ(list.items[1], "C");

    zephio_widget_destroy(&list.base);
}

TEST_BEGIN(list_remove_out_of_bounds)
{
    ZephioList list;
    zephio_list_init_ctx(&list, &g_test_ctx, 0, 0, 30, 10);
    zephio_list_add_item(&list, "A");

    zephio_list_remove_item(&list, -1);
    TEST_EQ(list.item_count, 1);

    zephio_list_remove_item(&list, 5);
    TEST_EQ(list.item_count, 1);

    zephio_widget_destroy(&list.base);
}

TEST_BEGIN(list_clear)
{
    ZephioList list;
    zephio_list_init_ctx(&list, &g_test_ctx, 0, 0, 30, 10);
    zephio_list_add_item(&list, "A");
    zephio_list_add_item(&list, "B");

    zephio_list_clear(&list);
    TEST_EQ(list.item_count, 0);
    TEST_EQ(list.selected, 0);
    TEST_EQ(list.scroll_offset, 0);
    TEST_EQ(list.base.dirty, 1);

    zephio_widget_destroy(&list.base);
}

TEST_BEGIN(list_get_selected)
{
    ZephioList list;
    zephio_list_init_ctx(&list, &g_test_ctx, 0, 0, 30, 10);
    zephio_list_add_item(&list, "A");
    zephio_list_add_item(&list, "B");

    TEST_EQ(zephio_list_get_selected(&list), 0);
    TEST_STR_EQ(zephio_list_get_selected_item(&list), "A");

    list.selected = 1;
    TEST_EQ(zephio_list_get_selected(&list), 1);
    TEST_STR_EQ(zephio_list_get_selected_item(&list), "B");

    zephio_widget_destroy(&list.base);
}

TEST_BEGIN(list_get_selected_null)
{
    TEST_EQ(zephio_list_get_selected(NULL), -1);
    TEST_ASSERT(zephio_list_get_selected_item(NULL) == NULL);
}

TEST_BEGIN(list_get_selected_empty)
{
    ZephioList list;
    zephio_list_init_ctx(&list, &g_test_ctx, 0, 0, 30, 10);
    TEST_ASSERT(zephio_list_get_selected_item(&list) == NULL);
    zephio_widget_destroy(&list.base);
}

TEST_BEGIN(list_set_colors)
{
    ZephioList list;
    zephio_list_init_ctx(&list, &g_test_ctx, 0, 0, 30, 10);

    ZephioColor fg = ZEPHIO_COLOR_INDEX(15);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    ZephioColor fgs = ZEPHIO_COLOR_INDEX(0);
    ZephioColor bgs = ZEPHIO_COLOR_INDEX(12);
    zephio_list_set_colors(&list, fg, bg, fgs, bgs);
    TEST_ASSERT(zephio_color_eq(list.fg, fg));
    TEST_ASSERT(zephio_color_eq(list.bg, bg));
    TEST_ASSERT(zephio_color_eq(list.fg_selected, fgs));
    TEST_ASSERT(zephio_color_eq(list.bg_selected, bgs));

    zephio_widget_destroy(&list.base);
}

TEST_BEGIN(list_input_navigation)
{
    ZephioList list;
    zephio_list_init_ctx(&list, &g_test_ctx, 0, 0, 30, 5);
    zephio_list_add_item(&list, "A");
    zephio_list_add_item(&list, "B");
    zephio_list_add_item(&list, "C");

    ZephioEvent down = {0};
    down.key = ZEPHIO_KEY_DOWN;
    zephio_widget_handle_input(&list.base, &down);
    TEST_EQ(list.selected, 1);

    zephio_widget_handle_input(&list.base, &down);
    TEST_EQ(list.selected, 2);

    ZephioEvent up = {0};
    up.key = ZEPHIO_KEY_UP;
    zephio_widget_handle_input(&list.base, &up);
    TEST_EQ(list.selected, 1);

    zephio_widget_destroy(&list.base);
}

TEST_BEGIN(list_input_select)
{
    g_list_selected_idx = -1;

    ZephioList list;
    zephio_list_init_ctx(&list, &g_test_ctx, 0, 0, 30, 5);
    zephio_list_add_item(&list, "A");
    zephio_list_add_item(&list, "B");
    zephio_list_set_on_select(&list, stub_list_on_select, NULL);

    ZephioEvent enter = {0};
    enter.key = ZEPHIO_KEY_ENTER;
    zephio_widget_handle_input(&list.base, &enter);
    TEST_EQ(g_list_selected_idx, 0);

    zephio_widget_destroy(&list.base);
}

TEST_BEGIN(list_render)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    ZephioList list;
    zephio_list_init_ctx(&list, &g_test_ctx, 0, 0, 20, 5);
    zephio_list_add_item(&list, "Hello");
    list.base.dirty = 1;
    list.base.focused = 1;

    zephio_widget_render(&list.base);
    TEST_EQ(list.base.dirty, 0);
    TEST_EQ(g_test_ctx.screen.back[0].ch[0], '>');
    TEST_EQ(g_test_ctx.screen.back[1].ch[0], 'H');

    zephio_widget_destroy(&list.base);
    zephio_screen_free(&g_test_ctx);
}

/* ── InputField ─────────────────────────────────────────────────── */

TEST_BEGIN(input_field_init)
{
    ZephioInputField f;
    ZephioResult res = zephio_input_field_init_ctx(&f, &g_test_ctx, 0, 0, 30, 256);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(f.base.focusable, 1);
    TEST_EQ(f.text_capacity, 256);
    TEST_EQ(f.cursor_pos, 0);
    TEST_EQ(f.scroll_offset, 0);
    TEST_ASSERT(f.text == NULL);
    zephio_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_init_null)
{
    TEST_NE(zephio_input_field_init_ctx(NULL, NULL, 0, 0, 30, 256), ZEPHIO_OK);
}

TEST_BEGIN(input_field_set_text)
{
    ZephioInputField f;
    zephio_input_field_init_ctx(&f, &g_test_ctx, 0, 0, 30, 256);

    zephio_input_field_set_text(&f, "Hello");
    TEST_ASSERT(f.text != NULL);
    TEST_STR_EQ(f.text, "Hello");
    TEST_EQ(f.cursor_pos, 5);
    TEST_EQ(f.base.dirty, 1);

    zephio_input_field_set_text(&f, NULL);
    TEST_ASSERT(f.text == NULL);
    TEST_EQ(f.cursor_pos, 0);

    zephio_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_get_text)
{
    ZephioInputField f;
    zephio_input_field_init_ctx(&f, &g_test_ctx, 0, 0, 30, 256);

    TEST_STR_EQ(zephio_input_field_get_text(&f), "");

    zephio_input_field_set_text(&f, "Test");
    TEST_STR_EQ(zephio_input_field_get_text(&f), "Test");

    TEST_ASSERT(zephio_input_field_get_text(NULL) == NULL);

    zephio_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_set_colors)
{
    ZephioInputField f;
    zephio_input_field_init_ctx(&f, &g_test_ctx, 0, 0, 30, 256);

    ZephioColor fg = ZEPHIO_COLOR_INDEX(15);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(234);
    ZephioColor cfg = ZEPHIO_COLOR_INDEX(0);
    ZephioColor cbg = ZEPHIO_COLOR_INDEX(15);
    zephio_input_field_set_colors(&f, fg, bg, cfg, cbg);
    TEST_ASSERT(zephio_color_eq(f.fg, fg));
    TEST_ASSERT(zephio_color_eq(f.bg, bg));
    TEST_ASSERT(zephio_color_eq(f.cursor_fg, cfg));
    TEST_ASSERT(zephio_color_eq(f.cursor_bg, cbg));

    zephio_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_typing)
{
    ZephioInputField f;
    zephio_input_field_init_ctx(&f, &g_test_ctx, 0, 0, 30, 256);

    ZephioEvent e = {0};
    e.key = ZEPHIO_KEY_UNKNOWN;

    e.codepoint = 'A';
    zephio_widget_handle_input(&f.base, &e);
    TEST_STR_EQ(f.text, "A");
    TEST_EQ(f.cursor_pos, 1);

    e.codepoint = 'B';
    zephio_widget_handle_input(&f.base, &e);
    TEST_STR_EQ(f.text, "AB");
    TEST_EQ(f.cursor_pos, 2);

    zephio_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_backspace)
{
    ZephioInputField f;
    zephio_input_field_init_ctx(&f, &g_test_ctx, 0, 0, 30, 256);
    zephio_input_field_set_text(&f, "ABC");
    f.cursor_pos = 3;

    ZephioEvent e = {0};
    e.key = ZEPHIO_KEY_BACKSPACE;
    zephio_widget_handle_input(&f.base, &e);
    TEST_STR_EQ(f.text, "AB");
    TEST_EQ(f.cursor_pos, 2);

    zephio_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_delete)
{
    ZephioInputField f;
    zephio_input_field_init_ctx(&f, &g_test_ctx, 0, 0, 30, 256);
    zephio_input_field_set_text(&f, "ABC");
    f.cursor_pos = 1;

    ZephioEvent e = {0};
    e.key = ZEPHIO_KEY_DELETE;
    zephio_widget_handle_input(&f.base, &e);
    TEST_STR_EQ(f.text, "AC");
    TEST_EQ(f.cursor_pos, 1);

    zephio_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_cursor_movement)
{
    ZephioInputField f;
    zephio_input_field_init_ctx(&f, &g_test_ctx, 0, 0, 30, 256);
    zephio_input_field_set_text(&f, "ABCDE");

    ZephioEvent left = {0};
    left.key = ZEPHIO_KEY_LEFT;

    ZephioEvent right = {0};
    right.key = ZEPHIO_KEY_RIGHT;

    zephio_widget_handle_input(&f.base, &left);
    TEST_EQ(f.cursor_pos, 4);

    zephio_widget_handle_input(&f.base, &left);
    TEST_EQ(f.cursor_pos, 3);

    zephio_widget_handle_input(&f.base, &right);
    TEST_EQ(f.cursor_pos, 4);

    zephio_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_home_end)
{
    ZephioInputField f;
    zephio_input_field_init_ctx(&f, &g_test_ctx, 0, 0, 30, 256);
    zephio_input_field_set_text(&f, "ABCDE");

    ZephioEvent home = {0};
    home.key = ZEPHIO_KEY_HOME;
    zephio_widget_handle_input(&f.base, &home);
    TEST_EQ(f.cursor_pos, 0);

    ZephioEvent end = {0};
    end.key = ZEPHIO_KEY_END;
    zephio_widget_handle_input(&f.base, &end);
    TEST_EQ(f.cursor_pos, 5);

    zephio_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_submit)
{
    g_field_submitted = 0;

    ZephioInputField f;
    zephio_input_field_init_ctx(&f, &g_test_ctx, 0, 0, 30, 256);
    zephio_input_field_set_on_submit(&f, stub_field_on_submit, NULL);

    ZephioEvent enter = {0};
    enter.key = ZEPHIO_KEY_ENTER;
    zephio_widget_handle_input(&f.base, &enter);
    TEST_EQ(g_field_submitted, 1);

    zephio_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_on_change)
{
    g_field_change_count = 0;

    ZephioInputField f;
    zephio_input_field_init_ctx(&f, &g_test_ctx, 0, 0, 30, 256);
    zephio_input_field_set_on_change(&f, stub_field_on_change, NULL);

    ZephioEvent e = {0};
    e.key = ZEPHIO_KEY_UNKNOWN;
    e.codepoint = 'X';
    zephio_widget_handle_input(&f.base, &e);
    TEST_EQ(g_field_change_count, 1);

    zephio_widget_destroy(&f.base);
}

TEST_BEGIN(input_field_render)
{
    zephio_screen_init(&g_test_ctx, 24, 80);
    ZephioInputField f;
    zephio_input_field_init_ctx(&f, &g_test_ctx, 0, 0, 20, 256);
    zephio_input_field_set_text(&f, "Hi");
    f.base.dirty = 1;
    f.base.focused = 1;

    zephio_widget_render(&f.base);
    TEST_EQ(f.base.dirty, 0);
    TEST_EQ(g_test_ctx.screen.back[0].ch[0], 'H');
    TEST_EQ(g_test_ctx.screen.back[1].ch[0], 'i');

    zephio_widget_destroy(&f.base);
    zephio_screen_free(&g_test_ctx);
}

TEST_BEGIN(input_field_capacity_truncation)
{
    ZephioInputField f;
    zephio_input_field_init_ctx(&f, &g_test_ctx, 0, 0, 20, 5);

    zephio_input_field_set_text(&f, "ABCDE");
    TEST_STR_EQ(f.text, "ABCD");

    zephio_widget_destroy(&f.base);
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
