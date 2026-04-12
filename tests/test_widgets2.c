#include "util.h"
#include "tui_container.h"
#include "tui_separator.h"
#include "tui_statusbar.h"
#include "tui_scroll_container.h"
#include "tui_split_pane.h"
#include "tui_dialog.h"
#include "tui_dropdown.h"
#include "tui_context_menu.h"
#include "tui_menubar.h"
#include "tui_tabbar.h"
#include "tui_table.h"
#include "tui_text_view.h"
#include "tui_textarea.h"
#include "tui_toast.h"
#include "tui_tree_view.h"
#include "tui_screen.h"

#include <string.h>
#include <stdlib.h>

static int g_tab_changed = 0;
static int g_table_selected = -1;
static int g_ctx_selected = -1;
static int g_dialog_btn = -1;

static void stub_tab_on_change(TuiTabBar *tb, int idx, const char *lbl, void *ud) {
    (void)tb; (void)lbl; (void)ud; g_tab_changed = idx;
}
static void stub_table_on_select(TuiWidget *w, int row, void *ud) {
    (void)w; (void)ud; g_table_selected = row;
}
static void stub_ctx_on_select(TuiContextMenu *m, int idx, const char *lbl, void *ud) {
    (void)m; (void)lbl; (void)ud; g_ctx_selected = idx;
}
static void stub_dialog_on_button(TuiDialog *d, int btn, void *ud) {
    (void)d; (void)ud; g_dialog_btn = btn;
}

/* ── Container ──────────────────────────────────────────────────── */

TEST_BEGIN(container_init)
{
    TuiContainer c;
    TuiResult res = tui_container_init(&c, 2, 3, 40, 10);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(c.base.x, 2);
    TEST_EQ(c.base.y, 3);
    TEST_EQ(c.base.width, 40);
    TEST_EQ(c.base.height, 10);
    TEST_EQ(c.base.focusable, 0);
    tui_widget_destroy(&c.base);
}

TEST_BEGIN(container_init_null)
{
    TEST_NE(tui_container_init(NULL, 0, 0, 10, 5), TUI_OK);
}

TEST_BEGIN(container_set_bg)
{
    TuiContainer c;
    tui_container_init(&c, 0, 0, 20, 5);
    TuiColor bg = TUI_COLOR_INDEX(4);
    tui_container_set_bg(&c, bg);
    TEST_ASSERT(tui_color_eq(c.bg, bg));
    TEST_EQ(c.base.dirty, 1);
    tui_widget_destroy(&c.base);
}

TEST_BEGIN(container_set_bg_null)
{
    tui_container_set_bg(NULL, TUI_COLOR_INDEX(0));
}

TEST_BEGIN(container_render)
{
    tui_screen_init(24, 80);
    TuiContainer c;
    tui_container_init(&c, 0, 0, 5, 3);
    c.base.dirty = 1;
    tui_widget_render(&c.base);
    TEST_EQ(c.base.dirty, 0);
    tui_widget_destroy(&c.base);
    tui_screen_free();
}

/* ── Separator ──────────────────────────────────────────────────── */

TEST_BEGIN(separator_init_h)
{
    TuiSeparator s;
    TuiResult res = tui_separator_init_h(&s, 0, 5, 30);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(s.horizontal, 1);
    TEST_EQ(s.base.width, 30);
    TEST_EQ(s.base.height, 1);
    TEST_EQ(s.base.focusable, 0);
    tui_widget_destroy(&s.base);
}

TEST_BEGIN(separator_init_v)
{
    TuiSeparator s;
    TuiResult res = tui_separator_init_v(&s, 10, 0, 15);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(s.horizontal, 0);
    TEST_EQ(s.base.width, 1);
    TEST_EQ(s.base.height, 15);
    tui_widget_destroy(&s.base);
}

TEST_BEGIN(separator_init_null)
{
    TEST_NE(tui_separator_init_h(NULL, 0, 0, 10), TUI_OK);
    TEST_NE(tui_separator_init_v(NULL, 0, 0, 10), TUI_OK);
}

TEST_BEGIN(separator_set_colors)
{
    TuiSeparator s;
    tui_separator_init_h(&s, 0, 0, 20);
    TuiColor fg = TUI_COLOR_INDEX(14);
    TuiColor bg = TUI_COLOR_INDEX(0);
    tui_separator_set_colors(&s, fg, bg);
    TEST_ASSERT(tui_color_eq(s.fg, fg));
    TEST_ASSERT(tui_color_eq(s.bg, bg));
    tui_widget_destroy(&s.base);
}

TEST_BEGIN(separator_set_attr)
{
    TuiSeparator s;
    tui_separator_init_h(&s, 0, 0, 20);
    tui_separator_set_attr(&s, TUI_ATTR_BOLD);
    TEST_EQ(s.attr, TUI_ATTR_BOLD);
    tui_widget_destroy(&s.base);
}

TEST_BEGIN(separator_render_h)
{
    tui_screen_init(24, 80);
    TuiSeparator s;
    tui_separator_init_h(&s, 0, 5, 10);
    s.base.dirty = 1;
    tui_widget_render(&s.base);
    TEST_EQ(s.base.dirty, 0);
    TEST_ASSERT(memcmp(g_screen.back[5 * 80].ch, "\xe2\x94\x80", 3) == 0);
    tui_widget_destroy(&s.base);
    tui_screen_free();
}

TEST_BEGIN(separator_render_v)
{
    tui_screen_init(24, 80);
    TuiSeparator s;
    tui_separator_init_v(&s, 10, 0, 3);
    s.base.dirty = 1;
    tui_widget_render(&s.base);
    TEST_EQ(s.base.dirty, 0);
    TEST_ASSERT(memcmp(g_screen.back[0 * 80 + 10].ch, "\xe2\x94\x82", 3) == 0);
    TEST_ASSERT(memcmp(g_screen.back[1 * 80 + 10].ch, "\xe2\x94\x82", 3) == 0);
    tui_widget_destroy(&s.base);
    tui_screen_free();
}

/* ── StatusBar ──────────────────────────────────────────────────── */

TEST_BEGIN(statusbar_init)
{
    TuiStatusBar sb;
    TuiResult res = tui_statusbar_init(&sb, 0, 23, 80);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(sb.base.width, 80);
    TEST_EQ(sb.base.height, 1);
    TEST_EQ(sb.base.focusable, 0);
    TEST_ASSERT(sb.text_left == NULL);
    TEST_ASSERT(sb.text_center == NULL);
    TEST_ASSERT(sb.text_right == NULL);
    TEST_ASSERT(sb.message == NULL);
    TEST_EQ(sb.message_ticks, 0);
    tui_widget_destroy(&sb.base);
}

TEST_BEGIN(statusbar_init_null)
{
    TEST_NE(tui_statusbar_init(NULL, 0, 0, 80), TUI_OK);
}

TEST_BEGIN(statusbar_set_text)
{
    TuiStatusBar sb;
    tui_statusbar_init(&sb, 0, 0, 80);
    tui_statusbar_set_text(&sb, "Left", "Center", "Right");
    TEST_ASSERT(sb.text_left != NULL);
    TEST_STR_EQ(sb.text_left, "Left");
    TEST_ASSERT(sb.text_center != NULL);
    TEST_STR_EQ(sb.text_center, "Center");
    TEST_ASSERT(sb.text_right != NULL);
    TEST_STR_EQ(sb.text_right, "Right");
    TEST_EQ(sb.base.dirty, 1);
    tui_widget_destroy(&sb.base);
}

TEST_BEGIN(statusbar_set_text_null)
{
    TuiStatusBar sb;
    tui_statusbar_init(&sb, 0, 0, 80);
    tui_statusbar_set_text(&sb, NULL, NULL, NULL);
    TEST_ASSERT(sb.text_left == NULL);
    TEST_ASSERT(sb.text_center == NULL);
    TEST_ASSERT(sb.text_right == NULL);
    tui_widget_destroy(&sb.base);
}

TEST_BEGIN(statusbar_set_message)
{
    TuiStatusBar sb;
    tui_statusbar_init(&sb, 0, 0, 80);
    tui_statusbar_set_message(&sb, "Saving...", 100);
    TEST_ASSERT(sb.message != NULL);
    TEST_STR_EQ(sb.message, "Saving...");
    TEST_EQ(sb.message_ticks, 100);
    tui_widget_destroy(&sb.base);
}

TEST_BEGIN(statusbar_set_message_null)
{
    TuiStatusBar sb;
    tui_statusbar_init(&sb, 0, 0, 80);
    tui_statusbar_set_message(&sb, NULL, 0);
    TEST_ASSERT(sb.message == NULL);
    tui_widget_destroy(&sb.base);
}

TEST_BEGIN(statusbar_tick)
{
    TuiStatusBar sb;
    tui_statusbar_init(&sb, 0, 0, 80);
    tui_statusbar_set_message(&sb, "Hi", 2);
    TEST_EQ(sb.message_ticks, 2);

    tui_statusbar_tick(&sb);
    TEST_EQ(sb.message_ticks, 1);
    TEST_ASSERT(sb.message != NULL);

    tui_statusbar_tick(&sb);
    TEST_EQ(sb.message_ticks, 0);
    TEST_ASSERT(sb.message == NULL);
    tui_widget_destroy(&sb.base);
}

TEST_BEGIN(statusbar_set_colors)
{
    TuiStatusBar sb;
    tui_statusbar_init(&sb, 0, 0, 80);
    TuiColor fg = TUI_COLOR_INDEX(15);
    TuiColor bg = TUI_COLOR_INDEX(4);
    tui_statusbar_set_colors(&sb, fg, bg);
    TEST_ASSERT(tui_color_eq(sb.fg, fg));
    TEST_ASSERT(tui_color_eq(sb.bg, bg));
    tui_widget_destroy(&sb.base);
}

TEST_BEGIN(statusbar_set_message_colors)
{
    TuiStatusBar sb;
    tui_statusbar_init(&sb, 0, 0, 80);
    TuiColor fg = TUI_COLOR_INDEX(0);
    TuiColor bg = TUI_COLOR_INDEX(11);
    tui_statusbar_set_message_colors(&sb, fg, bg);
    TEST_ASSERT(tui_color_eq(sb.fg_message, fg));
    TEST_ASSERT(tui_color_eq(sb.bg_message, bg));
    tui_widget_destroy(&sb.base);
}

/* ── ScrollContainer ────────────────────────────────────────────── */

TEST_BEGIN(scroll_container_init)
{
    TuiScrollContainer sc;
    TuiResult res = tui_scroll_container_init(&sc, 0, 0, 40, 20);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(sc.base.width, 40);
    TEST_EQ(sc.base.height, 20);
    TEST_EQ(sc.scroll_x, 0);
    TEST_EQ(sc.scroll_y, 0);
    TEST_EQ(sc.content_width, 40);
    TEST_EQ(sc.content_height, 20);
    TEST_EQ(sc.base.focusable, 1);
    TEST_EQ(sc.base.manages_children, 1);
    tui_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_init_null)
{
    TEST_NE(tui_scroll_container_init(NULL, 0, 0, 40, 20), TUI_OK);
}

TEST_BEGIN(scroll_container_set_content_size)
{
    TuiScrollContainer sc;
    tui_scroll_container_init(&sc, 0, 0, 40, 20);
    tui_scroll_container_set_content_size(&sc, 100, 50);
    TEST_EQ(sc.content_width, 100);
    TEST_EQ(sc.content_height, 50);
    tui_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_scroll_to)
{
    TuiScrollContainer sc;
    tui_scroll_container_init(&sc, 0, 0, 40, 20);
    tui_scroll_container_set_content_size(&sc, 100, 100);
    tui_scroll_container_scroll_to(&sc, 10, 20);
    TEST_EQ(sc.scroll_x, 10);
    TEST_EQ(sc.scroll_y, 20);
    tui_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_set_scroll_xy)
{
    TuiScrollContainer sc;
    tui_scroll_container_init(&sc, 0, 0, 40, 20);
    tui_scroll_container_set_scroll_x(&sc, 5);
    tui_scroll_container_set_scroll_y(&sc, 10);
    TEST_EQ(sc.scroll_x, 5);
    TEST_EQ(sc.scroll_y, 10);
    tui_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_clamp_scroll)
{
    TuiScrollContainer sc;
    tui_scroll_container_init(&sc, 0, 0, 40, 20);
    tui_scroll_container_set_content_size(&sc, 60, 50);
    sc.scroll_y = 100;
    sc.scroll_x = 100;
    tui_scroll_container_clamp_scroll(&sc, 39, 19);
    TEST_EQ(sc.scroll_y, 31);
    TEST_EQ(sc.scroll_x, 21);
    tui_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_clamp_negative)
{
    TuiScrollContainer sc;
    tui_scroll_container_init(&sc, 0, 0, 40, 20);
    sc.scroll_y = -5;
    sc.scroll_x = -3;
    tui_scroll_container_clamp_scroll(&sc, 40, 20);
    TEST_EQ(sc.scroll_y, 0);
    TEST_EQ(sc.scroll_x, 0);
    tui_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_auto_content_size)
{
    TuiScrollContainer sc;
    tui_scroll_container_init(&sc, 0, 0, 40, 20);
    TuiWidget c1, c2;
    tui_widget_init(&c1, 0, 0, 60, 10, NULL, NULL);
    tui_widget_init(&c2, 10, 10, 30, 40, NULL, NULL);
    tui_widget_add_child(&sc.base, &c1);
    tui_widget_add_child(&sc.base, &c2);
    tui_scroll_container_auto_content_size(&sc);
    TEST_EQ(sc.content_width, 60);
    TEST_EQ(sc.content_height, 50);
    tui_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_input_arrow_down)
{
    TuiScrollContainer sc;
    tui_scroll_container_init(&sc, 0, 0, 40, 20);
    tui_scroll_container_set_content_size(&sc, 40, 50);
    TuiEvent e = {0};
    e.key = TUI_KEY_DOWN;
    int handled = tui_scroll_container_handle_input(&sc.base, &e);
    TEST_EQ(handled, 1);
    TEST_EQ(sc.scroll_y, 1);
    tui_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_input_page_down)
{
    TuiScrollContainer sc;
    tui_scroll_container_init(&sc, 0, 0, 40, 20);
    tui_scroll_container_set_content_size(&sc, 40, 100);
    TuiEvent e = {0};
    e.key = TUI_KEY_PAGE_DOWN;
    tui_scroll_container_handle_input(&sc.base, &e);
    TEST_ASSERT(sc.scroll_y > 0);
    tui_widget_destroy(&sc.base);
}

TEST_BEGIN(scroll_container_input_home)
{
    TuiScrollContainer sc;
    tui_scroll_container_init(&sc, 0, 0, 40, 20);
    tui_scroll_container_set_content_size(&sc, 40, 100);
    sc.scroll_y = 10;
    sc.scroll_x = 5;
    TuiEvent e = {0};
    e.key = TUI_KEY_HOME;
    tui_scroll_container_handle_input(&sc.base, &e);
    TEST_EQ(sc.scroll_y, 0);
    TEST_EQ(sc.scroll_x, 0);
    tui_widget_destroy(&sc.base);
}

/* ── SplitPane ──────────────────────────────────────────────────── */

TEST_BEGIN(split_pane_init_h)
{
    TuiSplitPane sp;
    TuiResult res = tui_split_pane_init(&sp, 0, 0, 80, 24, TUI_SPLIT_HORIZONTAL);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(sp.orientation, TUI_SPLIT_HORIZONTAL);
    TEST_EQ(sp.base.width, 80);
    TEST_EQ(sp.base.height, 24);
    TEST_EQ(sp.separator_size, 1);
    TEST_EQ(sp.split_pos, 40);
    TEST_EQ(sp.base.manages_children, 1);
    tui_widget_destroy(&sp.base);
}

TEST_BEGIN(split_pane_init_v)
{
    TuiSplitPane sp;
    TuiResult res = tui_split_pane_init(&sp, 0, 0, 80, 24, TUI_SPLIT_VERTICAL);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(sp.orientation, TUI_SPLIT_VERTICAL);
    TEST_EQ(sp.split_pos, 12);
    tui_widget_destroy(&sp.base);
}

TEST_BEGIN(split_pane_init_null)
{
    TEST_NE(tui_split_pane_init(NULL, 0, 0, 80, 24, TUI_SPLIT_HORIZONTAL), TUI_OK);
}

TEST_BEGIN(split_pane_set_position)
{
    TuiSplitPane sp;
    tui_split_pane_init(&sp, 0, 0, 80, 24, TUI_SPLIT_HORIZONTAL);
    tui_split_pane_set_position(&sp, 30);
    TEST_EQ(tui_split_pane_get_position(&sp), 30);
    tui_split_pane_set_position(&sp, 5);
    TEST_ASSERT(tui_split_pane_get_position(&sp) >= sp.min_pane1);
    tui_widget_destroy(&sp.base);
}

TEST_BEGIN(split_pane_set_position_null)
{
    tui_split_pane_set_position(NULL, 10);
    TEST_EQ(tui_split_pane_get_position(NULL), 0);
}

TEST_BEGIN(split_pane_set_min_sizes)
{
    TuiSplitPane sp;
    tui_split_pane_init(&sp, 0, 0, 80, 24, TUI_SPLIT_HORIZONTAL);
    tui_split_pane_set_min_sizes(&sp, 10, 15);
    TEST_EQ(sp.min_pane1, 10);
    TEST_EQ(sp.min_pane2, 15);
    tui_widget_destroy(&sp.base);
}

TEST_BEGIN(split_pane_set_separator_style)
{
    TuiSplitPane sp;
    tui_split_pane_init(&sp, 0, 0, 80, 24, TUI_SPLIT_HORIZONTAL);
    TuiColor fg = TUI_COLOR_INDEX(14);
    TuiColor bg = TUI_COLOR_INDEX(0);
    tui_split_pane_set_separator_style(&sp, fg, bg, TUI_ATTR_BOLD);
    TEST_ASSERT(tui_color_eq(sp.sep_fg, fg));
    TEST_ASSERT(tui_color_eq(sp.sep_bg, bg));
    TEST_EQ(sp.sep_attr, TUI_ATTR_BOLD);
    tui_widget_destroy(&sp.base);
}

TEST_BEGIN(split_pane_set_panes)
{
    TuiSplitPane sp;
    tui_split_pane_init(&sp, 0, 0, 80, 24, TUI_SPLIT_HORIZONTAL);
    TuiWidget p1, p2;
    tui_widget_init(&p1, 0, 0, 40, 24, NULL, NULL);
    tui_widget_init(&p2, 0, 0, 40, 24, NULL, NULL);
    tui_split_pane_set_panes(&sp, &p1, &p2);
    TEST_EQ(sp.base.child_count, 2);
    TEST_EQ(p1.parent, &sp.base);
    TEST_EQ(p2.parent, &sp.base);
    tui_widget_destroy(&sp.base);
}

TEST_BEGIN(split_pane_get_pane_rects_h)
{
    TuiSplitPane sp;
    tui_split_pane_init(&sp, 5, 2, 80, 24, TUI_SPLIT_HORIZONTAL);
    tui_split_pane_set_position(&sp, 30);
    int x1, y1, w1, h1;
    int x2, y2, w2, h2;
    tui_split_pane_get_pane1_rect(&sp, &x1, &y1, &w1, &h1);
    tui_split_pane_get_pane2_rect(&sp, &x2, &y2, &w2, &h2);
    TEST_EQ(w1, 30);
    TEST_EQ(h1, 24);
    TEST_EQ(w2, 49);
    TEST_EQ(h2, 24);
    TEST_EQ(w1 + 1 + w2, 80);
    tui_widget_destroy(&sp.base);
}

TEST_BEGIN(split_pane_get_pane_rects_v)
{
    TuiSplitPane sp;
    tui_split_pane_init(&sp, 0, 0, 80, 24, TUI_SPLIT_VERTICAL);
    tui_split_pane_set_position(&sp, 10);
    int x1, y1, w1, h1;
    int x2, y2, w2, h2;
    tui_split_pane_get_pane1_rect(&sp, &x1, &y1, &w1, &h1);
    tui_split_pane_get_pane2_rect(&sp, &x2, &y2, &w2, &h2);
    TEST_EQ(h1, 10);
    TEST_EQ(w1, 80);
    TEST_EQ(h2, 13);
    TEST_EQ(h1 + 1 + h2, 24);
    tui_widget_destroy(&sp.base);
}

/* ── Dialog ─────────────────────────────────────────────────────── */

TEST_BEGIN(dialog_init)
{
    TuiDialog d;
    TuiResult res = tui_dialog_init(&d, "Title", "Message body");
    TEST_EQ(res, TUI_OK);
    TEST_EQ(d.base.focusable, 1);
    TEST_ASSERT(d.title != NULL);
    TEST_STR_EQ(d.title, "Title");
    TEST_ASSERT(d.message != NULL);
    TEST_STR_EQ(d.message, "Message body");
    TEST_EQ(d.button_count, 0);
    tui_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_init_null_widget)
{
    TEST_NE(tui_dialog_init(NULL, "T", "M"), TUI_OK);
}

TEST_BEGIN(dialog_init_null_strings)
{
    TuiDialog d;
    TuiResult res = tui_dialog_init(&d, NULL, NULL);
    TEST_EQ(res, TUI_OK);
    TEST_ASSERT(d.title == NULL);
    TEST_ASSERT(d.message == NULL);
    tui_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_add_button)
{
    TuiDialog d;
    tui_dialog_init(&d, NULL, NULL);
    int idx1 = tui_dialog_add_button(&d, "OK");
    int idx2 = tui_dialog_add_button(&d, "Cancel");
    TEST_EQ(idx1, 0);
    TEST_EQ(idx2, 1);
    TEST_EQ(d.button_count, 2);
    TEST_STR_EQ(d.button_labels[0], "OK");
    TEST_STR_EQ(d.button_labels[1], "Cancel");
    tui_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_add_button_max)
{
    TuiDialog d;
    tui_dialog_init(&d, NULL, NULL);
    tui_dialog_add_button(&d, "A");
    tui_dialog_add_button(&d, "B");
    tui_dialog_add_button(&d, "C");
    tui_dialog_add_button(&d, "D");
    int idx = tui_dialog_add_button(&d, "E");
    TEST_EQ(idx, -1);
    TEST_EQ(d.button_count, 4);
    tui_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_add_button_null)
{
    TEST_EQ(tui_dialog_add_button(NULL, "X"), -1);
    TuiDialog d;
    tui_dialog_init(&d, NULL, NULL);
    TEST_EQ(tui_dialog_add_button(&d, NULL), -1);
    tui_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_set_on_button)
{
    g_dialog_btn = -1;
    TuiDialog d;
    tui_dialog_init(&d, NULL, NULL);
    tui_dialog_add_button(&d, "OK");
    tui_dialog_set_on_button(&d, stub_dialog_on_button, NULL);

    TuiEvent e = {0};
    e.key = TUI_KEY_ENTER;
    tui_widget_handle_input(&d.base, &e);
    TEST_EQ(g_dialog_btn, 0);
    tui_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_input_tab)
{
    TuiDialog d;
    tui_dialog_init(&d, NULL, NULL);
    tui_dialog_add_button(&d, "A");
    tui_dialog_add_button(&d, "B");
    tui_dialog_add_button(&d, "C");

    TuiEvent e = {0};
    e.key = TUI_KEY_TAB;
    tui_widget_handle_input(&d.base, &e);
    TEST_EQ(d.selected_button, 1);
    tui_widget_handle_input(&d.base, &e);
    TEST_EQ(d.selected_button, 2);
    tui_widget_handle_input(&d.base, &e);
    TEST_EQ(d.selected_button, 0);
    tui_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_input_arrows)
{
    TuiDialog d;
    tui_dialog_init(&d, NULL, NULL);
    tui_dialog_add_button(&d, "A");
    tui_dialog_add_button(&d, "B");

    TuiEvent e = {0};
    e.key = TUI_KEY_LEFT;
    tui_widget_handle_input(&d.base, &e);
    TEST_EQ(d.selected_button, 1);

    e.key = TUI_KEY_RIGHT;
    tui_widget_handle_input(&d.base, &e);
    TEST_EQ(d.selected_button, 0);
    tui_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_get_selected)
{
    TuiDialog d;
    tui_dialog_init(&d, NULL, NULL);
    TEST_EQ(tui_dialog_get_selected(&d), 0);
    TEST_EQ(tui_dialog_get_selected(NULL), -1);
    tui_widget_destroy(&d.base);
}

TEST_BEGIN(dialog_center)
{
    tui_screen_init(24, 80);
    TuiDialog d;
    tui_dialog_init(&d, "Test", "Msg");
    tui_dialog_center(&d);
    TEST_ASSERT(d.base.x > 0);
    TEST_ASSERT(d.base.y > 0);
    tui_widget_destroy(&d.base);
    tui_screen_free();
}

/* ── Dropdown ───────────────────────────────────────────────────── */

TEST_BEGIN(dropdown_init)
{
    TuiDropdown dd;
    TuiResult res = tui_dropdown_init(&dd, 0, 0, 20);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(dd.base.width, 20);
    TEST_EQ(dd.base.focusable, 1);
    TEST_EQ(dd.item_count, 0);
    TEST_EQ(dd.selected, -1);
    TEST_EQ(dd.is_open, 0);
    tui_widget_destroy(&dd.base);
}

TEST_BEGIN(dropdown_init_null)
{
    TEST_NE(tui_dropdown_init(NULL, 0, 0, 20), TUI_OK);
}

TEST_BEGIN(dropdown_add_item)
{
    TuiDropdown dd;
    tui_dropdown_init(&dd, 0, 0, 20);
    tui_dropdown_add_item(&dd, "Option A");
    tui_dropdown_add_item(&dd, "Option B");
    tui_dropdown_add_item(&dd, "Option C");
    TEST_EQ(dd.item_count, 3);
    TEST_STR_EQ(dd.items[0], "Option A");
    tui_widget_destroy(&dd.base);
}

TEST_BEGIN(dropdown_clear)
{
    TuiDropdown dd;
    tui_dropdown_init(&dd, 0, 0, 20);
    tui_dropdown_add_item(&dd, "A");
    tui_dropdown_add_item(&dd, "B");
    tui_dropdown_clear(&dd);
    TEST_EQ(dd.item_count, 0);
    tui_widget_destroy(&dd.base);
}

TEST_BEGIN(dropdown_get_selected)
{
    TuiDropdown dd;
    tui_dropdown_init(&dd, 0, 0, 20);
    tui_dropdown_add_item(&dd, "A");
    tui_dropdown_add_item(&dd, "B");
    TEST_EQ(tui_dropdown_get_selected(&dd), 0);
    TEST_STR_EQ(tui_dropdown_get_selected_item(&dd), "A");
    tui_dropdown_set_selected(&dd, 1);
    TEST_EQ(tui_dropdown_get_selected(&dd), 1);
    tui_widget_destroy(&dd.base);
}

TEST_BEGIN(dropdown_set_colors)
{
    TuiDropdown dd;
    tui_dropdown_init(&dd, 0, 0, 20);
    TuiColor fg = TUI_COLOR_INDEX(15);
    TuiColor bg = TUI_COLOR_INDEX(0);
    tui_dropdown_set_colors(&dd, fg, bg, fg, bg);
    TEST_ASSERT(tui_color_eq(dd.fg, fg));
    TEST_ASSERT(tui_color_eq(dd.bg, bg));
    tui_widget_destroy(&dd.base);
}

TEST_BEGIN(dropdown_set_popup_colors)
{
    TuiDropdown dd;
    tui_dropdown_init(&dd, 0, 0, 20);
    TuiColor fg = TUI_COLOR_INDEX(7);
    TuiColor bg = TUI_COLOR_INDEX(0);
    tui_dropdown_set_popup_colors(&dd, fg, bg, fg, bg);
    TEST_ASSERT(tui_color_eq(dd.fg_popup, fg));
    tui_widget_destroy(&dd.base);
}

TEST_BEGIN(dropdown_set_max_visible)
{
    TuiDropdown dd;
    tui_dropdown_init(&dd, 0, 0, 20);
    tui_dropdown_set_max_visible(&dd, 5);
    TEST_EQ(dd.max_visible, 5);
    tui_widget_destroy(&dd.base);
}

/* ── ContextMenu ────────────────────────────────────────────────── */

TEST_BEGIN(ctx_menu_init)
{
    TuiContextMenu m;
    TuiResult res = tui_context_menu_init(&m);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(m.item_count, 0);
    TEST_EQ(m.highlighted, -1);
    TEST_EQ(m.is_visible, 0);
    tui_widget_destroy(&m.base);
}

TEST_BEGIN(ctx_menu_add_item)
{
    TuiContextMenu m;
    tui_context_menu_init(&m);
    tui_context_menu_add_item(&m, "Copy");
    tui_context_menu_add_item(&m, "Paste");
    tui_context_menu_add_separator(&m);
    tui_context_menu_add_item(&m, "Delete");
    TEST_EQ(m.item_count, 4);
    TEST_STR_EQ(m.items[0].label, "Copy");
    TEST_EQ(m.items[2].is_separator, 1);
    tui_widget_destroy(&m.base);
}

TEST_BEGIN(ctx_menu_clear)
{
    TuiContextMenu m;
    tui_context_menu_init(&m);
    tui_context_menu_add_item(&m, "A");
    tui_context_menu_add_item(&m, "B");
    tui_context_menu_clear(&m);
    TEST_EQ(m.item_count, 0);
    tui_widget_destroy(&m.base);
}

TEST_BEGIN(ctx_menu_set_colors)
{
    TuiContextMenu m;
    tui_context_menu_init(&m);
    TuiColor fg = TUI_COLOR_INDEX(15);
    TuiColor bg = TUI_COLOR_INDEX(0);
    tui_context_menu_set_colors(&m, fg, bg, fg, bg);
    TEST_ASSERT(tui_color_eq(m.fg, fg));
    TEST_ASSERT(tui_color_eq(m.bg, bg));
    tui_widget_destroy(&m.base);
}

TEST_BEGIN(ctx_menu_set_on_select)
{
    g_ctx_selected = -1;
    TuiContextMenu m;
    tui_context_menu_init(&m);
    tui_context_menu_set_on_select(&m, stub_ctx_on_select, NULL);
    TEST_ASSERT(m.on_select == stub_ctx_on_select);
    tui_widget_destroy(&m.base);
}

/* ── MenuBar ────────────────────────────────────────────────────── */

TEST_BEGIN(menubar_init)
{
    TuiMenuBar mb;
    TuiResult res = tui_menubar_init(&mb, 0, 0, 80);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(mb.base.width, 80);
    TEST_EQ(mb.menu_count, 0);
    TEST_EQ(mb.is_open, 0);
    tui_widget_destroy(&mb.base);
}

TEST_BEGIN(menubar_add_menu)
{
    TuiMenuBar mb;
    tui_menubar_init(&mb, 0, 0, 80);
    int idx = tui_menubar_add_menu(&mb, "File", 'F');
    TEST_EQ(idx, 0);
    TEST_EQ(mb.menu_count, 1);
    TEST_STR_EQ(mb.menus[0].label, "File");
    TEST_EQ(mb.menus[0].mnemonic, 'F');

    tui_menubar_add_menu(&mb, "Edit", 'E');
    TEST_EQ(mb.menu_count, 2);
    tui_widget_destroy(&mb.base);
}

TEST_BEGIN(menubar_add_menu_item)
{
    TuiMenuBar mb;
    tui_menubar_init(&mb, 0, 0, 80);
    int mi = tui_menubar_add_menu(&mb, "File", 'F');
    tui_menubar_add_menu_item(&mb, mi, "New");
    tui_menubar_add_menu_item(&mb, mi, "Open");
    tui_menubar_add_menu_separator(&mb, mi);
    tui_menubar_add_menu_item(&mb, mi, "Exit");
    TEST_EQ(mb.menus[0].item_count, 4);
    TEST_EQ(mb.menus[0].items[2].is_separator, 1);
    tui_widget_destroy(&mb.base);
}

TEST_BEGIN(menubar_set_colors)
{
    TuiMenuBar mb;
    tui_menubar_init(&mb, 0, 0, 80);
    TuiColor fg = TUI_COLOR_INDEX(15);
    TuiColor bg = TUI_COLOR_INDEX(4);
    tui_menubar_set_colors(&mb, fg, bg, fg, bg);
    TEST_ASSERT(tui_color_eq(mb.fg, fg));
    tui_widget_destroy(&mb.base);
}

TEST_BEGIN(menubar_set_popup_colors)
{
    TuiMenuBar mb;
    tui_menubar_init(&mb, 0, 0, 80);
    TuiColor fg = TUI_COLOR_INDEX(7);
    TuiColor bg = TUI_COLOR_INDEX(0);
    tui_menubar_set_popup_colors(&mb, fg, bg, fg, bg);
    TEST_ASSERT(tui_color_eq(mb.fg_popup, fg));
    tui_widget_destroy(&mb.base);
}

TEST_BEGIN(menubar_set_on_select)
{
    TuiMenuBar mb;
    tui_menubar_init(&mb, 0, 0, 80);
    tui_menubar_set_on_select(&mb, NULL, NULL);
    TEST_ASSERT(mb.on_select == NULL);
    tui_widget_destroy(&mb.base);
}

/* ── TabBar ─────────────────────────────────────────────────────── */

TEST_BEGIN(tabbar_init)
{
    TuiTabBar tb;
    TuiResult res = tui_tabbar_init(&tb, 0, 0, 60);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(tb.base.width, 60);
    TEST_EQ(tb.tab_count, 0);
    TEST_EQ(tb.active_tab, 0);
    tui_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_init_null)
{
    TEST_NE(tui_tabbar_init(NULL, 0, 0, 60), TUI_OK);
}

TEST_BEGIN(tabbar_add_tab)
{
    TuiTabBar tb;
    tui_tabbar_init(&tb, 0, 0, 60);
    int idx = tui_tabbar_add_tab(&tb, "Tab 1", NULL);
    TEST_EQ(idx, 0);
    tui_tabbar_add_tab(&tb, "Tab 2", NULL);
    tui_tabbar_add_tab(&tb, "Tab 3", NULL);
    TEST_EQ(tb.tab_count, 3);
    TEST_STR_EQ(tb.tabs[0].label, "Tab 1");
    tui_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_remove_tab)
{
    TuiTabBar tb;
    tui_tabbar_init(&tb, 0, 0, 60);
    tui_tabbar_add_tab(&tb, "A", NULL);
    tui_tabbar_add_tab(&tb, "B", NULL);
    tui_tabbar_add_tab(&tb, "C", NULL);
    tui_tabbar_remove_tab(&tb, 1);
    TEST_EQ(tb.tab_count, 2);
    TEST_STR_EQ(tb.tabs[0].label, "A");
    TEST_STR_EQ(tb.tabs[1].label, "C");
    tui_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_get_active)
{
    TuiTabBar tb;
    tui_tabbar_init(&tb, 0, 0, 60);
    tui_tabbar_add_tab(&tb, "A", NULL);
    tui_tabbar_add_tab(&tb, "B", NULL);
    TEST_EQ(tui_tabbar_get_active(&tb), 0);
    tui_tabbar_set_active(&tb, 1);
    TEST_EQ(tui_tabbar_get_active(&tb), 1);
    tui_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_set_tab_label)
{
    TuiTabBar tb;
    tui_tabbar_init(&tb, 0, 0, 60);
    tui_tabbar_add_tab(&tb, "Old", NULL);
    tui_tabbar_set_tab_label(&tb, 0, "New");
    TEST_STR_EQ(tb.tabs[0].label, "New");
    tui_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_set_tab_content)
{
    TuiTabBar tb;
    tui_tabbar_init(&tb, 0, 0, 60);
    tui_tabbar_add_tab(&tb, "Tab", NULL);
    TuiWidget content;
    tui_widget_init(&content, 0, 0, 40, 20, NULL, NULL);
    tui_tabbar_set_tab_content(&tb, 0, &content);
    TEST_ASSERT(tb.tabs[0].content == &content);
    tui_widget_destroy(&tb.base);
    tui_widget_destroy(&content);
}

TEST_BEGIN(tabbar_get_content)
{
    TuiTabBar tb;
    tui_tabbar_init(&tb, 0, 0, 60);
    TEST_ASSERT(tui_tabbar_get_content(&tb, 0) == NULL);
    tui_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_set_colors)
{
    TuiTabBar tb;
    tui_tabbar_init(&tb, 0, 0, 60);
    TuiColor fg = TUI_COLOR_INDEX(15);
    TuiColor bg = TUI_COLOR_INDEX(4);
    tui_tabbar_set_colors(&tb, fg, bg, fg, bg);
    TEST_ASSERT(tui_color_eq(tb.fg, fg));
    tui_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_set_hover_colors)
{
    TuiTabBar tb;
    tui_tabbar_init(&tb, 0, 0, 60);
    TuiColor fg = TUI_COLOR_INDEX(0);
    TuiColor bg = TUI_COLOR_INDEX(14);
    tui_tabbar_set_hover_colors(&tb, fg, bg);
    TEST_ASSERT(tui_color_eq(tb.fg_hover, fg));
    tui_widget_destroy(&tb.base);
}

TEST_BEGIN(tabbar_on_change)
{
    g_tab_changed = -1;
    TuiTabBar tb;
    tui_tabbar_init(&tb, 0, 0, 60);
    tui_tabbar_add_tab(&tb, "A", NULL);
    tui_tabbar_add_tab(&tb, "B", NULL);
    tui_tabbar_set_on_change(&tb, stub_tab_on_change, NULL);
    tui_tabbar_set_active(&tb, 1);
    TEST_EQ(g_tab_changed, 1);
    tui_widget_destroy(&tb.base);
}

/* ── Table ──────────────────────────────────────────────────────── */

TEST_BEGIN(table_init)
{
    TuiTable t;
    TuiResult res = tui_table_init(&t, 0, 0, 60, 15);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(t.col_count, 0);
    TEST_EQ(t.row_count, 0);
    TEST_EQ(t.selected, 0);
    TEST_EQ(t.base.focusable, 1);
    tui_widget_destroy(&t.base);
}

TEST_BEGIN(table_init_null)
{
    TEST_NE(tui_table_init(NULL, 0, 0, 60, 15), TUI_OK);
}

TEST_BEGIN(table_add_column)
{
    TuiTable t;
    tui_table_init(&t, 0, 0, 60, 15);
    tui_table_add_column(&t, "Name", 20);
    tui_table_add_column(&t, "Age", 10);
    tui_table_add_column(&t, "City", 20);
    TEST_EQ(t.col_count, 3);
    TEST_STR_EQ(t.columns[0].label, "Name");
    TEST_EQ(t.columns[0].width, 20);
    tui_widget_destroy(&t.base);
}

TEST_BEGIN(table_add_row)
{
    TuiTable t;
    tui_table_init(&t, 0, 0, 60, 15);
    tui_table_add_column(&t, "A", 10);
    tui_table_add_column(&t, "B", 10);
    const char *cells[] = {"X", "Y"};
    tui_table_add_row(&t, cells, 2);
    const char *cells2[] = {"P", "Q"};
    tui_table_add_row(&t, cells2, 2);
    TEST_EQ(t.row_count, 2);
    tui_widget_destroy(&t.base);
}

TEST_BEGIN(table_remove_row)
{
    TuiTable t;
    tui_table_init(&t, 0, 0, 60, 15);
    tui_table_add_column(&t, "A", 10);
    const char *c1[] = {"X"};
    const char *c2[] = {"Y"};
    const char *c3[] = {"Z"};
    tui_table_add_row(&t, c1, 1);
    tui_table_add_row(&t, c2, 1);
    tui_table_add_row(&t, c3, 1);
    tui_table_remove_row(&t, 1);
    TEST_EQ(t.row_count, 2);
    tui_widget_destroy(&t.base);
}

TEST_BEGIN(table_clear_rows)
{
    TuiTable t;
    tui_table_init(&t, 0, 0, 60, 15);
    tui_table_add_column(&t, "A", 10);
    const char *c[] = {"X"};
    tui_table_add_row(&t, c, 1);
    tui_table_add_row(&t, c, 1);
    tui_table_clear_rows(&t);
    TEST_EQ(t.row_count, 0);
    tui_widget_destroy(&t.base);
}

TEST_BEGIN(table_get_selected)
{
    TuiTable t;
    tui_table_init(&t, 0, 0, 60, 15);
    TEST_EQ(tui_table_get_selected(&t), 0);
    tui_widget_destroy(&t.base);
}

TEST_BEGIN(table_set_colors)
{
    TuiTable t;
    tui_table_init(&t, 0, 0, 60, 15);
    TuiColor fg = TUI_COLOR_INDEX(15);
    TuiColor bg = TUI_COLOR_INDEX(0);
    tui_table_set_colors(&t, fg, bg, fg, bg, fg, bg);
    TEST_ASSERT(tui_color_eq(t.fg, fg));
    tui_widget_destroy(&t.base);
}

TEST_BEGIN(table_set_on_select)
{
    g_table_selected = -1;
    TuiTable t;
    tui_table_init(&t, 0, 0, 60, 15);
    tui_table_set_on_select(&t, stub_table_on_select, NULL);
    TEST_ASSERT(t.on_select == stub_table_on_select);
    tui_widget_destroy(&t.base);
}

TEST_BEGIN(table_sort_by)
{
    TuiTable t;
    tui_table_init(&t, 0, 0, 60, 15);
    tui_table_add_column(&t, "Name", 20);
    tui_table_sort_by(&t, 0, TUI_SORT_ASC);
    TEST_EQ(t.sort_col, 0);
    tui_widget_destroy(&t.base);
}

/* ── TextView ───────────────────────────────────────────────────── */

TEST_BEGIN(text_view_init)
{
    TuiTextView tv;
    TuiResult res = tui_text_view_init(&tv, 0, 0, 40, 10);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(tv.text, NULL);
    TEST_EQ(tv.line_count, 0);
    TEST_EQ(tv.word_wrap, 1);
    tui_widget_destroy((TuiWidget *)&tv);
}

TEST_BEGIN(text_view_init_null)
{
    TEST_NE(tui_text_view_init(NULL, 0, 0, 40, 10), TUI_OK);
}

TEST_BEGIN(text_view_set_text)
{
    TuiTextView tv;
    tui_text_view_init(&tv, 0, 0, 40, 10);
    tui_text_view_set_text(&tv, "Hello world");
    TEST_ASSERT(tv.text != NULL);
    TEST_STR_EQ(tv.text, "Hello world");
    TEST_ASSERT(tv.line_count >= 1);

    tui_text_view_set_text(&tv, NULL);
    TEST_ASSERT(tv.text == NULL);
    tui_widget_destroy((TuiWidget *)&tv);
}

TEST_BEGIN(text_view_set_colors)
{
    TuiTextView tv;
    tui_text_view_init(&tv, 0, 0, 40, 10);
    TuiColor fg = TUI_COLOR_INDEX(15);
    TuiColor bg = TUI_COLOR_INDEX(0);
    tui_text_view_set_colors(&tv, fg, bg);
    TEST_ASSERT(tui_color_eq(tv.fg, fg));
    TEST_ASSERT(tui_color_eq(tv.bg, bg));
    tui_widget_destroy((TuiWidget *)&tv);
}

TEST_BEGIN(text_view_set_attr)
{
    TuiTextView tv;
    tui_text_view_init(&tv, 0, 0, 40, 10);
    tui_text_view_set_attr(&tv, TUI_ATTR_BOLD);
    TEST_EQ(tv.attr, TUI_ATTR_BOLD);
    tui_widget_destroy((TuiWidget *)&tv);
}

TEST_BEGIN(text_view_set_word_wrap)
{
    TuiTextView tv;
    tui_text_view_init(&tv, 0, 0, 40, 10);
    TEST_EQ(tv.word_wrap, 1);
    tui_text_view_set_word_wrap(&tv, 0);
    TEST_EQ(tv.word_wrap, 0);
    tui_widget_destroy((TuiWidget *)&tv);
}

TEST_BEGIN(text_view_get_line_count)
{
    TuiTextView tv;
    tui_text_view_init(&tv, 0, 0, 40, 10);
    TEST_EQ(tui_text_view_get_line_count(&tv), 0);
    tui_text_view_set_text(&tv, "Line 1\nLine 2\nLine 3");
    TEST_EQ(tui_text_view_get_line_count(&tv), 3);
    tui_widget_destroy((TuiWidget *)&tv);
}

TEST_BEGIN(text_view_multiline_wrap)
{
    TuiTextView tv;
    tui_text_view_init(&tv, 0, 0, 10, 5);
    tui_text_view_set_text(&tv, "This is a long line that should wrap around");
    TEST_ASSERT(tv.line_count > 1);
    tui_widget_destroy((TuiWidget *)&tv);
}

/* ── TextArea ───────────────────────────────────────────────────── */

TEST_BEGIN(textarea_init)
{
    TuiTextArea ta;
    TuiResult res = tui_textarea_init(&ta, 0, 0, 40, 10);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(ta.line_count, 1);
    TEST_EQ(ta.cursor_row, 0);
    TEST_EQ(ta.cursor_col, 0);
    TEST_EQ(ta.base.focusable, 1);
    tui_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_init_null)
{
    TEST_NE(tui_textarea_init(NULL, 0, 0, 40, 10), TUI_OK);
}

TEST_BEGIN(textarea_set_text)
{
    TuiTextArea ta;
    tui_textarea_init(&ta, 0, 0, 40, 10);
    tui_textarea_set_text(&ta, "Hello\nWorld");
    TEST_EQ(ta.line_count, 2);
    TEST_EQ(ta.cursor_row, 0);
    TEST_EQ(ta.cursor_col, 0);
    tui_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_set_text_null)
{
    TuiTextArea ta;
    tui_textarea_init(&ta, 0, 0, 40, 10);
    tui_textarea_set_text(&ta, NULL);
    TEST_EQ(ta.line_count, 1);
    tui_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_get_text)
{
    TuiTextArea ta;
    tui_textarea_init(&ta, 0, 0, 40, 10);
    tui_textarea_set_text(&ta, "Hello");
    char *out = tui_textarea_get_text(&ta);
    TEST_ASSERT(out != NULL);
    TEST_STR_EQ(out, "Hello");
    free(out);
    tui_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_get_text_null)
{
    char *out = tui_textarea_get_text(NULL);
    TEST_ASSERT(out != NULL);
    free(out);
}

TEST_BEGIN(textarea_set_colors)
{
    TuiTextArea ta;
    tui_textarea_init(&ta, 0, 0, 40, 10);
    TuiColor fg = TUI_COLOR_INDEX(15);
    TuiColor bg = TUI_COLOR_INDEX(0);
    tui_textarea_set_colors(&ta, fg, bg, TUI_ATTR_BOLD);
    TEST_ASSERT(tui_color_eq(ta.fg, fg));
    TEST_ASSERT(tui_color_eq(ta.bg, bg));
    TEST_EQ(ta.attr, TUI_ATTR_BOLD);
    tui_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_typing)
{
    TuiTextArea ta;
    tui_textarea_init(&ta, 0, 0, 40, 10);
    TuiEvent e = {0};
    e.key = TUI_KEY_UNKNOWN;
    e.codepoint = 'A';
    tui_widget_handle_input(&ta.base, &e);
    e.codepoint = 'B';
    tui_widget_handle_input(&ta.base, &e);
    char *out = tui_textarea_get_text(&ta);
    TEST_STR_EQ(out, "AB");
    free(out);
    tui_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_enter_splits_line)
{
    TuiTextArea ta;
    tui_textarea_init(&ta, 0, 0, 40, 10);
    tui_textarea_set_text(&ta, "HelloWorld");
    ta.cursor_col = 5;
    TuiEvent e = {0};
    e.key = TUI_KEY_ENTER;
    tui_widget_handle_input(&ta.base, &e);
    TEST_EQ(ta.line_count, 2);
    TEST_EQ(ta.cursor_row, 1);
    TEST_EQ(ta.cursor_col, 0);
    tui_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_backspace)
{
    TuiTextArea ta;
    tui_textarea_init(&ta, 0, 0, 40, 10);
    tui_textarea_set_text(&ta, "ABC");
    ta.cursor_col = 3;
    TuiEvent e = {0};
    e.key = TUI_KEY_BACKSPACE;
    tui_widget_handle_input(&ta.base, &e);
    char *out = tui_textarea_get_text(&ta);
    TEST_STR_EQ(out, "AB");
    free(out);
    tui_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_cursor_movement)
{
    TuiTextArea ta;
    tui_textarea_init(&ta, 0, 0, 40, 10);
    tui_textarea_set_text(&ta, "ABCDE");
    ta.cursor_col = 5;

    TuiEvent left = {0};
    left.key = TUI_KEY_LEFT;
    tui_widget_handle_input(&ta.base, &left);
    TEST_EQ(ta.cursor_col, 4);

    TuiEvent right = {0};
    right.key = TUI_KEY_RIGHT;
    tui_widget_handle_input(&ta.base, &right);
    TEST_EQ(ta.cursor_col, 5);
    tui_widget_destroy(&ta.base);
}

TEST_BEGIN(textarea_get_line_count)
{
    TuiTextArea ta;
    tui_textarea_init(&ta, 0, 0, 40, 10);
    TEST_EQ(tui_textarea_get_line_count(&ta), 1);
    tui_textarea_set_text(&ta, "A\nB\nC");
    TEST_EQ(tui_textarea_get_line_count(&ta), 3);
    tui_widget_destroy(&ta.base);
}

/* ── ToastManager ───────────────────────────────────────────────── */

TEST_BEGIN(toast_manager_init)
{
    TuiToastManager mgr;
    TuiResult res = tui_toast_manager_init(&mgr);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(mgr.count, 0);
    TEST_EQ(mgr.next_id, 1);
    tui_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_manager_init_null)
{
    TEST_NE(tui_toast_manager_init(NULL), TUI_OK);
}

TEST_BEGIN(toast_show)
{
    TuiToastManager mgr;
    tui_toast_manager_init(&mgr);
    int id = tui_toast_show(&mgr, TUI_TOAST_INFO, "Hello", 0);
    TEST_ASSERT(id >= 0);
    TEST_EQ(mgr.count, 1);
    TEST_STR_EQ(mgr.toasts[0].message, "Hello");
    TEST_EQ(mgr.toasts[0].severity, TUI_TOAST_INFO);
    tui_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_show_null)
{
    TEST_EQ(tui_toast_show(NULL, TUI_TOAST_INFO, "X", 0), -1);
}

TEST_BEGIN(toast_show_default_duration)
{
    TuiToastManager mgr;
    tui_toast_manager_init(&mgr);
    tui_toast_show(&mgr, TUI_TOAST_SUCCESS, "OK", 0);
    TEST_EQ((int)mgr.toasts[0].duration_ms, TUI_TOAST_DEFAULT_MS);
    tui_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_show_custom_duration)
{
    TuiToastManager mgr;
    tui_toast_manager_init(&mgr);
    tui_toast_show(&mgr, TUI_TOAST_WARNING, "Warn", 5000);
    TEST_EQ((int)mgr.toasts[0].duration_ms, 5000);
    tui_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_severity)
{
    TuiToastManager mgr;
    tui_toast_manager_init(&mgr);
    tui_toast_show(&mgr, TUI_TOAST_ERROR, "Err", 1000);
    TEST_EQ(mgr.toasts[0].severity, TUI_TOAST_ERROR);
    tui_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_dismiss)
{
    TuiToastManager mgr;
    tui_toast_manager_init(&mgr);
    int id = tui_toast_show(&mgr, TUI_TOAST_INFO, "Hi", 1000);
    tui_toast_dismiss(&mgr, id);
    TEST_EQ(mgr.toasts[0].state, TUI_TOAST_FADE_OUT);
    tui_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_dismiss_all)
{
    TuiToastManager mgr;
    tui_toast_manager_init(&mgr);
    tui_toast_show(&mgr, TUI_TOAST_INFO, "A", 1000);
    tui_toast_show(&mgr, TUI_TOAST_INFO, "B", 1000);
    tui_toast_dismiss_all(&mgr);
    tui_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_update_auto_dismiss)
{
    TuiToastManager mgr;
    tui_toast_manager_init(&mgr);
    tui_toast_show(&mgr, TUI_TOAST_INFO, "Hi", 100);
    mgr.toasts[0].state = TUI_TOAST_VISIBLE;

    tui_toast_update(&mgr, 50);
    TEST_EQ(mgr.toasts[0].state, TUI_TOAST_VISIBLE);

    tui_toast_update(&mgr, 60);
    TEST_EQ(mgr.toasts[0].state, TUI_TOAST_FADE_OUT);
    tui_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_has_active)
{
    TuiToastManager mgr;
    tui_toast_manager_init(&mgr);
    TEST_EQ(tui_toast_has_active(&mgr), 0);
    tui_toast_show(&mgr, TUI_TOAST_INFO, "Hi", 1000);
    TEST_EQ(tui_toast_has_active(&mgr), 1);
    tui_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_max_count)
{
    TuiToastManager mgr;
    tui_toast_manager_init(&mgr);
    for (int i = 0; i < TUI_TOAST_MAX_COUNT; i++) {
        int id = tui_toast_show(&mgr, TUI_TOAST_INFO, "X", 1000);
        TEST_ASSERT(id >= 0);
    }
    TEST_EQ(mgr.count, TUI_TOAST_MAX_COUNT);
    tui_toast_show(&mgr, TUI_TOAST_INFO, "Overflow", 1000);
    TEST_EQ(mgr.count, TUI_TOAST_MAX_COUNT);
    tui_toast_manager_free(&mgr);
}

TEST_BEGIN(toast_show_cb)
{
    TuiToastManager mgr;
    tui_toast_manager_init(&mgr);
    int id = tui_toast_show_cb(&mgr, TUI_TOAST_INFO, "CB", 1000, NULL, NULL);
    TEST_ASSERT(id >= 0);
    tui_toast_manager_free(&mgr);
}

/* ── TreeView ───────────────────────────────────────────────────── */

TEST_BEGIN(tree_view_init)
{
    TuiTreeView tv;
    TuiResult res = tui_tree_view_init(&tv, 0, 0, 30, 15);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(tv.base.focusable, 1);
    TEST_EQ(tv.selected, 0);
    TEST_EQ(tv.visible_count, 0);
    tui_widget_destroy(&tv.base);
}

TEST_BEGIN(tree_view_init_null)
{
    TEST_NE(tui_tree_view_init(NULL, 0, 0, 30, 15), TUI_OK);
}

TEST_BEGIN(tree_node_create)
{
    TuiTreeNode *node = tui_tree_node_create("Root", NULL);
    TEST_ASSERT(node != NULL);
    TEST_STR_EQ(node->text, "Root");
    TEST_EQ(node->child_count, 0);
    TEST_EQ(node->expanded, 0);
    tui_tree_node_destroy(node);
}

TEST_BEGIN(tree_node_create_null_text)
{
    TuiTreeNode *node = tui_tree_node_create(NULL, NULL);
    TEST_ASSERT(node != NULL);
    tui_tree_node_destroy(node);
}

TEST_BEGIN(tree_node_add_child)
{
    TuiTreeNode *root = tui_tree_node_create("Root", NULL);
    TuiTreeNode *c1 = tui_tree_node_create("Child1", NULL);
    TuiTreeNode *c2 = tui_tree_node_create("Child2", NULL);
    tui_tree_node_add_child(root, c1);
    tui_tree_node_add_child(root, c2);
    TEST_EQ(root->child_count, 2);
    TEST_ASSERT(c1->parent == root);
    TEST_ASSERT(c2->parent == root);
    tui_tree_node_destroy(root);
}

TEST_BEGIN(tree_view_set_root)
{
    TuiTreeView tv;
    tui_tree_view_init(&tv, 0, 0, 30, 15);
    TuiTreeNode *root = tui_tree_node_create("Root", NULL);
    tui_tree_node_add_child(root, tui_tree_node_create("A", NULL));
    tui_tree_node_add_child(root, tui_tree_node_create("B", NULL));
    tui_tree_view_set_root(&tv, root);
    TEST_ASSERT(tv.root == root);
    tui_widget_destroy(&tv.base);
}

TEST_BEGIN(tree_view_expand_collapse)
{
    TuiTreeView tv;
    tui_tree_view_init(&tv, 0, 0, 30, 15);
    TuiTreeNode *root = tui_tree_node_create("Root", NULL);
    TuiTreeNode *c1 = tui_tree_node_create("A", NULL);
    tui_tree_node_add_child(root, c1);
    tui_tree_view_set_root(&tv, root);

    TEST_EQ(root->expanded, 0);
    tui_tree_view_expand(&tv, root);
    TEST_EQ(root->expanded, 1);
    tui_tree_view_collapse(&tv, root);
    TEST_EQ(root->expanded, 0);
    tui_tree_view_toggle(&tv, root);
    TEST_EQ(root->expanded, 1);
    tui_tree_view_toggle(&tv, root);
    TEST_EQ(root->expanded, 0);
    tui_widget_destroy(&tv.base);
}

TEST_BEGIN(tree_view_expand_all)
{
    TuiTreeView tv;
    tui_tree_view_init(&tv, 0, 0, 30, 15);
    TuiTreeNode *root = tui_tree_node_create("Root", NULL);
    TuiTreeNode *c1 = tui_tree_node_create("A", NULL);
    TuiTreeNode *c2 = tui_tree_node_create("B", NULL);
    tui_tree_node_add_child(c1, c2);
    tui_tree_node_add_child(root, c1);
    tui_tree_view_set_root(&tv, root);
    tui_tree_view_expand_all(&tv);
    TEST_EQ(root->expanded, 1);
    TEST_EQ(c1->expanded, 1);
    tui_tree_view_collapse_all(&tv);
    TEST_EQ(root->expanded, 0);
    TEST_EQ(c1->expanded, 0);
    tui_widget_destroy(&tv.base);
}

TEST_BEGIN(tree_view_get_selected)
{
    TuiTreeView tv;
    tui_tree_view_init(&tv, 0, 0, 30, 15);
    TEST_EQ(tui_tree_view_get_selected(&tv), 0);
    TEST_ASSERT(tui_tree_view_get_selected_node(&tv) == NULL);
    tui_widget_destroy(&tv.base);
}

TEST_BEGIN(tree_view_set_colors)
{
    TuiTreeView tv;
    tui_tree_view_init(&tv, 0, 0, 30, 15);
    TuiColor fg = TUI_COLOR_INDEX(15);
    TuiColor bg = TUI_COLOR_INDEX(0);
    tui_tree_view_set_colors(&tv, fg, bg, fg, bg, fg);
    TEST_ASSERT(tui_color_eq(tv.fg, fg));
    tui_widget_destroy(&tv.base);
}

TEST_BEGIN(tree_view_set_on_select)
{
    TuiTreeView tv;
    tui_tree_view_init(&tv, 0, 0, 30, 15);
    tui_tree_view_set_on_select(&tv, NULL, NULL);
    TEST_ASSERT(tv.on_select == NULL);
    tui_widget_destroy(&tv.base);
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
