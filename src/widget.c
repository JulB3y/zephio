#define _POSIX_C_SOURCE 200809L

#include "zephio_widget.h"
#include "zephio_context.h"

#include <stdlib.h>
#include <string.h>

#define CHILDREN_INITIAL_CAPACITY 8
#define MAX_FOCUS_CHAIN 128

ZephioResult zephio_widget_init_ctx(ZephioWidget *widget, int x, int y, int width, int height,
                              ZephioWidgetVTable *vtable, struct ZephioContext *ctx, void *data)
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

    widget->visible          = 1;
    widget->dirty            = 1;
    widget->focusable        = 0;
    widget->focused          = 0;
    widget->disabled         = 0;
    widget->hovered          = 0;
    widget->tab_index        = 0;
    widget->manages_children = 0;

    widget->parent           = NULL;
    widget->children         = NULL;
    widget->child_count      = 0;
    widget->child_capacity   = 0;
    widget->focused_child_idx = -1;

    widget->theme  = NULL;
    widget->vtable = vtable;
    widget->ctx    = ctx;
    widget->data   = data;

    return ZEPHIO_OK;
}

void zephio_widget_destroy(ZephioWidget *widget)
{
    if (!widget) return;

    for (int i = 0; i < widget->child_count; i++) {
        zephio_widget_destroy(widget->children[i]);
    }

    free(widget->children);
    widget->children    = NULL;
    widget->child_count = 0;

    if (widget->vtable && widget->vtable->destroy) {
        widget->vtable->destroy(widget);
    }
}

ZephioResult zephio_widget_add_child(ZephioWidget *parent, ZephioWidget *child)
{
    if (!parent || !child) return TUI_ERR_MEMORY;

    if (parent->child_count >= parent->child_capacity) {
        int new_cap = parent->child_capacity == 0
            ? CHILDREN_INITIAL_CAPACITY
            : parent->child_capacity * 2;

        ZephioWidget **new_children = (ZephioWidget **)realloc(
            parent->children, (size_t)new_cap * sizeof(ZephioWidget *));
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

    return ZEPHIO_OK;
}

void zephio_widget_remove_child(ZephioWidget *parent, ZephioWidget *child)
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
        zephio_widget_blur(child);
    }

    if (parent->focused_child_idx == idx) {
        parent->focused_child_idx = -1;
    } else if (parent->focused_child_idx > idx) {
        parent->focused_child_idx--;
    }

    memmove(&parent->children[idx], &parent->children[idx + 1],
            (size_t)(parent->child_count - idx - 1) * sizeof(ZephioWidget *));
    parent->child_count--;
    parent->dirty = 1;

    child->parent = NULL;
}

void zephio_widget_remove_all_children(ZephioWidget *parent)
{
    if (!parent) return;

    for (int i = 0; i < parent->child_count; i++) {
        ZephioWidget *child = parent->children[i];
        if (child->focused) {
            zephio_widget_blur(child);
        }
        child->parent = NULL;
    }

    parent->child_count       = 0;
    parent->focused_child_idx = -1;
    parent->dirty             = 1;
}

static void compute_absolute_position(ZephioWidget *widget)
{
    if (widget->parent) {
        if (widget->parent->manages_children) {
            return;
        }
        widget->abs_x = widget->parent->abs_x + widget->x;
        widget->abs_y = widget->parent->abs_y + widget->y;
    } else {
        widget->abs_x = widget->x;
        widget->abs_y = widget->y;
    }
}

void zephio_widget_render(ZephioWidget *widget)
{
    if (!widget || !widget->visible) return;

    compute_absolute_position(widget);

    if (widget->vtable && widget->vtable->render) {
        widget->vtable->render(widget);
    }
    widget->dirty = 0;

    if (!widget->manages_children) {
        for (int i = 0; i < widget->child_count; i++) {
            zephio_widget_render(widget->children[i]);
        }
    }
}

int zephio_widget_handle_input(ZephioWidget *widget, const ZephioEvent *event)
{
    if (!widget || !widget->visible) return 0;

    if (widget->focused_child_idx >= 0 &&
        widget->focused_child_idx < widget->child_count) {
        ZephioWidget *focused = widget->children[widget->focused_child_idx];
        int handled = zephio_widget_handle_input(focused, event);
        if (handled) return 1;
    }

    if (widget->vtable && widget->vtable->handle_input) {
        return widget->vtable->handle_input(widget, event);
    }

    return 0;
}

static void update_focused_child_chain(ZephioWidget *widget, int child_idx)
{
    ZephioWidget *current = widget;
    ZephioWidget *parent  = current->parent;
    while (parent) {
        for (int i = 0; i < parent->child_count; i++) {
            if (parent->children[i] == current) {
                parent->focused_child_idx = child_idx >= 0 ? i : -1;
                break;
            }
        }
        current = parent;
        parent  = current->parent;
    }
}

void zephio_widget_focus(ZephioWidget *widget)
{
    if (!widget || widget->focused || !widget->focusable) return;

    ZephioWidget *root = widget;
    while (root->parent)
        root = root->parent;

    ZephioWidget *old = zephio_widget_get_focused(root);
    if (old && old != widget)
        zephio_widget_blur(old);

    widget->focused = 1;
    widget->dirty   = 1;

    update_focused_child_chain(widget, 1);

    if (widget->vtable && widget->vtable->on_focus) {
        widget->vtable->on_focus(widget);
    }
}

void zephio_widget_blur(ZephioWidget *widget)
{
    if (!widget || !widget->focused) return;

    if (widget->focused_child_idx >= 0 &&
        widget->focused_child_idx < widget->child_count) {
        zephio_widget_blur(widget->children[widget->focused_child_idx]);
    }

    widget->focused = 0;
    widget->dirty   = 1;

    ZephioWidget *child  = widget;
    ZephioWidget *parent = child->parent;
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

static int collect_focusable_dfs(ZephioWidget *root, ZephioWidget **out, int max_count)
{
    if (!root || !root->visible) return 0;
    int count = 0;
    for (int i = 0; i < root->child_count && count < max_count; i++) {
        ZephioWidget *child = root->children[i];
        if (!child->visible) continue;
        if (child->focusable) {
            out[count++] = child;
        }
        count += collect_focusable_dfs(child, out + count, max_count - count);
    }
    return count;
}

static void focus_by_offset(ZephioWidget *root, int offset)
{
    if (!root || root->child_count == 0) return;

    ZephioWidget *chain[MAX_FOCUS_CHAIN];
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
        zephio_widget_blur(chain[current_idx]);
    }

    int target = (current_idx + offset + count) % count;
    zephio_widget_focus(chain[target]);
}

void zephio_widget_focus_next(ZephioWidget *root)
{
    focus_by_offset(root, 1);
}

void zephio_widget_focus_prev(ZephioWidget *root)
{
    focus_by_offset(root, -1);
}

ZephioWidget *zephio_widget_get_focused(ZephioWidget *root)
{
    if (!root) return NULL;

    if (root->focused_child_idx >= 0 &&
        root->focused_child_idx < root->child_count) {
        return zephio_widget_get_focused(root->children[root->focused_child_idx]);
    }

    return root->focused ? root : NULL;
}

void zephio_widget_set_visible(ZephioWidget *widget, int visible)
{
    if (!widget) return;
    if (widget->visible == visible) return;

    widget->visible = visible;
    widget->dirty   = 1;

    if (!visible && widget->focused) {
        zephio_widget_blur(widget);
    }

    if (widget->parent) {
        widget->parent->dirty = 1;
    }
}

void zephio_widget_set_dirty(ZephioWidget *widget)
{
    if (!widget) return;
    widget->dirty = 1;
}

void zephio_widget_mark_dirty_recursive(ZephioWidget *widget)
{
    if (!widget) return;
    widget->dirty = 1;
    for (int i = 0; i < widget->child_count; i++) {
        zephio_widget_mark_dirty_recursive(widget->children[i]);
    }
}

int zephio_widget_is_dirty(ZephioWidget *root)
{
    if (!root) return 0;
    if (root->dirty) return 1;
    for (int i = 0; i < root->child_count; i++) {
        if (zephio_widget_is_dirty(root->children[i])) return 1;
    }
    return 0;
}

void zephio_widget_set_position(ZephioWidget *widget, int x, int y)
{
    if (!widget) return;
    widget->x = x;
    widget->y = y;
    widget->dirty = 1;
    zephio_widget_mark_dirty_recursive(widget);
}

void zephio_widget_set_size(ZephioWidget *widget, int width, int height)
{
    if (!widget) return;
    widget->width  = width;
    widget->height = height;
    widget->dirty  = 1;
    zephio_widget_mark_dirty_recursive(widget);
}

void zephio_widget_resize(ZephioWidget *widget, int width, int height)
{
    if (!widget) return;

    widget->width  = width;
    widget->height = height;
    widget->dirty  = 1;

    if (widget->vtable && widget->vtable->on_resize) {
        widget->vtable->on_resize(widget, width, height);
    }

    zephio_widget_mark_dirty_recursive(widget);
}

int zephio_widget_contains(ZephioWidget *widget, int row, int col)
{
    if (!widget) return 0;
    compute_absolute_position(widget);
    return row >= widget->abs_y && row < widget->abs_y + widget->height &&
           col >= widget->abs_x && col < widget->abs_x + widget->width;
}

ZephioWidget *zephio_widget_find_at(ZephioWidget *root, int row, int col)
{
    if (!root || !root->visible) return NULL;

    if (!zephio_widget_contains(root, row, col)) return NULL;

    for (int i = root->child_count - 1; i >= 0; i--) {
        ZephioWidget *found = zephio_widget_find_at(root->children[i], row, col);
        if (found) return found;
    }

    return root;
}

void zephio_widget_set_theme(ZephioWidget *widget, const ZephioTheme *theme)
{
    if (!widget) return;
    widget->theme = theme;
    widget->dirty = 1;
    for (int i = 0; i < widget->child_count; i++) {
        zephio_widget_set_theme(widget->children[i], theme);
    }
}

ZephioStyle zephio_widget_get_style(ZephioWidget *widget)
{
    if (!widget || !widget->theme) {
        return ZEPHIO_STYLE_NONE;
    }

    ZephioWidgetState state = ZEPHIO_STATE_NORMAL;

    if (widget->disabled) {
        state = ZEPHIO_STATE_DISABLED;
    } else if (widget->focused) {
        state = ZEPHIO_STATE_FOCUSED;
    } else if (widget->hovered) {
        state = ZEPHIO_STATE_HOVER;
    }

    return widget->theme->styles[state];
}

void zephio_widget_set_disabled(ZephioWidget *widget, int disabled)
{
    if (!widget) return;
    if (widget->disabled == disabled) return;

    widget->disabled = disabled;
    widget->dirty = 1;

    if (disabled && widget->focused) {
        zephio_widget_blur(widget);
    }
}

static void set_hovered_recursive(ZephioWidget *widget, int hovered)
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

void zephio_widget_set_hovered(ZephioWidget *root, ZephioWidget *widget)
{
    if (!root) return;
    set_hovered_recursive(root, 0);
    if (widget) {
        set_hovered_recursive(widget, 1);
    }
}

ZephioWidget *zephio_widget_get_hovered(ZephioWidget *root)
{
    if (!root) return NULL;
    if (root->hovered && root->child_count == 0) return root;
    for (int i = 0; i < root->child_count; i++) {
        ZephioWidget *found = zephio_widget_get_hovered(root->children[i]);
        if (found) return found;
    }
    if (root->hovered) return root;
    return NULL;
}

int zephio_widget_handle_mouse(ZephioWidget *root, const ZephioMouseEvent *mouse)
{
    if (!root || !mouse) return 0;

    ZephioWidget *target = zephio_widget_find_at(root, mouse->row, mouse->col);

    if (mouse->action == ZEPHIO_MOUSE_MOTION) {
        zephio_widget_set_hovered(root, target);
    }

    if (!target) return 0;

    if (mouse->action == ZEPHIO_MOUSE_PRESS && mouse->button == ZEPHIO_MOUSE_BTN_LEFT) {
        if (target->focusable && !target->focused) {
            zephio_widget_focus(target);
        }
    }

    ZephioWidget *w = target;
    while (w) {
        if (w->vtable && w->vtable->handle_mouse) {
            int handled = w->vtable->handle_mouse(w, mouse);
            if (handled) return 1;
        }
        w = w->parent;
    }

    return 0;
}
