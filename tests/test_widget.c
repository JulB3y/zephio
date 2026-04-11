#include "util.h"
#include "tui_widget.h"
#include "tui_screen.h"

static int g_resize_called = 0;
static int g_render_called = 0;
static int g_child_rendered = 0;

static void stub_on_resize(TuiWidget *w, int width, int height) {
    (void)w; (void)width; (void)height;
    g_resize_called = 1;
}

static void stub_on_render(TuiWidget *w) {
    (void)w;
    g_render_called = 1;
}

static void stub_child_render(TuiWidget *w) {
    (void)w;
    g_child_rendered = 1;
}

static TuiWidgetVTable g_resize_vt   = { NULL, NULL, NULL, NULL, stub_on_resize, NULL, NULL };
static TuiWidgetVTable g_render_vt   = { stub_on_render, NULL, NULL, NULL, NULL, NULL, NULL };
static TuiWidgetVTable g_child_vt    = { stub_child_render, NULL, NULL, NULL, NULL, NULL, NULL };

/* ── Widget Init ────────────────────────────────────────────────── */

TEST_BEGIN(widget_init_basic)
{
    TuiWidget w;
    TuiResult res = tui_widget_init(&w, 5, 3, 20, 10, NULL, NULL);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(w.x, 5);
    TEST_EQ(w.y, 3);
    TEST_EQ(w.width, 20);
    TEST_EQ(w.height, 10);
    TEST_EQ(w.abs_x, 5);
    TEST_EQ(w.abs_y, 3);
    TEST_EQ(w.visible, 1);
    TEST_EQ(w.dirty, 1);
    TEST_EQ(w.focusable, 0);
    TEST_EQ(w.focused, 0);
    TEST_EQ(w.disabled, 0);
    TEST_EQ(w.hovered, 0);
    TEST_EQ(w.parent, (void *)NULL);
    TEST_EQ(w.child_count, 0);
    TEST_EQ(w.theme, (void *)NULL);
    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_init_null)
{
    TEST_NE(tui_widget_init(NULL, 0, 0, 10, 10, NULL, NULL), TUI_OK);
}

TEST_BEGIN(widget_init_invalid_size)
{
    TuiWidget w;
    TEST_NE(tui_widget_init(&w, 0, 0, 0, 10, NULL, NULL), TUI_OK);
    TEST_NE(tui_widget_init(&w, 0, 0, 10, 0, NULL, NULL), TUI_OK);
    TEST_NE(tui_widget_init(&w, 0, 0, -1, 10, NULL, NULL), TUI_OK);
    TEST_NE(tui_widget_init(&w, 0, 0, 10, -5, NULL, NULL), TUI_OK);
}

/* ── Widget Tree ────────────────────────────────────────────────── */

TEST_BEGIN(widget_add_child)
{
    TuiWidget parent, child;
    tui_widget_init(&parent, 0, 0, 40, 20, NULL, NULL);
    tui_widget_init(&child, 2, 3, 10, 5, NULL, NULL);

    TuiResult res = tui_widget_add_child(&parent, &child);
    TEST_EQ(res, TUI_OK);
    TEST_EQ(parent.child_count, 1);
    TEST_EQ(child.parent, &parent);
    TEST_EQ(child.abs_x, 2);
    TEST_EQ(child.abs_y, 3);
    TEST_EQ(parent.dirty, 1);

    tui_widget_destroy(&parent);
}

TEST_BEGIN(widget_add_child_null)
{
    TuiWidget w;
    tui_widget_init(&w, 0, 0, 10, 10, NULL, NULL);
    TEST_NE(tui_widget_add_child(NULL, &w), TUI_OK);
    TEST_NE(tui_widget_add_child(&w, NULL), TUI_OK);
    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_add_multiple_children)
{
    TuiWidget parent, c1, c2, c3;
    tui_widget_init(&parent, 10, 5, 40, 20, NULL, NULL);
    tui_widget_init(&c1, 0, 0, 10, 5, NULL, NULL);
    tui_widget_init(&c2, 15, 0, 10, 5, NULL, NULL);
    tui_widget_init(&c3, 30, 0, 10, 5, NULL, NULL);

    tui_widget_add_child(&parent, &c1);
    tui_widget_add_child(&parent, &c2);
    tui_widget_add_child(&parent, &c3);

    TEST_EQ(parent.child_count, 3);
    TEST_EQ(c1.abs_x, 10);
    TEST_EQ(c1.abs_y, 5);
    TEST_EQ(c2.abs_x, 25);
    TEST_EQ(c2.abs_y, 5);
    TEST_EQ(c3.abs_x, 40);
    TEST_EQ(c3.abs_y, 5);

    tui_widget_destroy(&parent);
}

TEST_BEGIN(widget_nested_tree)
{
    TuiWidget root, mid, leaf;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&mid, 5, 2, 40, 15, NULL, NULL);
    tui_widget_init(&leaf, 3, 1, 20, 5, NULL, NULL);

    tui_widget_add_child(&root, &mid);
    tui_widget_add_child(&mid, &leaf);

    TEST_EQ(root.child_count, 1);
    TEST_EQ(mid.child_count, 1);
    TEST_EQ(leaf.parent, &mid);
    TEST_EQ(leaf.abs_x, 8);
    TEST_EQ(leaf.abs_y, 3);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_remove_child)
{
    TuiWidget parent, c1, c2;
    tui_widget_init(&parent, 0, 0, 40, 20, NULL, NULL);
    tui_widget_init(&c1, 0, 0, 10, 5, NULL, NULL);
    tui_widget_init(&c2, 15, 0, 10, 5, NULL, NULL);

    tui_widget_add_child(&parent, &c1);
    tui_widget_add_child(&parent, &c2);
    TEST_EQ(parent.child_count, 2);

    tui_widget_remove_child(&parent, &c1);
    TEST_EQ(parent.child_count, 1);
    TEST_EQ(c1.parent, (void *)NULL);
    TEST_EQ(parent.children[0], &c2);

    tui_widget_destroy(&parent);
    tui_widget_destroy(&c1);
}

TEST_BEGIN(widget_remove_child_not_found)
{
    TuiWidget parent, orphan;
    tui_widget_init(&parent, 0, 0, 40, 20, NULL, NULL);
    tui_widget_init(&orphan, 0, 0, 10, 5, NULL, NULL);

    tui_widget_remove_child(&parent, &orphan);
    TEST_EQ(parent.child_count, 0);

    tui_widget_destroy(&parent);
    tui_widget_destroy(&orphan);
}

TEST_BEGIN(widget_remove_all_children)
{
    TuiWidget parent, c1, c2, c3;
    tui_widget_init(&parent, 0, 0, 40, 20, NULL, NULL);
    tui_widget_init(&c1, 0, 0, 10, 5, NULL, NULL);
    tui_widget_init(&c2, 0, 0, 10, 5, NULL, NULL);
    tui_widget_init(&c3, 0, 0, 10, 5, NULL, NULL);

    tui_widget_add_child(&parent, &c1);
    tui_widget_add_child(&parent, &c2);
    tui_widget_add_child(&parent, &c3);

    tui_widget_remove_all_children(&parent);
    TEST_EQ(parent.child_count, 0);
    TEST_EQ(c1.parent, (void *)NULL);
    TEST_EQ(c2.parent, (void *)NULL);
    TEST_EQ(c3.parent, (void *)NULL);
    TEST_EQ(parent.dirty, 1);

    tui_widget_destroy(&parent);
    tui_widget_destroy(&c1);
    tui_widget_destroy(&c2);
    tui_widget_destroy(&c3);
}

TEST_BEGIN(widget_destroy_recursive)
{
    TuiWidget root, c1, c2, grandchild;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&c1, 0, 0, 40, 12, NULL, NULL);
    tui_widget_init(&c2, 40, 0, 40, 12, NULL, NULL);
    tui_widget_init(&grandchild, 0, 0, 20, 6, NULL, NULL);

    tui_widget_add_child(&root, &c1);
    tui_widget_add_child(&root, &c2);
    tui_widget_add_child(&c1, &grandchild);

    tui_widget_destroy(&root);
    TEST_EQ(root.child_count, 0);
    TEST_EQ(c1.child_count, 0);
}

TEST_BEGIN(widget_destroy_null)
{
    tui_widget_destroy(NULL);
}

/* ── Focus ──────────────────────────────────────────────────────── */

TEST_BEGIN(widget_focus_basic)
{
    TuiWidget root, a, b;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&a, 0, 0, 40, 12, NULL, NULL);
    tui_widget_init(&b, 40, 0, 40, 12, NULL, NULL);
    a.focusable = 1;
    b.focusable = 1;

    tui_widget_add_child(&root, &a);
    tui_widget_add_child(&root, &b);

    tui_widget_focus(&a);
    TEST_EQ(a.focused, 1);
    TEST_EQ(root.focused_child_idx, 0);
    TEST_EQ(a.dirty, 1);

    tui_widget_focus(&b);
    TEST_EQ(b.focused, 1);
    TEST_EQ(a.focused, 0);
    TEST_EQ(root.focused_child_idx, 1);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_blur)
{
    TuiWidget root, w;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&w, 0, 0, 40, 12, NULL, NULL);
    w.focusable = 1;

    tui_widget_add_child(&root, &w);
    tui_widget_focus(&w);
    TEST_EQ(w.focused, 1);

    tui_widget_blur(&w);
    TEST_EQ(w.focused, 0);
    TEST_EQ(root.focused_child_idx, -1);
    TEST_EQ(w.dirty, 1);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_focus_not_focusable)
{
    TuiWidget root, w;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&w, 0, 0, 40, 12, NULL, NULL);
    w.focusable = 0;

    tui_widget_add_child(&root, &w);
    tui_widget_focus(&w);
    TEST_EQ(w.focused, 0);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_focus_next_prev)
{
    TuiWidget root, a, b, c;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&a, 0, 0, 20, 5, NULL, NULL);
    tui_widget_init(&b, 20, 0, 20, 5, NULL, NULL);
    tui_widget_init(&c, 40, 0, 20, 5, NULL, NULL);
    a.focusable = 1;
    b.focusable = 1;
    c.focusable = 1;

    tui_widget_add_child(&root, &a);
    tui_widget_add_child(&root, &b);
    tui_widget_add_child(&root, &c);

    tui_widget_focus_next(&root);
    TEST_EQ(a.focused, 1);

    tui_widget_focus_next(&root);
    TEST_EQ(b.focused, 1);
    TEST_EQ(a.focused, 0);

    tui_widget_focus_next(&root);
    TEST_EQ(c.focused, 1);

    tui_widget_focus_next(&root);
    TEST_EQ(a.focused, 1);
    TEST_EQ(c.focused, 0);

    tui_widget_focus_prev(&root);
    TEST_EQ(c.focused, 1);

    tui_widget_focus_prev(&root);
    TEST_EQ(b.focused, 1);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_get_focused)
{
    TuiWidget root, a, b;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&a, 0, 0, 40, 12, NULL, NULL);
    tui_widget_init(&b, 40, 0, 40, 12, NULL, NULL);
    a.focusable = 1;
    b.focusable = 1;

    tui_widget_add_child(&root, &a);
    tui_widget_add_child(&root, &b);

    TEST_ASSERT(tui_widget_get_focused(&root) == NULL);

    tui_widget_focus(&a);
    TEST_ASSERT(tui_widget_get_focused(&root) == &a);

    tui_widget_focus(&b);
    TEST_ASSERT(tui_widget_get_focused(&root) == &b);

    tui_widget_blur(&b);
    TEST_ASSERT(tui_widget_get_focused(&root) == NULL);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_get_focused_null)
{
    TEST_ASSERT(tui_widget_get_focused(NULL) == NULL);
}

TEST_BEGIN(widget_focus_next_no_focusable)
{
    TuiWidget root, a;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&a, 0, 0, 40, 12, NULL, NULL);
    a.focusable = 0;

    tui_widget_add_child(&root, &a);
    tui_widget_focus_next(&root);
    TEST_EQ(a.focused, 0);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_focus_next_empty)
{
    TuiWidget root;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_focus_next(&root);
    tui_widget_focus_prev(&root);
    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_focus_next_null)
{
    tui_widget_focus_next(NULL);
    tui_widget_focus_prev(NULL);
}

TEST_BEGIN(widget_focus_auto_blurs_old)
{
    TuiWidget root, a, b;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&a, 0, 0, 40, 12, NULL, NULL);
    tui_widget_init(&b, 40, 0, 40, 12, NULL, NULL);
    a.focusable = 1;
    b.focusable = 1;

    tui_widget_add_child(&root, &a);
    tui_widget_add_child(&root, &b);

    tui_widget_focus(&a);
    TEST_EQ(a.focused, 1);

    tui_widget_focus(&b);
    TEST_EQ(b.focused, 1);
    TEST_EQ(a.focused, 0);

    tui_widget_destroy(&root);
}

/* ── Visibility ─────────────────────────────────────────────────── */

TEST_BEGIN(widget_set_visible)
{
    TuiWidget w;
    tui_widget_init(&w, 0, 0, 20, 10, NULL, NULL);
    TEST_EQ(w.visible, 1);

    tui_widget_set_visible(&w, 0);
    TEST_EQ(w.visible, 0);
    TEST_EQ(w.dirty, 1);

    tui_widget_set_visible(&w, 1);
    TEST_EQ(w.visible, 1);

    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_hide_blurs_focused)
{
    TuiWidget root, w;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&w, 0, 0, 40, 12, NULL, NULL);
    w.focusable = 1;

    tui_widget_add_child(&root, &w);
    tui_widget_focus(&w);
    TEST_EQ(w.focused, 1);

    tui_widget_set_visible(&w, 0);
    TEST_EQ(w.focused, 0);
    TEST_EQ(w.visible, 0);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_set_visible_propagates_parent_dirty)
{
    TuiWidget parent, child;
    tui_widget_init(&parent, 0, 0, 40, 20, NULL, NULL);
    tui_widget_init(&child, 0, 0, 10, 5, NULL, NULL);

    tui_widget_add_child(&parent, &child);
    parent.dirty = 0;

    tui_widget_set_visible(&child, 0);
    TEST_EQ(parent.dirty, 1);

    tui_widget_destroy(&parent);
}

/* ── Dirty Flags ────────────────────────────────────────────────── */

TEST_BEGIN(widget_dirty_on_init)
{
    TuiWidget w;
    tui_widget_init(&w, 0, 0, 10, 5, NULL, NULL);
    TEST_EQ(w.dirty, 1);
    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_set_dirty)
{
    TuiWidget w;
    tui_widget_init(&w, 0, 0, 10, 5, NULL, NULL);
    w.dirty = 0;

    tui_widget_set_dirty(&w);
    TEST_EQ(w.dirty, 1);

    tui_widget_set_dirty(NULL);
    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_mark_dirty_recursive)
{
    TuiWidget root, c1, c2;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&c1, 0, 0, 40, 12, NULL, NULL);
    tui_widget_init(&c2, 40, 0, 40, 12, NULL, NULL);

    tui_widget_add_child(&root, &c1);
    tui_widget_add_child(&root, &c2);

    root.dirty = 0;
    c1.dirty = 0;
    c2.dirty = 0;

    tui_widget_mark_dirty_recursive(&root);
    TEST_EQ(root.dirty, 1);
    TEST_EQ(c1.dirty, 1);
    TEST_EQ(c2.dirty, 1);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_is_dirty)
{
    TuiWidget root, child, grandchild;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&child, 0, 0, 40, 12, NULL, NULL);
    tui_widget_init(&grandchild, 0, 0, 20, 6, NULL, NULL);

    tui_widget_add_child(&root, &child);
    tui_widget_add_child(&child, &grandchild);

    root.dirty = 0;
    child.dirty = 0;
    grandchild.dirty = 0;

    TEST_EQ(tui_widget_is_dirty(&root), 0);

    grandchild.dirty = 1;
    TEST_EQ(tui_widget_is_dirty(&root), 1);

    TEST_EQ(tui_widget_is_dirty(NULL), 0);

    tui_widget_destroy(&root);
}

/* ── Mouse Hit-Testing ──────────────────────────────────────────── */

TEST_BEGIN(widget_contains)
{
    TuiWidget w;
    tui_widget_init(&w, 5, 2, 20, 10, NULL, NULL);
    w.abs_x = 5;
    w.abs_y = 2;

    TEST_EQ(tui_widget_contains(&w, 2, 5), 1);
    TEST_EQ(tui_widget_contains(&w, 11, 24), 1);
    TEST_EQ(tui_widget_contains(&w, 2, 4), 0);
    TEST_EQ(tui_widget_contains(&w, 12, 5), 0);
    TEST_EQ(tui_widget_contains(&w, 1, 5), 0);
    TEST_EQ(tui_widget_contains(&w, 2, 25), 0);
    TEST_EQ(tui_widget_contains(NULL, 0, 0), 0);

    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_find_at_basic)
{
    TuiWidget root, a, b;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&a, 0, 0, 40, 24, NULL, NULL);
    tui_widget_init(&b, 40, 0, 40, 24, NULL, NULL);

    tui_widget_add_child(&root, &a);
    tui_widget_add_child(&root, &b);

    TEST_ASSERT(tui_widget_find_at(&root, 0, 5) == &a);
    TEST_ASSERT(tui_widget_find_at(&root, 0, 50) == &b);
    TEST_ASSERT(tui_widget_find_at(&root, 0, 0) == &a);
    TEST_ASSERT(tui_widget_find_at(NULL, 0, 0) == NULL);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_find_at_hidden)
{
    TuiWidget root, a, b;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&a, 0, 0, 40, 24, NULL, NULL);
    tui_widget_init(&b, 40, 0, 40, 24, NULL, NULL);

    tui_widget_add_child(&root, &a);
    tui_widget_add_child(&root, &b);
    tui_widget_set_visible(&b, 0);

    TEST_ASSERT(tui_widget_find_at(&root, 0, 50) == &root);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_find_at_nested)
{
    TuiWidget root, outer, inner;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&outer, 5, 2, 40, 15, NULL, NULL);
    tui_widget_init(&inner, 3, 1, 10, 5, NULL, NULL);

    tui_widget_add_child(&root, &outer);
    tui_widget_add_child(&outer, &inner);

    TEST_ASSERT(tui_widget_find_at(&root, 3, 8) == &inner);
    TEST_ASSERT(tui_widget_find_at(&root, 2, 6) == &outer);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_find_at_outside)
{
    TuiWidget root;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    TEST_ASSERT(tui_widget_find_at(&root, 0, 80) == NULL);
    TEST_ASSERT(tui_widget_find_at(&root, 24, 0) == NULL);
    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_handle_mouse_focus_on_click)
{
    TuiWidget root, a;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&a, 0, 0, 40, 12, NULL, NULL);
    a.focusable = 1;

    tui_widget_add_child(&root, &a);
    TEST_EQ(a.focused, 0);

    TuiMouseEvent mouse = { 0, 5, TUI_MOUSE_BTN_LEFT, TUI_MOUSE_PRESS, 0 };
    int handled = tui_widget_handle_mouse(&root, &mouse);
    TEST_EQ(handled, 0);
    TEST_EQ(a.focused, 1);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_handle_mouse_hover)
{
    TuiWidget root, a, b;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&a, 0, 0, 40, 24, NULL, NULL);
    tui_widget_init(&b, 40, 0, 40, 24, NULL, NULL);

    tui_widget_add_child(&root, &a);
    tui_widget_add_child(&root, &b);

    TuiMouseEvent motion_a = { 0, 5, TUI_MOUSE_BTN_NONE, TUI_MOUSE_MOTION, 0 };
    tui_widget_handle_mouse(&root, &motion_a);
    TEST_EQ(a.hovered, 1);
    TEST_EQ(b.hovered, 0);

    TuiMouseEvent motion_b = { 0, 50, TUI_MOUSE_BTN_NONE, TUI_MOUSE_MOTION, 0 };
    tui_widget_handle_mouse(&root, &motion_b);
    TEST_EQ(a.hovered, 0);
    TEST_EQ(b.hovered, 1);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_handle_mouse_null)
{
    TuiWidget root;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    TEST_EQ(tui_widget_handle_mouse(&root, NULL), 0);
    TEST_EQ(tui_widget_handle_mouse(NULL, NULL), 0);
    tui_widget_destroy(&root);
}

/* ── Theme Propagation ──────────────────────────────────────────── */

TEST_BEGIN(widget_set_theme)
{
    TuiWidget root, child, grandchild;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&child, 0, 0, 40, 12, NULL, NULL);
    tui_widget_init(&grandchild, 0, 0, 20, 6, NULL, NULL);

    tui_widget_add_child(&root, &child);
    tui_widget_add_child(&child, &grandchild);

    TuiTheme theme = tui_theme_default();
    tui_widget_set_theme(&root, &theme);

    TEST_ASSERT(root.theme == &theme);
    TEST_ASSERT(child.theme == &theme);
    TEST_ASSERT(grandchild.theme == &theme);
    TEST_EQ(root.dirty, 1);
    TEST_EQ(child.dirty, 1);
    TEST_EQ(grandchild.dirty, 1);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_set_theme_null)
{
    TuiWidget w;
    tui_widget_init(&w, 0, 0, 10, 5, NULL, NULL);
    tui_widget_set_theme(NULL, NULL);
    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_get_style_normal)
{
    TuiWidget w;
    tui_widget_init(&w, 0, 0, 10, 5, NULL, NULL);
    TuiTheme theme = tui_theme_default();
    w.theme = &theme;

    TuiStyle s = tui_widget_get_style(&w);
    TEST_EQ(s.attr, theme.styles[TUI_STATE_NORMAL].attr);

    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_get_style_focused)
{
    TuiWidget w;
    tui_widget_init(&w, 0, 0, 10, 5, NULL, NULL);
    TuiTheme theme = tui_theme_default();
    w.theme = &theme;
    w.focused = 1;

    TuiStyle s = tui_widget_get_style(&w);
    TEST_EQ(s.attr, theme.styles[TUI_STATE_FOCUSED].attr);

    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_get_style_disabled)
{
    TuiWidget w;
    tui_widget_init(&w, 0, 0, 10, 5, NULL, NULL);
    TuiTheme theme = tui_theme_default();
    w.theme = &theme;
    w.disabled = 1;

    TuiStyle s = tui_widget_get_style(&w);
    TEST_EQ(s.attr, theme.styles[TUI_STATE_DISABLED].attr);

    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_get_style_no_theme)
{
    TuiWidget w;
    tui_widget_init(&w, 0, 0, 10, 5, NULL, NULL);
    TuiStyle s = tui_widget_get_style(&w);
    TEST_EQ(s.attr, TUI_STYLE_NONE.attr);
    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_get_style_null)
{
    TuiStyle s = tui_widget_get_style(NULL);
    TEST_EQ(s.attr, TUI_STYLE_NONE.attr);
}

/* ── Disabled ───────────────────────────────────────────────────── */

TEST_BEGIN(widget_set_disabled)
{
    TuiWidget w;
    tui_widget_init(&w, 0, 0, 10, 5, NULL, NULL);
    TEST_EQ(w.disabled, 0);

    tui_widget_set_disabled(&w, 1);
    TEST_EQ(w.disabled, 1);
    TEST_EQ(w.dirty, 1);

    tui_widget_set_disabled(&w, 0);
    TEST_EQ(w.disabled, 0);

    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_disabled_blurs_focused)
{
    TuiWidget root, w;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&w, 0, 0, 40, 12, NULL, NULL);
    w.focusable = 1;

    tui_widget_add_child(&root, &w);
    tui_widget_focus(&w);
    TEST_EQ(w.focused, 1);

    tui_widget_set_disabled(&w, 1);
    TEST_EQ(w.focused, 0);
    TEST_EQ(w.disabled, 1);

    tui_widget_destroy(&root);
}

/* ── Position & Size ────────────────────────────────────────────── */

TEST_BEGIN(widget_set_position)
{
    TuiWidget w;
    tui_widget_init(&w, 0, 0, 10, 5, NULL, NULL);
    w.dirty = 0;

    tui_widget_set_position(&w, 5, 3);
    TEST_EQ(w.x, 5);
    TEST_EQ(w.y, 3);
    TEST_EQ(w.dirty, 1);

    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_set_size)
{
    TuiWidget w;
    tui_widget_init(&w, 0, 0, 10, 5, NULL, NULL);
    w.dirty = 0;

    tui_widget_set_size(&w, 30, 15);
    TEST_EQ(w.width, 30);
    TEST_EQ(w.height, 15);
    TEST_EQ(w.dirty, 1);

    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_resize)
{
    g_resize_called = 0;

    TuiWidget w;
    tui_widget_init(&w, 0, 0, 10, 5, &g_resize_vt, NULL);
    w.dirty = 0;

    tui_widget_resize(&w, 20, 10);
    TEST_EQ(w.width, 20);
    TEST_EQ(w.height, 10);
    TEST_EQ(w.dirty, 1);
    TEST_EQ(g_resize_called, 1);

    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_resize_null)
{
    tui_widget_resize(NULL, 10, 10);
}

/* ── Hovered ────────────────────────────────────────────────────── */

TEST_BEGIN(widget_set_hovered)
{
    TuiWidget root, a, b;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&a, 0, 0, 40, 24, NULL, NULL);
    tui_widget_init(&b, 40, 0, 40, 24, NULL, NULL);

    tui_widget_add_child(&root, &a);
    tui_widget_add_child(&root, &b);

    tui_widget_set_hovered(&root, &a);
    TEST_EQ(a.hovered, 1);
    TEST_EQ(b.hovered, 0);

    tui_widget_set_hovered(&root, &b);
    TEST_EQ(a.hovered, 0);
    TEST_EQ(b.hovered, 1);

    tui_widget_set_hovered(&root, NULL);
    TEST_EQ(a.hovered, 0);
    TEST_EQ(b.hovered, 0);

    tui_widget_destroy(&root);
}

TEST_BEGIN(widget_get_hovered)
{
    TuiWidget root, a;
    tui_widget_init(&root, 0, 0, 80, 24, NULL, NULL);
    tui_widget_init(&a, 0, 0, 40, 24, NULL, NULL);

    tui_widget_add_child(&root, &a);

    TEST_ASSERT(tui_widget_get_hovered(&root) == NULL);

    tui_widget_set_hovered(&root, &a);
    TEST_ASSERT(tui_widget_get_hovered(&root) == &a);

    tui_widget_set_hovered(&root, NULL);
    TEST_ASSERT(tui_widget_get_hovered(&root) == NULL);

    TEST_ASSERT(tui_widget_get_hovered(NULL) == NULL);

    tui_widget_destroy(&root);
}

/* ── Render ─────────────────────────────────────────────────────── */

TEST_BEGIN(widget_render_skips_hidden)
{
    TuiWidget w;
    tui_widget_init(&w, 0, 0, 10, 5, NULL, NULL);
    w.visible = 0;
    w.dirty = 1;

    tui_widget_render(&w);
    TEST_EQ(w.dirty, 1);

    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_render_clears_dirty)
{
    g_render_called = 0;

    TuiWidget w;
    tui_widget_init(&w, 0, 0, 10, 5, &g_render_vt, NULL);

    tui_widget_render(&w);
    TEST_EQ(w.dirty, 0);
    TEST_EQ(g_render_called, 1);

    tui_widget_destroy(&w);
}

TEST_BEGIN(widget_render_children)
{
    g_child_rendered = 0;

    TuiWidget parent, child;
    tui_widget_init(&parent, 0, 0, 40, 20, NULL, NULL);
    tui_widget_init(&child, 2, 3, 10, 5, &g_child_vt, NULL);

    tui_widget_add_child(&parent, &child);

    tui_widget_render(&parent);
    TEST_EQ(g_child_rendered, 1);
    TEST_EQ(child.dirty, 0);
    TEST_EQ(child.abs_x, 2);
    TEST_EQ(child.abs_y, 3);

    tui_widget_destroy(&parent);
}

TEST_BEGIN(widget_render_null)
{
    tui_widget_render(NULL);
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void)
{
    fprintf(stderr, "Running widget tests...\n\n");

    TEST_RUN(widget_init_basic);
    TEST_RUN(widget_init_null);
    TEST_RUN(widget_init_invalid_size);

    TEST_RUN(widget_add_child);
    TEST_RUN(widget_add_child_null);
    TEST_RUN(widget_add_multiple_children);
    TEST_RUN(widget_nested_tree);
    TEST_RUN(widget_remove_child);
    TEST_RUN(widget_remove_child_not_found);
    TEST_RUN(widget_remove_all_children);
    TEST_RUN(widget_destroy_recursive);
    TEST_RUN(widget_destroy_null);

    TEST_RUN(widget_focus_basic);
    TEST_RUN(widget_blur);
    TEST_RUN(widget_focus_not_focusable);
    TEST_RUN(widget_focus_next_prev);
    TEST_RUN(widget_get_focused);
    TEST_RUN(widget_get_focused_null);
    TEST_RUN(widget_focus_next_no_focusable);
    TEST_RUN(widget_focus_next_empty);
    TEST_RUN(widget_focus_next_null);
    TEST_RUN(widget_focus_auto_blurs_old);

    TEST_RUN(widget_set_visible);
    TEST_RUN(widget_hide_blurs_focused);
    TEST_RUN(widget_set_visible_propagates_parent_dirty);

    TEST_RUN(widget_dirty_on_init);
    TEST_RUN(widget_set_dirty);
    TEST_RUN(widget_mark_dirty_recursive);
    TEST_RUN(widget_is_dirty);

    TEST_RUN(widget_contains);
    TEST_RUN(widget_find_at_basic);
    TEST_RUN(widget_find_at_hidden);
    TEST_RUN(widget_find_at_nested);
    TEST_RUN(widget_find_at_outside);
    TEST_RUN(widget_handle_mouse_focus_on_click);
    TEST_RUN(widget_handle_mouse_hover);
    TEST_RUN(widget_handle_mouse_null);

    TEST_RUN(widget_set_theme);
    TEST_RUN(widget_set_theme_null);
    TEST_RUN(widget_get_style_normal);
    TEST_RUN(widget_get_style_focused);
    TEST_RUN(widget_get_style_disabled);
    TEST_RUN(widget_get_style_no_theme);
    TEST_RUN(widget_get_style_null);

    TEST_RUN(widget_set_disabled);
    TEST_RUN(widget_disabled_blurs_focused);

    TEST_RUN(widget_set_position);
    TEST_RUN(widget_set_size);
    TEST_RUN(widget_resize);
    TEST_RUN(widget_resize_null);

    TEST_RUN(widget_set_hovered);
    TEST_RUN(widget_get_hovered);

    TEST_RUN(widget_render_skips_hidden);
    TEST_RUN(widget_render_clears_dirty);
    TEST_RUN(widget_render_children);
    TEST_RUN(widget_render_null);

    TEST_SUMMARY();
}
