#include "util.h"
#include "zephio_container.h"
#include "zephio_separator.h"
#include "zephio_statusbar.h"
#include "zephio_scroll_container.h"
#include "zephio_split_pane.h"
#include "zephio_dialog.h"
#include "zephio_dropdown.h"
#include "zephio_context_menu.h"
#include "zephio_menubar.h"
#include "zephio_tabbar.h"
#include "zephio_table.h"
#include "zephio_text_view.h"
#include "zephio_textarea.h"
#include "zephio_toast.h"
#include "zephio_tree_view.h"
#include "zephio_screen.h"

#include <string.h>
#include <stdlib.h>

static int g_tab_changed = 0;
static int g_table_selected = -1;
static int g_ctx_selected = -1;
static int g_dialog_btn = -1;

static void stub_tab_on_change(ZephioTabBar *tb, int idx, const char *lbl, void *ud) {
    (void)tb; (void)lbl; (void)ud; g_tab_changed = idx;
}
static void stub_table_on_select(ZephioWidget *w, int row, void *ud) {
    (void)w; (void)ud; g_table_selected = row;
}
static void stub_ctx_on_select(ZephioContextMenu *m, int idx, const char *lbl, void *ud) {
    (void)m; (void)lbl; (void)ud; g_ctx_selected = idx;
}
static void stub_dialog_on_button(ZephioDialog *d, int btn, void *ud) {
    (void)d; (void)ud; g_dialog_btn = btn;
}

/* ── Container ──────────────────────────────────────────────────── */

TEST_BEGIN(container_init)
{
    ZephioContainer c;
    ZephioResult res = zephio_container_init(&c, 2, 3, 40, 10);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(c.base.x, 2);
    TEST_EQ(c.base.y, 3);
    TEST_EQ(c.base.width, 40);
    TEST_EQ(c.base.height, 10);
    TEST_EQ(c.base.focusable, 0);
    zephio_widget_destroy(&c.base);
}

TEST_BEGIN(container_init_null)
{
    TEST_NE(zephio_container_init(NULL, 0, 0, 10, 5), ZEPHIO_OK);
}

TEST_BEGIN(container_set_bg)
{
    ZephioContainer c;
    zephio_container_init(&c, 0, 0, 20, 5);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(4);
    zephio_container_set_bg(&c, bg);
    TEST_ASSERT(zephio_color_eq(c.bg, bg));
    TEST_EQ(c.base.dirty, 1);
    zephio_widget_destroy(&c.base);
}

TEST_BEGIN(container_set_bg_null)
{
    zephio_container_set_bg(NULL, ZEPHIO_COLOR_INDEX(0));
}

TEST_BEGIN(container_render)
{
    zephio_screen_init(24, 80);
    ZephioContainer c;
    zephio_container_init(&c, 0, 0, 5, 3);
    c.base.dirty = 1;
    zephio_widget_render(&c.base);
    TEST_EQ(c.base.dirty, 0);
    zephio_widget_destroy(&c.base);
    zephio_screen_free();
}

/* ── Separator ──────────────────────────────────────────────────── */

TEST_BEGIN(separator_init_h)
{
    ZephioSeparator s;
    ZephioResult res = zephio_separator_init_h(&s, 0, 5, 30);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(s.horizontal, 1);
    TEST_EQ(s.base.width, 30);
    TEST_EQ(s.base.height, 1);
    TEST_EQ(s.base.focusable, 0);
    zephio_widget_destroy(&s.base);
}

TEST_BEGIN(separator_init_v)
{
    ZephioSeparator s;
    ZephioResult res = zephio_separator_init_v(&s, 10, 0, 15);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(s.horizontal, 0);
    TEST_EQ(s.base.width, 1);
    TEST_EQ(s.base.height, 15);
    zephio_widget_destroy(&s.base);
}

TEST_BEGIN(separator_init_null)
{
    TEST_NE(zephio_separator_init_h(NULL, 0, 0, 10), ZEPHIO_OK);
    TEST_NE(zephio_separator_init_v(NULL, 0, 0, 10), ZEPHIO_OK);
}

TEST_BEGIN(separator_set_colors)
{
    ZephioSeparator s;
    zephio_separator_init_h(&s, 0, 0, 20);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(14);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    zephio_separator_set_colors(&s, fg, bg);
    TEST_ASSERT(zephio_color_eq(s.fg, fg));
    TEST_ASSERT(zephio_color_eq(s.bg, bg));
    zephio_widget_destroy(&s.base);
}

TEST_BEGIN(separator_set_attr)
{
    ZephioSeparator s;
    zephio_separator_init_h(&s, 0, 0, 20);
    zephio_separator_set_attr(&s, ZEPHIO_ATTR_BOLD);
    TEST_EQ(s.attr, ZEPHIO_ATTR_BOLD);
    zephio_widget_destroy(&s.base);
}

TEST_BEGIN(separator_render_h)
{
    zephio_screen_init(24, 80);
    ZephioSeparator s;
    zephio_separator_init_h(&s, 0, 5, 10);
    s.base.dirty = 1;
    zephio_widget_render(&s.base);
    TEST_EQ(s.base.dirty, 0);
    TEST_ASSERT(memcmp(g_screen.back[5 * 80].ch, "\xe2\x94\x80", 3) == 0);
    zephio_widget_destroy(&s.base);
    zephio_screen_free();
}

TEST_BEGIN(separator_render_v)
{
    zephio_screen_init(24, 80);
    ZephioSeparator s;
    zephio_separator_init_v(&s, 10, 0, 3);
    s.base.dirty = 1;
    zephio_widget_render(&s.base);
    TEST_EQ(s.base.dirty, 0);
    TEST_ASSERT(memcmp(g_screen.back[0 * 80 + 10].ch, "\xe2\x94\x82", 3) == 0);
    TEST_ASSERT(memcmp(g_screen.back[1 * 80 + 10].ch, "\xe2\x94\x82", 3) == 0);
    zephio_widget_destroy(&s.base);
    zephio_screen_free();
}

/* ── StatusBar ──────────────────────────────────────────────────── */

TEST_BEGIN(statusbar_init)
{
    ZephioStatusBar sb;
    ZephioResult res = zephio_statusbar_init(&sb, 0, 23, 80);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(sb.base.width, 80);
    TEST_EQ(sb.base.height, 1);
    TEST_EQ(sb.base.focusable, 0);
    TEST_ASSERT(sb.text_left == NULL);
    TEST_ASSERT(sb.text_center == NULL);
    TEST_ASSERT(sb.text_right == NULL);
    TEST_ASSERT(sb.message == NULL);
    TEST_EQ(sb.message_ticks, 0);
    zephio_widget_destroy(&sb.base);
}

TEST_BEGIN(statusbar_init_null)
{
    TEST_NE(zephio_statusbar_init(NULL, 0, 0, 80), ZEPHIO_OK);
}

TEST_BEGIN(statusbar_set_text)
{
    ZephioStatusBar sb;
    zephio_statusbar_init(&sb, 0, 0, 80);
    zephio_statusbar_set_text(&sb, "Left", "Center", "Right");
    TEST_ASSERT(sb.text_left != NULL);
    TEST_STR_EQ(sb.text_left, "Left");
    TEST_ASSERT(sb.text_center != NULL);
    TEST_STR_EQ(sb.text_center, "Center");
    TEST_ASSERT(sb.text_right != NULL);
    TEST_STR_EQ(sb.text_right, "Right");
    TEST_EQ(sb.base.dirty, 1);
    zephio_widget_destroy(&sb.base);
}

TEST_BEGIN(statusbar_set_text_null)
{
    ZephioStatusBar sb;
    zephio_statusbar_init(&sb, 0, 0, 80);
    zephio_statusbar_set_text(&sb, NULL, NULL, NULL);
    TEST_ASSERT(sb.text_left == NULL);
    TEST_ASSERT(sb.text_center == NULL);
    TEST_ASSERT(sb.text_right == NULL);
    zephio_widget_destroy(&sb.base);
}

TEST_BEGIN(statusbar_set_message)
{
    ZephioStatusBar sb;
    zephio_statusbar_init(&sb, 0, 0, 80);
    zephio_statusbar_set_message(&sb, "Saving...", 100);
    TEST_ASSERT(sb.message != NULL);
    TEST_STR_EQ(sb.message, "Saving...");
    TEST_EQ(sb.message_ticks, 100);
    zephio_widget_destroy(&sb.base);
}

TEST_BEGIN(statusbar_set_message_null)
{
    ZephioStatusBar sb;
    zephio_statusbar_init(&sb, 0, 0, 80);
    zephio_statusbar_set_message(&sb, NULL, 0);
    TEST_ASSERT(sb.message == NULL);
    zephio_widget_destroy(&sb.base);
}

TEST_BEGIN(statusbar_tick)
{
    ZephioStatusBar sb;
    zephio_statusbar_init(&sb, 0, 0, 80);
    zephio_statusbar_set_message(&sb, "Hi", 2);
    TEST_EQ(sb.message_ticks, 2);

    zephio_statusbar_tick(&sb);
    TEST_EQ(sb.message_ticks, 1);
    TEST_ASSERT(sb.message != NULL);

    zephio_statusbar_tick(&sb);
    TEST_EQ(sb.message_ticks, 0);
    TEST_ASSERT(sb.message == NULL);
    zephio_widget_destroy(&sb.base);
}

TEST_BEGIN(statusbar_set_colors)
{
    ZephioStatusBar sb;
    zephio_statusbar_init(&sb, 0, 0, 80);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(15);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(4);
    zephio_statusbar_set_colors(&sb, fg, bg);
    TEST_ASSERT(zephio_color_eq(sb.fg, fg));
    TEST_ASSERT(zephio_color_eq(sb.bg, bg));
    zephio_widget_destroy(&sb.base);
}

TEST_BEGIN(statusbar_set_message_colors)
{
    ZephioStatusBar sb;
    zephio_statusbar_init(&sb, 0, 0, 80);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(0);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(11);
    zephio_statusbar_set_message_colors(&sb, fg, bg);
    TEST_ASSERT(zephio_color_eq(sb.fg_message, fg));
    TEST_ASSERT(zephio_color_eq(sb.bg_message, bg));
    zephio_widget_destroy(&sb.base);
}

/* ── ScrollContainer ────────────────────────────────────────────── */

TEST_BEGIN(scroll_container_init)
{
    ZephioScrollContainer sc;
    ZephioResult res = zephio_scroll_container_init(&sc, 0, 0, 40, 20);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(sc.base.width, 40);
    TEST_EQ(sc.base.height, 20);
    TEST_EQ(sc.scroll_x, 0);
    TEST_EQ(sc.scroll_y, 0);
    TEST_EQ(sc.content_width, 40);
    TEST_EQ(sc.content_height, 20);
    TEST_EQ(sc.base.focusable, 1);
    TEST_EQ(sc.base.manages_children, 1);
    zephio_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_init_null)
{
    TEST_NE(zephio_scroll_container_init(NULL, 0, 0, 40, 20), ZEPHIO_OK);
}

TEST_BEGIN(scroll_container_set_content_size)
{
    ZephioScrollContainer sc;
    zephio_scroll_container_init(&sc, 0, 0, 40, 20);
    zephio_scroll_container_set_content_size(&sc, 100, 50);
    TEST_EQ(sc.content_width, 100);
    TEST_EQ(sc.content_height, 50);
    zephio_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_scroll_to)
{
    ZephioScrollContainer sc;
    zephio_scroll_container_init(&sc, 0, 0, 40, 20);
    zephio_scroll_container_set_content_size(&sc, 100, 100);
    zephio_scroll_container_scroll_to(&sc, 10, 20);
    TEST_EQ(sc.scroll_x, 10);
    TEST_EQ(sc.scroll_y, 20);
    zephio_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_set_scroll_xy)
{
    ZephioScrollContainer sc;
    zephio_scroll_container_init(&sc, 0, 0, 40, 20);
    zephio_scroll_container_set_scroll_x(&sc, 5);
    zephio_scroll_container_set_scroll_y(&sc, 10);
    TEST_EQ(sc.scroll_x, 5);
    TEST_EQ(sc.scroll_y, 10);
    zephio_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_clamp_scroll)
{
    ZephioScrollContainer sc;
    zephio_scroll_container_init(&sc, 0, 0, 40, 20);
    zephio_scroll_container_set_content_size(&sc, 60, 50);
    sc.scroll_y = 100;
    sc.scroll_x = 100;
    zephio_scroll_container_clamp_scroll(&sc, 39, 19);
    TEST_EQ(sc.scroll_y, 31);
    TEST_EQ(sc.scroll_x, 21);
    zephio_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_clamp_negative)
{
    ZephioScrollContainer sc;
    zephio_scroll_container_init(&sc, 0, 0, 40, 20);
    sc.scroll_y = -5;
    sc.scroll_x = -3;
    zephio_scroll_container_clamp_scroll(&sc, 40, 20);
    TEST_EQ(sc.scroll_y, 0);
    TEST_EQ(sc.scroll_x, 0);
    zephio_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_auto_content_size)
{
    ZephioScrollContainer sc;
    zephio_scroll_container_init(&sc, 0, 0, 40, 20);
    ZephioWidget c1, c2;
    zephio_widget_init(&c1, 0, 0, 60, 10, NULL, NULL);
    zephio_widget_init(&c2, 10, 10, 30, 40, NULL, NULL);
    zephio_widget_add_child(&sc.base, &c1);
    zephio_widget_add_child(&sc.base, &c2);
    zephio_scroll_container_auto_content_size(&sc);
    TEST_EQ(sc.content_width, 60);
    TEST_EQ(sc.content_height, 50);
    zephio_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_input_arrow_down)
{
    ZephioScrollContainer sc;
    zephio_scroll_container_init(&sc, 0, 0, 40, 20);
    zephio_scroll_container_set_content_size(&sc, 40, 50);
    ZephioEvent e = {0};
    e.key = ZEPHIO_KEY_DOWN;
    int handled = zephio_scroll_container_handle_input(&sc.base, &e);
    TEST_EQ(handled, 1);
    TEST_EQ(sc.scroll_y, 1);
    zephio_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_input_page_down)
{
    ZephioScrollContainer sc;
    zephio_scroll_container_init(&sc, 0, 0, 40, 20);
    zephio_scroll_container_set_content_size(&sc, 40, 100);
    ZephioEvent e = {0};
    e.key = ZEPHIO_KEY_PAGE_DOWN;
    zephio_scroll_container_handle_input(&sc.base, &e);
    TEST_ASSERT(sc.scroll_y > 0);
    zephio_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_input_home)
{
    ZephioScrollContainer sc;
    zephio_scroll_container_init(&sc, 0, 0, 40, 20);
    zephio_scroll_container_set_content_size(&sc, 40, 100);
    sc.scroll_y = 10;
    sc.scroll_x = 5;
    ZephioEvent e = {0};
    e.key = ZEPHIO_KEY_HOME;
    zephio_scroll_container_handle_input(&sc.base, &e);
    TEST_EQ(sc.scroll_y, 0);
    TEST_EQ(sc.scroll_x, 0);
    zephio_widget_destroy(&sc.base);
}

/* ── SplitPane ──────────────────────────────────────────────────── */

TEST_BEGIN(split_pane_init_h)
{
    ZephioSplitPane sp;
    ZephioResult res = zephio_split_pane_init(&sp, 0, 0, 80, 24, ZEPHIO_SPLIT_HORIZONTAL);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(sp.orientation, ZEPHIO_SPLIT_HORIZONTAL);
    TEST_EQ(sp.base.width, 80);
    TEST_EQ(sp.base.height, 24);
    TEST_EQ(sp.separator_size, 1);
    TEST_EQ(sp.split_pos, 40);
    TEST_EQ(sp.base.manages_children, 1);
    zephio_widget_destroy(&sp.base);
}

TEST_BEGIN(split_pane_init_v)
{
    ZephioSplitPane sp;
    ZephioResult res = zephio_split_pane_init(&sp, 0, 0, 80, 24, ZEPHIO_SPLIT_VERTICAL);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(sp.orientation, ZEPHIO_SPLIT_VERTICAL);
    TEST_EQ(sp.split_pos, 12);
    zephio_widget_destroy(&sp.base);
}

TEST_BEGIN(split_pane_init_null)
{
    TEST_NE(zephio_split_pane_init(NULL, 0, 0, 80, 24, ZEPHIO_SPLIT_HORIZONTAL), ZEPHIO_OK);
}

TEST_BEGIN(split_pane_set_position)
{
    ZephioSplitPane sp;
    zephio_split_pane_init(&sp, 0, 0, 80, 24, ZEPHIO_SPLIT_HORIZONTAL);
    zephio_split_pane_set_position(&sp, 30);
    TEST_EQ(zephio_split_pane_get_position(&sp), 30);
    zephio_split_pane_set_position(&sp, 5);
    TEST_ASSERT(zephio_split_pane_get_position(&sp) >= sp.min_pane1);
    zephio_widget_destroy(&sp.base);
}

TEST_BEGIN(split_pane_set_position_null)
{
    zephio_split_pane_set_position(NULL, 10);
    TEST_EQ(zephio_split_pane_get_position(NULL), 0);
}

TEST_BEGIN(split_pane_set_min_sizes)
{
    ZephioSplitPane sp;
    zephio_split_pane_init(&sp, 0, 0, 80, 24, ZEPHIO_SPLIT_HORIZONTAL);
    zephio_split_pane_set_min_sizes(&sp, 10, 15);
    TEST_EQ(sp.min_pane1, 10);
    TEST_EQ(sp.min_pane2, 15);
    zephio_widget_destroy(&sp.base);
}

TEST_BEGIN(split_pane_set_separator_style)
{
    ZephioSplitPane sp;
    zephio_split_pane_init(&sp, 0, 0, 80, 24, ZEPHIO_SPLIT_HORIZONTAL);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(14);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    zephio_split_pane_set_separator_style(&sp, fg, bg, ZEPHIO_ATTR_BOLD);
    TEST_ASSERT(zephio_color_eq(sp.sep_fg, fg));
    TEST_ASSERT(zephio_color_eq(sp.sep_bg, bg));
    TEST_EQ(sp.sep_attr, ZEPHIO_ATTR_BOLD);
    zephio_widget_destroy(&sp.base);
}

TEST_BEGIN(split_pane_set_panes)
{
    ZephioSplitPane sp;
    zephio_split_pane_init(&sp, 0, 0, 80, 24, ZEPHIO_SPLIT_HORIZONTAL);
    ZephioWidget p1, p2;
    zephio_widget_init(&p1, 0, 0, 40, 24, NULL, NULL);
    zephio_widget_init(&p2, 0, 0, 40, 24, NULL, NULL);
    zephio_split_pane_set_panes(&sp, &p1, &p2);
    TEST_EQ(sp.base.child_count, 2);
    TEST_EQ(p1.parent, &sp.base);
    TEST_EQ(p2.parent, &sp.base);
    zephio_widget_destroy(&sp.base);
}

TEST_BEGIN(split_pane_get_pane_rects_h)
{
    ZephioSplitPane sp;
    zephio_split_pane_init(&sp, 5, 2, 80, 24, ZEPHIO_SPLIT_HORIZONTAL);
    zephio_split_pane_set_position(&sp, 30);
    int x1, y1, w1, h1;
    int x2, y2, w2, h2;
    tui_split_pane_get_pane1_rect(&sp, &x1, &y1, &w1, &h1);
    tui_split_pane_get_pane2_rect(&sp, &x2, &y2, &w2, &h2);
    TEST_EQ(w1, 30);
    TEST_EQ(h1, 24);
    TEST_EQ(w2, 49);
    TEST_EQ(h2, 24);
    TEST_EQ(w1 + 1 + w2, 80);
    zephio_widget_destroy(&sp.base);
}

TEST_BEGIN(split_pane_get_pane_rects_v)
{
    ZephioSplitPane sp;
    zephio_split_pane_init(&sp, 0, 0, 80, 24, ZEPHIO_SPLIT_VERTICAL);
    zephio_split_pane_set_position(&sp, 10);
    int x1, y1, w1, h1;
    int x2, y2, w2, h2;
    tui_split_pane_get_pane1_rect(&sp, &x1, &y1, &w1, &h1);
    tui_split_pane_get_pane2_rect(&sp, &x2, &y2, &w2, &h2);
    TEST_EQ(h1, 10);
    TEST_EQ(w1, 80);
    TEST_EQ(h2, 13);
    TEST_EQ(h1 + 1 + h2, 24);
    zephio_widget_destroy(&sp.base);
}

/* ── Dialog ─────────────────────────────────────────────────────── */

TEST_BEGIN(dialog_init)
{
    ZephioDialog d;
    ZephioResult res = zephio_dialog_init(&d, "Title", "Message body");
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(d.base.focusable, 1);
    TEST_ASSERT(d.title != NULL);
    TEST_STR_EQ(d.title, "Title");
    TEST_ASSERT(d.message != NULL);
    TEST_STR_EQ(d.message, "Message body");
    TEST_EQ(d.button_count, 0);
    zephio_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_init_null_widget)
{
    TEST_NE(zephio_dialog_init(NULL, "T", "M"), ZEPHIO_OK);
}

TEST_BEGIN(dialog_init_null_strings)
{
    ZephioDialog d;
    ZephioResult res = zephio_dialog_init(&d, NULL, NULL);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_ASSERT(d.title == NULL);
    TEST_ASSERT(d.message == NULL);
    zephio_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_add_button)
{
    ZephioDialog d;
    zephio_dialog_init(&d, NULL, NULL);
    int idx1 = zephio_dialog_add_button(&d, "OK");
    int idx2 = zephio_dialog_add_button(&d, "Cancel");
    TEST_EQ(idx1, 0);
    TEST_EQ(idx2, 1);
    TEST_EQ(d.button_count, 2);
    TEST_STR_EQ(d.button_labels[0], "OK");
    TEST_STR_EQ(d.button_labels[1], "Cancel");
    zephio_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_add_button_max)
{
    ZephioDialog d;
    zephio_dialog_init(&d, NULL, NULL);
    zephio_dialog_add_button(&d, "A");
    zephio_dialog_add_button(&d, "B");
    zephio_dialog_add_button(&d, "C");
    zephio_dialog_add_button(&d, "D");
    int idx = zephio_dialog_add_button(&d, "E");
    TEST_EQ(idx, -1);
    TEST_EQ(d.button_count, 4);
    zephio_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_add_button_null)
{
    TEST_EQ(zephio_dialog_add_button(NULL, "X"), -1);
    ZephioDialog d;
    zephio_dialog_init(&d, NULL, NULL);
    TEST_EQ(zephio_dialog_add_button(&d, NULL), -1);
    zephio_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_set_on_button)
{
    g_dialog_btn = -1;
    ZephioDialog d;
    zephio_dialog_init(&d, NULL, NULL);
    zephio_dialog_add_button(&d, "OK");
    zephio_dialog_set_on_button(&d, stub_dialog_on_button, NULL);

    ZephioEvent e = {0};
    e.key = ZEPHIO_KEY_ENTER;
    zephio_widget_handle_input(&d.base, &e);
    TEST_EQ(g_dialog_btn, 0);
    zephio_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_input_tab)
{
    ZephioDialog d;
    zephio_dialog_init(&d, NULL, NULL);
    zephio_dialog_add_button(&d, "A");
    zephio_dialog_add_button(&d, "B");
    zephio_dialog_add_button(&d, "C");

    ZephioEvent e = {0};
    e.key = ZEPHIO_KEY_TAB;
    zephio_widget_handle_input(&d.base, &e);
    TEST_EQ(d.selected_button, 1);
    zephio_widget_handle_input(&d.base, &e);
    TEST_EQ(d.selected_button, 2);
    zephio_widget_handle_input(&d.base, &e);
    TEST_EQ(d.selected_button, 0);
    zephio_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_input_arrows)
{
    ZephioDialog d;
    zephio_dialog_init(&d, NULL, NULL);
    zephio_dialog_add_button(&d, "A");
    zephio_dialog_add_button(&d, "B");

    ZephioEvent e = {0};
    e.key = ZEPHIO_KEY_LEFT;
    zephio_widget_handle_input(&d.base, &e);
    TEST_EQ(d.selected_button, 1);

    e.key = ZEPHIO_KEY_RIGHT;
    zephio_widget_handle_input(&d.base, &e);
    TEST_EQ(d.selected_button, 0);
    zephio_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_get_selected)
{
    ZephioDialog d;
    zephio_dialog_init(&d, NULL, NULL);
    TEST_EQ(zephio_dialog_get_selected(&d), 0);
    TEST_EQ(zephio_dialog_get_selected(NULL), -1);
    zephio_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_center)
{
    zephio_screen_init(24, 80);
    ZephioDialog d;
    zephio_dialog_init(&d, "Test", "Msg");
    zephio_dialog_center(&d);
    TEST_ASSERT(d.base.x > 0);
    TEST_ASSERT(d.base.y > 0);
    zephio_widget_destroy(&d.base);
    zephio_screen_free();
}

/* ── Dropdown ───────────────────────────────────────────────────── */

TEST_BEGIN(dropdown_init)
{
    ZephioDropdown dd;
    ZephioResult res = zephio_dropdown_init(&dd, 0, 0, 20);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(dd.base.width, 20);
    TEST_EQ(dd.base.focusable, 1);
    TEST_EQ(dd.item_count, 0);
    TEST_EQ(dd.selected, -1);
    TEST_EQ(dd.is_open, 0);
    zephio_widget_destroy(&dd.base);
}

TEST_BEGIN(dropdown_init_null)
{
    TEST_NE(zephio_dropdown_init(NULL, 0, 0, 20), ZEPHIO_OK);
}

TEST_BEGIN(dropdown_add_item)
{
    ZephioDropdown dd;
    zephio_dropdown_init(&dd, 0, 0, 20);
    zephio_dropdown_add_item(&dd, "Option A");
    zephio_dropdown_add_item(&dd, "Option B");
    zephio_dropdown_add_item(&dd, "Option C");
    TEST_EQ(dd.item_count, 3);
    TEST_STR_EQ(dd.items[0], "Option A");
    zephio_widget_destroy(&dd.base);
}

TEST_BEGIN(dropdown_clear)
{
    ZephioDropdown dd;
    zephio_dropdown_init(&dd, 0, 0, 20);
    zephio_dropdown_add_item(&dd, "A");
    zephio_dropdown_add_item(&dd, "B");
    zephio_dropdown_clear(&dd);
    TEST_EQ(dd.item_count, 0);
    zephio_widget_destroy(&dd.base);
}

TEST_BEGIN(dropdown_get_selected)
{
    ZephioDropdown dd;
    zephio_dropdown_init(&dd, 0, 0, 20);
    zephio_dropdown_add_item(&dd, "A");
    zephio_dropdown_add_item(&dd, "B");
    TEST_EQ(zephio_dropdown_get_selected(&dd), 0);
    TEST_STR_EQ(zephio_dropdown_get_selected_item(&dd), "A");
    zephio_dropdown_set_selected(&dd, 1);
    TEST_EQ(zephio_dropdown_get_selected(&dd), 1);
    zephio_widget_destroy(&dd.base);
}

TEST_BEGIN(dropdown_set_colors)
{
    ZephioDropdown dd;
    zephio_dropdown_init(&dd, 0, 0, 20);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(15);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    zephio_dropdown_set_colors(&dd, fg, bg, fg, bg);
    TEST_ASSERT(zephio_color_eq(dd.fg, fg));
    TEST_ASSERT(zephio_color_eq(dd.bg, bg));
    zephio_widget_destroy(&dd.base);
}

TEST_BEGIN(dropdown_set_popup_colors)
{
    ZephioDropdown dd;
    zephio_dropdown_init(&dd, 0, 0, 20);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(7);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    zephio_dropdown_set_popup_colors(&dd, fg, bg, fg, bg);
    TEST_ASSERT(zephio_color_eq(dd.fg_popup, fg));
    zephio_widget_destroy(&dd.base);
}

TEST_BEGIN(dropdown_set_max_visible)
{
    ZephioDropdown dd;
    zephio_dropdown_init(&dd, 0, 0, 20);
    zephio_dropdown_set_max_visible(&dd, 5);
    TEST_EQ(dd.max_visible, 5);
    zephio_widget_destroy(&dd.base);
}

/* ── ContextMenu ────────────────────────────────────────────────── */

TEST_BEGIN(ctx_menu_init)
{
    ZephioContextMenu m;
    ZephioResult res = zephio_context_menu_init(&m);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(m.item_count, 0);
    TEST_EQ(m.highlighted, -1);
    TEST_EQ(m.is_visible, 0);
    zephio_widget_destroy(&m.base);
}

TEST_BEGIN(ctx_menu_add_item)
{
    ZephioContextMenu m;
    zephio_context_menu_init(&m);
    zephio_context_menu_add_item(&m, "Copy");
    zephio_context_menu_add_item(&m, "Paste");
    zephio_context_menu_add_separator(&m);
    zephio_context_menu_add_item(&m, "Delete");
    TEST_EQ(m.item_count, 4);
    TEST_STR_EQ(m.items[0].label, "Copy");
    TEST_EQ(m.items[2].is_separator, 1);
    zephio_widget_destroy(&m.base);
}

TEST_BEGIN(ctx_menu_clear)
{
    ZephioContextMenu m;
    zephio_context_menu_init(&m);
    zephio_context_menu_add_item(&m, "A");
    zephio_context_menu_add_item(&m, "B");
    zephio_context_menu_clear(&m);
    TEST_EQ(m.item_count, 0);
    zephio_widget_destroy(&m.base);
}

TEST_BEGIN(ctx_menu_set_colors)
{
    ZephioContextMenu m;
    zephio_context_menu_init(&m);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(15);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    zephio_context_menu_set_colors(&m, fg, bg, fg, bg);
    TEST_ASSERT(zephio_color_eq(m.fg, fg));
    TEST_ASSERT(zephio_color_eq(m.bg, bg));
    zephio_widget_destroy(&m.base);
}

TEST_BEGIN(ctx_menu_set_on_select)
{
    g_ctx_selected = -1;
    ZephioContextMenu m;
    zephio_context_menu_init(&m);
    zephio_context_menu_set_on_select(&m, stub_ctx_on_select, NULL);
    TEST_ASSERT(m.on_select == stub_ctx_on_select);
    zephio_widget_destroy(&m.base);
}

/* ── MenuBar ────────────────────────────────────────────────────── */

TEST_BEGIN(menubar_init)
{
    ZephioMenuBar mb;
    ZephioResult res = zephio_menubar_init(&mb, 0, 0, 80);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(mb.base.width, 80);
    TEST_EQ(mb.menu_count, 0);
    TEST_EQ(mb.is_open, 0);
    zephio_widget_destroy(&mb.base);
}

TEST_BEGIN(menubar_add_menu)
{
    ZephioMenuBar mb;
    zephio_menubar_init(&mb, 0, 0, 80);
    int idx = zephio_menubar_add_menu(&mb, "File", 'F');
    TEST_EQ(idx, 0);
    TEST_EQ(mb.menu_count, 1);
    TEST_STR_EQ(mb.menus[0].label, "File");
    TEST_EQ(mb.menus[0].mnemonic, 'F');

    zephio_menubar_add_menu(&mb, "Edit", 'E');
    TEST_EQ(mb.menu_count, 2);
    zephio_widget_destroy(&mb.base);
}

TEST_BEGIN(menubar_add_menu_item)
{
    ZephioMenuBar mb;
    zephio_menubar_init(&mb, 0, 0, 80);
    int mi = zephio_menubar_add_menu(&mb, "File", 'F');
    zephio_menubar_add_menu_item(&mb, mi, "New");
    zephio_menubar_add_menu_item(&mb, mi, "Open");
    zephio_menubar_add_menu_separator(&mb, mi);
    zephio_menubar_add_menu_item(&mb, mi, "Exit");
    TEST_EQ(mb.menus[0].item_count, 4);
    TEST_EQ(mb.menus[0].items[2].is_separator, 1);
    zephio_widget_destroy(&mb.base);
}

TEST_BEGIN(menubar_set_colors)
{
    ZephioMenuBar mb;
    zephio_menubar_init(&mb, 0, 0, 80);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(15);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(4);
    zephio_menubar_set_colors(&mb, fg, bg, fg, bg);
    TEST_ASSERT(zephio_color_eq(mb.fg, fg));
    zephio_widget_destroy(&mb.base);
}

TEST_BEGIN(menubar_set_popup_colors)
{
    ZephioMenuBar mb;
    zephio_menubar_init(&mb, 0, 0, 80);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(7);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    zephio_menubar_set_popup_colors(&mb, fg, bg, fg, bg);
    TEST_ASSERT(zephio_color_eq(mb.fg_popup, fg));
    zephio_widget_destroy(&mb.base);
}

TEST_BEGIN(menubar_set_on_select)
{
    ZephioMenuBar mb;
    zephio_menubar_init(&mb, 0, 0, 80);
    zephio_menubar_set_on_select(&mb, NULL, NULL);
    TEST_ASSERT(mb.on_select == NULL);
    zephio_widget_destroy(&mb.base);
}

/* ── TabBar ─────────────────────────────────────────────────────── */

TEST_BEGIN(tabbar_init)
{
    ZephioTabBar tb;
    ZephioResult res = zephio_tabbar_init(&tb, 0, 0, 60);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(tb.base.width, 60);
    TEST_EQ(tb.tab_count, 0);
    TEST_EQ(tb.active_tab, 0);
    zephio_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_init_null)
{
    TEST_NE(zephio_tabbar_init(NULL, 0, 0, 60), ZEPHIO_OK);
}

TEST_BEGIN(tabbar_add_tab)
{
    ZephioTabBar tb;
    zephio_tabbar_init(&tb, 0, 0, 60);
    int idx = zephio_tabbar_add_tab(&tb, "Tab 1", NULL);
    TEST_EQ(idx, 0);
    zephio_tabbar_add_tab(&tb, "Tab 2", NULL);
    zephio_tabbar_add_tab(&tb, "Tab 3", NULL);
    TEST_EQ(tb.tab_count, 3);
    TEST_STR_EQ(tb.tabs[0].label, "Tab 1");
    zephio_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_remove_tab)
{
    ZephioTabBar tb;
    zephio_tabbar_init(&tb, 0, 0, 60);
    zephio_tabbar_add_tab(&tb, "A", NULL);
    zephio_tabbar_add_tab(&tb, "B", NULL);
    zephio_tabbar_add_tab(&tb, "C", NULL);
    zephio_tabbar_remove_tab(&tb, 1);
    TEST_EQ(tb.tab_count, 2);
    TEST_STR_EQ(tb.tabs[0].label, "A");
    TEST_STR_EQ(tb.tabs[1].label, "C");
    zephio_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_get_active)
{
    ZephioTabBar tb;
    zephio_tabbar_init(&tb, 0, 0, 60);
    zephio_tabbar_add_tab(&tb, "A", NULL);
    zephio_tabbar_add_tab(&tb, "B", NULL);
    TEST_EQ(zephio_tabbar_get_active(&tb), 0);
    zephio_tabbar_set_active(&tb, 1);
    TEST_EQ(zephio_tabbar_get_active(&tb), 1);
    zephio_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_set_tab_label)
{
    ZephioTabBar tb;
    zephio_tabbar_init(&tb, 0, 0, 60);
    zephio_tabbar_add_tab(&tb, "Old", NULL);
    zephio_tabbar_set_tab_label(&tb, 0, "New");
    TEST_STR_EQ(tb.tabs[0].label, "New");
    zephio_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_set_tab_content)
{
    ZephioTabBar tb;
    zephio_tabbar_init(&tb, 0, 0, 60);
    zephio_tabbar_add_tab(&tb, "Tab", NULL);
    ZephioWidget content;
    zephio_widget_init(&content, 0, 0, 40, 20, NULL, NULL);
    zephio_tabbar_set_tab_content(&tb, 0, &content);
    TEST_ASSERT(tb.tabs[0].content == &content);
    zephio_widget_destroy(&tb.base);
    zephio_widget_destroy(&content);
}

TEST_BEGIN(tabbar_get_content)
{
    ZephioTabBar tb;
    zephio_tabbar_init(&tb, 0, 0, 60);
    TEST_ASSERT(zephio_tabbar_get_content(&tb, 0) == NULL);
    zephio_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_set_colors)
{
    ZephioTabBar tb;
    zephio_tabbar_init(&tb, 0, 0, 60);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(15);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(4);
    zephio_tabbar_set_colors(&tb, fg, bg, fg, bg);
    TEST_ASSERT(zephio_color_eq(tb.fg, fg));
    zephio_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_set_hover_colors)
{
    ZephioTabBar tb;
    zephio_tabbar_init(&tb, 0, 0, 60);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(0);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(14);
    zephio_tabbar_set_hover_colors(&tb, fg, bg);
    TEST_ASSERT(zephio_color_eq(tb.fg_hover, fg));
    zephio_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_on_change)
{
    g_tab_changed = -1;
    ZephioTabBar tb;
    zephio_tabbar_init(&tb, 0, 0, 60);
    zephio_tabbar_add_tab(&tb, "A", NULL);
    zephio_tabbar_add_tab(&tb, "B", NULL);
    zephio_tabbar_set_on_change(&tb, stub_tab_on_change, NULL);
    zephio_tabbar_set_active(&tb, 1);
    TEST_EQ(g_tab_changed, 1);
    zephio_widget_destroy(&tb.base);
}

/* ── Table ──────────────────────────────────────────────────────── */

TEST_BEGIN(table_init)
{
    ZephioTable t;
    ZephioResult res = zephio_table_init(&t, 0, 0, 60, 15);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(t.col_count, 0);
    TEST_EQ(t.row_count, 0);
    TEST_EQ(t.selected, 0);
    TEST_EQ(t.base.focusable, 1);
    zephio_widget_destroy(&t.base);
}

TEST_BEGIN(table_init_null)
{
    TEST_NE(zephio_table_init(NULL, 0, 0, 60, 15), ZEPHIO_OK);
}

TEST_BEGIN(table_add_column)
{
    ZephioTable t;
    zephio_table_init(&t, 0, 0, 60, 15);
    zephio_table_add_column(&t, "Name", 20);
    zephio_table_add_column(&t, "Age", 10);
    zephio_table_add_column(&t, "City", 20);
    TEST_EQ(t.col_count, 3);
    TEST_STR_EQ(t.columns[0].label, "Name");
    TEST_EQ(t.columns[0].width, 20);
    zephio_widget_destroy(&t.base);
}

TEST_BEGIN(table_add_row)
{
    ZephioTable t;
    zephio_table_init(&t, 0, 0, 60, 15);
    zephio_table_add_column(&t, "A", 10);
    zephio_table_add_column(&t, "B", 10);
    const char *cells[] = {"X", "Y"};
    zephio_table_add_row(&t, cells, 2);
    const char *cells2[] = {"P", "Q"};
    zephio_table_add_row(&t, cells2, 2);
    TEST_EQ(t.row_count, 2);
    zephio_widget_destroy(&t.base);
}

TEST_BEGIN(table_remove_row)
{
    ZephioTable t;
    zephio_table_init(&t, 0, 0, 60, 15);
    zephio_table_add_column(&t, "A", 10);
    const char *c1[] = {"X"};
    const char *c2[] = {"Y"};
    const char *c3[] = {"Z"};
    zephio_table_add_row(&t, c1, 1);
    zephio_table_add_row(&t, c2, 1);
    zephio_table_add_row(&t, c3, 1);
    zephio_table_remove_row(&t, 1);
    TEST_EQ(t.row_count, 2);
    zephio_widget_destroy(&t.base);
}

TEST_BEGIN(table_clear_rows)
{
    ZephioTable t;
    zephio_table_init(&t, 0, 0, 60, 15);
    zephio_table_add_column(&t, "A", 10);
    const char *c[] = {"X"};
    zephio_table_add_row(&t, c, 1);
    zephio_table_add_row(&t, c, 1);
    zephio_table_clear_rows(&t);
    TEST_EQ(t.row_count, 0);
    zephio_widget_destroy(&t.base);
}

TEST_BEGIN(table_get_selected)
{
    ZephioTable t;
    zephio_table_init(&t, 0, 0, 60, 15);
    TEST_EQ(zephio_table_get_selected(&t), 0);
    zephio_widget_destroy(&t.base);
}

TEST_BEGIN(table_set_colors)
{
    ZephioTable t;
    zephio_table_init(&t, 0, 0, 60, 15);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(15);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    zephio_table_set_colors(&t, fg, bg, fg, bg, fg, bg);
    TEST_ASSERT(zephio_color_eq(t.fg, fg));
    zephio_widget_destroy(&t.base);
}

TEST_BEGIN(table_set_on_select)
{
    g_table_selected = -1;
    ZephioTable t;
    zephio_table_init(&t, 0, 0, 60, 15);
    zephio_table_set_on_select(&t, stub_table_on_select, NULL);
    TEST_ASSERT(t.on_select == stub_table_on_select);
    zephio_widget_destroy(&t.base);
}

TEST_BEGIN(table_sort_by)
{
    ZephioTable t;
    zephio_table_init(&t, 0, 0, 60, 15);
    zephio_table_add_column(&t, "Name", 20);
    zephio_table_sort_by(&t, 0, ZEPHIO_SORT_ASC);
    TEST_EQ(t.sort_col, 0);
    zephio_widget_destroy(&t.base);
}

/* ── TextView ───────────────────────────────────────────────────── */

TEST_BEGIN(text_view_init)
{
    ZephioTextView tv;
    ZephioResult res = zephio_text_view_init(&tv, 0, 0, 40, 10);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(tv.text, NULL);
    TEST_EQ(tv.line_count, 0);
    TEST_EQ(tv.word_wrap, 1);
    zephio_widget_destroy((ZephioWidget *)&tv);
}

TEST_BEGIN(text_view_init_null)
{
    TEST_NE(zephio_text_view_init(NULL, 0, 0, 40, 10), ZEPHIO_OK);
}

TEST_BEGIN(text_view_set_text)
{
    ZephioTextView tv;
    zephio_text_view_init(&tv, 0, 0, 40, 10);
    zephio_text_view_set_text(&tv, "Hello world");
    TEST_ASSERT(tv.text != NULL);
    TEST_STR_EQ(tv.text, "Hello world");
    TEST_ASSERT(tv.line_count >= 1);

    zephio_text_view_set_text(&tv, NULL);
    TEST_ASSERT(tv.text == NULL);
    zephio_widget_destroy((ZephioWidget *)&tv);
}

TEST_BEGIN(text_view_set_colors)
{
    ZephioTextView tv;
    zephio_text_view_init(&tv, 0, 0, 40, 10);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(15);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    zephio_text_view_set_colors(&tv, fg, bg);
    TEST_ASSERT(zephio_color_eq(tv.fg, fg));
    TEST_ASSERT(zephio_color_eq(tv.bg, bg));
    zephio_widget_destroy((ZephioWidget *)&tv);
}

TEST_BEGIN(text_view_set_attr)
{
    ZephioTextView tv;
    zephio_text_view_init(&tv, 0, 0, 40, 10);
    zephio_text_view_set_attr(&tv, ZEPHIO_ATTR_BOLD);
    TEST_EQ(tv.attr, ZEPHIO_ATTR_BOLD);
    zephio_widget_destroy((ZephioWidget *)&tv);
}

TEST_BEGIN(text_view_set_word_wrap)
{
    ZephioTextView tv;
    zephio_text_view_init(&tv, 0, 0, 40, 10);
    TEST_EQ(tv.word_wrap, 1);
    zephio_text_view_set_word_wrap(&tv, 0);
    TEST_EQ(tv.word_wrap, 0);
    zephio_widget_destroy((ZephioWidget *)&tv);
}

TEST_BEGIN(text_view_get_line_count)
{
    ZephioTextView tv;
    zephio_text_view_init(&tv, 0, 0, 40, 10);
    TEST_EQ(zephio_text_view_get_line_count(&tv), 0);
    zephio_text_view_set_text(&tv, "Line 1\nLine 2\nLine 3");
    TEST_EQ(zephio_text_view_get_line_count(&tv), 3);
    zephio_widget_destroy((ZephioWidget *)&tv);
}

TEST_BEGIN(text_view_multiline_wrap)
{
    ZephioTextView tv;
    zephio_text_view_init(&tv, 0, 0, 10, 5);
    zephio_text_view_set_text(&tv, "This is a long line that should wrap around");
    TEST_ASSERT(tv.line_count > 1);
    zephio_widget_destroy((ZephioWidget *)&tv);
}

/* ── TextArea ───────────────────────────────────────────────────── */

TEST_BEGIN(textarea_init)
{
    ZephioTextArea ta;
    ZephioResult res = zephio_textarea_init(&ta, 0, 0, 40, 10);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(ta.line_count, 1);
    TEST_EQ(ta.cursor_row, 0);
    TEST_EQ(ta.cursor_col, 0);
    TEST_EQ(ta.base.focusable, 1);
    zephio_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_init_null)
{
    TEST_NE(zephio_textarea_init(NULL, 0, 0, 40, 10), ZEPHIO_OK);
}

TEST_BEGIN(textarea_set_text)
{
    ZephioTextArea ta;
    zephio_textarea_init(&ta, 0, 0, 40, 10);
    zephio_textarea_set_text(&ta, "Hello\nWorld");
    TEST_EQ(ta.line_count, 2);
    TEST_EQ(ta.cursor_row, 0);
    TEST_EQ(ta.cursor_col, 0);
    zephio_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_set_text_null)
{
    ZephioTextArea ta;
    zephio_textarea_init(&ta, 0, 0, 40, 10);
    zephio_textarea_set_text(&ta, NULL);
    TEST_EQ(ta.line_count, 1);
    zephio_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_get_text)
{
    ZephioTextArea ta;
    zephio_textarea_init(&ta, 0, 0, 40, 10);
    zephio_textarea_set_text(&ta, "Hello");
    char *out = zephio_textarea_get_text(&ta);
    TEST_ASSERT(out != NULL);
    TEST_STR_EQ(out, "Hello");
    free(out);
    zephio_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_get_text_null)
{
    char *out = zephio_textarea_get_text(NULL);
    TEST_ASSERT(out != NULL);
    free(out);
}

TEST_BEGIN(textarea_set_colors)
{
    ZephioTextArea ta;
    zephio_textarea_init(&ta, 0, 0, 40, 10);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(15);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    zephio_textarea_set_colors(&ta, fg, bg, ZEPHIO_ATTR_BOLD);
    TEST_ASSERT(zephio_color_eq(ta.fg, fg));
    TEST_ASSERT(zephio_color_eq(ta.bg, bg));
    TEST_EQ(ta.attr, ZEPHIO_ATTR_BOLD);
    zephio_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_typing)
{
    ZephioTextArea ta;
    zephio_textarea_init(&ta, 0, 0, 40, 10);
    ZephioEvent e = {0};
    e.key = ZEPHIO_KEY_UNKNOWN;
    e.codepoint = 'A';
    zephio_widget_handle_input(&ta.base, &e);
    e.codepoint = 'B';
    zephio_widget_handle_input(&ta.base, &e);
    char *out = zephio_textarea_get_text(&ta);
    TEST_STR_EQ(out, "AB");
    free(out);
    zephio_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_enter_splits_line)
{
    ZephioTextArea ta;
    zephio_textarea_init(&ta, 0, 0, 40, 10);
    zephio_textarea_set_text(&ta, "HelloWorld");
    ta.cursor_col = 5;
    ZephioEvent e = {0};
    e.key = ZEPHIO_KEY_ENTER;
    zephio_widget_handle_input(&ta.base, &e);
    TEST_EQ(ta.line_count, 2);
    TEST_EQ(ta.cursor_row, 1);
    TEST_EQ(ta.cursor_col, 0);
    zephio_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_backspace)
{
    ZephioTextArea ta;
    zephio_textarea_init(&ta, 0, 0, 40, 10);
    zephio_textarea_set_text(&ta, "ABC");
    ta.cursor_col = 3;
    ZephioEvent e = {0};
    e.key = ZEPHIO_KEY_BACKSPACE;
    zephio_widget_handle_input(&ta.base, &e);
    char *out = zephio_textarea_get_text(&ta);
    TEST_STR_EQ(out, "AB");
    free(out);
    zephio_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_cursor_movement)
{
    ZephioTextArea ta;
    zephio_textarea_init(&ta, 0, 0, 40, 10);
    zephio_textarea_set_text(&ta, "ABCDE");
    ta.cursor_col = 5;

    ZephioEvent left = {0};
    left.key = ZEPHIO_KEY_LEFT;
    zephio_widget_handle_input(&ta.base, &left);
    TEST_EQ(ta.cursor_col, 4);

    ZephioEvent right = {0};
    right.key = ZEPHIO_KEY_RIGHT;
    zephio_widget_handle_input(&ta.base, &right);
    TEST_EQ(ta.cursor_col, 5);
    zephio_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_get_line_count)
{
    ZephioTextArea ta;
    zephio_textarea_init(&ta, 0, 0, 40, 10);
    TEST_EQ(zephio_textarea_get_line_count(&ta), 1);
    zephio_textarea_set_text(&ta, "A\nB\nC");
    TEST_EQ(zephio_textarea_get_line_count(&ta), 3);
    zephio_widget_destroy(&ta.base);
}

/* ── ToastManager ───────────────────────────────────────────────── */

TEST_BEGIN(toast_manager_init)
{
    ZephioToastManager mgr;
    ZephioResult res = zephio_toast_manager_init(&mgr);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(mgr.count, 0);
    TEST_EQ(mgr.next_id, 1);
    zephio_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_manager_init_null)
{
    TEST_NE(zephio_toast_manager_init(NULL), ZEPHIO_OK);
}

TEST_BEGIN(toast_show)
{
    ZephioToastManager mgr;
    zephio_toast_manager_init(&mgr);
    int id = zephio_toast_show(&mgr, ZEPHIO_TOAST_INFO, "Hello", 0);
    TEST_ASSERT(id >= 0);
    TEST_EQ(mgr.count, 1);
    TEST_STR_EQ(mgr.toasts[0].message, "Hello");
    TEST_EQ(mgr.toasts[0].severity, ZEPHIO_TOAST_INFO);
    zephio_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_show_null)
{
    TEST_EQ(zephio_toast_show(NULL, ZEPHIO_TOAST_INFO, "X", 0), -1);
}

TEST_BEGIN(toast_show_default_duration)
{
    ZephioToastManager mgr;
    zephio_toast_manager_init(&mgr);
    zephio_toast_show(&mgr, ZEPHIO_TOAST_SUCCESS, "OK", 0);
    TEST_EQ((int)mgr.toasts[0].duration_ms, ZEPHIO_TOAST_DEFAULT_MS);
    zephio_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_show_custom_duration)
{
    ZephioToastManager mgr;
    zephio_toast_manager_init(&mgr);
    zephio_toast_show(&mgr, ZEPHIO_TOAST_WARNING, "Warn", 5000);
    TEST_EQ((int)mgr.toasts[0].duration_ms, 5000);
    zephio_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_severity)
{
    ZephioToastManager mgr;
    zephio_toast_manager_init(&mgr);
    zephio_toast_show(&mgr, ZEPHIO_TOAST_ERROR, "Err", 1000);
    TEST_EQ(mgr.toasts[0].severity, ZEPHIO_TOAST_ERROR);
    zephio_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_dismiss)
{
    ZephioToastManager mgr;
    zephio_toast_manager_init(&mgr);
    int id = zephio_toast_show(&mgr, ZEPHIO_TOAST_INFO, "Hi", 1000);
    zephio_toast_dismiss(&mgr, id);
    TEST_EQ(mgr.toasts[0].state, ZEPHIO_TOAST_FADE_OUT);
    zephio_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_dismiss_all)
{
    ZephioToastManager mgr;
    zephio_toast_manager_init(&mgr);
    zephio_toast_show(&mgr, ZEPHIO_TOAST_INFO, "A", 1000);
    zephio_toast_show(&mgr, ZEPHIO_TOAST_INFO, "B", 1000);
    zephio_toast_dismiss_all(&mgr);
    zephio_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_update_auto_dismiss)
{
    ZephioToastManager mgr;
    zephio_toast_manager_init(&mgr);
    zephio_toast_show(&mgr, ZEPHIO_TOAST_INFO, "Hi", 100);
    mgr.toasts[0].state = ZEPHIO_TOAST_VISIBLE;

    zephio_toast_update(&mgr, 50);
    TEST_EQ(mgr.toasts[0].state, ZEPHIO_TOAST_VISIBLE);

    zephio_toast_update(&mgr, 60);
    TEST_EQ(mgr.toasts[0].state, ZEPHIO_TOAST_FADE_OUT);
    zephio_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_has_active)
{
    ZephioToastManager mgr;
    zephio_toast_manager_init(&mgr);
    TEST_EQ(zephio_toast_has_active(&mgr), 0);
    zephio_toast_show(&mgr, ZEPHIO_TOAST_INFO, "Hi", 1000);
    TEST_EQ(zephio_toast_has_active(&mgr), 1);
    zephio_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_max_count)
{
    ZephioToastManager mgr;
    zephio_toast_manager_init(&mgr);
    for (int i = 0; i < ZEPHIO_TOAST_MAX_COUNT; i++) {
        int id = zephio_toast_show(&mgr, ZEPHIO_TOAST_INFO, "X", 1000);
        TEST_ASSERT(id >= 0);
    }
    TEST_EQ(mgr.count, ZEPHIO_TOAST_MAX_COUNT);
    zephio_toast_show(&mgr, ZEPHIO_TOAST_INFO, "Overflow", 1000);
    TEST_EQ(mgr.count, ZEPHIO_TOAST_MAX_COUNT);
    zephio_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_show_cb)
{
    ZephioToastManager mgr;
    zephio_toast_manager_init(&mgr);
    int id = zephio_toast_show_cb(&mgr, ZEPHIO_TOAST_INFO, "CB", 1000, NULL, NULL);
    TEST_ASSERT(id >= 0);
    zephio_toast_manager_free(&mgr);
}

/* ── TreeView ───────────────────────────────────────────────────── */

TEST_BEGIN(tree_view_init)
{
    ZephioTreeView tv;
    ZephioResult res = zephio_tree_view_init(&tv, 0, 0, 30, 15);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(tv.base.focusable, 1);
    TEST_EQ(tv.selected, 0);
    TEST_EQ(tv.visible_count, 0);
    zephio_widget_destroy(&tv.base);
}

TEST_BEGIN(tree_view_init_null)
{
    TEST_NE(zephio_tree_view_init(NULL, 0, 0, 30, 15), ZEPHIO_OK);
}

TEST_BEGIN(tree_node_create)
{
    ZephioTreeNode *node = zephio_tree_node_create("Root", NULL);
    TEST_ASSERT(node != NULL);
    TEST_STR_EQ(node->text, "Root");
    TEST_EQ(node->child_count, 0);
    TEST_EQ(node->expanded, 0);
    zephio_tree_node_destroy(node);
}

TEST_BEGIN(tree_node_create_null_text)
{
    ZephioTreeNode *node = zephio_tree_node_create(NULL, NULL);
    TEST_ASSERT(node != NULL);
    zephio_tree_node_destroy(node);
}

TEST_BEGIN(tree_node_add_child)
{
    ZephioTreeNode *root = zephio_tree_node_create("Root", NULL);
    ZephioTreeNode *c1 = zephio_tree_node_create("Child1", NULL);
    ZephioTreeNode *c2 = zephio_tree_node_create("Child2", NULL);
    zephio_tree_node_add_child(root, c1);
    zephio_tree_node_add_child(root, c2);
    TEST_EQ(root->child_count, 2);
    TEST_ASSERT(c1->parent == root);
    TEST_ASSERT(c2->parent == root);
    zephio_tree_node_destroy(root);
}

TEST_BEGIN(tree_view_set_root)
{
    ZephioTreeView tv;
    zephio_tree_view_init(&tv, 0, 0, 30, 15);
    ZephioTreeNode *root = zephio_tree_node_create("Root", NULL);
    zephio_tree_node_add_child(root, zephio_tree_node_create("A", NULL));
    zephio_tree_node_add_child(root, zephio_tree_node_create("B", NULL));
    zephio_tree_view_set_root(&tv, root);
    TEST_ASSERT(tv.root == root);
    zephio_widget_destroy(&tv.base);
}

TEST_BEGIN(tree_view_expand_collapse)
{
    ZephioTreeView tv;
    zephio_tree_view_init(&tv, 0, 0, 30, 15);
    ZephioTreeNode *root = zephio_tree_node_create("Root", NULL);
    ZephioTreeNode *c1 = zephio_tree_node_create("A", NULL);
    zephio_tree_node_add_child(root, c1);
    zephio_tree_view_set_root(&tv, root);

    TEST_EQ(root->expanded, 0);
    zephio_tree_view_expand(&tv, root);
    TEST_EQ(root->expanded, 1);
    zephio_tree_view_collapse(&tv, root);
    TEST_EQ(root->expanded, 0);
    zephio_tree_view_toggle(&tv, root);
    TEST_EQ(root->expanded, 1);
    zephio_tree_view_toggle(&tv, root);
    TEST_EQ(root->expanded, 0);
    zephio_widget_destroy(&tv.base);
}

TEST_BEGIN(tree_view_expand_all)
{
    ZephioTreeView tv;
    zephio_tree_view_init(&tv, 0, 0, 30, 15);
    ZephioTreeNode *root = zephio_tree_node_create("Root", NULL);
    ZephioTreeNode *c1 = zephio_tree_node_create("A", NULL);
    ZephioTreeNode *c2 = zephio_tree_node_create("B", NULL);
    zephio_tree_node_add_child(c1, c2);
    zephio_tree_node_add_child(root, c1);
    zephio_tree_view_set_root(&tv, root);
    zephio_tree_view_expand_all(&tv);
    TEST_EQ(root->expanded, 1);
    TEST_EQ(c1->expanded, 1);
    zephio_tree_view_collapse_all(&tv);
    TEST_EQ(root->expanded, 0);
    TEST_EQ(c1->expanded, 0);
    zephio_widget_destroy(&tv.base);
}

TEST_BEGIN(tree_view_get_selected)
{
    ZephioTreeView tv;
    zephio_tree_view_init(&tv, 0, 0, 30, 15);
    TEST_EQ(zephio_tree_view_get_selected(&tv), 0);
    TEST_ASSERT(zephio_tree_view_get_selected_node(&tv) == NULL);
    zephio_widget_destroy(&tv.base);
}

TEST_BEGIN(tree_view_set_colors)
{
    ZephioTreeView tv;
    zephio_tree_view_init(&tv, 0, 0, 30, 15);
    ZephioColor fg = ZEPHIO_COLOR_INDEX(15);
    ZephioColor bg = ZEPHIO_COLOR_INDEX(0);
    zephio_tree_view_set_colors(&tv, fg, bg, fg, bg, fg);
    TEST_ASSERT(zephio_color_eq(tv.fg, fg));
    zephio_widget_destroy(&tv.base);
}

TEST_BEGIN(tree_view_set_on_select)
{
    ZephioTreeView tv;
    zephio_tree_view_init(&tv, 0, 0, 30, 15);
    zephio_tree_view_set_on_select(&tv, NULL, NULL);
    TEST_ASSERT(tv.on_select == NULL);
    zephio_widget_destroy(&tv.base);
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void)
{
    fprintf(stderr, "Running widget-2 tests...\n\n");

    TEST_RUN(container_init);
    TEST_RUN(container_init_null);
    TEST_RUN(container_set_bg);
    TEST_RUN(container_set_bg_null);
    TEST_RUN(container_render);

    TEST_RUN(separator_init_h);
    TEST_RUN(separator_init_v);
    TEST_RUN(separator_init_null);
    TEST_RUN(separator_set_colors);
    TEST_RUN(separator_set_attr);
    TEST_RUN(separator_render_h);
    TEST_RUN(separator_render_v);

    TEST_RUN(statusbar_init);
    TEST_RUN(statusbar_init_null);
    TEST_RUN(statusbar_set_text);
    TEST_RUN(statusbar_set_text_null);
    TEST_RUN(statusbar_set_message);
    TEST_RUN(statusbar_set_message_null);
    TEST_RUN(statusbar_tick);
    TEST_RUN(statusbar_set_colors);
    TEST_RUN(statusbar_set_message_colors);

    TEST_RUN(scroll_container_init);
    TEST_RUN(scroll_container_init_null);
    TEST_RUN(scroll_container_set_content_size);
    TEST_RUN(scroll_container_scroll_to);
    TEST_RUN(scroll_container_set_scroll_xy);
    TEST_RUN(scroll_container_clamp_scroll);
    TEST_RUN(scroll_container_clamp_negative);
    TEST_RUN(scroll_container_auto_content_size);
    TEST_RUN(scroll_container_input_arrow_down);
    TEST_RUN(scroll_container_input_page_down);
    TEST_RUN(scroll_container_input_home);

    TEST_RUN(split_pane_init_h);
    TEST_RUN(split_pane_init_v);
    TEST_RUN(split_pane_init_null);
    TEST_RUN(split_pane_set_position);
    TEST_RUN(split_pane_set_position_null);
    TEST_RUN(split_pane_set_min_sizes);
    TEST_RUN(split_pane_set_separator_style);
    TEST_RUN(split_pane_set_panes);
    TEST_RUN(split_pane_get_pane_rects_h);
    TEST_RUN(split_pane_get_pane_rects_v);

    TEST_RUN(dialog_init);
    TEST_RUN(dialog_init_null_widget);
    TEST_RUN(dialog_init_null_strings);
    TEST_RUN(dialog_add_button);
    TEST_RUN(dialog_add_button_max);
    TEST_RUN(dialog_add_button_null);
    TEST_RUN(dialog_set_on_button);
    TEST_RUN(dialog_input_tab);
    TEST_RUN(dialog_input_arrows);
    TEST_RUN(dialog_get_selected);
    TEST_RUN(dialog_center);

    TEST_RUN(dropdown_init);
    TEST_RUN(dropdown_init_null);
    TEST_RUN(dropdown_add_item);
    TEST_RUN(dropdown_clear);
    TEST_RUN(dropdown_get_selected);
    TEST_RUN(dropdown_set_colors);
    TEST_RUN(dropdown_set_popup_colors);
    TEST_RUN(dropdown_set_max_visible);

    TEST_RUN(ctx_menu_init);
    TEST_RUN(ctx_menu_add_item);
    TEST_RUN(ctx_menu_clear);
    TEST_RUN(ctx_menu_set_colors);
    TEST_RUN(ctx_menu_set_on_select);

    TEST_RUN(menubar_init);
    TEST_RUN(menubar_add_menu);
    TEST_RUN(menubar_add_menu_item);
    TEST_RUN(menubar_set_colors);
    TEST_RUN(menubar_set_popup_colors);
    TEST_RUN(menubar_set_on_select);

    TEST_RUN(tabbar_init);
    TEST_RUN(tabbar_init_null);
    TEST_RUN(tabbar_add_tab);
    TEST_RUN(tabbar_remove_tab);
    TEST_RUN(tabbar_get_active);
    TEST_RUN(tabbar_set_tab_label);
    TEST_RUN(tabbar_set_tab_content);
    TEST_RUN(tabbar_get_content);
    TEST_RUN(tabbar_set_colors);
    TEST_RUN(tabbar_set_hover_colors);
    TEST_RUN(tabbar_on_change);

    TEST_RUN(table_init);
    TEST_RUN(table_init_null);
    TEST_RUN(table_add_column);
    TEST_RUN(table_add_row);
    TEST_RUN(table_remove_row);
    TEST_RUN(table_clear_rows);
    TEST_RUN(table_get_selected);
    TEST_RUN(table_set_colors);
    TEST_RUN(table_set_on_select);
    TEST_RUN(table_sort_by);

    TEST_RUN(text_view_init);
    TEST_RUN(text_view_init_null);
    TEST_RUN(text_view_set_text);
    TEST_RUN(text_view_set_colors);
    TEST_RUN(text_view_set_attr);
    TEST_RUN(text_view_set_word_wrap);
    TEST_RUN(text_view_get_line_count);
    TEST_RUN(text_view_multiline_wrap);

    TEST_RUN(textarea_init);
    TEST_RUN(textarea_init_null);
    TEST_RUN(textarea_set_text);
    TEST_RUN(textarea_set_text_null);
    TEST_RUN(textarea_get_text);
    TEST_RUN(textarea_get_text_null);
    TEST_RUN(textarea_set_colors);
    TEST_RUN(textarea_typing);
    TEST_RUN(textarea_enter_splits_line);
    TEST_RUN(textarea_backspace);
    TEST_RUN(textarea_cursor_movement);
    TEST_RUN(textarea_get_line_count);

    TEST_RUN(toast_manager_init);
    TEST_RUN(toast_manager_init_null);
    TEST_RUN(toast_show);
    TEST_RUN(toast_show_null);
    TEST_RUN(toast_show_default_duration);
    TEST_RUN(toast_show_custom_duration);
    TEST_RUN(toast_severity);
    TEST_RUN(toast_dismiss);
    TEST_RUN(toast_dismiss_all);
    TEST_RUN(toast_update_auto_dismiss);
    TEST_RUN(toast_has_active);
    TEST_RUN(toast_max_count);
    TEST_RUN(toast_show_cb);

    TEST_RUN(tree_view_init);
    TEST_RUN(tree_view_init_null);
    TEST_RUN(tree_node_create);
    TEST_RUN(tree_node_create_null_text);
    TEST_RUN(tree_node_add_child);
    TEST_RUN(tree_view_set_root);
    TEST_RUN(tree_view_expand_collapse);
    TEST_RUN(tree_view_expand_all);
    TEST_RUN(tree_view_get_selected);
    TEST_RUN(tree_view_set_colors);
    TEST_RUN(tree_view_set_on_select);

    TEST_SUMMARY();
}
