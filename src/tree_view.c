#define _POSIX_C_SOURCE 200809L

#include "tui_tree_view.h"
#include "tui_context.h"
#include "tui_screen.h"

#include <stdlib.h>
#include <string.h>

#define INITIAL_CAP 16

#define UTF8_PIPE     "\xe2\x94\x82"
#define UTF8_TEE      "\xe2\x94\x9c"
#define UTF8_CORNER   "\xe2\x94\x94"
#define UTF8_HORIZ    "\xe2\x94\x80"
#define UTF8_EXPAND   "+"
#define UTF8_COLLAPSE "-"

static void flatten_recursive(TuiTreeView *tv, TuiTreeNode *node,
                              int depth, int is_last,
                              int *last_at_depth, int depth_cap)
{
    if (tv->visible_count >= tv->visible_capacity) {
        int new_cap = tv->visible_capacity == 0
            ? INITIAL_CAP : tv->visible_capacity * 2;
        TuiFlatEntry *nv = (TuiFlatEntry *)realloc(
            tv->visible, (size_t)new_cap * sizeof(TuiFlatEntry));
        if (!nv) return;
        tv->visible          = nv;
        tv->visible_capacity = new_cap;
    }

    TuiFlatEntry *entry = &tv->visible[tv->visible_count];

    entry->node    = node;
    entry->depth   = depth;
    entry->is_last = is_last;

    if (depth + 1 > depth_cap) {
        int new_dc = depth_cap == 0 ? 16 : depth_cap * 2;
        while (new_dc <= depth + 1) new_dc *= 2;
        int *nd = (int *)realloc(last_at_depth, (size_t)new_dc * sizeof(int));
        if (!nd) return;
        for (int i = depth_cap; i < new_dc; i++) nd[i] = 0;
        last_at_depth = nd;
        depth_cap = new_dc;
    }

    last_at_depth[depth] = is_last;

    entry->last_at_depth     = last_at_depth;
    entry->last_depth_capacity = depth_cap;

    tv->visible_count++;

    if (node->expanded && node->child_count > 0) {
        for (int i = 0; i < node->child_count; i++) {
            flatten_recursive(tv, node->children[i], depth + 1,
                              i == node->child_count - 1,
                              last_at_depth, depth_cap);
        }
    }
}

static void rebuild_visible(TuiTreeView *tv)
{
    tv->visible_count = 0;

    if (!tv->root) return;

    int *ld = (int *)calloc(16, sizeof(int));
    if (!ld) return;

    for (int i = 0; i < tv->root->child_count; i++) {
        flatten_recursive(tv, tv->root->children[i], 0,
                          i == tv->root->child_count - 1, ld, 16);
    }
}

static void ensure_visible(TuiTreeView *tv)
{
    if (tv->selected < tv->scroll_offset) {
        tv->scroll_offset = tv->selected;
    }
    if (tv->selected >= tv->scroll_offset + tv->base.height) {
        tv->scroll_offset = tv->selected - tv->base.height + 1;
    }
}

static void rebuild_clamp(TuiTreeView *tv)
{
    rebuild_visible(tv);
    if (tv->selected >= tv->visible_count)
        tv->selected = tv->visible_count - 1;
    if (tv->selected < 0) tv->selected = 0;
    ensure_visible(tv);
    tv->base.dirty = 1;
}

static void render_scrollbar(TuiWidget *widget, int total, int offset)
{
    int wh = widget->height;
    int scroll_col = widget->abs_x + widget->width - 1;
    TuiColor track_fg = ZEPHIO_COLOR_INDEX(TUI_COLOR_GRAY_DARK);
    TuiColor track_bg = ZEPHIO_COLOR_INDEX(0);

    tui_screen_fill(widget->ctx, widget->abs_y, scroll_col, 1, wh, " ",
                    track_fg, track_bg, ZEPHIO_ATTR_DIM);

    int max_scroll = total - wh;
    if (max_scroll > 0) {
        int thumb_h = wh * wh / total;
        if (thumb_h < 1) thumb_h = 1;
        int thumb_y = offset * (wh - thumb_h) / max_scroll;
        for (int t = 0; t < thumb_h; t++) {
            tui_screen_set_cell(widget->ctx, widget->abs_y + thumb_y + t, scroll_col,
                                "\xe2\x96\x88",
                                ZEPHIO_COLOR_INDEX(TUI_COLOR_BRIGHT_WHITE),
                                ZEPHIO_COLOR_INDEX(TUI_COLOR_GRAY_MID),
                                ZEPHIO_ATTR_NONE);
        }
    }
}

static void tree_render(TuiWidget *widget)
{
    TuiTreeView *tv = (TuiTreeView *)widget;
    int wx = widget->abs_x;
    int wy = widget->abs_y;

    for (int i = 0; i < widget->height; i++) {
        int idx = tv->scroll_offset + i;
        if (idx >= tv->visible_count) break;

        TuiFlatEntry *entry = &tv->visible[idx];

        TuiColor fg, bg;
        TuiAttr  attr;

        if (widget->theme) {
            TuiWidgetState state = TUI_STATE_NORMAL;
            if (widget->disabled)
                state = TUI_STATE_DISABLED;
            else if (idx == tv->selected && widget->focused)
                state = TUI_STATE_FOCUSED;
            TuiStyle s = widget->theme->styles[state];
            fg = s.fg; bg = s.bg; attr = s.attr;
        } else {
            fg = tv->fg; bg = tv->bg; attr = ZEPHIO_ATTR_NONE;
            if (idx == tv->selected && widget->focused) {
                fg = tv->fg_selected; bg = tv->bg_selected;
            }
        }

        tui_screen_fill(widget->ctx, wy + i, wx, widget->width, 1, " ", fg, bg, attr);

        int col = wx;

        TuiColor conn_fg = tv->fg_connector.type != TUI_COLOR_TYPE_NONE
            ? tv->fg_connector : fg;
        TuiColor conn_bg = bg;

        for (int d = 0; d < entry->depth; d++) {
            if (col >= wx + widget->width - 1) break;

            if (d < entry->depth - 1) {
                if (entry->last_at_depth && !entry->last_at_depth[d + 1]) {
                    tui_screen_write(widget->ctx, wy + i, col, UTF8_PIPE, conn_fg, conn_bg, attr);
                } else {
                    tui_screen_write(widget->ctx, wy + i, col, " ", conn_fg, conn_bg, attr);
                }
            } else {
                if (entry->is_last) {
                    tui_screen_write(widget->ctx, wy + i, col, UTF8_CORNER, conn_fg, conn_bg, attr);
                    if (col + 1 < wx + widget->width)
                        tui_screen_write(widget->ctx, wy + i, col + 1, UTF8_HORIZ, conn_fg, conn_bg, attr);
                } else {
                    tui_screen_write(widget->ctx, wy + i, col, UTF8_TEE, conn_fg, conn_bg, attr);
                    if (col + 1 < wx + widget->width)
                        tui_screen_write(widget->ctx, wy + i, col + 1, UTF8_HORIZ, conn_fg, conn_bg, attr);
                }
            }
            col += 2;
        }

        if (entry->node->child_count > 0) {
            if (col >= wx + widget->width) continue;
            const char *exp = entry->node->expanded ? UTF8_COLLAPSE : UTF8_EXPAND;
            tui_screen_write(widget->ctx, wy + i, col, exp, fg, bg, attr | ZEPHIO_ATTR_BOLD);
            col += 1;
        } else {
            col += 1;
        }

        col += 1;

        if (entry->node->text && col < wx + widget->width) {
            int max_w = wx + widget->width - col;
            int len = (int)strlen(entry->node->text);
            int w = len < max_w ? len : max_w;
            char buf[256];
            int cl = w < (int)sizeof(buf) - 1 ? w : (int)sizeof(buf) - 1;
            memcpy(buf, entry->node->text, (size_t)cl);
            buf[cl] = '\0';
            tui_screen_write(widget->ctx, wy + i, col, buf, fg, bg, attr);
        }
    }

    {
        int filled = tv->visible_count - tv->scroll_offset;
        if (filled < 0) filled = 0;
        int empty_start = wy + (filled > widget->height ? widget->height : filled);
        int empty_rows = wy + widget->height - empty_start;

        if (empty_rows > 0 && tv->bg_empty.type != TUI_COLOR_TYPE_NONE) {
            tui_screen_fill(widget->ctx, empty_start, wx, widget->width, empty_rows,
                            " ", tv->fg, tv->bg_empty, ZEPHIO_ATTR_NONE);
        }
    }

    if (tv->visible_count > widget->height && widget->height > 0) {
        render_scrollbar(widget, tv->visible_count, tv->scroll_offset);
    }
}

static int tree_handle_input(TuiWidget *widget, const TuiEvent *event)
{
    TuiTreeView *tv = (TuiTreeView *)widget;

    switch (event->key) {
    case TUI_KEY_UP:
        if (tv->selected > 0) {
            tv->selected--;
            ensure_visible(tv);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_DOWN:
        if (tv->selected < tv->visible_count - 1) {
            tv->selected++;
            ensure_visible(tv);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_PAGE_UP:
        if (tv->selected > 0) {
            tv->selected -= widget->height;
            if (tv->selected < 0) tv->selected = 0;
            ensure_visible(tv);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_PAGE_DOWN:
        if (tv->visible_count > 0) {
            tv->selected += widget->height;
            if (tv->selected >= tv->visible_count)
                tv->selected = tv->visible_count - 1;
            ensure_visible(tv);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_HOME:
        tv->selected = 0;
        tv->scroll_offset = 0;
        widget->dirty = 1;
        return 1;

    case TUI_KEY_END:
        if (tv->visible_count > 0) {
            tv->selected = tv->visible_count - 1;
            ensure_visible(tv);
            widget->dirty = 1;
        }
        return 1;

    case TUI_KEY_RIGHT:
    case TUI_KEY_ENTER:
        if (tv->selected >= 0 && tv->selected < tv->visible_count) {
            TuiTreeNode *node = tv->visible[tv->selected].node;
            if (node->child_count > 0) {
                tui_tree_view_toggle(tv, node);
                widget->dirty = 1;
            } else if (tv->on_select) {
                tv->on_select(widget, node, tv->user_data);
            }
        }
        return 1;

    case TUI_KEY_LEFT:
        if (tv->selected >= 0 && tv->selected < tv->visible_count) {
            TuiTreeNode *node = tv->visible[tv->selected].node;
            if (node->expanded) {
                tui_tree_view_collapse(tv, node);
                widget->dirty = 1;
            } else if (node->parent && node->parent != tv->root) {
                for (int i = 0; i < tv->visible_count; i++) {
                    if (tv->visible[i].node == node->parent) {
                        tv->selected = i;
                        ensure_visible(tv);
                        break;
                    }
                }
                widget->dirty = 1;
            }
        }
        return 1;

    default:
        break;
    }

    return 0;
}

static int tree_handle_mouse(TuiWidget *widget, const TuiMouseEvent *mouse)
{
    TuiTreeView *tv = (TuiTreeView *)widget;

    if (mouse->action == TUI_MOUSE_WHEEL_UP) {
        if (tv->scroll_offset > 0) {
            tv->scroll_offset -= 3;
            if (tv->scroll_offset < 0) tv->scroll_offset = 0;
            widget->dirty = 1;
        }
        return 1;
    }

    if (mouse->action == TUI_MOUSE_WHEEL_DOWN) {
        int max_off = tv->visible_count - widget->height;
        if (max_off < 0) max_off = 0;
        if (tv->scroll_offset < max_off) {
            tv->scroll_offset += 3;
            if (tv->scroll_offset > max_off) tv->scroll_offset = max_off;
            widget->dirty = 1;
        }
        return 1;
    }

    if (mouse->action != TUI_MOUSE_PRESS || mouse->button != TUI_MOUSE_BTN_LEFT)
        return 0;

    int rel_row = mouse->row - widget->abs_y;
    if (rel_row < 0 || rel_row >= widget->height) return 0;

    int idx = tv->scroll_offset + rel_row;
    if (idx < 0 || idx >= tv->visible_count) return 0;

    tv->selected = idx;
    ensure_visible(tv);
    widget->dirty = 1;

    TuiTreeNode *node = tv->visible[idx].node;
    if (node->child_count > 0) {
        tui_tree_view_toggle(tv, node);
    } else if (tv->on_select) {
        tv->on_select(widget, node, tv->user_data);
    }

    return 1;
}

static void tree_destroy(TuiWidget *widget)
{
    TuiTreeView *tv = (TuiTreeView *)widget;

    if (tv->root) {
        tui_tree_node_destroy(tv->root);
        tv->root = NULL;
    }

    if (tv->visible && tv->visible_count > 0) {
        free(tv->visible[0].last_at_depth);
    }
    free(tv->visible);
    tv->visible         = NULL;
    tv->visible_count   = 0;
    tv->visible_capacity = 0;
}

static TuiWidgetVTable tree_vtable = {
    .render       = tree_render,
    .handle_input = tree_handle_input,
    .handle_mouse = tree_handle_mouse,
    .destroy      = tree_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_tree_view_init_ctx(TuiTreeView *tv, TuiContext *ctx, int x, int y, int width, int height)
{
    if (!tv) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init_ctx(&tv->base, x, y, width, height,
                                        &tree_vtable, ctx, NULL);
    if (res != TUI_OK) return res;

    tv->base.focusable = 1;

    tv->root            = NULL;
    tv->visible         = NULL;
    tv->visible_count   = 0;
    tv->visible_capacity = 0;
    tv->selected        = 0;
    tv->scroll_offset   = 0;
    tv->on_select       = NULL;
    tv->user_data       = NULL;

    tv->fg              = ZEPHIO_COLOR_INDEX(15);
    tv->bg              = ZEPHIO_COLOR_INDEX(0);
    tv->fg_selected     = ZEPHIO_COLOR_INDEX(0);
    tv->bg_selected     = ZEPHIO_COLOR_INDEX(12);
    tv->fg_connector    = ZEPHIO_COLOR_INDEX(TUI_COLOR_BRIGHT_BLACK);
    tv->bg_empty        = ZEPHIO_COLOR_NONE;

    return TUI_OK;
}

TuiTreeNode *tui_tree_node_create(const char *text, void *user_data)
{
    TuiTreeNode *node = (TuiTreeNode *)calloc(1, sizeof(TuiTreeNode));
    if (!node) return NULL;

    node->text      = text ? strdup(text) : NULL;
    node->user_data = user_data;
    node->parent    = NULL;
    node->children  = NULL;
    node->child_count    = 0;
    node->child_capacity = 0;
    node->expanded  = 0;

    return node;
}

void tui_tree_node_destroy(TuiTreeNode *node)
{
    if (!node) return;

    for (int i = 0; i < node->child_count; i++)
        tui_tree_node_destroy(node->children[i]);

    free(node->children);
    free(node->text);
    free(node);
}

TuiResult tui_tree_node_add_child(TuiTreeNode *parent, TuiTreeNode *child)
{
    if (!parent || !child) return TUI_ERR_MEMORY;

    if (parent->child_count >= parent->child_capacity) {
        int new_cap = parent->child_capacity == 0
            ? INITIAL_CAP : parent->child_capacity * 2;
        TuiTreeNode **nc = (TuiTreeNode **)realloc(
            parent->children, (size_t)new_cap * sizeof(TuiTreeNode *));
        if (!nc) return TUI_ERR_MEMORY;
        parent->children      = nc;
        parent->child_capacity = new_cap;
    }

    parent->children[parent->child_count] = child;
    parent->child_count++;
    child->parent = parent;

    return TUI_OK;
}

void tui_tree_view_set_root(TuiTreeView *tv, TuiTreeNode *root)
{
    if (!tv) return;
    tv->root = root;
    rebuild_visible(tv);
    tv->selected      = 0;
    tv->scroll_offset = 0;
    tv->base.dirty    = 1;
}

void tui_tree_view_expand(TuiTreeView *tv, TuiTreeNode *node)
{
    if (!tv || !node) return;
    node->expanded = 1;
    rebuild_clamp(tv);
}

void tui_tree_view_collapse(TuiTreeView *tv, TuiTreeNode *node)
{
    if (!tv || !node) return;
    node->expanded = 0;
    rebuild_clamp(tv);
}

void tui_tree_view_toggle(TuiTreeView *tv, TuiTreeNode *node)
{
    if (!tv || !node) return;
    if (node->expanded)
        tui_tree_view_collapse(tv, node);
    else
        tui_tree_view_expand(tv, node);
}

static void expand_recursive(TuiTreeNode *node)
{
    if (!node) return;
    if (node->child_count > 0) node->expanded = 1;
    for (int i = 0; i < node->child_count; i++)
        expand_recursive(node->children[i]);
}

static void collapse_recursive(TuiTreeNode *node)
{
    if (!node) return;
    node->expanded = 0;
    for (int i = 0; i < node->child_count; i++)
        collapse_recursive(node->children[i]);
}

void tui_tree_view_expand_all(TuiTreeView *tv)
{
    if (!tv || !tv->root) return;
    expand_recursive(tv->root);
    rebuild_clamp(tv);
}

void tui_tree_view_collapse_all(TuiTreeView *tv)
{
    if (!tv || !tv->root) return;
    collapse_recursive(tv->root);
    rebuild_visible(tv);
    tv->selected      = 0;
    tv->scroll_offset = 0;
    tv->base.dirty    = 1;
}

int tui_tree_view_get_selected(TuiTreeView *tv)
{
    if (!tv) return -1;
    return tv->selected;
}

TuiTreeNode *tui_tree_view_get_selected_node(TuiTreeView *tv)
{
    if (!tv || tv->selected < 0 || tv->selected >= tv->visible_count)
        return NULL;
    return tv->visible[tv->selected].node;
}

void tui_tree_view_set_colors(TuiTreeView *tv, TuiColor fg, TuiColor bg,
                              TuiColor fg_selected, TuiColor bg_selected,
                              TuiColor fg_connector)
{
    if (!tv) return;
    tv->fg            = fg;
    tv->bg            = bg;
    tv->fg_selected   = fg_selected;
    tv->bg_selected   = bg_selected;
    tv->fg_connector  = fg_connector;
    tv->base.dirty    = 1;
}

void tui_tree_view_set_on_select(TuiTreeView *tv, TuiTreeViewCallback callback,
                                 void *user_data)
{
    if (!tv) return;
    tv->on_select = callback;
    tv->user_data = user_data;
}

void tui_tree_view_set_bg_empty(TuiTreeView *tv, TuiColor bg_empty)
{
    if (!tv) return;
    tv->bg_empty   = bg_empty;
    tv->base.dirty = 1;
}
