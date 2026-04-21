/**
 * @file zephio_widget.h
 * @brief Base widget type and widget tree operations.
 *
 * All widgets embed a ZephioWidget as their first member (struct composition).
 * Polymorphism is achieved via a vtable of function pointers (ZephioWidgetVTable).
 *
 * Widgets form a tree: each widget has a parent and a dynamic array of
 * children. The framework provides tree-wide operations for rendering,
 * focus management, mouse hit-testing, and dirty-flag propagation.
 *
 * ## Creating a custom widget
 *
 * 1. Define a struct with ZephioWidget as the first member.
 * 2. Implement the relevant vtable functions (at minimum: render, destroy).
 * 3. Call zephio_widget_init() in your widget's init function.
 * 4. Add the widget to a parent via zephio_widget_add_child().
 */

#ifndef ZEPHIO_WIDGET_H
#define ZEPHIO_WIDGET_H

#include "tui.h"
#include "zephio_input.h"
#include "zephio_style.h"
#include <stdint.h>

typedef struct ZephioWidget ZephioWidget;

/**
 * @brief Virtual method table for widget polymorphism.
 *
 * Any function pointer may be NULL; the dispatch functions check
 * before calling.
 */
typedef struct {
    void (*render)(ZephioWidget *widget);
    int  (*handle_input)(ZephioWidget *widget, const ZephioEvent *event);
    int  (*handle_mouse)(ZephioWidget *widget, const ZephioMouseEvent *mouse);
    void (*destroy)(ZephioWidget *widget);
    void (*on_resize)(ZephioWidget *widget, int width, int height);
    void (*on_focus)(ZephioWidget *widget);
    void (*on_blur)(ZephioWidget *widget);
} ZephioWidgetVTable;

/**
 * @brief Base widget struct — embed as first member in all widget types.
 */
struct ZephioWidget {
    int x, y;
    int width, height;
    int abs_x, abs_y;

    struct {
        unsigned int visible          : 1;
        unsigned int dirty            : 1;
        unsigned int focusable        : 1;
        unsigned int focused          : 1;
        unsigned int disabled         : 1;
        unsigned int hovered          : 1;
        unsigned int manages_children : 1;
    };
    int16_t tab_index;

    ZephioWidget  *parent;
    ZephioWidget **children;
    int         child_count;
    int         child_capacity;
    int         focused_child_idx;

    const ZephioTheme  *theme;
    ZephioWidgetVTable *vtable;
    void            *data;
    struct ZephioContext *ctx;
};

/**
 * @brief Initialize a base widget with context.
 *
 * Sets position, size, vtable, context, and user-data pointer. The widget starts
 * visible but not focused, not dirty.
 *
 * @param widget   Pointer to the widget struct.
 * @param x        Column offset relative to parent.
 * @param y        Row offset relative to parent.
 * @param width    Widget width in columns.
 * @param height   Widget height in rows.
 * @param vtable   Pointer to the widget's vtable (may be NULL).
 * @param ctx      Pointer to the ZephioContext (may be NULL for backward compatibility).
 * @param data     Opaque user-data pointer (may be NULL).
 * @return ZEPHIO_OK on success, TUI_ERR_MEMORY on allocation failure.
 */
ZephioResult zephio_widget_init_ctx(ZephioWidget *widget, int x, int y, int width, int height,
                               ZephioWidgetVTable *vtable, struct ZephioContext *ctx, void *data);

/**
 * @brief Destroy a widget and all its children.
 *
 * Calls the vtable's destroy function for each widget in the tree
 * (if non-NULL), then frees the children array. Does NOT free the
 * widget struct itself (the caller manages the widget's storage).
 */
void zephio_widget_destroy(ZephioWidget *widget);

/**
 * @brief Add a child widget to a parent.
 *
 * @param parent  Parent widget.
 * @param child   Child widget (its parent pointer is set).
 * @return ZEPHIO_OK on success, TUI_ERR_MEMORY on allocation failure.
 */
ZephioResult zephio_widget_add_child(ZephioWidget *parent, ZephioWidget *child);

/**
 * @brief Remove a child widget from its parent.
 *
 * Does not destroy the child.
 */
void zephio_widget_remove_child(ZephioWidget *parent, ZephioWidget *child);

/**
 * @brief Remove all children from a widget.
 *
 * Does not destroy the children.
 */
void zephio_widget_remove_all_children(ZephioWidget *parent);

/**
 * @brief Render a widget and all its visible children (pre-order DFS).
 *
 * Calls vtable->render for each visible widget. Also recomputes
 * absolute positions from the parent chain.
 */
void zephio_widget_render(ZephioWidget *widget);

/**
 * @brief Dispatch an input event to the focused widget.
 *
 * @return 1 if the event was consumed, 0 otherwise.
 */
int zephio_widget_handle_input(ZephioWidget *widget, const ZephioEvent *event);

/**
 * @brief Set focus on a widget (and blur the previously focused widget).
 */
void zephio_widget_focus(ZephioWidget *widget);

/**
 * @brief Remove focus from a widget.
 */
void zephio_widget_blur(ZephioWidget *widget);

/**
 * @brief Advance focus to the next focusable widget (DFS order).
 */
void zephio_widget_focus_next(ZephioWidget *root);

/**
 * @brief Move focus to the previous focusable widget (DFS order).
 */
void zephio_widget_focus_prev(ZephioWidget *root);

/**
 * @brief Return the currently focused widget in the tree, or NULL.
 */
ZephioWidget *zephio_widget_get_focused(ZephioWidget *root);

/** @brief Show or hide a widget. Hidden widgets are skipped during render. */
void zephio_widget_set_visible(ZephioWidget *widget, int visible);

/** @brief Mark a widget as needing re-render. */
void zephio_widget_set_dirty(ZephioWidget *widget);

/** @brief Mark the entire subtree as dirty. */
void zephio_widget_mark_dirty_recursive(ZephioWidget *widget);

/** @brief Check if any widget in the tree is dirty. */
int zephio_widget_is_dirty(ZephioWidget *root);

/** @brief Update the widget's position relative to its parent. */
void zephio_widget_set_position(ZephioWidget *widget, int x, int y);

/** @brief Update the widget's dimensions. */
void zephio_widget_set_size(ZephioWidget *widget, int width, int height);

/**
 * @brief Resize a widget and notify it via vtable->on_resize.
 *
 * Also marks the widget as dirty.
 */
void zephio_widget_resize(ZephioWidget *widget, int width, int height);

/** @brief Test whether a (row, col) coordinate falls inside a widget. */
int zephio_widget_contains(ZephioWidget *widget, int row, int col);

/**
 * @brief Find the deepest visible widget at (row, col).
 *
 * @param root  Root of the widget tree.
 * @param row   Absolute row.
 * @param col   Absolute column.
 * @return The deepest widget at that position, or NULL.
 */
ZephioWidget *zephio_widget_find_at(ZephioWidget *root, int row, int col);

/**
 * @brief Dispatch a mouse event to the widget at the mouse position.
 *
 * @return 1 if any widget consumed the event, 0 otherwise.
 */
int zephio_widget_handle_mouse(ZephioWidget *root, const ZephioMouseEvent *mouse);

/** @brief Set the hovered widget in the tree. */
void zephio_widget_set_hovered(ZephioWidget *root, ZephioWidget *widget);

/** @brief Return the currently hovered widget, or NULL. */
ZephioWidget *zephio_widget_get_hovered(ZephioWidget *root);

/**
 * @brief Set the theme for a widget (propagates to all children).
 */
void zephio_widget_set_theme(ZephioWidget *widget, const ZephioTheme *theme);

/**
 * @brief Get the effective style for a widget based on its current state.
 *
 * Considers: disabled, focused, hovered, and normal states.
 * Falls back to the widget's theme or the default theme.
 */
ZephioStyle zephio_widget_get_style(ZephioWidget *widget);

/** @brief Enable or disable a widget. */
void zephio_widget_set_disabled(ZephioWidget *widget, int disabled);

#endif
