/**
 * @file tui_tree_view.h
 * @brief Tree view widget with expand/collapse and box-drawing connectors.
 *
 * Displays hierarchical data as a collapsible tree. Each node can have
 * children and may be expanded or collapsed. Connector characters
 * (Unicode box-drawing: │├└─) visualize the tree structure.
 *
 * Keyboard: Up/Down to navigate, Enter/Right to expand, Left to collapse.
 * Mouse: click to select, double-click to toggle expand/collapse.
 */

#ifndef ZEPHIO_TREE_VIEW_H
#define ZEPHIO_TREE_VIEW_H

#include "tui_widget.h"

typedef struct TuiTreeNode TuiTreeNode;

typedef void (*TuiTreeViewCallback)(TuiWidget *widget, TuiTreeNode *node,
                                    void *user_data);

struct TuiTreeNode {
    char           *text;
    TuiTreeNode    *parent;
    TuiTreeNode   **children;
    int             child_count;
    int             child_capacity;
    int             expanded;
    void           *user_data;
};

typedef struct {
    TuiTreeNode     *node;
    int              depth;
    int              is_last;
    int             *last_at_depth;
    int              last_depth_capacity;
} TuiFlatEntry;

typedef struct {
    TuiWidget           base;

    TuiTreeNode        *root;
    TuiFlatEntry       *visible;
    int                 visible_count;
    int                 visible_capacity;

    int                 selected;
    int                 scroll_offset;

    TuiColor            fg;
    TuiColor            bg;
    TuiColor            fg_selected;
    TuiColor            bg_selected;
    TuiColor            fg_connector;
    TuiColor            bg_empty;

    TuiTreeViewCallback on_select;
    void               *user_data;
} TuiTreeView;

TuiResult tui_tree_view_init_ctx(TuiTreeView *tv, TuiContext *ctx, int x, int y, int width, int height);

TuiTreeNode *tui_tree_node_create(const char *text, void *user_data);

void tui_tree_node_destroy(TuiTreeNode *node);

TuiResult tui_tree_node_add_child(TuiTreeNode *parent, TuiTreeNode *child);

void tui_tree_view_set_root(TuiTreeView *tv, TuiTreeNode *root);

void tui_tree_view_expand(TuiTreeView *tv, TuiTreeNode *node);

void tui_tree_view_collapse(TuiTreeView *tv, TuiTreeNode *node);

void tui_tree_view_toggle(TuiTreeView *tv, TuiTreeNode *node);

void tui_tree_view_expand_all(TuiTreeView *tv);

void tui_tree_view_collapse_all(TuiTreeView *tv);

int tui_tree_view_get_selected(TuiTreeView *tv);

TuiTreeNode *tui_tree_view_get_selected_node(TuiTreeView *tv);

void tui_tree_view_set_colors(TuiTreeView *tv, TuiColor fg, TuiColor bg,
                              TuiColor fg_selected, TuiColor bg_selected,
                              TuiColor fg_connector);

void tui_tree_view_set_bg_empty(TuiTreeView *tv, TuiColor bg_empty);

void tui_tree_view_set_on_select(TuiTreeView *tv, TuiTreeViewCallback callback,
                                 void *user_data);

#endif
