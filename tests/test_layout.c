#include "util.h"
#include "zephio_layout.h"
#include "zephio_widget.h"

/* ── Layout Init ────────────────────────────────────────────────── */

TEST_BEGIN(layout_init_vertical)
{
    ZephioLayout layout;
    ZephioResult res = zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(layout.direction, ZEPHIO_LAYOUT_VERTICAL);
    TEST_EQ(layout.base.width, 40);
    TEST_EQ(layout.base.height, 20);
    TEST_EQ(layout.padding, 0);
    TEST_EQ(layout.item_count, 0);
    TEST_EQ(layout.margin_top, 0);
    TEST_EQ(layout.margin_bottom, 0);
    TEST_EQ(layout.margin_left, 0);
    TEST_EQ(layout.margin_right, 0);
    zephio_widget_destroy(&layout.base);
}

TEST_BEGIN(layout_init_horizontal)
{
    ZephioLayout layout;
    ZephioResult res = zephio_layout_init(&layout, ZEPHIO_LAYOUT_HORIZONTAL, 5, 3, 60, 15);
    TEST_EQ(res, ZEPHIO_OK);
    TEST_EQ(layout.direction, ZEPHIO_LAYOUT_HORIZONTAL);
    TEST_EQ(layout.base.x, 5);
    TEST_EQ(layout.base.y, 3);
    zephio_widget_destroy(&layout.base);
}

TEST_BEGIN(layout_init_null)
{
    TEST_NE(zephio_layout_init(NULL, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20), ZEPHIO_OK);
}

TEST_BEGIN(layout_init_invalid_size)
{
    ZephioLayout layout;
    TEST_NE(zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 0, 20), ZEPHIO_OK);
    TEST_NE(zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 0), ZEPHIO_OK);
    TEST_NE(zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, -5, 20), ZEPHIO_OK);
}

/* ── Layout Add / Remove ────────────────────────────────────────── */

TEST_BEGIN(layout_add_fixed)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20);

    ZephioWidget a, b;
    zephio_widget_init(&a, 0, 0, 40, 5, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 40, 10, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FIXED(5));
    zephio_layout_add(&layout, &b, ZEPHIO_LAYOUT_FIXED(10));

    TEST_EQ(layout.item_count, 2);
    TEST_EQ(layout.base.child_count, 2);
    TEST_EQ(a.parent, &layout.base);
    TEST_EQ(b.parent, &layout.base);

    zephio_widget_destroy(&layout.base);
}

TEST_BEGIN(layout_add_null)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20);

    ZephioWidget w;
    zephio_widget_init(&w, 0, 0, 10, 5, NULL, NULL);

    TEST_NE(zephio_layout_add(NULL, &w, ZEPHIO_LAYOUT_FIXED(5)), ZEPHIO_OK);
    TEST_NE(zephio_layout_add(&layout, NULL, ZEPHIO_LAYOUT_FIXED(5)), ZEPHIO_OK);

    zephio_widget_destroy(&layout.base);
    zephio_widget_destroy(&w);
}

TEST_BEGIN(layout_add_duplicate)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20);

    ZephioWidget w;
    zephio_widget_init(&w, 0, 0, 10, 5, NULL, NULL);

    ZephioResult r1 = zephio_layout_add(&layout, &w, ZEPHIO_LAYOUT_FIXED(5));
    TEST_EQ(r1, ZEPHIO_OK);
    TEST_EQ(layout.item_count, 1);

    ZephioResult r2 = zephio_layout_add(&layout, &w, ZEPHIO_LAYOUT_FIXED(5));
    TEST_EQ(r2, ZEPHIO_OK);
    TEST_EQ(layout.item_count, 1);

    zephio_widget_destroy(&layout.base);
}

TEST_BEGIN(layout_remove)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20);

    ZephioWidget a, b, c;
    zephio_widget_init(&a, 0, 0, 40, 5, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 40, 5, NULL, NULL);
    zephio_widget_init(&c, 0, 0, 40, 5, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FIXED(5));
    zephio_layout_add(&layout, &b, ZEPHIO_LAYOUT_FIXED(5));
    zephio_layout_add(&layout, &c, ZEPHIO_LAYOUT_FIXED(5));

    zephio_layout_remove(&layout, &b);
    TEST_EQ(layout.item_count, 2);
    TEST_EQ(layout.base.child_count, 2);
    TEST_EQ(b.parent, (void *)NULL);
    TEST_EQ(layout.items[0].widget, &a);
    TEST_EQ(layout.items[1].widget, &c);

    zephio_widget_destroy(&layout.base);
    zephio_widget_destroy(&b);
}

TEST_BEGIN(layout_remove_all)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20);

    ZephioWidget a, b;
    zephio_widget_init(&a, 0, 0, 40, 5, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 40, 5, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FIXED(5));
    zephio_layout_add(&layout, &b, ZEPHIO_LAYOUT_FIXED(5));

    zephio_layout_remove_all(&layout);
    TEST_EQ(layout.item_count, 0);
    TEST_EQ(layout.base.child_count, 0);
    TEST_EQ(a.parent, (void *)NULL);
    TEST_EQ(b.parent, (void *)NULL);

    zephio_widget_destroy(&layout.base);
    zephio_widget_destroy(&a);
    zephio_widget_destroy(&b);
}

/* ── Layout Vertical Sizing ─────────────────────────────────────── */

TEST_BEGIN(layout_vertical_fixed)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20);

    ZephioWidget a, b;
    zephio_widget_init(&a, 0, 0, 40, 5, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 40, 10, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FIXED(8));
    zephio_layout_add(&layout, &b, ZEPHIO_LAYOUT_FIXED(12));

    TEST_EQ(a.y, 0);
    TEST_EQ(a.height, 8);
    TEST_EQ(a.width, 40);
    TEST_EQ(b.y, 8);
    TEST_EQ(b.height, 12);
    TEST_EQ(b.width, 40);

    zephio_widget_destroy(&layout.base);
}

TEST_BEGIN(layout_vertical_fill_equal)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20);

    ZephioWidget a, b;
    zephio_widget_init(&a, 0, 0, 40, 1, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 40, 1, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FILL);
    zephio_layout_add(&layout, &b, ZEPHIO_LAYOUT_FILL);

    TEST_EQ(a.y, 0);
    TEST_EQ(a.height, 10);
    TEST_EQ(b.y, 10);
    TEST_EQ(b.height, 10);
    TEST_EQ(a.width, 40);
    TEST_EQ(b.width, 40);

    zephio_widget_destroy(&layout.base);
}

TEST_BEGIN(layout_vertical_fill_weighted)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 30);

    ZephioWidget a, b;
    zephio_widget_init(&a, 0, 0, 40, 1, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 40, 1, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FILL_WEIGHT(2.0f));
    zephio_layout_add(&layout, &b, ZEPHIO_LAYOUT_FILL_WEIGHT(1.0f));

    TEST_EQ(a.y, 0);
    TEST_EQ(a.height, 20);
    TEST_EQ(b.y, 20);
    TEST_EQ(b.height, 10);

    zephio_widget_destroy(&layout.base);
}

TEST_BEGIN(layout_vertical_auto)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20);

    ZephioWidget a;
    zephio_widget_init(&a, 0, 0, 40, 7, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_AUTO);

    TEST_EQ(a.y, 0);
    TEST_EQ(a.height, 7);

    zephio_widget_destroy(&layout.base);
}

TEST_BEGIN(layout_vertical_mixed)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20);

    ZephioWidget fixed_w, auto_w, fill_w;
    zephio_widget_init(&fixed_w, 0, 0, 40, 1, NULL, NULL);
    zephio_widget_init(&auto_w, 0, 0, 40, 3, NULL, NULL);
    zephio_widget_init(&fill_w, 0, 0, 40, 1, NULL, NULL);

    zephio_layout_add(&layout, &fixed_w, ZEPHIO_LAYOUT_FIXED(5));
    zephio_layout_add(&layout, &auto_w, ZEPHIO_LAYOUT_AUTO);
    zephio_layout_add(&layout, &fill_w, ZEPHIO_LAYOUT_FILL);

    TEST_EQ(fixed_w.y, 0);
    TEST_EQ(fixed_w.height, 5);
    TEST_EQ(auto_w.y, 5);
    TEST_EQ(auto_w.height, 3);
    TEST_EQ(fill_w.y, 8);
    TEST_EQ(fill_w.height, 12);

    zephio_widget_destroy(&layout.base);
}

/* ── Layout Horizontal Sizing ───────────────────────────────────── */

TEST_BEGIN(layout_horizontal_fixed)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_HORIZONTAL, 0, 0, 80, 24);

    ZephioWidget a, b;
    zephio_widget_init(&a, 0, 0, 20, 24, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 40, 24, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FIXED(25));
    zephio_layout_add(&layout, &b, ZEPHIO_LAYOUT_FIXED(55));

    TEST_EQ(a.x, 0);
    TEST_EQ(a.width, 25);
    TEST_EQ(a.height, 24);
    TEST_EQ(b.x, 25);
    TEST_EQ(b.width, 55);
    TEST_EQ(b.height, 24);

    zephio_widget_destroy(&layout.base);
}

TEST_BEGIN(layout_horizontal_fill_equal)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_HORIZONTAL, 0, 0, 80, 24);

    ZephioWidget a, b, c;
    zephio_widget_init(&a, 0, 0, 1, 24, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 1, 24, NULL, NULL);
    zephio_widget_init(&c, 0, 0, 1, 24, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FILL);
    zephio_layout_add(&layout, &b, ZEPHIO_LAYOUT_FILL);
    zephio_layout_add(&layout, &c, ZEPHIO_LAYOUT_FILL);

    TEST_EQ(a.x, 0);
    TEST_EQ(a.width, 26);
    TEST_EQ(b.x, 26);
    TEST_EQ(b.width, 26);
    TEST_EQ(c.x, 52);
    TEST_EQ(c.width, 28);

    zephio_widget_destroy(&layout.base);
}

TEST_BEGIN(layout_horizontal_fill_weighted)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_HORIZONTAL, 0, 0, 90, 24);

    ZephioWidget a, b;
    zephio_widget_init(&a, 0, 0, 1, 24, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 1, 24, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FILL_WEIGHT(3.0f));
    zephio_layout_add(&layout, &b, ZEPHIO_LAYOUT_FILL_WEIGHT(1.0f));

    TEST_EQ(a.x, 0);
    TEST_EQ(a.height, 24);
    TEST_EQ(b.height, 24);

    TEST_EQ(a.width + b.width, 90);
    TEST_ASSERT(a.width > b.width);

    zephio_widget_destroy(&layout.base);
}

TEST_BEGIN(layout_horizontal_mixed)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_HORIZONTAL, 0, 0, 80, 24);

    ZephioWidget sidebar, main;
    zephio_widget_init(&sidebar, 0, 0, 20, 24, NULL, NULL);
    zephio_widget_init(&main, 0, 0, 60, 24, NULL, NULL);

    zephio_layout_add(&layout, &sidebar, ZEPHIO_LAYOUT_FIXED(20));
    zephio_layout_add(&layout, &main, ZEPHIO_LAYOUT_FILL);

    TEST_EQ(sidebar.x, 0);
    TEST_EQ(sidebar.width, 20);
    TEST_EQ(main.x, 20);
    TEST_EQ(main.width, 60);

    zephio_widget_destroy(&layout.base);
}

/* ── Padding ────────────────────────────────────────────────────── */

TEST_BEGIN(layout_padding_vertical)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20);
    zephio_layout_set_padding(&layout, 2);

    ZephioWidget a, b;
    zephio_widget_init(&a, 0, 0, 40, 1, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 40, 1, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FIXED(5));
    zephio_layout_add(&layout, &b, ZEPHIO_LAYOUT_FIXED(5));

    TEST_EQ(a.y, 0);
    TEST_EQ(b.y, 7);

    zephio_widget_destroy(&layout.base);
}

TEST_BEGIN(layout_padding_horizontal)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_HORIZONTAL, 0, 0, 80, 24);
    zephio_layout_set_padding(&layout, 5);

    ZephioWidget a, b;
    zephio_widget_init(&a, 0, 0, 1, 24, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 1, 24, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FIXED(30));
    zephio_layout_add(&layout, &b, ZEPHIO_LAYOUT_FIXED(30));

    TEST_EQ(a.x, 0);
    TEST_EQ(b.x, 35);

    zephio_widget_destroy(&layout.base);
}

/* ── Margin ─────────────────────────────────────────────────────── */

TEST_BEGIN(layout_margin_vertical)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20);
    zephio_layout_set_margin(&layout, 2, 1, 2, 1);

    ZephioWidget a;
    zephio_widget_init(&a, 0, 0, 40, 1, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FILL);

    TEST_EQ(a.y, 2);
    TEST_EQ(a.height, 16);
    TEST_EQ(a.x, 1);
    TEST_EQ(a.width, 38);

    zephio_widget_destroy(&layout.base);
}

TEST_BEGIN(layout_margin_horizontal)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_HORIZONTAL, 0, 0, 80, 24);
    zephio_layout_set_margin(&layout, 1, 3, 1, 3);

    ZephioWidget a;
    zephio_widget_init(&a, 0, 0, 1, 24, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FILL);

    TEST_EQ(a.x, 3);
    TEST_EQ(a.width, 74);
    TEST_EQ(a.y, 1);
    TEST_EQ(a.height, 22);

    zephio_widget_destroy(&layout.base);
}

/* ── Set Direction ──────────────────────────────────────────────── */

TEST_BEGIN(layout_set_direction)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20);

    ZephioWidget a, b;
    zephio_widget_init(&a, 0, 0, 40, 1, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 40, 1, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FIXED(10));
    zephio_layout_add(&layout, &b, ZEPHIO_LAYOUT_FIXED(10));

    TEST_EQ(a.y, 0);
    TEST_EQ(b.y, 10);

    zephio_layout_set_direction(&layout, ZEPHIO_LAYOUT_HORIZONTAL);
    TEST_EQ(layout.direction, ZEPHIO_LAYOUT_HORIZONTAL);
    TEST_EQ(a.x, 0);
    TEST_EQ(b.x, 10);
    TEST_EQ(a.height, 20);
    TEST_EQ(b.height, 20);

    zephio_widget_destroy(&layout.base);
}

/* ── Weight Rounding (last FILL gets remainder) ─────────────────── */

TEST_BEGIN(layout_fill_rounding_last_gets_remainder)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 10);

    ZephioWidget a, b, c;
    zephio_widget_init(&a, 0, 0, 40, 1, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 40, 1, NULL, NULL);
    zephio_widget_init(&c, 0, 0, 40, 1, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FILL);
    zephio_layout_add(&layout, &b, ZEPHIO_LAYOUT_FILL);
    zephio_layout_add(&layout, &c, ZEPHIO_LAYOUT_FILL);

    int total = a.height + b.height + c.height;
    TEST_EQ(total, 10);

    zephio_widget_destroy(&layout.base);
}

TEST_BEGIN(layout_fill_rounding_weighted)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_HORIZONTAL, 0, 0, 100, 24);

    ZephioWidget a, b, c;
    zephio_widget_init(&a, 0, 0, 1, 24, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 1, 24, NULL, NULL);
    zephio_widget_init(&c, 0, 0, 1, 24, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FILL_WEIGHT(1.0f));
    zephio_layout_add(&layout, &b, ZEPHIO_LAYOUT_FILL_WEIGHT(1.0f));
    zephio_layout_add(&layout, &c, ZEPHIO_LAYOUT_FILL_WEIGHT(1.0f));

    int total = a.width + b.width + c.width;
    TEST_EQ(total, 100);

    zephio_widget_destroy(&layout.base);
}

TEST_BEGIN(layout_single_fill_takes_all)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 25);

    ZephioWidget w;
    zephio_widget_init(&w, 0, 0, 40, 1, NULL, NULL);

    zephio_layout_add(&layout, &w, ZEPHIO_LAYOUT_FILL);

    TEST_EQ(w.height, 25);
    TEST_EQ(w.width, 40);

    zephio_widget_destroy(&layout.base);
}

/* ── Dual-tracking sync ─────────────────────────────────────────── */

TEST_BEGIN(layout_items_children_sync)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20);

    ZephioWidget a, b, c;
    zephio_widget_init(&a, 0, 0, 40, 1, NULL, NULL);
    zephio_widget_init(&b, 0, 0, 40, 1, NULL, NULL);
    zephio_widget_init(&c, 0, 0, 40, 1, NULL, NULL);

    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FIXED(5));
    zephio_layout_add(&layout, &b, ZEPHIO_LAYOUT_FIXED(5));
    zephio_layout_add(&layout, &c, ZEPHIO_LAYOUT_FIXED(5));

    TEST_EQ(layout.item_count, layout.base.child_count);

    zephio_layout_remove(&layout, &b);
    TEST_EQ(layout.item_count, 2);
    TEST_EQ(layout.base.child_count, 2);

    for (int i = 0; i < layout.item_count; i++) {
        TEST_ASSERT(layout.items[i].widget == layout.base.children[i]);
    }

    zephio_widget_destroy(&layout.base);
    zephio_widget_destroy(&b);
}

/* ── Layout destroy ─────────────────────────────────────────────── */

TEST_BEGIN(layout_destroy)
{
    ZephioLayout layout;
    zephio_layout_init(&layout, ZEPHIO_LAYOUT_VERTICAL, 0, 0, 40, 20);

    ZephioWidget a;
    zephio_widget_init(&a, 0, 0, 40, 1, NULL, NULL);
    zephio_layout_add(&layout, &a, ZEPHIO_LAYOUT_FIXED(5));

    zephio_widget_destroy(&layout.base);
    TEST_EQ(layout.items, (void *)NULL);
    TEST_EQ(layout.item_count, 0);
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void)
{
    fprintf(stderr, "Running layout tests...\n\n");

    TEST_RUN(layout_init_vertical);
    TEST_RUN(layout_init_horizontal);
    TEST_RUN(layout_init_null);
    TEST_RUN(layout_init_invalid_size);

    TEST_RUN(layout_add_fixed);
    TEST_RUN(layout_add_null);
    TEST_RUN(layout_add_duplicate);
    TEST_RUN(layout_remove);
    TEST_RUN(layout_remove_all);

    TEST_RUN(layout_vertical_fixed);
    TEST_RUN(layout_vertical_fill_equal);
    TEST_RUN(layout_vertical_fill_weighted);
    TEST_RUN(layout_vertical_auto);
    TEST_RUN(layout_vertical_mixed);

    TEST_RUN(layout_horizontal_fixed);
    TEST_RUN(layout_horizontal_fill_equal);
    TEST_RUN(layout_horizontal_fill_weighted);
    TEST_RUN(layout_horizontal_mixed);

    TEST_RUN(layout_padding_vertical);
    TEST_RUN(layout_padding_horizontal);

    TEST_RUN(layout_margin_vertical);
    TEST_RUN(layout_margin_horizontal);

    TEST_RUN(layout_set_direction);

    TEST_RUN(layout_fill_rounding_last_gets_remainder);
    TEST_RUN(layout_fill_rounding_weighted);
    TEST_RUN(layout_single_fill_takes_all);

    TEST_RUN(layout_items_children_sync);

    TEST_RUN(layout_destroy);

    TEST_SUMMARY();
}
