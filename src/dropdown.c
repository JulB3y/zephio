#define _POSIX_C_SOURCE 200809L

#include "zephio_dropdown.h"
#include "zephio_context.h"
#include "zephio_app.h"
#include "zephio_screen.h"

#include <stdlib.h>
#include <string.h>

static void popup_ensure_visible(ZephioDropdownPopup *popup)
{
    ZephioDropdown *dd = popup->owner;
    int visible = dd->item_count < dd->max_visible
                  ? dd->item_count : dd->max_visible;

    if (popup->highlighted < popup->scroll_offset)
        popup->scroll_offset = popup->highlighted;
    if (popup->highlighted >= popup->scroll_offset + visible)
        popup->scroll_offset = popup->highlighted - visible + 1;
}

static void popup_close(ZephioDropdownPopup *popup)
{
    ZephioDropdown *dd = popup->owner;
    if (!dd->is_open) return;
    dd->is_open = 0;
    ZephioApp *app = (ZephioApp *)dd->app;
    if (app) zephio_app_pop_overlay(app);
    zephio_widget_focus(&dd->base);
}

static void popup_render(ZephioWidget *widget)
{
    ZephioDropdownPopup *popup = (ZephioDropdownPopup *)widget;
    ZephioDropdown *dd = popup->owner;

    ZephioColor fg  = dd->fg_popup;
    ZephioColor bg  = dd->bg_popup;
    ZephioColor bfg = ZEPHIO_COLOR_INDEX(14);
    ZephioColor bbg = dd->bg_popup;

    zephio_screen_fill(widget->ctx, widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ", fg, bg, ZEPHIO_ATTR_NONE);
    zephio_screen_box_single(widget->ctx, widget->abs_y, widget->abs_x,
                          widget->width, widget->height, bfg, bbg, ZEPHIO_ATTR_BOLD);

    int visible = dd->item_count < dd->max_visible
                  ? dd->item_count : dd->max_visible;
    for (int i = 0; i < visible; i++) {
        int idx = popup->scroll_offset + i;
        if (idx >= dd->item_count) break;

        ZephioColor ifg = fg;
        ZephioColor ibg = bg;
        ZephioAttr  iat = ZEPHIO_ATTR_NONE;

        if (idx == popup->highlighted) {
            ifg = dd->fg_selected;
            ibg = dd->bg_selected;
            iat = ZEPHIO_ATTR_REVERSE;
        }

        zephio_screen_fill(widget->ctx, widget->abs_y + 1 + i, widget->abs_x + 1,
                        widget->width - 2, 1, " ", ifg, ibg, iat);

        if (dd->items[idx]) {
            int len = (int)strlen(dd->items[idx]);
            int maxw = widget->width - 4;
            int wlen = len < maxw ? len : maxw;
            char buf[256];
            int clen = wlen < (int)sizeof(buf) - 1 ? wlen : (int)sizeof(buf) - 1;
            memcpy(buf, dd->items[idx], (size_t)clen);
            buf[clen] = '\0';
            zephio_screen_write(widget->ctx, widget->abs_y + 1 + i,
                             widget->abs_x + 2, buf, ifg, ibg, iat);
        }
    }
}

static int popup_handle_input(ZephioWidget *widget, const ZephioEvent *event)
{
    ZephioDropdownPopup *popup = (ZephioDropdownPopup *)widget;
    ZephioDropdown *dd = popup->owner;

    if (event->key == ZEPHIO_KEY_ESCAPE) {
        popup_close(popup);
        return 1;
    }

    if (event->key == ZEPHIO_KEY_UP) {
        if (popup->highlighted > 0) {
            popup->highlighted--;
            popup_ensure_visible(popup);
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == ZEPHIO_KEY_DOWN) {
        if (popup->highlighted < dd->item_count - 1) {
            popup->highlighted++;
            popup_ensure_visible(popup);
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == ZEPHIO_KEY_ENTER) {
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

static int popup_handle_mouse(ZephioWidget *widget, const ZephioMouseEvent *mouse)
{
    ZephioDropdownPopup *popup = (ZephioDropdownPopup *)widget;
    ZephioDropdown *dd = popup->owner;

    if (mouse->action == ZEPHIO_MOUSE_MOTION) {
        if (zephio_widget_contains(widget, mouse->row, mouse->col)) {
            int idx = mouse->row - widget->abs_y - 1 + popup->scroll_offset;
            if (idx >= 0 && idx < dd->item_count &&
                popup->highlighted != idx) {
                popup->highlighted = idx;
                widget->dirty = 1;
            }
        }
        return 1;
    }

    if (mouse->action == ZEPHIO_MOUSE_WHEEL_UP) {
        if (popup->scroll_offset > 0) {
            popup->scroll_offset--;
            widget->dirty = 1;
        }
        return 1;
    }

    if (mouse->action == ZEPHIO_MOUSE_WHEEL_DOWN) {
        int visible = dd->item_count < dd->max_visible
                      ? dd->item_count : dd->max_visible;
        if (popup->scroll_offset + visible < dd->item_count) {
            popup->scroll_offset++;
            widget->dirty = 1;
        }
        return 1;
    }

    if (mouse->action == ZEPHIO_MOUSE_PRESS && mouse->button == ZEPHIO_MOUSE_BTN_LEFT) {
        if (!zephio_widget_contains(widget, mouse->row, mouse->col)) {
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

static ZephioWidgetVTable popup_vtable = {
    .render       = popup_render,
    .handle_input = popup_handle_input,
    .handle_mouse = popup_handle_mouse,
    .destroy      = NULL,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

static void dropdown_render(ZephioWidget *widget)
{
    ZephioDropdown *dd = (ZephioDropdown *)widget;

    ZephioColor fg  = dd->fg;
    ZephioColor bg  = dd->bg;
    ZephioAttr  attr = dd->attr;

    if (widget->focused || dd->is_open) {
        fg = dd->fg_selected;
        bg = dd->bg_selected;
        attr |= ZEPHIO_ATTR_REVERSE;
    }

    zephio_screen_fill(widget->ctx, widget->abs_y, widget->abs_x,
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

    zephio_screen_write(widget->ctx, widget->abs_y, widget->abs_x + 1, buf, fg, bg, attr);
    zephio_screen_write(widget->ctx, widget->abs_y, widget->abs_x + widget->width - 2,
                     "\xe2\x96\xbc", fg, bg, attr);
}

static int dropdown_handle_input(ZephioWidget *widget, const ZephioEvent *event)
{
    ZephioDropdown *dd = (ZephioDropdown *)widget;
    if (dd->is_open) return 0;

    if (event->key == ZEPHIO_KEY_ENTER || event->codepoint == ' ' ||
        event->key == ZEPHIO_KEY_DOWN) {
        if (dd->app && dd->item_count > 0)
            zephio_dropdown_open(dd, dd->app);
        return 1;
    }

    if (event->key == ZEPHIO_KEY_UP && dd->selected > 0) {
        dd->selected--;
        widget->dirty = 1;
        if (dd->on_change)
            dd->on_change(dd, dd->selected,
                          dd->items[dd->selected], dd->user_data);
        return 1;
    }

    return 0;
}

static int dropdown_handle_mouse(ZephioWidget *widget, const ZephioMouseEvent *mouse)
{
    ZephioDropdown *dd = (ZephioDropdown *)widget;
    if (dd->is_open) return 0;

    if (mouse->action == ZEPHIO_MOUSE_PRESS && mouse->button == ZEPHIO_MOUSE_BTN_LEFT) {
        if (dd->app && dd->item_count > 0)
            zephio_dropdown_open(dd, dd->app);
        return 1;
    }
    return 0;
}

static void dropdown_destroy(ZephioWidget *widget)
{
    ZephioDropdown *dd = (ZephioDropdown *)widget;
    if (dd->is_open && dd->app) {
        ZephioApp *app = (ZephioApp *)dd->app;
        zephio_app_pop_overlay(app);
        dd->is_open = 0;
    }
    for (int i = 0; i < dd->item_count; i++)
        free(dd->items[i]);
    free(dd->items);
    dd->items        = NULL;
    dd->item_count   = 0;
    dd->item_capacity = 0;
}

static ZephioWidgetVTable dropdown_vtable = {
    .render       = dropdown_render,
    .handle_input = dropdown_handle_input,
    .handle_mouse = dropdown_handle_mouse,
    .destroy      = dropdown_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

ZephioResult zephio_dropdown_init_ctx(ZephioDropdown *dropdown, ZephioContext *ctx, int x, int y, int width)
{
    if (!dropdown) return TUI_ERR_MEMORY;

    ZephioResult res = zephio_widget_init_ctx(&dropdown->base, x, y, width, 1,
                                        &dropdown_vtable, ctx, NULL);
    if (res != ZEPHIO_OK) return res;

    dropdown->base.focusable = 1;

    dropdown->items         = NULL;
    dropdown->item_count    = 0;
    dropdown->item_capacity = 0;
    dropdown->selected      = -1;
    dropdown->max_visible   = ZEPHIO_DROPDOWN_MAX_VISIBLE;
    dropdown->is_open       = 0;
    dropdown->app           = NULL;

    memset(&dropdown->popup, 0, sizeof(dropdown->popup));

    dropdown->fg          = ZEPHIO_COLOR_INDEX(15);
    dropdown->bg          = ZEPHIO_COLOR_INDEX(236);
    dropdown->fg_popup    = ZEPHIO_COLOR_INDEX(15);
    dropdown->bg_popup    = ZEPHIO_COLOR_INDEX(234);
    dropdown->fg_selected = ZEPHIO_COLOR_INDEX(0);
    dropdown->bg_selected = ZEPHIO_COLOR_INDEX(12);
    dropdown->attr        = ZEPHIO_ATTR_NONE;

    dropdown->on_change = NULL;
    dropdown->user_data = NULL;

    return ZEPHIO_OK;
}

ZephioResult zephio_dropdown_add_item(ZephioDropdown *dropdown, const char *item)
{
    if (!dropdown) return TUI_ERR_MEMORY;

    if (dropdown->item_count >= dropdown->item_capacity) {
        int newcap = dropdown->item_capacity == 0
                     ? ZEPHIO_DROPDOWN_ITEMS_INIT_CAP
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
    return ZEPHIO_OK;
}

void zephio_dropdown_clear(ZephioDropdown *dropdown)
{
    if (!dropdown) return;
    for (int i = 0; i < dropdown->item_count; i++)
        free(dropdown->items[i]);
    dropdown->item_count = 0;
    dropdown->selected   = -1;
    dropdown->base.dirty = 1;
}

int zephio_dropdown_get_selected(const ZephioDropdown *dropdown)
{
    return dropdown ? dropdown->selected : -1;
}

const char *zephio_dropdown_get_selected_item(const ZephioDropdown *dropdown)
{
    if (!dropdown || dropdown->selected < 0 ||
        dropdown->selected >= dropdown->item_count)
        return NULL;
    return dropdown->items[dropdown->selected];
}

void zephio_dropdown_set_selected(ZephioDropdown *dropdown, int index)
{
    if (!dropdown) return;
    if (index < -1 || index >= dropdown->item_count) return;
    dropdown->selected   = index;
    dropdown->base.dirty = 1;
}

void zephio_dropdown_set_colors(ZephioDropdown *dropdown,
                             ZephioColor fg, ZephioColor bg,
                             ZephioColor fg_focused, ZephioColor bg_focused)
{
    if (!dropdown) return;
    dropdown->fg          = fg;
    dropdown->bg          = bg;
    dropdown->fg_selected = fg_focused;
    dropdown->bg_selected = bg_focused;
    dropdown->base.dirty  = 1;
}

void zephio_dropdown_set_popup_colors(ZephioDropdown *dropdown,
                                   ZephioColor fg, ZephioColor bg,
                                   ZephioColor fg_hl, ZephioColor bg_hl)
{
    if (!dropdown) return;
    dropdown->fg_popup    = fg;
    dropdown->bg_popup    = bg;
    dropdown->fg_selected = fg_hl;
    dropdown->bg_selected = bg_hl;
    dropdown->base.dirty  = 1;
}

void zephio_dropdown_set_on_change(ZephioDropdown *dropdown,
                                ZephioDropdownCallback callback,
                                void *user_data)
{
    if (!dropdown) return;
    dropdown->on_change = callback;
    dropdown->user_data = user_data;
}

void zephio_dropdown_set_max_visible(ZephioDropdown *dropdown, int max_visible)
{
    if (!dropdown) return;
    dropdown->max_visible = max_visible > 0 ? max_visible : ZEPHIO_DROPDOWN_MAX_VISIBLE;
}

void zephio_dropdown_open(ZephioDropdown *dropdown, void *app)
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

    zephio_widget_init_ctx(&dropdown->popup.base, px, py, pw, ph,
                        &popup_vtable, dropdown->base.ctx, NULL);
    dropdown->popup.base.focusable = 1;
    dropdown->popup.owner       = dropdown;
    dropdown->popup.highlighted = dropdown->selected >= 0
                                  ? dropdown->selected : 0;
    dropdown->popup.scroll_offset = 0;
    popup_ensure_visible(&dropdown->popup);

    zephio_app_push_overlay((ZephioApp *)app, &dropdown->popup.base);
}

void zephio_dropdown_close(ZephioDropdown *dropdown)
{
    if (!dropdown || !dropdown->is_open) return;
    popup_close(&dropdown->popup);
}
