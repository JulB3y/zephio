#define _POSIX_C_SOURCE 200809L

#include "tui_list.h"

#include <stdlib.h>
#include <string.h>

#define ITEMS_INITIAL_CAPACITY 8

static void list_render(TuiWidget *widget)
{
    TuiList *list = (TuiList *)widget;

    for (int i = 0; i < widget->height; i++) {
        int idx = list->scroll_offset + i;
        if (idx >= list->item_count) break;

        TuiColor fg;
        TuiColor bg;
        TuiAttr attr;

        if (widget->theme) {
            TuiWidgetState state = TUI_STATE_NORMAL;
            if (widget->disabled) {
                state = TUI_STATE_DISABLED;
            } else if (idx == list->selected && widget->focused) {
                state = TUI_STATE_FOCUSED;
            }
            TuiStyle style = widget->theme->styles[state];
            fg   = style.fg;
            bg   = style.bg;
            attr = style.attr;
        } else {
            fg   = list->fg;
            bg   = list->bg;
            attr = list->attr;

            if (idx == list->selected && widget->focused) {
                fg   = list->fg_selected;
                bg   = list->bg_selected;
                attr |= TUI_ATTR_REVERSE;
            }
        }

        tui_screen_fill(widget->abs_y + i, widget->abs_x,
                        widget->width, 1, " ", fg, bg, attr);

        if (list->items[idx]) {
            int item_len = (int)strlen(list->items[idx]);
            int max_width = widget->width - 2;
            if (max_width < 0) max_width = 0;
            int write_len = item_len < max_width ? item_len : max_width;

            char buf[256];
            int copy_len = write_len < (int)sizeof(buf) - 1 ? write_len : (int)sizeof(buf) - 1;
            memcpy(buf, list->items[idx], (size_t)copy_len);
            buf[copy_len] = '\0';

            char prefix[2] = " ";
            if (idx == list->selected && widget->focused) {
                prefix[0] = '>';
            }

            tui_screen_write(widget->abs_y + i, widget->abs_x,
                             prefix, fg, bg, attr);
            tui_screen_write(widget->abs_y + i, widget->abs_x + 1,
                             buf, fg, bg, attr);
        }
    }
}

static void list_ensure_visible(TuiList *list)
{
    if (list->selected < list->scroll_offset) {
        list->scroll_offset = list->selected;
    }

    if (list->selected >= list->scroll_offset + list->base.height) {
        list->scroll_offset = list->selected - list->base.height + 1;
    }
}

static int list_handle_input(TuiWidget *widget, const TuiEvent *event)
{
    TuiList *list = (TuiList *)widget;

    if (event->key == TUI_KEY_UP) {
        if (list->selected > 0) {
            list->selected--;
            list_ensure_visible(list);
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == TUI_KEY_DOWN) {
        if (list->selected < list->item_count - 1) {
            list->selected++;
            list_ensure_visible(list);
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == TUI_KEY_ENTER) {
        if (list->on_select && list->item_count > 0) {
            list->on_select(widget, list->selected,
                            list->items[list->selected], list->user_data);
        }
        return 1;
    }

    return 0;
}

static int list_handle_mouse(TuiWidget *widget, const TuiMouseEvent *mouse)
{
    TuiList *list = (TuiList *)widget;

    if (mouse->action != TUI_MOUSE_PRESS || mouse->button != TUI_MOUSE_BTN_LEFT)
        return 0;

    int idx = list->scroll_offset + (mouse->row - widget->abs_y);
    if (idx < 0 || idx >= list->item_count)
        return 0;

    list->selected = idx;
    list_ensure_visible(list);
    widget->dirty = 1;

    if (list->on_select) {
        list->on_select(widget, idx, list->items[idx], list->user_data);
    }

    return 1;
}

static void list_destroy(TuiWidget *widget)
{
    TuiList *list = (TuiList *)widget;

    for (int i = 0; i < list->item_count; i++) {
        free(list->items[i]);
    }
    free(list->items);
    list->items        = NULL;
    list->item_count   = 0;
    list->item_capacity = 0;
}

static TuiWidgetVTable list_vtable = {
    .render       = list_render,
    .handle_input = list_handle_input,
    .handle_mouse = list_handle_mouse,
    .destroy      = list_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_list_init(TuiList *list, int x, int y, int width, int height)
{
    if (!list) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init(&list->base, x, y, width, height,
                                    &list_vtable, NULL);
    if (res != TUI_OK) return res;

    list->base.focusable = 1;

    list->items         = NULL;
    list->item_count    = 0;
    list->item_capacity = 0;
    list->selected      = 0;
    list->scroll_offset = 0;
    list->fg            = TUI_COLOR_INDEX(15);
    list->bg            = TUI_COLOR_INDEX(0);
    list->fg_selected   = TUI_COLOR_INDEX(0);
    list->bg_selected   = TUI_COLOR_INDEX(12);
    list->attr          = TUI_ATTR_NONE;
    list->on_select     = NULL;
    list->user_data     = NULL;

    return TUI_OK;
}

TuiResult tui_list_add_item(TuiList *list, const char *item)
{
    if (!list) return TUI_ERR_MEMORY;

    if (list->item_count >= list->item_capacity) {
        int new_cap = list->item_capacity == 0
            ? ITEMS_INITIAL_CAPACITY
            : list->item_capacity * 2;

        char **new_items = (char **)realloc(
            list->items, (size_t)new_cap * sizeof(char *));
        if (!new_items) return TUI_ERR_MEMORY;

        list->items        = new_items;
        list->item_capacity = new_cap;
    }

    list->items[list->item_count] = item ? strdup(item) : NULL;
    list->item_count++;
    list->base.dirty = 1;

    return TUI_OK;
}

void tui_list_remove_item(TuiList *list, int index)
{
    if (!list || index < 0 || index >= list->item_count) return;

    free(list->items[index]);
    memmove(&list->items[index], &list->items[index + 1],
            (size_t)(list->item_count - index - 1) * sizeof(char *));
    list->item_count--;

    if (list->selected >= list->item_count && list->item_count > 0) {
        list->selected = list->item_count - 1;
    }

    list_ensure_visible(list);
    list->base.dirty = 1;
}

void tui_list_clear(TuiList *list)
{
    if (!list) return;

    for (int i = 0; i < list->item_count; i++) {
        free(list->items[i]);
    }
    list->item_count   = 0;
    list->selected     = 0;
    list->scroll_offset = 0;
    list->base.dirty   = 1;
}

int tui_list_get_selected(TuiList *list)
{
    if (!list) return -1;
    return list->selected;
}

const char *tui_list_get_selected_item(TuiList *list)
{
    if (!list || list->item_count == 0) return NULL;
    return list->items[list->selected];
}

void tui_list_set_colors(TuiList *list, TuiColor fg, TuiColor bg,
                         TuiColor fg_selected, TuiColor bg_selected)
{
    if (!list) return;
    list->fg           = fg;
    list->bg           = bg;
    list->fg_selected  = fg_selected;
    list->bg_selected  = bg_selected;
    list->base.dirty   = 1;
}

void tui_list_set_on_select(TuiList *list, TuiListCallback callback,
                            void *user_data)
{
    if (!list) return;
    list->on_select = callback;
    list->user_data = user_data;
}
