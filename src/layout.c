#define _POSIX_C_SOURCE 200809L

#include "zephio_layout.h"

#include <stdlib.h>
#include <string.h>

#define ITEMS_INITIAL_CAPACITY 8

static void zephio_layout_recalculate(ZephioLayout *layout);

static void layout_on_resize(ZephioWidget *widget, int width, int height);
static void layout_destroy(ZephioWidget *widget);

static ZephioWidgetVTable layout_vtable = {
    .render        = NULL,
    .handle_input  = NULL,
    .handle_mouse  = NULL,
    .destroy       = layout_destroy,
    .on_resize     = layout_on_resize,
    .on_focus      = NULL,
    .on_blur       = NULL
};

static void layout_on_resize(ZephioWidget *widget, int width, int height)
{
    ZephioLayout *layout = (ZephioLayout *)widget;
    (void)width;
    (void)height;
    zephio_layout_recalculate(layout);
}

static void layout_destroy(ZephioWidget *widget)
{
    ZephioLayout *layout = (ZephioLayout *)widget;

    free(layout->items);
    layout->items       = NULL;
    layout->item_count  = 0;
    layout->item_capacity = 0;
}

ZephioResult zephio_layout_init_ctx(ZephioLayout *layout, ZephioContext *ctx, ZephioLayoutDirection direction,
                              int x, int y, int width, int height)
{
    if (!layout) return TUI_ERR_MEMORY;
    if (width <= 0 || height <= 0) return TUI_ERR_MEMORY;

    ZephioResult res = zephio_widget_init_ctx(&layout->base, x, y, width, height,
                                        &layout_vtable, ctx, NULL);
    if (res != ZEPHIO_OK) return res;

    layout->direction     = direction;
    layout->items         = NULL;
    layout->item_count    = 0;
    layout->item_capacity = 0;
    layout->padding       = 0;
    layout->margin_top    = 0;
    layout->margin_bottom = 0;
    layout->margin_left   = 0;
    layout->margin_right  = 0;

    return ZEPHIO_OK;
}

static int find_item_index(ZephioLayout *layout, ZephioWidget *child)
{
    for (int i = 0; i < layout->item_count; i++) {
        if (layout->items[i].widget == child) return i;
    }
    return -1;
}

ZephioResult zephio_layout_add(ZephioLayout *layout, ZephioWidget *child,
                         ZephioLayoutConstraints constraints)
{
    if (!layout || !child) return TUI_ERR_MEMORY;

    if (find_item_index(layout, child) >= 0) return ZEPHIO_OK;

    if (layout->item_count >= layout->item_capacity) {
        int new_cap = layout->item_capacity == 0
            ? ITEMS_INITIAL_CAPACITY
            : layout->item_capacity * 2;

        ZephioLayoutItem *new_items = (ZephioLayoutItem *)realloc(
            layout->items, (size_t)new_cap * sizeof(ZephioLayoutItem));
        if (!new_items) return TUI_ERR_MEMORY;

        layout->items       = new_items;
        layout->item_capacity = new_cap;
    }

    ZephioResult res = zephio_widget_add_child(&layout->base, child);
    if (res != ZEPHIO_OK) return res;

    layout->items[layout->item_count].widget      = child;
    layout->items[layout->item_count].constraints  = constraints;
    layout->item_count++;

    zephio_layout_recalculate(layout);

    return ZEPHIO_OK;
}

void zephio_layout_remove(ZephioLayout *layout, ZephioWidget *child)
{
    if (!layout || !child) return;

    int idx = find_item_index(layout, child);
    if (idx < 0) return;

    memmove(&layout->items[idx], &layout->items[idx + 1],
            (size_t)(layout->item_count - idx - 1) * sizeof(ZephioLayoutItem));
    layout->item_count--;

    zephio_widget_remove_child(&layout->base, child);

    zephio_layout_recalculate(layout);
}

void zephio_layout_remove_all(ZephioLayout *layout)
{
    if (!layout) return;

    layout->item_count = 0;

    zephio_widget_remove_all_children(&layout->base);
}

static int clamp_size(int size, int min_size, int max_size)
{
    if (min_size > 0 && size < min_size) size = min_size;
    if (max_size > 0 && size > max_size) size = max_size;
    return size;
}

static void zephio_layout_recalculate(ZephioLayout *layout)
{
    if (!layout) return;

    int total_width  = layout->base.width  - layout->margin_left - layout->margin_right;
    int total_height = layout->base.height - layout->margin_top  - layout->margin_bottom;
    int count = layout->item_count;

    if (count == 0) return;

    int is_vertical = (layout->direction == ZEPHIO_LAYOUT_VERTICAL);
    int available = is_vertical ? total_height : total_width;
    int cross_size = is_vertical ? total_width : total_height;

    if (available <= 0 || cross_size <= 0) return;

    int total_padding = (count > 1) ? layout->padding * (count - 1) : 0;
    available -= total_padding;
    if (available < 0) available = 0;

    int *sizes = (int *)calloc((size_t)count, sizeof(int));
    if (!sizes) return;

    int remaining = available;
    float total_weight = 0.0f;

    for (int i = 0; i < count; i++) {
        ZephioLayoutConstraints *c = &layout->items[i].constraints;
        ZephioWidget *w = layout->items[i].widget;

        switch (c->size_type) {
        case ZEPHIO_LAYOUT_SIZE_FIXED:
            sizes[i] = clamp_size(c->fixed_size, c->min_size, c->max_size);
            remaining -= sizes[i];
            break;

        case ZEPHIO_LAYOUT_SIZE_AUTO:
            sizes[i] = clamp_size(
                is_vertical ? w->height : w->width,
                c->min_size, c->max_size);
            remaining -= sizes[i];
            break;

        case ZEPHIO_LAYOUT_SIZE_FILL:
            total_weight += c->weight;
            break;
        }

        if (remaining < 0) remaining = 0;
    }

    if (total_weight > 0.0f && remaining > 0) {
        int last_fill = -1;
        int distributed = 0;

        for (int i = 0; i < count; i++) {
            if (layout->items[i].constraints.size_type != ZEPHIO_LAYOUT_SIZE_FILL)
                continue;

            last_fill = i;
            int size = (int)((float)remaining * layout->items[i].constraints.weight
                             / total_weight);
            sizes[i] = clamp_size(size,
                                  layout->items[i].constraints.min_size,
                                  layout->items[i].constraints.max_size);
            distributed += sizes[i];
        }

        if (last_fill >= 0) {
            int total_distributed = 0;
            for (int i = 0; i < count; i++) total_distributed += sizes[i];
            total_distributed += total_padding;
            int gap = (is_vertical ? total_height : total_width) - total_distributed;
            if (gap > 0) {
                sizes[last_fill] += gap;
            }
        }
    }

    int pos = is_vertical ? layout->margin_top : layout->margin_left;
    int cross_pos = is_vertical ? layout->margin_left : layout->margin_top;

    for (int i = 0; i < count; i++) {
        ZephioWidget *child = layout->items[i].widget;

        int main_size = sizes[i];
        if (main_size < 0) main_size = 0;

        if (is_vertical) {
            if (pos + main_size > total_height + layout->margin_top) break;
            zephio_widget_set_position(child, cross_pos, pos);
            zephio_widget_set_size(child, cross_size, main_size);
        } else {
            if (pos + main_size > total_width + layout->margin_left) break;
            zephio_widget_set_position(child, pos, cross_pos);
            zephio_widget_set_size(child, main_size, cross_size);
        }

        if (child->vtable && child->vtable->on_resize) {
            child->vtable->on_resize(child, child->width, child->height);
        }

        pos += sizes[i] + layout->padding;
    }

    free(sizes);

    layout->base.dirty = 1;
    zephio_widget_mark_dirty_recursive(&layout->base);
}

void zephio_layout_set_direction(ZephioLayout *layout, ZephioLayoutDirection direction)
{
    if (!layout) return;
    layout->direction = direction;
    zephio_layout_recalculate(layout);
}

void zephio_layout_set_padding(ZephioLayout *layout, int padding)
{
    if (!layout) return;
    layout->padding = padding;
    zephio_layout_recalculate(layout);
}

void zephio_layout_set_margin(ZephioLayout *layout, int top, int right, int bottom, int left)
{
    if (!layout) return;
    layout->margin_top    = top;
    layout->margin_right  = right;
    layout->margin_bottom = bottom;
    layout->margin_left   = left;
    zephio_layout_recalculate(layout);
}
