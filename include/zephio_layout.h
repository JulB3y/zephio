/**
 * @file zephio_layout.h
 * @brief Horizontal/vertical layout engine with flexible sizing.
 *
 * ZephioLayout is itself a widget (embeds ZephioWidget) that arranges its
 * children according to constraints. Each child is paired with a
 * ZephioLayoutConstraints value that controls its size:
 *
 *   - ZEPHIO_LAYOUT_FIXED(n):  exact size in columns/rows
 *   - ZEPHIO_LAYOUT_FILL:      share remaining space equally (weight 1.0)
 *   - ZEPHIO_LAYOUT_FILL_WEIGHT(w): share remaining space with weight w
 *   - ZEPHIO_LAYOUT_AUTO:      use the child's natural size
 *
 * Call zephio_layout_recalculate() after adding/removing children or
 * changing the layout size.
 */

#ifndef ZEPHIO_LAYOUT_H
#define ZEPHIO_LAYOUT_H

#include "zephio_widget.h"

typedef enum {
    ZEPHIO_LAYOUT_HORIZONTAL,
    ZEPHIO_LAYOUT_VERTICAL
} ZephioLayoutDirection;

typedef enum {
    ZEPHIO_LAYOUT_SIZE_FIXED,
    ZEPHIO_LAYOUT_SIZE_FILL,
    ZEPHIO_LAYOUT_SIZE_AUTO
} ZephioLayoutSizeType;

/**
 * @brief Sizing constraints for a layout child.
 */
typedef struct {
    ZephioLayoutSizeType size_type;
    int fixed_size;
    float weight;
    int min_size;
    int max_size;
} ZephioLayoutConstraints;

#define ZEPHIO_LAYOUT_FIXED(sz)       ((ZephioLayoutConstraints){ ZEPHIO_LAYOUT_SIZE_FIXED, (sz), 0.0f, 0, 0 })
#define ZEPHIO_LAYOUT_FILL            ((ZephioLayoutConstraints){ ZEPHIO_LAYOUT_SIZE_FILL,   0,  1.0f, 0, 0 })
#define ZEPHIO_LAYOUT_FILL_WEIGHT(w)  ((ZephioLayoutConstraints){ ZEPHIO_LAYOUT_SIZE_FILL,   0,  (w),  0, 0 })
#define ZEPHIO_LAYOUT_AUTO            ((ZephioLayoutConstraints){ ZEPHIO_LAYOUT_SIZE_AUTO,    0,  0.0f, 0, 0 })

typedef struct {
    ZephioWidget           *widget;
    ZephioLayoutConstraints constraints;
} ZephioLayoutItem;

/**
 * @brief A widget that arranges its children in a row or column.
 */
typedef struct {
    ZephioWidget           base;
    ZephioLayoutDirection  direction;
    ZephioLayoutItem      *items;
    int                 item_count;
    int                 item_capacity;
    int                 padding;
    int                 margin_top;
    int                 margin_bottom;
    int                 margin_left;
    int                 margin_right;
} ZephioLayout;

/**
 * @brief Initialize a layout widget with context.
 *
 * @param layout     Layout struct to initialize.
 * @param ctx        TUI context.
 * @param direction  ZEPHIO_LAYOUT_HORIZONTAL or ZEPHIO_LAYOUT_VERTICAL.
 * @param x          Column offset.
 * @param y          Row offset.
 * @param width      Layout width.
 * @param height     Layout height.
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_layout_init_ctx(ZephioLayout *layout, ZephioContext *ctx, ZephioLayoutDirection direction,
                              int x, int y, int width, int height);

/**
 * @brief Destroy the layout and free internal data.
 *
 * Intended to be used as the vtable's destroy function.
 */
void zephio_layout_destroy(ZephioWidget *widget);

/**
 * @brief Add a child widget with sizing constraints.
 *
 * The child is also added to the widget tree via zephio_widget_add_child().
 *
 * @param layout      Layout to add to.
 * @param child       Child widget.
 * @param constraints Sizing constraints.
 * @return ZEPHIO_OK on success, TUI_ERR_MEMORY on failure.
 */
ZephioResult zephio_layout_add(ZephioLayout *layout, ZephioWidget *child,
                         ZephioLayoutConstraints constraints);

/**
 * @brief Remove a child widget from the layout.
 */
void zephio_layout_remove(ZephioLayout *layout, ZephioWidget *child);

/**
 * @brief Remove all children from the layout.
 */
void zephio_layout_remove_all(ZephioLayout *layout);

/** @brief Change the layout direction (triggers recalculate). */
void zephio_layout_set_direction(ZephioLayout *layout, ZephioLayoutDirection direction);

/** @brief Set inner padding between children. */
void zephio_layout_set_padding(ZephioLayout *layout, int padding);

/** @brief Set outer margins. */
void zephio_layout_set_margin(ZephioLayout *layout, int top, int right, int bottom, int left);

#endif
