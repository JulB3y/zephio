/**
 * @file tui_widget.h
 * @brief Base widget type and widget tree operations.
 *
 * All widgets embed a TuiWidget as their first member (struct composition).
 * Polymorphism is achieved via a vtable of function pointers (TuiWidgetVTable).
 *
 * Widgets form a tree: each widget has a parent and a dynamic array of
 * children. The framework provides tree-wide operations for rendering,
 * focus management, mouse hit-testing, and dirty-flag propagation.
 *
 * ## Creating a custom widget
 *
 * 1. Define a struct with TuiWidget as the first member.
 * 2. Implement the relevant vtable functions (at minimum: render, destroy).
 * 3. Call tui_widget_init() in your widget's init function.
 * 4. Add the widget to a parent via tui_widget_add_child().
 */

#ifndef TUI_WIDGET_H
#define TUI_WIDGET_H

#include "tui.h"
#include "tui_input.h"
#include "tui_screen.h"
#include "tui_style.h"
#include <stdint.h>

typedef struct TuiWidget TuiWidget;

/**
 * @brief Virtual method table for widget polymorphism.
 *
 * Any function pointer may be NULL; the dispatch functions check
 * before calling.
 */
typedef struct {
    void (*render)(TuiWidget *widget);
    int  (*handle_input)(TuiWidget *widget, const TuiEvent *event);
    int  (*handle_mouse)(TuiWidget *widget, const TuiMouseEvent *mouse);
    void (*destroy)(TuiWidget *widget);
    void (*on_resize)(TuiWidget *widget, int width, int height);
    void (*on_focus)(TuiWidget *widget);
    void (*on_blur)(TuiWidget *widget);
} TuiWidgetVTable;

/**
 * @brief Base widget struct — embed as first member in all widget types.
 */
struct TuiWidget {
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

    TuiWidget  *parent;
    TuiWidget **children;
    int         child_count;
    int         child_capacity;
    int         focused_child_idx;

    const TuiTheme  *theme;
    TuiWidgetVTable *vtable;
    void            *data;
};

/**
 * @brief Initialize a base widget.
 *
 * Sets position, size, vtable, and user-data pointer. The widget starts
 * visible but not focused, not dirty.
 *
 * @param widget   Pointer to the widget struct.
 * @param x        Column offset relative to parent.
 * @param y        Row offset relative to parent.
 * @param width    Widget width in columns.
 * @param height   Widget height in rows.
 * @param vtable   Pointer to the widget's vtable (may be NULL).
 * @param data     Opaque user-data pointer (may be NULL).
 * @return TUI_OK on success, TUI_ERR_MEMORY on allocation failure.
 */
TuiResult tui_widget_init(TuiWidget *widget, int x, int y, int width, int height,
                          TuiWidgetVTable *vtable, void *data);

/**
 * @brief Destroy a widget and all its children.
 *
 * Calls the vtable's destroy function for each widget in the tree
 * (if non-NULL), then frees the children array. Does NOT free the
 * widget struct itself (the caller manages the widget's storage).
 */
void tui_widget_destroy(TuiWidget *widget);

/**
 * @brief Add a child widget to a parent.
 *
 * @param parent  Parent widget.
 * @param child   Child widget (its parent pointer is set).
 * @return TUI_OK on success, TUI_ERR_MEMORY on allocation failure.
 */
TuiResult tui_widget_add_child(TuiWidget *parent, TuiWidget *child);

/**
 * @brief Remove a child widget from its parent.
 *
 * Does not destroy the child.
 */
void tui_widget_remove_child(TuiWidget *parent, TuiWidget *child);

/**
 * @brief Remove all children from a widget.
 *
 * Does not destroy the children.
 */
void tui_widget_remove_all_children(TuiWidget *parent);

/**
 * @brief Render a widget and all its visible children (pre-order DFS).
 *
 * Calls vtable->render for each visible widget. Also recomputes
 * absolute positions from the parent chain.
 */
void tui_widget_render(TuiWidget *widget);

/**
 * @brief Dispatch an input event to the focused widget.
 *
 * @return 1 if the event was consumed, 0 otherwise.
 */
int tui_widget_handle_input(TuiWidget *widget, const TuiEvent *event);

/**
 * @brief Set focus on a widget (and blur the previously focused widget).
 */
void tui_widget_focus(TuiWidget *widget);

/**
 * @brief Remove focus from a widget.
 */
void tui_widget_blur(TuiWidget *widget);

/**
 * @brief Advance focus to the next focusable widget (DFS order).
 */
void tui_widget_focus_next(TuiWidget *root);

/**
 * @brief Move focus to the previous focusable widget (DFS order).
 */
void tui_widget_focus_prev(TuiWidget *root);

/**
 * @brief Return the currently focused widget in the tree, or NULL.
 */
TuiWidget *tui_widget_get_focused(TuiWidget *root);

/** @brief Show or hide a widget. Hidden widgets are skipped during render. */
void tui_widget_set_visible(TuiWidget *widget, int visible);

/** @brief Mark a widget as needing re-render. */
void tui_widget_set_dirty(TuiWidget *widget);

/** @brief Mark the entire subtree as dirty. */
void tui_widget_mark_dirty_recursive(TuiWidget *widget);

/** @brief Check if any widget in the tree is dirty. */
int tui_widget_is_dirty(TuiWidget *root);

/** @brief Update the widget's position relative to its parent. */
void tui_widget_set_position(TuiWidget *widget, int x, int y);

/** @brief Update the widget's dimensions. */
void tui_widget_set_size(TuiWidget *widget, int width, int height);

/**
 * @brief Resize a widget and notify it via vtable->on_resize.
 *
 * Also marks the widget as dirty.
 */
void tui_widget_resize(TuiWidget *widget, int width, int height);

/** @brief Test whether a (row, col) coordinate falls inside a widget. */
int tui_widget_contains(TuiWidget *widget, int row, int col);

/**
 * @brief Find the deepest visible widget at (row, col).
 *
 * @param root  Root of the widget tree.
 * @param row   Absolute row.
 * @param col   Absolute column.
 * @return The deepest widget at that position, or NULL.
 */
TuiWidget *tui_widget_find_at(TuiWidget *root, int row, int col);

/**
 * @brief Dispatch a mouse event to the widget at the mouse position.
 *
 * @return 1 if any widget consumed the event, 0 otherwise.
 */
int tui_widget_handle_mouse(TuiWidget *root, const TuiMouseEvent *mouse);

/** @brief Set the hovered widget in the tree. */
void tui_widget_set_hovered(TuiWidget *root, TuiWidget *widget);

/** @brief Return the currently hovered widget, or NULL. */
TuiWidget *tui_widget_get_hovered(TuiWidget *root);

/**
 * @brief Set the theme for a widget (propagates to all children).
 */
void tui_widget_set_theme(TuiWidget *widget, const TuiTheme *theme);

/**
 * @brief Get the effective style for a widget based on its current state.
 *
 * Considers: disabled, focused, hovered, and normal states.
 * Falls back to the widget's theme or the default theme.
 */
TuiStyle tui_widget_get_style(TuiWidget *widget);

/** @brief Enable or disable a widget. */
void tui_widget_set_disabled(TuiWidget *widget, int disabled);

#endif
