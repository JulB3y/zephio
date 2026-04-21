#define _POSIX_C_SOURCE 200809L

#include "tui_tabbar.h"
#include "tui_context.h"

#include <stdlib.h>
#include <string.h>

static void tabbar_recompute_layout(TuiTabBar *tb)
{
    int x = 0;
    for (int i = 0; i < tb->tab_count; i++) {
        TuiTab *tab = &tb->tabs[i];
        tab->label_width = tab->label ? (int)strlen(tab->label) : 0;
        tab->tab_width   = tab->label_width + 4;
        tab->start_x     = x;
        x += tab->tab_width;
    }
}

static void tabbar_ensure_visible(TuiTabBar *tb)
{
    if (tb->active_tab < 0 || tb->active_tab >= tb->tab_count) return;

    TuiTab *active = &tb->tabs[tb->active_tab];
    int tab_end = active->start_x + active->tab_width;

    if (active->start_x < tb->scroll_offset) {
        tb->scroll_offset = active->start_x;
    }
    if (tab_end > tb->scroll_offset + tb->base.width) {
        tb->scroll_offset = tab_end - tb->base.width;
    }
    if (tb->scroll_offset < 0) tb->scroll_offset = 0;
}

static void tabbar_render(TuiWidget *widget)
{
    TuiTabBar *tb = (TuiTabBar *)widget;

    tui_screen_fill(widget->ctx, widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ",
                    tb->fg, tb->bg, tb->attr);

    for (int i = 0; i < tb->tab_count; i++) {
        TuiTab *tab = &tb->tabs[i];

        int screen_x = tab->start_x - tb->scroll_offset;
        if (screen_x + tab->tab_width <= 0 || screen_x >= widget->width)
            continue;

        TuiColor fg = tb->fg;
        TuiColor bg = tb->bg;
        TuiAttr  at = tb->attr;

        if (i == tb->active_tab) {
            fg = tb->fg_active;
            bg = tb->bg_active;
            at |= ZEPHIO_ATTR_BOLD;
        } else if (widget->hovered && i == tb->active_tab) {
            fg = tb->fg_hover;
            bg = tb->bg_hover;
        }

        int fill_start = screen_x < 0 ?0 : screen_x;
        int fill_end   = screen_x + tab->tab_width;
        if (fill_end > widget->width) fill_end = widget->width;
        if (fill_start < fill_end) {
            tui_screen_fill(widget->ctx, widget->abs_y, widget->abs_x + fill_start,
                            fill_end - fill_start, 1, " ", fg, bg, at);
        }

        if (tab->label && screen_x >= 0 && screen_x + 2 < widget->width) {
            int maxw = tab->tab_width -4;
            if (maxw > widget->width - screen_x - 2)
                maxw = widget->width - screen_x - 2;
            if (maxw > 0) {
                int len = tab->label_width;
                int wlen = len < maxw ? len : maxw;
                char buf[256];
                int clen = wlen < (int)sizeof(buf) - 1
                           ? wlen : (int)sizeof(buf) - 1;
                memcpy(buf, tab->label, (size_t)clen);
                buf[clen] = '\0';
                tui_screen_write(widget->ctx, widget->abs_y,
                                 widget->abs_x + screen_x + 2,
                                 buf, fg, bg, at);
            }
        }

        if (screen_x + tab->tab_width - 1 >= 0 &&
            screen_x + tab->tab_width - 1 < widget->width) {
            tui_screen_set_cell(widget->ctx, widget->abs_y,
                                widget->abs_x + screen_x + tab->tab_width - 1,
                                "\xe2\x94\x82", tb->fg, tb->bg, ZEPHIO_ATTR_DIM);
        }
    }
}

static int tabbar_handle_input(TuiWidget *widget, const TuiEvent *event)
{
    TuiTabBar *tb = (TuiTabBar *)widget;
    if (tb->tab_count == 0) return 0;

    if (event->key == TUI_KEY_RIGHT) {
        if (tb->active_tab < tb->tab_count - 1) {
            tb->active_tab++;
            tabbar_ensure_visible(tb);
            widget->dirty = 1;
            if (tb->on_change)
                tb->on_change(tb, tb->active_tab,
                              tb->tabs[tb->active_tab].label,
                              tb->user_data);
        }
        return 1;
    }

    if (event->key == TUI_KEY_LEFT) {
        if (tb->active_tab > 0) {
            tb->active_tab--;
            tabbar_ensure_visible(tb);
            widget->dirty = 1;
            if (tb->on_change)
                tb->on_change(tb, tb->active_tab,
                              tb->tabs[tb->active_tab].label,
                              tb->user_data);
        }
        return 1;
    }

    if (event->key == TUI_KEY_HOME) {
        if (tb->active_tab != 0) {
            tb->active_tab = 0;
            tabbar_ensure_visible(tb);
            widget->dirty = 1;
            if (tb->on_change)
                tb->on_change(tb, 0, tb->tabs[0].label,
                              tb->user_data);
        }
        return 1;
    }

    if (event->key == TUI_KEY_END) {
        int last = tb->tab_count - 1;
        if (tb->active_tab != last) {
            tb->active_tab = last;
            tabbar_ensure_visible(tb);
            widget->dirty = 1;
            if (tb->on_change)
                tb->on_change(tb, last, tb->tabs[last].label,
                              tb->user_data);
        }
        return 1;
    }

    if (event->modifiers & TUI_MOD_CTRL && event->key == TUI_KEY_TAB) {
        int next = (tb->active_tab + 1) % tb->tab_count;
        if (next != tb->active_tab) {
            tb->active_tab = next;
            tabbar_ensure_visible(tb);
            widget->dirty = 1;
            if (tb->on_change)
                tb->on_change(tb, next, tb->tabs[next].label,
                              tb->user_data);
        }
        return 1;
    }

    return 0;
}

static int tabbar_handle_mouse(TuiWidget *widget, const TuiMouseEvent *mouse)
{
    TuiTabBar *tb = (TuiTabBar *)widget;
    if (tb->tab_count == 0) return 0;

    if (mouse->action == TUI_MOUSE_PRESS && mouse->button == TUI_MOUSE_BTN_LEFT) {
        int col = mouse->col - widget->abs_x + tb->scroll_offset;
        for (int i = 0; i < tb->tab_count; i++) {
            TuiTab *tab = &tb->tabs[i];
            if (col >= tab->start_x && col < tab->start_x + tab->tab_width) {
                if (tb->active_tab != i) {
                    tb->active_tab = i;
                    tabbar_ensure_visible(tb);
                    widget->dirty = 1;
                    if (tb->on_change)
                        tb->on_change(tb, i, tab->label,
                                      tb->user_data);
                }
                return 1;
            }
        }
    }

    if (mouse->action == TUI_MOUSE_MOTION) {
        return 1;
    }

    return 0;
}

static void tabbar_destroy(TuiWidget *widget)
{
    TuiTabBar *tb = (TuiTabBar *)widget;
    for (int i = 0; i < tb->tab_count; i++)
        free(tb->tabs[i].label);
    free(tb->tabs);
    tb->tabs       = NULL;
    tb->tab_count  = 0;
    tb->tab_capacity = 0;
}

static TuiWidgetVTable tabbar_vtable = {
    .render       = tabbar_render,
    .handle_input = tabbar_handle_input,
    .handle_mouse = tabbar_handle_mouse,
    .destroy      = tabbar_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_tabbar_init_ctx(TuiTabBar *tabbar, TuiContext *ctx, int x, int y, int width)
{
    if (!tabbar) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init_ctx(&tabbar->base, x, y, width, 1,
                                        &tabbar_vtable, ctx, NULL);
    if (res != TUI_OK) return res;

    tabbar->base.focusable = 1;

    tabbar->tabs         = NULL;
    tabbar->tab_count    = 0;
    tabbar->tab_capacity = 0;
    tabbar->active_tab   = 0;
    tabbar->scroll_offset = 0;

    tabbar->fg         = ZEPHIO_COLOR_INDEX(15);
    tabbar->bg         = ZEPHIO_COLOR_INDEX(236);
    tabbar->fg_active  = ZEPHIO_COLOR_INDEX(0);
    tabbar->bg_active  = ZEPHIO_COLOR_INDEX(12);
    tabbar->fg_hover   = ZEPHIO_COLOR_INDEX(15);
    tabbar->bg_hover   = ZEPHIO_COLOR_INDEX(238);
    tabbar->attr       = ZEPHIO_ATTR_NONE;

    tabbar->on_change = NULL;
    tabbar->user_data = NULL;

    return TUI_OK;
}

int tui_tabbar_add_tab(TuiTabBar *tabbar, const char *label,
                       TuiWidget *content)
{
    if (!tabbar) return -1;

    if (tabbar->tab_count >= tabbar->tab_capacity) {
        int newcap = tabbar->tab_capacity == 0
                     ? ZEPHIO_TABBAR_INIT_CAP
                     : tabbar->tab_capacity * 2;
        TuiTab *nt = (TuiTab *)realloc(tabbar->tabs,
                                       (size_t)newcap * sizeof(TuiTab));
        if (!nt) return -1;
        tabbar->tabs         = nt;
        tabbar->tab_capacity = newcap;
    }

    int idx = tabbar->tab_count++;
    TuiTab *tab = &tabbar->tabs[idx];
    tab->label        = label ? strdup(label) : NULL;
    tab->content      = content;
    tab->start_x      = 0;
    tab->label_width  = 0;
    tab->tab_width    = 0;

    tabbar_recompute_layout(tabbar);
    tabbar->base.dirty = 1;
    return idx;
}

void tui_tabbar_remove_tab(TuiTabBar *tabbar, int index)
{
    if (!tabbar || index < 0 || index >= tabbar->tab_count) return;

    free(tabbar->tabs[index].label);
    for (int i = index; i < tabbar->tab_count - 1; i++)
        tabbar->tabs[i] = tabbar->tabs[i + 1];
    tabbar->tab_count--;

    if (tabbar->active_tab >= tabbar->tab_count)
        tabbar->active_tab = tabbar->tab_count - 1;
    if (tabbar->active_tab < 0)
        tabbar->active_tab = 0;

    tabbar_recompute_layout(tabbar);
    tabbar->base.dirty = 1;
}

int tui_tabbar_get_active(const TuiTabBar *tabbar)
{
    return tabbar ? tabbar->active_tab : -1;
}

TuiWidget *tui_tabbar_get_content(const TuiTabBar *tabbar, int index)
{
    if (!tabbar || index < 0 || index >= tabbar->tab_count)
        return NULL;
    return tabbar->tabs[index].content;
}

void tui_tabbar_set_active(TuiTabBar *tabbar, int index)
{
    if (!tabbar || index < 0 || index >= tabbar->tab_count) return;
    if (tabbar->active_tab == index) return;
    tabbar->active_tab = index;
    tabbar_ensure_visible(tabbar);
    tabbar->base.dirty = 1;
    if (tabbar->on_change)
        tabbar->on_change(tabbar, index,
                          tabbar->tabs[index].label,
                          tabbar->user_data);
}

void tui_tabbar_set_tab_label(TuiTabBar *tabbar, int index,
                              const char *label)
{
    if (!tabbar || index < 0 || index >= tabbar->tab_count) return;
    free(tabbar->tabs[index].label);
    tabbar->tabs[index].label = label ? strdup(label) : NULL;
    tabbar_recompute_layout(tabbar);
    tabbar->base.dirty = 1;
}

void tui_tabbar_set_tab_content(TuiTabBar *tabbar, int index,
                                TuiWidget *content)
{
    if (!tabbar || index < 0 || index >= tabbar->tab_count) return;
    tabbar->tabs[index].content = content;
}

void tui_tabbar_set_colors(TuiTabBar *tabbar,
                           TuiColor fg, TuiColor bg,
                           TuiColor fg_active, TuiColor bg_active)
{
    if (!tabbar) return;
    tabbar->fg        = fg;
    tabbar->bg        = bg;
    tabbar->fg_active = fg_active;
    tabbar->bg_active = bg_active;
    tabbar->base.dirty = 1;
}

void tui_tabbar_set_hover_colors(TuiTabBar *tabbar,
                                 TuiColor fg, TuiColor bg)
{
    if (!tabbar) return;
    tabbar->fg_hover = fg;
    tabbar->bg_hover = bg;
    tabbar->base.dirty = 1;
}

void tui_tabbar_set_on_change(TuiTabBar *tabbar,
                              TuiTabBarCallback callback,
                              void *user_data)
{
    if (!tabbar) return;
    tabbar->on_change = callback;
    tabbar->user_data = user_data;
}
