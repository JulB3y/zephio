/**
 * @file zephio_tree_view.h
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

#include "zephio_widget.h"

typedef struct ZephioTreeNode ZephioTreeNode;

typedef void (*ZephioTreeViewCallback)(ZephioWidget *widget, ZephioTreeNode *node,
                                    void *user_data);

struct ZephioTreeNode {
    char           *text;
    ZephioTreeNode    *parent;
    ZephioTreeNode   **children;
    int             child_count;
    int             child_capacity;
    int             expanded;
    void           *user_data;
};

typedef struct {
    ZephioTreeNode     *node;
    int              depth;
    int              is_last;
    int             *last_at_depth;
    int              last_depth_capacity;
} ZephioFlatEntry;

typedef struct {
    ZephioWidget           base;

    ZephioTreeNode        *root;
    ZephioFlatEntry       *visible;
    int                 visible_count;
    int                 visible_capacity;

    int                 selected;
    int                 scroll_offset;

    ZephioColor            fg;
    ZephioColor            bg;
    ZephioColor            fg_selected;
    ZephioColor            bg_selected;
    ZephioColor            fg_connector;
    ZephioColor            bg_empty;

    ZephioTreeViewCallback on_select;
    void               *user_data;
} ZephioTreeView;

ZephioResult zephio_tree_view_init_ctx(ZephioTreeView *tv, ZephioContext *ctx, int x, int y, int width, int height);

ZephioTreeNode *zephio_tree_node_create(const char *text, void *user_data);

void zephio_tree_node_destroy(ZephioTreeNode *node);

ZephioResult zephio_tree_node_add_child(ZephioTreeNode *parent, ZephioTreeNode *child);

void zephio_tree_view_set_root(ZephioTreeView *tv, ZephioTreeNode *root);

void zephio_tree_view_expand(ZephioTreeView *tv, ZephioTreeNode *node);

void zephio_tree_view_collapse(ZephioTreeView *tv, ZephioTreeNode *node);

void zephio_tree_view_toggle(ZephioTreeView *tv, ZephioTreeNode *node);

void zephio_tree_view_expand_all(ZephioTreeView *tv);

void zephio_tree_view_collapse_all(ZephioTreeView *tv);

int zephio_tree_view_get_selected(ZephioTreeView *tv);

ZephioTreeNode *zephio_tree_view_get_selected_node(ZephioTreeView *tv);

void zephio_tree_view_set_colors(ZephioTreeView *tv, ZephioColor fg, ZephioColor bg,
                              ZephioColor fg_selected, ZephioColor bg_selected,
                              ZephioColor fg_connector);

void zephio_tree_view_set_bg_empty(ZephioTreeView *tv, ZephioColor bg_empty);

void zephio_tree_view_set_on_select(ZephioTreeView *tv, ZephioTreeViewCallback callback,
                                 void *user_data);

#endif
