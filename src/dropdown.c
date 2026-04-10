#define _POSIX_C_SOURCE 200809L

#include "tui_dropdown.h"
#include "tui_app.h"
#include "tui_screen.h"

#include <stdlib.h>
#include <string.h>

static void popup_ensure_visible(TuiDropdownPopup *popup)
{
    TuiDropdown *dd = popup->owner;
    int visible = dd->item_count < dd->max_visible
                  ? dd->item_count : dd->max_visible;

    if (popup->highlighted < popup->scroll_offset)
        popup->scroll_offset = popup->highlighted;
    if (popup->highlighted >= popup->scroll_offset + visible)
        popup->scroll_offset = popup->highlighted - visible + 1;
}

static void popup_close(TuiDropdownPopup *popup)
{
    TuiDropdown *dd = popup->owner;
    if (!dd->is_open) return;
    dd->is_open = 0;
    TuiApp *app = (TuiApp *)dd->app;
    if (app) tui_app_pop_overlay(app);
    tui_widget_focus(&dd->base);
}

static void popup_render(TuiWidget *widget)
{
    TuiDropdownPopup *popup = (TuiDropdownPopup *)widget;
    TuiDropdown *dd = popup->owner;

    TuiColor fg  = dd->fg_popup;
    TuiColor bg  = dd->bg_popup;
    TuiColor bfg = TUI_COLOR_INDEX(14);
    TuiColor bbg = dd->bg_popup;

    tui_screen_fill(widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ", fg, bg, TUI_ATTR_NONE);
    tui_screen_box_single(widget->abs_y, widget->abs_x,
                          widget->width, widget->height, bfg, bbg, TUI_ATTR_BOLD);

    int visible = dd->item_count < dd->max_visible
                  ? dd->item_count : dd->max_visible;
    for (int i = 0; i < visible; i++) {
        int idx = popup->scroll_offset + i;
        if (idx >= dd->item_count) break;

        TuiColor ifg = fg;
        TuiColor ibg = bg;
        TuiAttr  iat = TUI_ATTR_NONE;

        if (idx == popup->highlighted) {
            ifg = dd->fg_selected;
            ibg = dd->bg_selected;
            iat = TUI_ATTR_REVERSE;
        }

        tui_screen_fill(widget->abs_y + 1 + i, widget->abs_x + 1,
                        widget->width - 2, 1, " ", ifg, ibg, iat);

        if (dd->items[idx]) {
            int len = (int)strlen(dd->items[idx]);
            int maxw = widget->width - 4;
            int wlen = len < maxw ? len : maxw;
            char buf[256];
            int clen = wlen < (int)sizeof(buf) - 1 ? wlen : (int)sizeof(buf) - 1;
            memcpy(buf, dd->items[idx], (size_t)clen);
            buf[clen] = '\0';
            tui_screen_write(widget->abs_y + 1 + i,
                             widget->abs_x + 2, buf, ifg, ibg, iat);
        }
    }
}

static int popup_handle_input(TuiWidget *widget, const TuiEvent *event)
{
    TuiDropdownPopup *popup = (TuiDropdownPopup *)widget;
    TuiDropdown *dd = popup->owner;

    if (event->key == TUI_KEY_ESCAPE) {
        popup_close(popup);
        return 1;
    }

    if (event->key == TUI_KEY_UP) {
        if (popup->highlighted > 0) {
            popup->highlighted--;
            popup_ensure_visible(popup);
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == TUI_KEY_DOWN) {
        if (popup->highlighted < dd->item_count - 1) {
            popup->highlighted++;
            popup_ensure_visible(popup);
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == TUI_KEY_ENTER) {
        if (dd->item_count > 0) {
            dd->selected = popup->highlighted;
            dd->base.dirty = 1;
            if (dd->on_change)
                dd->on_change(dd, dd->selected,
                              dd->items[dd->selected], dd->user_data);
        }
        popup_close(popup);
        return 1;
    }

    return 1;
}

static int popup_handle_mouse(TuiWidget *widget, const TuiMouseEvent *mouse)
{
    TuiDropdownPopup *popup = (TuiDropdownPopup *)widget;
    TuiDropdown *dd = popup->owner;

    if (mouse->action == TUI_MOUSE_MOTION) {
        if (tui_widget_contains(widget, mouse->row, mouse->col)) {
            int idx = mouse->row - widget->abs_y - 1 + popup->scroll_offset;
            if (idx >= 0 && idx < dd->item_count &&
                popup->highlighted != idx) {
                popup->highlighted = idx;
                widget->dirty = 1;
            }
        }
        return 1;
    }

    if (mouse->action == TUI_MOUSE_WHEEL_UP) {
        if (popup->scroll_offset > 0) {
            popup->scroll_offset--;
            widget->dirty = 1;
        }
        return 1;
    }

    if (mouse->action == TUI_MOUSE_WHEEL_DOWN) {
        int visible = dd->item_count < dd->max_visible
                      ? dd->item_count : dd->max_visible;
        if (popup->scroll_offset + visible < dd->item_count) {
            popup->scroll_offset++;
            widget->dirty = 1;
        }
        return 1;
    }

    if (mouse->action == TUI_MOUSE_PRESS && mouse->button == TUI_MOUSE_BTN_LEFT) {
        if (!tui_widget_contains(widget, mouse->row, mouse->col)) {
            popup_close(popup);
            return 1;
        }
        int idx = mouse->row - widget->abs_y - 1 + popup->scroll_offset;
        if (idx >= 0 && idx < dd->item_count) {
            dd->selected = idx;
            dd->base.dirty = 1;
            if (dd->on_change)
                dd->on_change(dd, idx, dd->items[idx], dd->user_data);
        }
        popup_close(popup);
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

static void dropdown_render(TuiWidget *widget)
{
    TuiDropdown *dd = (TuiDropdown *)widget;

    TuiColor fg  = dd->fg;
    TuiColor bg  = dd->bg;
    TuiAttr  attr = dd->attr;

    if (widget->focused || dd->is_open) {
        fg = dd->fg_selected;
        bg = dd->bg_selected;
        attr |= TUI_ATTR_REVERSE;
    }

    tui_screen_fill(widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ", fg, bg, attr);

    const char *text = "Select...";
    if (dd->item_count > 0 && dd->selected >= 0 && dd->selected < dd->item_count)
        text = dd->items[dd->selected];

    int tlen = (int)strlen(text);
    int maxw = widget->width - 3;
    if (maxw < 0) maxw = 0;
    int wlen = tlen < maxw ? tlen : maxw;
    char buf[256];
    int clen = wlen < (int)sizeof(buf) - 1 ? wlen : (int)sizeof(buf) - 1;
    memcpy(buf, text, (size_t)clen);
    buf[clen] = '\0';

    tui_screen_write(widget->abs_y, widget->abs_x + 1, buf, fg, bg, attr);
    tui_screen_write(widget->abs_y, widget->abs_x + widget->width - 2,
                     "\xe2\x96\xbc", fg, bg, attr);
}

static int dropdown_handle_input(TuiWidget *widget, const TuiEvent *event)
{
    TuiDropdown *dd = (TuiDropdown *)widget;
    if (dd->is_open) return 0;

    if (event->key == TUI_KEY_ENTER || event->codepoint == ' ' ||
        event->key == TUI_KEY_DOWN) {
        if (dd->app && dd->item_count > 0)
            tui_dropdown_open(dd, dd->app);
        return 1;
    }

    if (event->key == TUI_KEY_UP && dd->selected > 0) {
        dd->selected--;
        widget->dirty = 1;
        if (dd->on_change)
            dd->on_change(dd, dd->selected,
                          dd->items[dd->selected], dd->user_data);
        return 1;
    }

    return 0;
}

static int dropdown_handle_mouse(TuiWidget *widget, const TuiMouseEvent *mouse)
{
    TuiDropdown *dd = (TuiDropdown *)widget;
    if (dd->is_open) return 0;

    if (mouse->action == TUI_MOUSE_PRESS && mouse->button == TUI_MOUSE_BTN_LEFT) {
        if (dd->app && dd->item_count > 0)
            tui_dropdown_open(dd, dd->app);
        return 1;
    }
    return 0;
}

static void dropdown_destroy(TuiWidget *widget)
{
    TuiDropdown *dd = (TuiDropdown *)widget;
    if (dd->is_open && dd->app) {
        TuiApp *app = (TuiApp *)dd->app;
        tui_app_pop_overlay(app);
        dd->is_open = 0;
    }
    for (int i = 0; i < dd->item_count; i++)
        free(dd->items[i]);
    free(dd->items);
    dd->items        = NULL;
    dd->item_count   = 0;
    dd->item_capacity = 0;
}

static TuiWidgetVTable dropdown_vtable = {
    .render       = dropdown_render,
    .handle_input = dropdown_handle_input,
    .handle_mouse = dropdown_handle_mouse,
    .destroy      = dropdown_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_dropdown_init(TuiDropdown *dropdown, int x, int y, int width)
{
    if (!dropdown) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init(&dropdown->base, x, y, width, 1,
                                    &dropdown_vtable, NULL);
    if (res != TUI_OK) return res;

    dropdown->base.focusable = 1;

    dropdown->items         = NULL;
    dropdown->item_count    = 0;
    dropdown->item_capacity = 0;
    dropdown->selected      = -1;
    dropdown->max_visible   = TUI_DROPDOWN_MAX_VISIBLE;
    dropdown->is_open       = 0;
    dropdown->app           = NULL;

    memset(&dropdown->popup, 0, sizeof(dropdown->popup));

    dropdown->fg          = TUI_COLOR_INDEX(15);
    dropdown->bg          = TUI_COLOR_INDEX(236);
    dropdown->fg_popup    = TUI_COLOR_INDEX(15);
    dropdown->bg_popup    = TUI_COLOR_INDEX(234);
    dropdown->fg_selected = TUI_COLOR_INDEX(0);
    dropdown->bg_selected = TUI_COLOR_INDEX(12);
    dropdown->attr        = TUI_ATTR_NONE;

    dropdown->on_change = NULL;
    dropdown->user_data = NULL;

    return TUI_OK;
}

TuiResult tui_dropdown_add_item(TuiDropdown *dropdown, const char *item)
{
    if (!dropdown) return TUI_ERR_MEMORY;

    if (dropdown->item_count >= dropdown->item_capacity) {
        int newcap = dropdown->item_capacity == 0
                     ? TUI_DROPDOWN_ITEMS_INIT_CAP
                     : dropdown->item_capacity * 2;
        char **ni = (char **)realloc(dropdown->items,
                                     (size_t)newcap * sizeof(char *));
        if (!ni) return TUI_ERR_MEMORY;
        dropdown->items        = ni;
        dropdown->item_capacity = newcap;
    }

    dropdown->items[dropdown->item_count++] = item ? strdup(item) : NULL;
    if (dropdown->selected < 0) dropdown->selected = 0;
    dropdown->base.dirty = 1;
    return TUI_OK;
}

void tui_dropdown_clear(TuiDropdown *dropdown)
{
    if (!dropdown) return;
    for (int i = 0; i < dropdown->item_count; i++)
        free(dropdown->items[i]);
    dropdown->item_count = 0;
    dropdown->selected   = -1;
    dropdown->base.dirty = 1;
}

int tui_dropdown_get_selected(const TuiDropdown *dropdown)
{
    return dropdown ? dropdown->selected : -1;
}

const char *tui_dropdown_get_selected_item(const TuiDropdown *dropdown)
{
    if (!dropdown || dropdown->selected < 0 ||
        dropdown->selected >= dropdown->item_count)
        return NULL;
    return dropdown->items[dropdown->selected];
}

void tui_dropdown_set_selected(TuiDropdown *dropdown, int index)
{
    if (!dropdown) return;
    if (index < -1 || index >= dropdown->item_count) return;
    dropdown->selected   = index;
    dropdown->base.dirty = 1;
}

void tui_dropdown_set_colors(TuiDropdown *dropdown,
                             TuiColor fg, TuiColor bg,
                             TuiColor fg_focused, TuiColor bg_focused)
{
    if (!dropdown) return;
    dropdown->fg          = fg;
    dropdown->bg          = bg;
    dropdown->fg_selected = fg_focused;
    dropdown->bg_selected = bg_focused;
    dropdown->base.dirty  = 1;
}

void tui_dropdown_set_popup_colors(TuiDropdown *dropdown,
                                   TuiColor fg, TuiColor bg,
                                   TuiColor fg_hl, TuiColor bg_hl)
{
    if (!dropdown) return;
    dropdown->fg_popup    = fg;
    dropdown->bg_popup    = bg;
    dropdown->fg_selected = fg_hl;
    dropdown->bg_selected = bg_hl;
    dropdown->base.dirty  = 1;
}

void tui_dropdown_set_on_change(TuiDropdown *dropdown,
                                TuiDropdownCallback callback,
                                void *user_data)
{
    if (!dropdown) return;
    dropdown->on_change = callback;
    dropdown->user_data = user_data;
}

void tui_dropdown_set_max_visible(TuiDropdown *dropdown, int max_visible)
{
    if (!dropdown) return;
    dropdown->max_visible = max_visible > 0 ? max_visible : TUI_DROPDOWN_MAX_VISIBLE;
}

void tui_dropdown_open(TuiDropdown *dropdown, void *app)
{
    if (!dropdown || !app || dropdown->is_open) return;
    if (dropdown->item_count == 0) return;

    dropdown->app     = app;
    dropdown->is_open = 1;

    int max_item_len = 0;
    for (int i = 0; i < dropdown->item_count; i++) {
        if (dropdown->items[i]) {
            int len = (int)strlen(dropdown->items[i]);
            if (len > max_item_len) max_item_len = len;
        }
    }
    int pw = max_item_len + 4;
    if (pw < dropdown->base.width) pw = dropdown->base.width;

    int visible = dropdown->item_count < dropdown->max_visible
                  ? dropdown->item_count : dropdown->max_visible;
    int ph = visible + 2;

    int px = dropdown->base.abs_x;
    int py = dropdown->base.abs_y + dropdown->base.height;

    tui_widget_init(&dropdown->popup.base, px, py, pw, ph,
                    &popup_vtable, NULL);
    dropdown->popup.base.focusable = 1;
    dropdown->popup.owner       = dropdown;
    dropdown->popup.highlighted = dropdown->selected >= 0
                                  ? dropdown->selected : 0;
    dropdown->popup.scroll_offset = 0;
    popup_ensure_visible(&dropdown->popup);

    tui_app_push_overlay((TuiApp *)app, &dropdown->popup.base);
}

void tui_dropdown_close(TuiDropdown *dropdown)
{
    if (!dropdown || !dropdown->is_open) return;
    popup_close(&dropdown->popup);
}
