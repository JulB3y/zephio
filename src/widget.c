#define _POSIX_C_SOURCE 200809L

#include "tui_widget.h"

#include <stdlib.h>
#include <string.h>

#define CHILDREN_INITIAL_CAPACITY 8

TuiResult tui_widget_init(TuiWidget *widget, int x, int y, int width, int height,
                          TuiWidgetVTable *vtable, void *data)
{
    if (!widget) return TUI_ERR_MEMORY;
    if (width <= 0 || height <= 0) return TUI_ERR_MEMORY;

    memset(widget, 0, sizeof(*widget));

    widget->x      = x;
    widget->y      = y;
    widget->width  = width;
    widget->height = height;
    widget->abs_x  = x;
    widget->abs_y  = y;

    widget->visible   = 1;
    widget->dirty     = 1;
    widget->focusable = 0;
    widget->focused   = 0;
    widget->disabled  = 0;
    widget->hovered   = 0;
    widget->tab_index = 0;

    widget->parent           = NULL;
    widget->children         = NULL;
    widget->child_count      = 0;
    widget->child_capacity   = 0;
    widget->focused_child_idx = -1;

    widget->theme  = NULL;
    widget->vtable = vtable;
    widget->data   = data;

    return TUI_OK;
}

void tui_widget_destroy(TuiWidget *widget)
{
    if (!widget) return;

    for (int i = 0; i < widget->child_count; i++) {
        tui_widget_destroy(widget->children[i]);
    }

    free(widget->children);
    widget->children    = NULL;
    widget->child_count = 0;

    if (widget->vtable && widget->vtable->destroy) {
        widget->vtable->destroy(widget);
    }
}

TuiResult tui_widget_add_child(TuiWidget *parent, TuiWidget *child)
{
    if (!parent || !child) return TUI_ERR_MEMORY;

    if (parent->child_count >= parent->child_capacity) {
        int new_cap = parent->child_capacity == 0
            ? CHILDREN_INITIAL_CAPACITY
            : parent->child_capacity * 2;

        TuiWidget **new_children = (TuiWidget **)realloc(
            parent->children, (size_t)new_cap * sizeof(TuiWidget *));
        if (!new_children) return TUI_ERR_MEMORY;

        parent->children      = new_children;
        parent->child_capacity = new_cap;
    }

    child->parent = parent;
    child->abs_x  = parent->abs_x + child->x;
    child->abs_y  = parent->abs_y + child->y;

    if (child->tab_index == 0) {
        child->tab_index = parent->child_count;
    }

    parent->children[parent->child_count++] = child;
    parent->dirty = 1;

    return TUI_OK;
}

void tui_widget_remove_child(TuiWidget *parent, TuiWidget *child)
{
    if (!parent || !child) return;

    int idx = -1;
    for (int i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == child) {
            idx = i;
            break;
        }
    }

    if (idx < 0) return;

    if (child->focused) {
        tui_widget_blur(child);
    }

    if (parent->focused_child_idx == idx) {
        parent->focused_child_idx = -1;
    } else if (parent->focused_child_idx > idx) {
        parent->focused_child_idx--;
    }

    memmove(&parent->children[idx], &parent->children[idx + 1],
            (size_t)(parent->child_count - idx - 1) * sizeof(TuiWidget *));
    parent->child_count--;
    parent->dirty = 1;

    child->parent = NULL;
}

void tui_widget_remove_all_children(TuiWidget *parent)
{
    if (!parent) return;

    for (int i = 0; i < parent->child_count; i++) {
        TuiWidget *child = parent->children[i];
        if (child->focused) {
            tui_widget_blur(child);
        }
        child->parent = NULL;
    }

    parent->child_count       = 0;
    parent->focused_child_idx = -1;
    parent->dirty             = 1;
}

static void compute_absolute_position(TuiWidget *widget)
{
    if (widget->parent) {
        widget->abs_x = widget->parent->abs_x + widget->x;
        widget->abs_y = widget->parent->abs_y + widget->y;
    } else {
        widget->abs_x = widget->x;
        widget->abs_y = widget->y;
    }
}

void tui_widget_render(TuiWidget *widget)
{
    if (!widget || !widget->visible) return;

    compute_absolute_position(widget);

    if (widget->vtable && widget->vtable->render) {
        widget->vtable->render(widget);
    }
    widget->dirty = 0;

    for (int i = 0; i < widget->child_count; i++) {
        tui_widget_render(widget->children[i]);
    }
}

int tui_widget_handle_input(TuiWidget *widget, const TuiEvent *event)
{
    if (!widget || !widget->visible) return 0;

    if (widget->focused_child_idx >= 0 &&
        widget->focused_child_idx < widget->child_count) {
        TuiWidget *focused = widget->children[widget->focused_child_idx];
        int handled = tui_widget_handle_input(focused, event);
        if (handled) return 1;
    }

    if (widget->vtable && widget->vtable->handle_input) {
        return widget->vtable->handle_input(widget, event);
    }

    return 0;
}

/* Forward declarations — needed for auto-blur in tui_widget_focus() */
extern void tui_widget_blur(TuiWidget *widget);
extern TuiWidget *tui_widget_get_focused(TuiWidget *root);

void tui_widget_focus(TuiWidget *widget)
{
    if (!widget || widget->focused || !widget->focusable) return;

    /* Blur the previously focused widget (if any) so that only one
     * widget is focused at a time — required for mouse-click focus
     * changes where the old widget is not explicitly blurred first. */
    TuiWidget *root = widget;
    while (root->parent)
        root = root->parent;

    TuiWidget *old = tui_widget_get_focused(root);
    if (old && old != widget)
        tui_widget_blur(old);

    widget->focused = 1;
    widget->dirty   = 1;

    TuiWidget *current = widget;
    TuiWidget *parent  = current->parent;
    while (parent) {
        for (int i = 0; i < parent->child_count; i++) {
            if (parent->children[i] == current) {
                parent->focused_child_idx = i;
                break;
            }
        }
        current = parent;
        parent  = current->parent;
    }

    if (widget->vtable && widget->vtable->on_focus) {
        widget->vtable->on_focus(widget);
    }
}

void tui_widget_blur(TuiWidget *widget)
{
    if (!widget || !widget->focused) return;

    if (widget->focused_child_idx >= 0 &&
        widget->focused_child_idx < widget->child_count) {
        tui_widget_blur(widget->children[widget->focused_child_idx]);
    }

    widget->focused = 0;
    widget->dirty   = 1;

    TuiWidget *child  = widget;
    TuiWidget *parent = child->parent;
    while (parent) {
        if (parent->focused_child_idx >= 0 &&
            parent->focused_child_idx < parent->child_count &&
            parent->children[parent->focused_child_idx] == child) {
            parent->focused_child_idx = -1;
        }
        child  = parent;
        parent = child->parent;
    }

    if (widget->vtable && widget->vtable->on_blur) {
        widget->vtable->on_blur(widget);
    }
}

#define MAX_FOCUS_CHAIN 128

static int collect_focusable_dfs(TuiWidget *root, TuiWidget **out, int max_count)
{
    if (!root || !root->visible) return 0;
    int count = 0;
    for (int i = 0; i < root->child_count && count < max_count; i++) {
        TuiWidget *child = root->children[i];
        if (!child->visible) continue;
        if (child->focusable) {
            out[count++] = child;
        }
        count += collect_focusable_dfs(child, out + count, max_count - count);
    }
    return count;
}

void tui_widget_focus_next(TuiWidget *root)
{
    if (!root || root->child_count == 0) return;

    TuiWidget *chain[MAX_FOCUS_CHAIN];
    int count = collect_focusable_dfs(root, chain, MAX_FOCUS_CHAIN);
    if (count == 0) return;

    int current_idx = -1;
    for (int i = 0; i < count; i++) {
        if (chain[i]->focused) {
            current_idx = i;
            break;
        }
    }

    if (current_idx >= 0) {
        tui_widget_blur(chain[current_idx]);
    }

    int next_idx = (current_idx + 1) % count;
    tui_widget_focus(chain[next_idx]);
}

void tui_widget_focus_prev(TuiWidget *root)
{
    if (!root || root->child_count == 0) return;

    TuiWidget *chain[MAX_FOCUS_CHAIN];
    int count = collect_focusable_dfs(root, chain, MAX_FOCUS_CHAIN);
    if (count == 0) return;

    int current_idx = -1;
    for (int i = 0; i < count; i++) {
        if (chain[i]->focused) {
            current_idx = i;
            break;
        }
    }

    if (current_idx >= 0) {
        tui_widget_blur(chain[current_idx]);
    }

    int prev_idx = current_idx <= 0 ? count - 1 : current_idx - 1;
    tui_widget_focus(chain[prev_idx]);
}

TuiWidget *tui_widget_get_focused(TuiWidget *root)
{
    if (!root) return NULL;

    if (root->focused_child_idx >= 0 &&
        root->focused_child_idx < root->child_count) {
        return tui_widget_get_focused(root->children[root->focused_child_idx]);
    }

    return root->focused ? root : NULL;
}

void tui_widget_set_visible(TuiWidget *widget, int visible)
{
    if (!widget) return;
    if (widget->visible == visible) return;

    widget->visible = visible;
    widget->dirty   = 1;

    if (!visible && widget->focused) {
        tui_widget_blur(widget);
    }

    if (widget->parent) {
        widget->parent->dirty = 1;
    }
}

void tui_widget_set_dirty(TuiWidget *widget)
{
    if (!widget) return;
    widget->dirty = 1;
}

void tui_widget_mark_dirty_recursive(TuiWidget *widget)
{
    if (!widget) return;
    widget->dirty = 1;
    for (int i = 0; i < widget->child_count; i++) {
        tui_widget_mark_dirty_recursive(widget->children[i]);
    }
}

int tui_widget_is_dirty(TuiWidget *root)
{
    if (!root) return 0;
    if (root->dirty) return 1;
    for (int i = 0; i < root->child_count; i++) {
        if (tui_widget_is_dirty(root->children[i])) return 1;
    }
    return 0;
}

void tui_widget_set_position(TuiWidget *widget, int x, int y)
{
    if (!widget) return;
    widget->x = x;
    widget->y = y;
    widget->dirty = 1;
    tui_widget_mark_dirty_recursive(widget);
}

void tui_widget_set_size(TuiWidget *widget, int width, int height)
{
    if (!widget) return;
    widget->width  = width;
    widget->height = height;
    widget->dirty  = 1;
    tui_widget_mark_dirty_recursive(widget);
}

void tui_widget_resize(TuiWidget *widget, int width, int height)
{
    if (!widget) return;

    widget->width  = width;
    widget->height = height;
    widget->dirty  = 1;

    if (widget->vtable && widget->vtable->on_resize) {
        widget->vtable->on_resize(widget, width, height);
    }

    tui_widget_mark_dirty_recursive(widget);
}

int tui_widget_contains(TuiWidget *widget, int row, int col)
{
    if (!widget) return 0;
    compute_absolute_position(widget);
    return row >= widget->abs_y && row < widget->abs_y + widget->height &&
           col >= widget->abs_x && col < widget->abs_x + widget->width;
}

TuiWidget *tui_widget_find_at(TuiWidget *root, int row, int col)
{
    if (!root || !root->visible) return NULL;

    if (!tui_widget_contains(root, row, col)) return NULL;

    for (int i = root->child_count - 1; i >= 0; i--) {
        TuiWidget *found = tui_widget_find_at(root->children[i], row, col);
        if (found) return found;
    }

    return root;
}

void tui_widget_set_theme(TuiWidget *widget, const TuiTheme *theme)
{
    if (!widget) return;
    widget->theme = theme;
    widget->dirty = 1;
    for (int i = 0; i < widget->child_count; i++) {
        tui_widget_set_theme(widget->children[i], theme);
    }
}

TuiStyle tui_widget_get_style(TuiWidget *widget)
{
    if (!widget || !widget->theme) {
        return TUI_STYLE_NONE;
    }

    TuiWidgetState state = TUI_STATE_NORMAL;

    if (widget->disabled) {
        state = TUI_STATE_DISABLED;
    } else if (widget->focused) {
        state = TUI_STATE_FOCUSED;
    } else if (widget->hovered) {
        state = TUI_STATE_HOVER;
    }

    return widget->theme->styles[state];
}

void tui_widget_set_disabled(TuiWidget *widget, int disabled)
{
    if (!widget) return;
    if (widget->disabled == disabled) return;

    widget->disabled = disabled;
    widget->dirty = 1;

    if (disabled && widget->focused) {
        tui_widget_blur(widget);
    }
}

static void set_hovered_recursive(TuiWidget *widget, int hovered)
{
    if (!widget) return;
    if (widget->hovered != hovered) {
        widget->hovered = hovered;
        widget->dirty = 1;
    }
    for (int i = 0; i < widget->child_count; i++) {
        set_hovered_recursive(widget->children[i], hovered);
    }
}

static void clear_hover_tree(TuiWidget *root)
{
    if (!root) return;
    if (root->hovered) {
        root->hovered = 0;
        root->dirty = 1;
    }
    for (int i = 0; i < root->child_count; i++) {
        clear_hover_tree(root->children[i]);
    }
}

void tui_widget_set_hovered(TuiWidget *root, TuiWidget *widget)
{
    if (!root) return;
    clear_hover_tree(root);
    if (widget) {
        set_hovered_recursive(widget, 1);
    }
}

TuiWidget *tui_widget_get_hovered(TuiWidget *root)
{
    if (!root) return NULL;
    if (root->hovered && root->child_count == 0) return root;
    for (int i = 0; i < root->child_count; i++) {
        TuiWidget *found = tui_widget_get_hovered(root->children[i]);
        if (found) return found;
    }
    if (root->hovered) return root;
    return NULL;
}

int tui_widget_handle_mouse(TuiWidget *root, const TuiMouseEvent *mouse)
{
    if (!root || !mouse) return 0;

    TuiWidget *target = tui_widget_find_at(root, mouse->row, mouse->col);

    if (mouse->action == TUI_MOUSE_MOTION) {
        tui_widget_set_hovered(root, target);
    }

    if (!target) return 0;

    if (mouse->action == TUI_MOUSE_PRESS && mouse->button == TUI_MOUSE_BTN_LEFT) {
        if (target->focusable && !target->focused) {
            tui_widget_focus(target);
        }
    }

    TuiWidget *w = target;
    while (w) {
        if (w->vtable && w->vtable->handle_mouse) {
            int handled = w->vtable->handle_mouse(w, mouse);
            if (handled) return 1;
        }
        w = w->parent;
    }

    return 0;
}
