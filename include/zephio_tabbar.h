/**
 * @file zephio_tabbar.h
 * @brief Tab bar widget for switching between content panels.
 *
 * Renders a horizontal row of tabs with an active tab highlight.
 * Supports keyboard navigation (Left/Right arrows, Home/End),
 * mouse click selection, and an optional Ctrl+Tab shortcut.
 *
 * Each tab can hold an optional content widget pointer. When the
 * active tab changes, the callback receives the new tab index so
 * the application can show/hide the corresponding content panel.
 *
 * Usage:
 *   1. zephio_tabbar_init() with position and width.
 *   2. zephio_tabbar_add_tab() for each tab label.
 *   3. zephio_tabbar_set_on_change() for the selection callback.
 *   4. Add to widget tree via zephio_widget_add_child().
 */

#ifndef ZEPHIO_TABBAR_H
#define ZEPHIO_TABBAR_H

#include "zephio_widget.h"

#define ZEPHIO_TABBAR_INIT_CAP 8

typedef struct ZephioTabBar ZephioTabBar;
typedef struct ZephioTab    ZephioTab;

typedef void (*ZephioTabBarCallback)(ZephioTabBar *tabbar, int tab_index,
                                  const char *label, void *user_data);

struct ZephioTab {
    char     *label;
    ZephioWidget *content;
    int        start_x;
    int        label_width;
    int        tab_width;
};

struct ZephioTabBar {
    ZephioWidget base;

    ZephioTab  *tabs;
    int      tab_count;
    int      tab_capacity;
    int      active_tab;

    int scroll_offset;

    ZephioColor fg;
    ZephioColor bg;
    ZephioColor fg_active;
    ZephioColor bg_active;
    ZephioColor fg_hover;
    ZephioColor bg_hover;
    ZephioAttr  attr;

    ZephioTabBarCallback on_change;
    void             *user_data;
};

ZephioResult zephio_tabbar_init_ctx(ZephioTabBar *tabbar, ZephioContext *ctx, int x, int y, int width);

int zephio_tabbar_add_tab(ZephioTabBar *tabbar, const char *label, ZephioWidget *content);

void zephio_tabbar_remove_tab(ZephioTabBar *tabbar, int index);

int zephio_tabbar_get_active(const ZephioTabBar *tabbar);

ZephioWidget *zephio_tabbar_get_content(const ZephioTabBar *tabbar, int index);

void zephio_tabbar_set_active(ZephioTabBar *tabbar, int index);

void zephio_tabbar_set_tab_label(ZephioTabBar *tabbar, int index, const char *label);

void zephio_tabbar_set_tab_content(ZephioTabBar *tabbar, int index, ZephioWidget *content);

void zephio_tabbar_set_colors(ZephioTabBar *tabbar,
                           ZephioColor fg, ZephioColor bg,
                           ZephioColor fg_active, ZephioColor bg_active);

void zephio_tabbar_set_hover_colors(ZephioTabBar *tabbar,
                                 ZephioColor fg, ZephioColor bg);

void zephio_tabbar_set_on_change(ZephioTabBar *tabbar,
                              ZephioTabBarCallback callback,
                              void *user_data);

#endif
