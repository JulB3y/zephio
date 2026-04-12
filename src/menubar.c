#define _POSIX_C_SOURCE 200809L

#include "tui_menubar.h"
#include "tui_app.h"
#include "tui_screen.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static void menubar_recompute_layout(TuiMenuBar *mb)
{
    int x = 0;
    for (int i = 0; i < mb->menu_count; i++) {
        mb->menus[i].start_x     = x;
        mb->menus[i].label_width = mb->menus[i].label
                                   ? (int)strlen(mb->menus[i].label) : 0;
        x += mb->menus[i].label_width + 2;
    }
}

static void menu_popup_close(TuiMenuPopup *popup)
{
    TuiMenuBar *mb = popup->owner;
    if (!mb->is_open) return;
    mb->is_open = 0;
    TuiApp *app = (TuiApp *)mb->app;
    if (app) tui_app_pop_overlay(app);
    tui_widget_focus(&mb->base);
}

static void popup_render(TuiWidget *widget)
{
    TuiMenuPopup *popup = (TuiMenuPopup *)widget;
    TuiMenuBar *mb  = popup->owner;
    TuiMenu *menu = &mb->menus[popup->menu_index];

    TuiColor fg  = mb->fg_popup;
    TuiColor bg  = mb->bg_popup;
    TuiColor bfg = TUI_COLOR_INDEX(14);

    tui_screen_fill(widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ", fg, bg, TUI_ATTR_NONE);
    tui_screen_box_single(widget->abs_y, widget->abs_x,
                          widget->width, widget->height, bfg, bg, TUI_ATTR_BOLD);

    for (int i = 0; i < menu->item_count; i++) {
        TuiMenuItem *item = &menu->items[i];
        int row = widget->abs_y + 1 + i;

        if (item->is_separator) {
            for (int c = 1; c < widget->width - 1; c++)
                tui_screen_set_cell(row, widget->abs_x + c,
                                    "\xe2\x94\x80", bfg, bg, TUI_ATTR_DIM);
            continue;
        }

        TuiColor ifg = fg;
        TuiColor ibg = bg;
        TuiAttr  iat = TUI_ATTR_NONE;

        if (i == popup->highlighted) {
            ifg = mb->fg_popup_hl;
            ibg = mb->bg_popup_hl;
            iat = TUI_ATTR_REVERSE;
        }

        tui_screen_fill(row, widget->abs_x + 1,
                        widget->width - 2, 1, " ", ifg, ibg, iat);

        if (item->label) {
            int len = (int)strlen(item->label);
            int maxw = widget->width - 4;
            int wlen = len < maxw ? len : maxw;
            char buf[256];
            int clen = wlen < (int)sizeof(buf) - 1 ? wlen : (int)sizeof(buf) - 1;
            memcpy(buf, item->label, (size_t)clen);
            buf[clen] = '\0';
            tui_screen_write(row, widget->abs_x + 2, buf, ifg, ibg, iat);
        }
    }
}

static int popup_next_item(TuiMenu *menu, int from, int dir)
{
    int idx = from + dir;
    while (idx >= 0 && idx < menu->item_count) {
        if (!menu->items[idx].is_separator) return idx;
        idx += dir;
    }
    return from;
}

static void popup_navigate(TuiMenuPopup *popup, TuiMenu *menu, int dir, TuiWidget *widget)
{
    int next = popup_next_item(menu, popup->highlighted, dir);
    if (next != popup->highlighted) {
        popup->highlighted = next;
        widget->dirty = 1;
    }
}

static void popup_switch_menu(TuiMenuPopup *popup, TuiMenuBar *mb, int new_idx)
{
    if (new_idx >= 0 && new_idx < mb->menu_count) {
        menu_popup_close(popup);
        tui_menubar_open_menu(mb, new_idx);
    }
}

static void popup_activate(TuiMenuPopup *popup, TuiMenuBar *mb, TuiMenu *menu)
{
    if (popup->highlighted >= 0 && popup->highlighted < menu->item_count) {
        TuiMenuItem *item = &menu->items[popup->highlighted];
        if (!item->is_separator && mb->on_select) {
            mb->on_select(mb, menu->label,
                          popup->highlighted, item->label, mb->user_data);
        }
    }
    menu_popup_close(popup);
}

static int popup_handle_input(TuiWidget *widget, const TuiEvent *event)
{
    TuiMenuPopup *popup = (TuiMenuPopup *)widget;
    TuiMenuBar *mb  = popup->owner;
    TuiMenu *menu = &mb->menus[popup->menu_index];

    if (event->key == TUI_KEY_ESCAPE) {
        menu_popup_close(popup);
        return 1;
    }

    if (event->key == TUI_KEY_UP) {
        popup_navigate(popup, menu, -1, widget);
        return 1;
    }

    if (event->key == TUI_KEY_DOWN) {
        popup_navigate(popup, menu, 1, widget);
        return 1;
    }

    if (event->key == TUI_KEY_LEFT) {
        popup_switch_menu(popup, mb, popup->menu_index - 1);
        return 1;
    }

    if (event->key == TUI_KEY_RIGHT) {
        popup_switch_menu(popup, mb, popup->menu_index + 1);
        return 1;
    }

    if (event->key == TUI_KEY_ENTER) {
        popup_activate(popup, mb, menu);
        return 1;
    }

    return 1;
}

static int popup_handle_mouse(TuiWidget *widget, const TuiMouseEvent *mouse)
{
    TuiMenuPopup *popup = (TuiMenuPopup *)widget;
    TuiMenuBar *mb  = popup->owner;
    TuiMenu *menu = &mb->menus[popup->menu_index];

    if (mouse->action == TUI_MOUSE_MOTION) {
        if (tui_widget_contains(widget, mouse->row, mouse->col)) {
            int idx = mouse->row - widget->abs_y - 1;
            if (idx >= 0 && idx < menu->item_count &&
                !menu->items[idx].is_separator &&
                popup->highlighted != idx) {
                popup->highlighted = idx;
                widget->dirty = 1;
            }
        }
        return 1;
    }

    if (mouse->action == TUI_MOUSE_PRESS && mouse->button == TUI_MOUSE_BTN_LEFT) {
        if (!tui_widget_contains(widget, mouse->row, mouse->col)) {
            menu_popup_close(popup);
            return 1;
        }
        int idx = mouse->row - widget->abs_y - 1;
        if (idx >= 0 && idx < menu->item_count &&
            !menu->items[idx].is_separator) {
            if (mb->on_select)
                mb->on_select(mb, menu->label, idx,
                              menu->items[idx].label, mb->user_data);
            menu_popup_close(popup);
        }
        return 1;
    }

    return 1;
}

static TuiWidgetVTable popup_vtable = {
    .render       = popup_render,
    .handle_input = popup_handle_input,
    .handle_mouse = popup_handle_mouse,
    .destroy      = NULL,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

static void menubar_render(TuiWidget *widget)
{
    TuiMenuBar *mb = (TuiMenuBar *)widget;

    tui_screen_fill(widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ",
                    mb->fg, mb->bg, mb->attr);

    for (int i = 0; i < mb->menu_count; i++) {
        TuiMenu *m = &mb->menus[i];
        TuiColor fg = mb->fg;
        TuiColor bg = mb->bg;
        TuiAttr  at = mb->attr;

        if (i == mb->active_menu && (widget->focused || mb->is_open)) {
            fg = mb->fg_active;
            bg = mb->bg_active;
            at |= TUI_ATTR_REVERSE;
        }

        char buf[128];
        int clen = m->label_width + 2;
        if (clen > (int)sizeof(buf) - 1) clen = (int)sizeof(buf) - 1;
        buf[0] = ' ';
        if (m->label) {
            int copy = clen - 1;
            if (copy > m->label_width) copy = m->label_width;
            memcpy(buf + 1, m->label, (size_t)copy);
            buf[1 + copy] = ' ';
            buf[1 + copy + 1] = '\0';
        } else {
            buf[1] = ' ';
            buf[2] = '\0';
        }

        tui_screen_write(widget->abs_y,
                         widget->abs_x + m->start_x,
                         buf, fg, bg, at);

        if (m->mnemonic && m->label) {
            for (int j = 0; m->label[j]; j++) {
                if (tolower((unsigned char)m->label[j]) == tolower((unsigned char)m->mnemonic)) {
                    tui_screen_set_cell(widget->abs_y,
                                        widget->abs_x + m->start_x + 1 + j,
                                        &m->label[j], fg, bg,
                                        at | TUI_ATTR_UNDERLINE);
                    break;
                }
            }
        }
    }
}

static int menubar_handle_input(TuiWidget *widget, const TuiEvent *event)
{
    TuiMenuBar *mb = (TuiMenuBar *)widget;
    if (mb->is_open) return 0;

    if (event->key == TUI_KEY_LEFT) {
        if (mb->active_menu > 0) {
            mb->active_menu--;
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == TUI_KEY_RIGHT) {
        if (mb->active_menu < mb->menu_count - 1) {
            mb->active_menu++;
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == TUI_KEY_DOWN || event->key == TUI_KEY_ENTER) {
        if (mb->active_menu >= 0 && mb->app &&
            mb->menus[mb->active_menu].item_count > 0) {
            tui_menubar_open_menu(mb, mb->active_menu);
        }
        return 1;
    }

    if (event->modifiers & TUI_MOD_ALT && event->codepoint) {
        char ch = tolower(event->codepoint);
        for (int i = 0; i < mb->menu_count; i++) {
            if (mb->menus[i].mnemonic &&
                tolower((unsigned char)mb->menus[i].mnemonic) == ch) {
                mb->active_menu = i;
                widget->dirty = 1;
                if (mb->app && mb->menus[i].item_count > 0)
                    tui_menubar_open_menu(mb, i);
                return 1;
            }
        }
    }

    return 0;
}

static int menubar_handle_mouse(TuiWidget *widget, const TuiMouseEvent *mouse)
{
    TuiMenuBar *mb = (TuiMenuBar *)widget;
    if (mb->is_open) return 0;

    if (mouse->action == TUI_MOUSE_PRESS && mouse->button == TUI_MOUSE_BTN_LEFT) {
        int col = mouse->col - widget->abs_x;
        for (int i = 0; i < mb->menu_count; i++) {
            int end = mb->menus[i].start_x + mb->menus[i].label_width + 2;
            if (col >= mb->menus[i].start_x && col < end) {
                mb->active_menu = i;
                widget->dirty = 1;
                if (mb->app && mb->menus[i].item_count > 0)
                    tui_menubar_open_menu(mb, i);
                return 1;
            }
        }
    }
    return 0;
}

static void destroy_menu(TuiMenu *menu)
{
    for (int i = 0; i < menu->item_count; i++)
        free(menu->items[i].label);
    free(menu->items);
    free(menu->label);
    menu->items      = NULL;
    menu->item_count = 0;
    menu->label      = NULL;
}

static void menubar_destroy(TuiWidget *widget)
{
    TuiMenuBar *mb = (TuiMenuBar *)widget;
    if (mb->is_open && mb->app) {
        TuiApp *app = (TuiApp *)mb->app;
        tui_app_pop_overlay(app);
        mb->is_open = 0;
    }
    for (int i = 0; i < mb->menu_count; i++)
        destroy_menu(&mb->menus[i]);
    free(mb->menus);
    mb->menus      = NULL;
    mb->menu_count = 0;
}

static TuiWidgetVTable menubar_vtable = {
    .render       = menubar_render,
    .handle_input = menubar_handle_input,
    .handle_mouse = menubar_handle_mouse,
    .destroy      = menubar_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_menubar_init(TuiMenuBar *menubar, int x, int y, int width)
{
    if (!menubar) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init(&menubar->base, x, y, width, 1,
                                    &menubar_vtable, NULL);
    if (res != TUI_OK) return res;

    menubar->base.focusable = 1;

    menubar->menus        = NULL;
    menubar->menu_count   = 0;
    menubar->menu_capacity = 0;
    menubar->active_menu  = 0;
    menubar->is_open      = 0;
    menubar->app          = NULL;

    memset(&menubar->popup, 0, sizeof(menubar->popup));

    menubar->fg          = TUI_COLOR_INDEX(15);
    menubar->bg          = TUI_COLOR_INDEX(4);
    menubar->fg_active   = TUI_COLOR_INDEX(0);
    menubar->bg_active   = TUI_COLOR_INDEX(14);
    menubar->fg_popup    = TUI_COLOR_INDEX(15);
    menubar->bg_popup    = TUI_COLOR_INDEX(234);
    menubar->fg_popup_hl = TUI_COLOR_INDEX(0);
    menubar->bg_popup_hl = TUI_COLOR_INDEX(12);
    menubar->attr        = TUI_ATTR_BOLD;

    menubar->on_select = NULL;
    menubar->user_data = NULL;

    return TUI_OK;
}

int tui_menubar_add_menu(TuiMenuBar *menubar, const char *label, char mnemonic)
{
    if (!menubar || !label) return -1;

    if (menubar->menu_count >= menubar->menu_capacity) {
        int newcap = menubar->menu_capacity == 0
                     ? TUI_MENUS_INIT_CAP
                     : menubar->menu_capacity * 2;
        TuiMenu *nm = (TuiMenu *)realloc(menubar->menus,
                                         (size_t)newcap * sizeof(TuiMenu));
        if (!nm) return -1;
        menubar->menus        = nm;
        menubar->menu_capacity = newcap;
    }

    int idx = menubar->menu_count++;
    TuiMenu *m = &menubar->menus[idx];
    m->label        = strdup(label);
    m->mnemonic     = mnemonic;
    m->items        = NULL;
    m->item_count   = 0;
    m->item_capacity = 0;

    menubar_recompute_layout(menubar);
    menubar->base.dirty = 1;
    return idx;
}

static int ensure_item_capacity(TuiMenu *m)
{
    if (m->item_count >= m->item_capacity) {
        int newcap = m->item_capacity == 0
                     ? TUI_MENU_ITEMS_INIT_CAP
                     : m->item_capacity * 2;
        TuiMenuItem *ni = (TuiMenuItem *)realloc(m->items,
                            (size_t)newcap * sizeof(TuiMenuItem));
        if (!ni) return -1;
        m->items        = ni;
        m->item_capacity = newcap;
    }
    return 0;
}

TuiResult tui_menubar_add_menu_item(TuiMenuBar *menubar, int menu_index,
                                    const char *label)
{
    if (!menubar || menu_index < 0 || menu_index >= menubar->menu_count)
        return TUI_ERR_MEMORY;

    TuiMenu *m = &menubar->menus[menu_index];
    if (ensure_item_capacity(m) < 0) return TUI_ERR_MEMORY;

    TuiMenuItem *it = &m->items[m->item_count++];
    it->label        = label ? strdup(label) : NULL;
    it->is_separator = 0;

    menubar->base.dirty = 1;
    return TUI_OK;
}

void tui_menubar_add_menu_separator(TuiMenuBar *menubar, int menu_index)
{
    if (!menubar || menu_index < 0 || menu_index >= menubar->menu_count) return;

    TuiMenu *m = &menubar->menus[menu_index];
    if (ensure_item_capacity(m) < 0) return;

    TuiMenuItem *it = &m->items[m->item_count++];
    it->label        = NULL;
    it->is_separator = 1;

    menubar->base.dirty = 1;
}

void tui_menubar_set_colors(TuiMenuBar *menubar,
                            TuiColor fg, TuiColor bg,
                            TuiColor fg_active, TuiColor bg_active)
{
    if (!menubar) return;
    menubar->fg        = fg;
    menubar->bg        = bg;
    menubar->fg_active = fg_active;
    menubar->bg_active = bg_active;
    menubar->base.dirty = 1;
}

void tui_menubar_set_popup_colors(TuiMenuBar *menubar,
                                  TuiColor fg, TuiColor bg,
                                  TuiColor fg_hl, TuiColor bg_hl)
{
    if (!menubar) return;
    menubar->fg_popup    = fg;
    menubar->bg_popup    = bg;
    menubar->fg_popup_hl = fg_hl;
    menubar->bg_popup_hl = bg_hl;
    menubar->base.dirty  = 1;
}

void tui_menubar_set_on_select(TuiMenuBar *menubar,
                               TuiMenuBarCallback callback,
                               void *user_data)
{
    if (!menubar) return;
    menubar->on_select = callback;
    menubar->user_data = user_data;
}

void tui_menubar_open_menu(TuiMenuBar *menubar, int menu_index)
{
    if (!menubar || menu_index < 0 || menu_index >= menubar->menu_count)
        return;
    if (!menubar->app) return;

    TuiMenu *menu = &menubar->menus[menu_index];
    if (menu->item_count == 0) return;

    menubar->active_menu = menu_index;
    menubar->is_open     = 1;
    menubar->base.dirty  = 1;

    int max_label_len = 0;
    for (int i = 0; i < menu->item_count; i++) {
        if (menu->items[i].label) {
            int len = (int)strlen(menu->items[i].label);
            if (len > max_label_len) max_label_len = len;
        }
    }
    int pw = max_label_len + 4;
    int ph = menu->item_count + 2;

    int px = menubar->base.abs_x + menu->start_x;
    int py = menubar->base.abs_y + menubar->base.height;

    tui_widget_init(&menubar->popup.base, px, py, pw, ph,
                    &popup_vtable, NULL);
    menubar->popup.base.focusable = 1;
    menubar->popup.owner       = menubar;
    menubar->popup.menu_index  = menu_index;
    menubar->popup.highlighted = popup_next_item(menu, -1, 1);

    tui_app_push_overlay((TuiApp *)menubar->app, &menubar->popup.base);
}

void tui_menubar_close_menu(TuiMenuBar *menubar)
{
    if (!menubar || !menubar->is_open) return;
    menu_popup_close(&menubar->popup);
}
