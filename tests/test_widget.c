#include "util.h"
#include "zephio_widget.h"
#include "zephio_screen.h"
#include "zephio_context.h"

static ZephioContext g_test_ctx;
static int g_resize_called = 0;
static int g_render_called = 0;
static int g_child_rendered = 0;

static void stub_on_resize(ZephioWidget *w, int width, int height) {
    (void)w; (void)width; (void)height;
    g_resize_called = 1;
}

static void stub_on_render(ZephioWidget *w) {
    (void)w;
    g_render_called = 1;
}

static void stub_child_render(ZephioWidget *w) {
    (void)w;
    g_child_rendered = 1;
}

static ZephioWidgetVTable g_resize_vt   = { NULL, NULL, NULL, NULL, stub_on_resize, NULL, NULL };
static ZephioWidgetVTable g_render_vt   = { stub_on_render, NULL, NULL, NULL, NULL, NULL, NULL };
static ZephioWidgetVTable g_child_vt    = { stub_child_render, NULL, NULL, NULL, NULL, NULL, NULL };

/* ── Widget Init ────────────────────────────────────────────────── */

TEST_BEGIN(widget_init_basic)
{
    ZephioWidget w;
    ZephioResult res = zephio_widget_init_ctx(&w, 5, 3, 20, 10, NULL, &g_test_ctx, NULL);
    TEST_EQ(res, ZEPHIO_OK);
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
    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_init_null)
{
    TEST_NE(zephio_widget_init_ctx(NULL, 0, 0, 10, 10, NULL, &g_test_ctx, NULL), ZEPHIO_OK);
}

TEST_BEGIN(widget_init_invalid_size)
{
    ZephioWidget w;
    TEST_NE(zephio_widget_init_ctx(&w, 0, 0, 0, 10, NULL, &g_test_ctx, NULL), ZEPHIO_OK);
    TEST_NE(zephio_widget_init_ctx(&w, 0, 0, 10, 0, NULL, &g_test_ctx, NULL), ZEPHIO_OK);
    TEST_NE(zephio_widget_init_ctx(&w, 0, 0, -1, 10, NULL, &g_test_ctx, NULL), ZEPHIO_OK);
    TEST_NE(zephio_widget_init_ctx(&w, 0, 0, 10, -5, NULL, &g_test_ctx, NULL), ZEPHIO_OK);
}

/* ── Widget Tree ────────────────────────────────────────────────── */

TEST_BEGIN(widget_add_child)
{
    ZephioWidget parent, child;
    zephio_widget_init_ctx(&parent, 0, 0, 40, 20, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&child, 2, 3, 10, 5, NULL, &g_test_ctx, NULL);

    ZephioResult res = zephio_widget_add_child(&parent, &child);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(parent.child_count, 1);
    TEST_EQ(child.parent, &parent);
    TEST_EQ(child.abs_x, 2);
    TEST_EQ(child.abs_y, 3);
    TEST_EQ(parent.dirty, 1);

    zephio_widget_destroy(&parent);
}

TEST_BEGIN(widget_add_child_null)
{
    ZephioWidget w;
    zephio_widget_init_ctx(&w, 0, 0, 10, 10, NULL, &g_test_ctx, NULL);
    TEST_NE(zephio_widget_add_child(NULL, &w), ZEPHIO_OK);
    TEST_NE(zephio_widget_add_child(&w, NULL), ZEPHIO_OK);
    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_add_multiple_children)
{
    ZephioWidget parent, c1, c2, c3;
    zephio_widget_init_ctx(&parent, 10, 5, 40, 20, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&c1, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&c2, 15, 0, 10, 5, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&c3, 30, 0, 10, 5, NULL, &g_test_ctx, NULL);

    zephio_widget_add_child(&parent, &c1);
    zephio_widget_add_child(&parent, &c2);
    zephio_widget_add_child(&parent, &c3);

    TEST_EQ(parent.child_count, 3);
    TEST_EQ(c1.abs_x, 10);
    TEST_EQ(c1.abs_y, 5);
    TEST_EQ(c2.abs_x, 25);
    TEST_EQ(c2.abs_y, 5);
    TEST_EQ(c3.abs_x, 40);
    TEST_EQ(c3.abs_y, 5);

    zephio_widget_destroy(&parent);
}

TEST_BEGIN(widget_nested_tree)
{
    ZephioWidget root, mid, leaf;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&mid, 5, 2, 40, 15, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&leaf, 3, 1, 20, 5, NULL, &g_test_ctx, NULL);

    zephio_widget_add_child(&root, &mid);
    zephio_widget_add_child(&mid, &leaf);

    TEST_EQ(root.child_count, 1);
    TEST_EQ(mid.child_count, 1);
    TEST_EQ(leaf.parent, &mid);
    TEST_EQ(leaf.abs_x, 8);
    TEST_EQ(leaf.abs_y, 3);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_remove_child)
{
    ZephioWidget parent, c1, c2;
    zephio_widget_init_ctx(&parent, 0, 0, 40, 20, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&c1, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&c2, 15, 0, 10, 5, NULL, &g_test_ctx, NULL);

    zephio_widget_add_child(&parent, &c1);
    zephio_widget_add_child(&parent, &c2);
    TEST_EQ(parent.child_count, 2);

    zephio_widget_remove_child(&parent, &c1);
    TEST_EQ(parent.child_count, 1);
    TEST_EQ(c1.parent, (void *)NULL);
    TEST_EQ(parent.children[0], &c2);

    zephio_widget_destroy(&parent);
    zephio_widget_destroy(&c1);
}

TEST_BEGIN(widget_remove_child_not_found)
{
    ZephioWidget parent, orphan;
    zephio_widget_init_ctx(&parent, 0, 0, 40, 20, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&orphan, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);

    zephio_widget_remove_child(&parent, &orphan);
    TEST_EQ(parent.child_count, 0);

    zephio_widget_destroy(&parent);
    zephio_widget_destroy(&orphan);
}

TEST_BEGIN(widget_remove_all_children)
{
    ZephioWidget parent, c1, c2, c3;
    zephio_widget_init_ctx(&parent, 0, 0, 40, 20, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&c1, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&c2, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&c3, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);

    zephio_widget_add_child(&parent, &c1);
    zephio_widget_add_child(&parent, &c2);
    zephio_widget_add_child(&parent, &c3);

    zephio_widget_remove_all_children(&parent);
    TEST_EQ(parent.child_count, 0);
    TEST_EQ(c1.parent, (void *)NULL);
    TEST_EQ(c2.parent, (void *)NULL);
    TEST_EQ(c3.parent, (void *)NULL);
    TEST_EQ(parent.dirty, 1);

    zephio_widget_destroy(&parent);
    zephio_widget_destroy(&c1);
    zephio_widget_destroy(&c2);
    zephio_widget_destroy(&c3);
}

TEST_BEGIN(widget_destroy_recursive)
{
    ZephioWidget root, c1, c2, grandchild;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&c1, 0, 0, 40, 12, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&c2, 40, 0, 40, 12, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&grandchild, 0, 0, 20, 6, NULL, &g_test_ctx, NULL);

    zephio_widget_add_child(&root, &c1);
    zephio_widget_add_child(&root, &c2);
    zephio_widget_add_child(&c1, &grandchild);

    zephio_widget_destroy(&root);
    TEST_EQ(root.child_count, 0);
    TEST_EQ(c1.child_count, 0);
}

TEST_BEGIN(widget_destroy_null)
{
    zephio_widget_destroy(NULL);
}

/* ── Focus ──────────────────────────────────────────────────────── */

TEST_BEGIN(widget_focus_basic)
{
    ZephioWidget root, a, b;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&a, 0, 0, 40, 12, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&b, 40, 0, 40, 12, NULL, &g_test_ctx, NULL);
    a.focusable = 1;
    b.focusable = 1;

    zephio_widget_add_child(&root, &a);
    zephio_widget_add_child(&root, &b);

    zephio_widget_focus(&a);
    TEST_EQ(a.focused, 1);
    TEST_EQ(root.focused_child_idx, 0);
    TEST_EQ(a.dirty, 1);

    zephio_widget_focus(&b);
    TEST_EQ(b.focused, 1);
    TEST_EQ(a.focused, 0);
    TEST_EQ(root.focused_child_idx, 1);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_blur)
{
    ZephioWidget root, w;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&w, 0, 0, 40, 12, NULL, &g_test_ctx, NULL);
    w.focusable = 1;

    zephio_widget_add_child(&root, &w);
    zephio_widget_focus(&w);
    TEST_EQ(w.focused, 1);

    zephio_widget_blur(&w);
    TEST_EQ(w.focused, 0);
    TEST_EQ(root.focused_child_idx, -1);
    TEST_EQ(w.dirty, 1);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_focus_not_focusable)
{
    ZephioWidget root, w;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&w, 0, 0, 40, 12, NULL, &g_test_ctx, NULL);
    w.focusable = 0;

    zephio_widget_add_child(&root, &w);
    zephio_widget_focus(&w);
    TEST_EQ(w.focused, 0);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_focus_next_prev)
{
    ZephioWidget root, a, b, c;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&a, 0, 0, 20, 5, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&b, 20, 0, 20, 5, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&c, 40, 0, 20, 5, NULL, &g_test_ctx, NULL);
    a.focusable = 1;
    b.focusable = 1;
    c.focusable = 1;

    zephio_widget_add_child(&root, &a);
    zephio_widget_add_child(&root, &b);
    zephio_widget_add_child(&root, &c);

    zephio_widget_focus_next(&root);
    TEST_EQ(a.focused, 1);

    zephio_widget_focus_next(&root);
    TEST_EQ(b.focused, 1);
    TEST_EQ(a.focused, 0);

    zephio_widget_focus_next(&root);
    TEST_EQ(c.focused, 1);

    zephio_widget_focus_next(&root);
    TEST_EQ(a.focused, 1);
    TEST_EQ(c.focused, 0);

    zephio_widget_focus_prev(&root);
    TEST_EQ(c.focused, 1);

    zephio_widget_focus_prev(&root);
    TEST_EQ(b.focused, 1);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_get_focused)
{
    ZephioWidget root, a, b;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&a, 0, 0, 40, 12, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&b, 40, 0, 40, 12, NULL, &g_test_ctx, NULL);
    a.focusable = 1;
    b.focusable = 1;

    zephio_widget_add_child(&root, &a);
    zephio_widget_add_child(&root, &b);

    TEST_ASSERT(zephio_widget_get_focused(&root) == NULL);

    zephio_widget_focus(&a);
    TEST_ASSERT(zephio_widget_get_focused(&root) == &a);

    zephio_widget_focus(&b);
    TEST_ASSERT(zephio_widget_get_focused(&root) == &b);

    zephio_widget_blur(&b);
    TEST_ASSERT(zephio_widget_get_focused(&root) == NULL);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_get_focused_null)
{
    TEST_ASSERT(zephio_widget_get_focused(NULL) == NULL);
}

TEST_BEGIN(widget_focus_next_no_focusable)
{
    ZephioWidget root, a;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&a, 0, 0, 40, 12, NULL, &g_test_ctx, NULL);
    a.focusable = 0;

    zephio_widget_add_child(&root, &a);
    zephio_widget_focus_next(&root);
    TEST_EQ(a.focused, 0);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_focus_next_empty)
{
    ZephioWidget root;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_focus_next(&root);
    zephio_widget_focus_prev(&root);
    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_focus_next_null)
{
    zephio_widget_focus_next(NULL);
    zephio_widget_focus_prev(NULL);
}

TEST_BEGIN(widget_focus_auto_blurs_old)
{
    ZephioWidget root, a, b;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&a, 0, 0, 40, 12, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&b, 40, 0, 40, 12, NULL, &g_test_ctx, NULL);
    a.focusable = 1;
    b.focusable = 1;

    zephio_widget_add_child(&root, &a);
    zephio_widget_add_child(&root, &b);

    zephio_widget_focus(&a);
    TEST_EQ(a.focused, 1);

    zephio_widget_focus(&b);
    TEST_EQ(b.focused, 1);
    TEST_EQ(a.focused, 0);

    zephio_widget_destroy(&root);
}

/* ── Visibility ─────────────────────────────────────────────────── */

TEST_BEGIN(widget_set_visible)
{
    ZephioWidget w;
    zephio_widget_init_ctx(&w, 0, 0, 20, 10, NULL, &g_test_ctx, NULL);
    TEST_EQ(w.visible, 1);

    zephio_widget_set_visible(&w, 0);
    TEST_EQ(w.visible, 0);
    TEST_EQ(w.dirty, 1);

    zephio_widget_set_visible(&w, 1);
    TEST_EQ(w.visible, 1);

    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_hide_blurs_focused)
{
    ZephioWidget root, w;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&w, 0, 0, 40, 12, NULL, &g_test_ctx, NULL);
    w.focusable = 1;

    zephio_widget_add_child(&root, &w);
    zephio_widget_focus(&w);
    TEST_EQ(w.focused, 1);

    zephio_widget_set_visible(&w, 0);
    TEST_EQ(w.focused, 0);
    TEST_EQ(w.visible, 0);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_set_visible_propagates_parent_dirty)
{
    ZephioWidget parent, child;
    zephio_widget_init_ctx(&parent, 0, 0, 40, 20, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&child, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);

    zephio_widget_add_child(&parent, &child);
    parent.dirty = 0;

    zephio_widget_set_visible(&child, 0);
    TEST_EQ(parent.dirty, 1);

    zephio_widget_destroy(&parent);
}

/* ── Dirty Flags ────────────────────────────────────────────────── */

TEST_BEGIN(widget_dirty_on_init)
{
    ZephioWidget w;
    zephio_widget_init_ctx(&w, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);
    TEST_EQ(w.dirty, 1);
    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_set_dirty)
{
    ZephioWidget w;
    zephio_widget_init_ctx(&w, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);
    w.dirty = 0;

    zephio_widget_set_dirty(&w);
    TEST_EQ(w.dirty, 1);

    zephio_widget_set_dirty(NULL);
    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_mark_dirty_recursive)
{
    ZephioWidget root, c1, c2;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&c1, 0, 0, 40, 12, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&c2, 40, 0, 40, 12, NULL, &g_test_ctx, NULL);

    zephio_widget_add_child(&root, &c1);
    zephio_widget_add_child(&root, &c2);

    root.dirty = 0;
    c1.dirty = 0;
    c2.dirty = 0;

    zephio_widget_mark_dirty_recursive(&root);
    TEST_EQ(root.dirty, 1);
    TEST_EQ(c1.dirty, 1);
    TEST_EQ(c2.dirty, 1);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_is_dirty)
{
    ZephioWidget root, child, grandchild;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&child, 0, 0, 40, 12, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&grandchild, 0, 0, 20, 6, NULL, &g_test_ctx, NULL);

    zephio_widget_add_child(&root, &child);
    zephio_widget_add_child(&child, &grandchild);

    root.dirty = 0;
    child.dirty = 0;
    grandchild.dirty = 0;

    TEST_EQ(zephio_widget_is_dirty(&root), 0);

    grandchild.dirty = 1;
    TEST_EQ(zephio_widget_is_dirty(&root), 1);

    TEST_EQ(zephio_widget_is_dirty(NULL), 0);

    zephio_widget_destroy(&root);
}

/* ── Mouse Hit-Testing ──────────────────────────────────────────── */

TEST_BEGIN(widget_contains)
{
    ZephioWidget w;
    zephio_widget_init_ctx(&w, 5, 2, 20, 10, NULL, &g_test_ctx, NULL);
    w.abs_x = 5;
    w.abs_y = 2;

    TEST_EQ(zephio_widget_contains(&w, 2, 5), 1);
    TEST_EQ(zephio_widget_contains(&w, 11, 24), 1);
    TEST_EQ(zephio_widget_contains(&w, 2, 4), 0);
    TEST_EQ(zephio_widget_contains(&w, 12, 5), 0);
    TEST_EQ(zephio_widget_contains(&w, 1, 5), 0);
    TEST_EQ(zephio_widget_contains(&w, 2, 25), 0);
    TEST_EQ(zephio_widget_contains(NULL, 0, 0), 0);

    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_find_at_basic)
{
    ZephioWidget root, a, b;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&a, 0, 0, 40, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&b, 40, 0, 40, 24, NULL, &g_test_ctx, NULL);

    zephio_widget_add_child(&root, &a);
    zephio_widget_add_child(&root, &b);

    TEST_ASSERT(zephio_widget_find_at(&root, 0, 5) == &a);
    TEST_ASSERT(zephio_widget_find_at(&root, 0, 50) == &b);
    TEST_ASSERT(zephio_widget_find_at(&root, 0, 0) == &a);
    TEST_ASSERT(zephio_widget_find_at(NULL, 0, 0) == NULL);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_find_at_hidden)
{
    ZephioWidget root, a, b;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&a, 0, 0, 40, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&b, 40, 0, 40, 24, NULL, &g_test_ctx, NULL);

    zephio_widget_add_child(&root, &a);
    zephio_widget_add_child(&root, &b);
    zephio_widget_set_visible(&b, 0);

    TEST_ASSERT(zephio_widget_find_at(&root, 0, 50) == &root);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_find_at_nested)
{
    ZephioWidget root, outer, inner;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&outer, 5, 2, 40, 15, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&inner, 3, 1, 10, 5, NULL, &g_test_ctx, NULL);

    zephio_widget_add_child(&root, &outer);
    zephio_widget_add_child(&outer, &inner);

    TEST_ASSERT(zephio_widget_find_at(&root, 3, 8) == &inner);
    TEST_ASSERT(zephio_widget_find_at(&root, 2, 6) == &outer);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_find_at_outside)
{
    ZephioWidget root;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    TEST_ASSERT(zephio_widget_find_at(&root, 0, 80) == NULL);
    TEST_ASSERT(zephio_widget_find_at(&root, 24, 0) == NULL);
    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_handle_mouse_focus_on_click)
{
    ZephioWidget root, a;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&a, 0, 0, 40, 12, NULL, &g_test_ctx, NULL);
    a.focusable = 1;

    zephio_widget_add_child(&root, &a);
    TEST_EQ(a.focused, 0);

    ZephioMouseEvent mouse = { 0, 5, ZEPHIO_MOUSE_BTN_LEFT, ZEPHIO_MOUSE_PRESS, 0 };
    int handled = zephio_widget_handle_mouse(&root, &mouse);
    TEST_EQ(handled, 0);
    TEST_EQ(a.focused, 1);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_handle_mouse_hover)
{
    ZephioWidget root, a, b;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&a, 0, 0, 40, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&b, 40, 0, 40, 24, NULL, &g_test_ctx, NULL);

    zephio_widget_add_child(&root, &a);
    zephio_widget_add_child(&root, &b);

    ZephioMouseEvent motion_a = { 0, 5, ZEPHIO_MOUSE_BTN_NONE, ZEPHIO_MOUSE_MOTION, 0 };
    zephio_widget_handle_mouse(&root, &motion_a);
    TEST_EQ(a.hovered, 1);
    TEST_EQ(b.hovered, 0);

    ZephioMouseEvent motion_b = { 0, 50, ZEPHIO_MOUSE_BTN_NONE, ZEPHIO_MOUSE_MOTION, 0 };
    zephio_widget_handle_mouse(&root, &motion_b);
    TEST_EQ(a.hovered, 0);
    TEST_EQ(b.hovered, 1);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_handle_mouse_null)
{
    ZephioWidget root;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    TEST_EQ(zephio_widget_handle_mouse(&root, NULL), 0);
    TEST_EQ(zephio_widget_handle_mouse(NULL, NULL), 0);
    zephio_widget_destroy(&root);
}

/* ── Theme Propagation ──────────────────────────────────────────── */

TEST_BEGIN(widget_set_theme)
{
    ZephioWidget root, child, grandchild;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&child, 0, 0, 40, 12, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&grandchild, 0, 0, 20, 6, NULL, &g_test_ctx, NULL);

    zephio_widget_add_child(&root, &child);
    zephio_widget_add_child(&child, &grandchild);

    ZephioTheme theme = zephio_theme_default();
    zephio_widget_set_theme(&root, &theme);

    TEST_ASSERT(root.theme == &theme);
    TEST_ASSERT(child.theme == &theme);
    TEST_ASSERT(grandchild.theme == &theme);
    TEST_EQ(root.dirty, 1);
    TEST_EQ(child.dirty, 1);
    TEST_EQ(grandchild.dirty, 1);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_set_theme_null)
{
    ZephioWidget w;
    zephio_widget_init_ctx(&w, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);
    zephio_widget_set_theme(NULL, NULL);
    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_get_style_normal)
{
    ZephioWidget w;
    zephio_widget_init_ctx(&w, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);
    ZephioTheme theme = zephio_theme_default();
    w.theme = &theme;

    ZephioStyle s = zephio_widget_get_style(&w);
    TEST_EQ(s.attr, theme.styles[ZEPHIO_STATE_NORMAL].attr);

    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_get_style_focused)
{
    ZephioWidget w;
    zephio_widget_init_ctx(&w, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);
    ZephioTheme theme = zephio_theme_default();
    w.theme = &theme;
    w.focused = 1;

    ZephioStyle s = zephio_widget_get_style(&w);
    TEST_EQ(s.attr, theme.styles[ZEPHIO_STATE_FOCUSED].attr);

    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_get_style_disabled)
{
    ZephioWidget w;
    zephio_widget_init_ctx(&w, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);
    ZephioTheme theme = zephio_theme_default();
    w.theme = &theme;
    w.disabled = 1;

    ZephioStyle s = zephio_widget_get_style(&w);
    TEST_EQ(s.attr, theme.styles[ZEPHIO_STATE_DISABLED].attr);

    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_get_style_no_theme)
{
    ZephioWidget w;
    zephio_widget_init_ctx(&w, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);
    ZephioStyle s = zephio_widget_get_style(&w);
    TEST_EQ(s.attr, ZEPHIO_STYLE_NONE.attr);
    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_get_style_null)
{
    ZephioStyle s = zephio_widget_get_style(NULL);
    TEST_EQ(s.attr, ZEPHIO_STYLE_NONE.attr);
}

/* ── Disabled ───────────────────────────────────────────────────── */

TEST_BEGIN(widget_set_disabled)
{
    ZephioWidget w;
    zephio_widget_init_ctx(&w, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);
    TEST_EQ(w.disabled, 0);

    zephio_widget_set_disabled(&w, 1);
    TEST_EQ(w.disabled, 1);
    TEST_EQ(w.dirty, 1);

    zephio_widget_set_disabled(&w, 0);
    TEST_EQ(w.disabled, 0);

    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_disabled_blurs_focused)
{
    ZephioWidget root, w;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&w, 0, 0, 40, 12, NULL, &g_test_ctx, NULL);
    w.focusable = 1;

    zephio_widget_add_child(&root, &w);
    zephio_widget_focus(&w);
    TEST_EQ(w.focused, 1);

    zephio_widget_set_disabled(&w, 1);
    TEST_EQ(w.focused, 0);
    TEST_EQ(w.disabled, 1);

    zephio_widget_destroy(&root);
}

/* ── Position & Size ────────────────────────────────────────────── */

TEST_BEGIN(widget_set_position)
{
    ZephioWidget w;
    zephio_widget_init_ctx(&w, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);
    w.dirty = 0;

    zephio_widget_set_position(&w, 5, 3);
    TEST_EQ(w.x, 5);
    TEST_EQ(w.y, 3);
    TEST_EQ(w.dirty, 1);

    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_set_size)
{
    ZephioWidget w;
    zephio_widget_init_ctx(&w, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);
    w.dirty = 0;

    zephio_widget_set_size(&w, 30, 15);
    TEST_EQ(w.width, 30);
    TEST_EQ(w.height, 15);
    TEST_EQ(w.dirty, 1);

    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_resize)
{
    g_resize_called = 0;

    ZephioWidget w;
    zephio_widget_init_ctx(&w, 0, 0, 10, 5, &g_resize_vt, &g_test_ctx, NULL);
    w.dirty = 0;

    zephio_widget_resize(&w, 20, 10);
    TEST_EQ(w.width, 20);
    TEST_EQ(w.height, 10);
    TEST_EQ(w.dirty, 1);
    TEST_EQ(g_resize_called, 1);

    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_resize_null)
{
    zephio_widget_resize(NULL, 10, 10);
}

/* ── Hovered ────────────────────────────────────────────────────── */

TEST_BEGIN(widget_set_hovered)
{
    ZephioWidget root, a, b;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&a, 0, 0, 40, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&b, 40, 0, 40, 24, NULL, &g_test_ctx, NULL);

    zephio_widget_add_child(&root, &a);
    zephio_widget_add_child(&root, &b);

    zephio_widget_set_hovered(&root, &a);
    TEST_EQ(a.hovered, 1);
    TEST_EQ(b.hovered, 0);

    zephio_widget_set_hovered(&root, &b);
    TEST_EQ(a.hovered, 0);
    TEST_EQ(b.hovered, 1);

    zephio_widget_set_hovered(&root, NULL);
    TEST_EQ(a.hovered, 0);
    TEST_EQ(b.hovered, 0);

    zephio_widget_destroy(&root);
}

TEST_BEGIN(widget_get_hovered)
{
    ZephioWidget root, a;
    zephio_widget_init_ctx(&root, 0, 0, 80, 24, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&a, 0, 0, 40, 24, NULL, &g_test_ctx, NULL);

    zephio_widget_add_child(&root, &a);

    TEST_ASSERT(zephio_widget_get_hovered(&root) == NULL);

    zephio_widget_set_hovered(&root, &a);
    TEST_ASSERT(zephio_widget_get_hovered(&root) == &a);

    zephio_widget_set_hovered(&root, NULL);
    TEST_ASSERT(zephio_widget_get_hovered(&root) == NULL);

    TEST_ASSERT(zephio_widget_get_hovered(NULL) == NULL);

    zephio_widget_destroy(&root);
}

/* ── Render ─────────────────────────────────────────────────────── */

TEST_BEGIN(widget_render_skips_hidden)
{
    ZephioWidget w;
    zephio_widget_init_ctx(&w, 0, 0, 10, 5, NULL, &g_test_ctx, NULL);
    w.visible = 0;
    w.dirty = 1;

    zephio_widget_render(&w);
    TEST_EQ(w.dirty, 1);

    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_render_clears_dirty)
{
    g_render_called = 0;

    ZephioWidget w;
    zephio_widget_init_ctx(&w, 0, 0, 10, 5, &g_render_vt, &g_test_ctx, NULL);

    zephio_widget_render(&w);
    TEST_EQ(w.dirty, 0);
    TEST_EQ(g_render_called, 1);

    zephio_widget_destroy(&w);
}

TEST_BEGIN(widget_render_children)
{
    g_child_rendered = 0;

    ZephioWidget parent, child;
    zephio_widget_init_ctx(&parent, 0, 0, 40, 20, NULL, &g_test_ctx, NULL);
    zephio_widget_init_ctx(&child, 2, 3, 10, 5, &g_child_vt, &g_test_ctx, NULL);

    zephio_widget_add_child(&parent, &child);

    zephio_widget_render(&parent);
    TEST_EQ(g_child_rendered, 1);
    TEST_EQ(child.dirty, 0);
    TEST_EQ(child.abs_x, 2);
    TEST_EQ(child.abs_y, 3);

    zephio_widget_destroy(&parent);
}

TEST_BEGIN(widget_render_null)
{
    zephio_widget_render(NULL);
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
