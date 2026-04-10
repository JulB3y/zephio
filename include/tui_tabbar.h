/**
 * @file tui_tabbar.h
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
 *   1. tui_tabbar_init() with position and width.
 *   2. tui_tabbar_add_tab() for each tab label.
 *   3. tui_tabbar_set_on_change() for the selection callback.
 *   4. Add to widget tree via tui_widget_add_child().
 */

#ifndef TUI_TABBAR_H
#define TUI_TABBAR_H

#include "tui_widget.h"

#define TUI_TABBAR_INIT_CAP 8

typedef struct TuiTabBar TuiTabBar;
typedef struct TuiTab    TuiTab;

typedef void (*TuiTabBarCallback)(TuiTabBar *tabbar, int tab_index,
                                  const char *label, void *user_data);

struct TuiTab {
    char     *label;
    TuiWidget *content;
    int        start_x;
    int        label_width;
    int        tab_width;
};

struct TuiTabBar {
    TuiWidget base;

    TuiTab  *tabs;
    int      tab_count;
    int      tab_capacity;
    int      active_tab;

    int scroll_offset;

    TuiColor fg;
    TuiColor bg;
    TuiColor fg_active;
    TuiColor bg_active;
    TuiColor fg_hover;
    TuiColor bg_hover;
    TuiAttr  attr;

    TuiTabBarCallback on_change;
    void             *user_data;
};

TuiResult tui_tabbar_init(TuiTabBar *tabbar, int x, int y, int width);

int tui_tabbar_add_tab(TuiTabBar *tabbar, const char *label, TuiWidget *content);

void tui_tabbar_remove_tab(TuiTabBar *tabbar, int index);

int tui_tabbar_get_active(const TuiTabBar *tabbar);

TuiWidget *tui_tabbar_get_content(const TuiTabBar *tabbar, int index);

void tui_tabbar_set_active(TuiTabBar *tabbar, int index);

void tui_tabbar_set_tab_label(TuiTabBar *tabbar, int index, const char *label);

void tui_tabbar_set_tab_content(TuiTabBar *tabbar, int index, TuiWidget *content);

void tui_tabbar_set_colors(TuiTabBar *tabbar,
                           TuiColor fg, TuiColor bg,
                           TuiColor fg_active, TuiColor bg_active);

void tui_tabbar_set_hover_colors(TuiTabBar *tabbar,
                                 TuiColor fg, TuiColor bg);

void tui_tabbar_set_on_change(TuiTabBar *tabbar,
                              TuiTabBarCallback callback,
                              void *user_data);

#endif
