/**
 * @file tui_layout.h
 * @brief Horizontal/vertical layout engine with flexible sizing.
 *
 * TuiLayout is itself a widget (embeds TuiWidget) that arranges its
 * children according to constraints. Each child is paired with a
 * TuiLayoutConstraints value that controls its size:
 *
 *   - TUI_LAYOUT_FIXED(n):  exact size in columns/rows
 *   - TUI_LAYOUT_FILL:      share remaining space equally (weight 1.0)
 *   - TUI_LAYOUT_FILL_WEIGHT(w): share remaining space with weight w
 *   - TUI_LAYOUT_AUTO:      use the child's natural size
 *
 * Call tui_layout_recalculate() after adding/removing children or
 * changing the layout size.
 */

#ifndef TUI_LAYOUT_H
#define TUI_LAYOUT_H

#include "tui_widget.h"

typedef enum {
    TUI_LAYOUT_HORIZONTAL,
    TUI_LAYOUT_VERTICAL
} TuiLayoutDirection;

typedef enum {
    TUI_LAYOUT_SIZE_FIXED,
    TUI_LAYOUT_SIZE_FILL,
    TUI_LAYOUT_SIZE_AUTO
} TuiLayoutSizeType;

/**
 * @brief Sizing constraints for a layout child.
 */
typedef struct {
    TuiLayoutSizeType size_type;
    int fixed_size;
    float weight;
    int min_size;
    int max_size;
} TuiLayoutConstraints;

#define TUI_LAYOUT_FIXED(sz)       ((TuiLayoutConstraints){ TUI_LAYOUT_SIZE_FIXED, (sz), 0.0f, 0, 0 })
#define TUI_LAYOUT_FILL            ((TuiLayoutConstraints){ TUI_LAYOUT_SIZE_FILL,   0,  1.0f, 0, 0 })
#define TUI_LAYOUT_FILL_WEIGHT(w)  ((TuiLayoutConstraints){ TUI_LAYOUT_SIZE_FILL,   0,  (w),  0, 0 })
#define TUI_LAYOUT_AUTO            ((TuiLayoutConstraints){ TUI_LAYOUT_SIZE_AUTO,    0,  0.0f, 0, 0 })

typedef struct {
    TuiWidget           *widget;
    TuiLayoutConstraints constraints;
} TuiLayoutItem;

/**
 * @brief A widget that arranges its children in a row or column.
 */
typedef struct {
    TuiWidget           base;
    TuiLayoutDirection  direction;
    TuiLayoutItem      *items;
    int                 item_count;
    int                 item_capacity;
    int                 padding;
    int                 margin_top;
    int                 margin_bottom;
    int                 margin_left;
    int                 margin_right;
} TuiLayout;

/**
 * @brief Initialize a layout widget.
 *
 * @param layout     Layout struct to initialize.
 * @param direction  TUI_LAYOUT_HORIZONTAL or TUI_LAYOUT_VERTICAL.
 * @param x          Column offset.
 * @param y          Row offset.
 * @param width      Layout width.
 * @param height     Layout height.
 * @return TUI_OK on success.
 */
TuiResult tui_layout_init(TuiLayout *layout, TuiLayoutDirection direction,
                          int x, int y, int width, int height);

/**
 * @brief Destroy the layout and free internal data.
 *
 * Intended to be used as the vtable's destroy function.
 */
void tui_layout_destroy(TuiWidget *widget);

/**
 * @brief Add a child widget with sizing constraints.
 *
 * The child is also added to the widget tree via tui_widget_add_child().
 *
 * @param layout      Layout to add to.
 * @param child       Child widget.
 * @param constraints Sizing constraints.
 * @return TUI_OK on success, TUI_ERR_MEMORY on failure.
 */
TuiResult tui_layout_add(TuiLayout *layout, TuiWidget *child,
                         TuiLayoutConstraints constraints);

/**
 * @brief Remove a child widget from the layout.
 */
void tui_layout_remove(TuiLayout *layout, TuiWidget *child);

/**
 * @brief Remove all children from the layout.
 */
void tui_layout_remove_all(TuiLayout *layout);

/**
 * @brief Recompute child positions and sizes based on constraints.
 *
 * Should be called after adding/removing children or resizing.
 */
void tui_layout_recalculate(TuiLayout *layout);

/** @brief Change the layout direction (triggers recalculate). */
void tui_layout_set_direction(TuiLayout *layout, TuiLayoutDirection direction);

/** @brief Set inner padding between children. */
void tui_layout_set_padding(TuiLayout *layout, int padding);

/** @brief Set outer margins. */
void tui_layout_set_margin(TuiLayout *layout, int top, int right, int bottom, int left);

#endif
