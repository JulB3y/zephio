#define _POSIX_C_SOURCE 200809L

#include "zephio_dialog.h"
#include "zephio_context.h"
#include "zephio_screen.h"

#include <stdlib.h>
#include <string.h>

static int count_lines(const char *s, int width, int *max_line_len)
{
    if (!s || !*s || width <= 0) {
        if (max_line_len) *max_line_len = 0;
        return 0;
    }

    int lines = 1;
    int cur_len = 0;
    int max_len = 0;

    for (const char *p = s; *p; p++) {
        if (*p == '\n') {
            if (cur_len > max_len) max_len = cur_len;
            cur_len = 0;
            lines++;
        } else {
            cur_len++;
        }
    }
    if (cur_len > max_len) max_len = cur_len;

    if (max_line_len) *max_line_len = max_len;
    return lines;
}

static void dialog_render(ZephioWidget *widget)
{
    ZephioDialog *dialog = (ZephioDialog *)widget;

    ZephioColor border_fg = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_CYAN);
    ZephioColor border_bg = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BG_DARK);
    ZephioColor title_fg  = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_YELLOW);
    ZephioColor text_fg   = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_WHITE);
    ZephioColor text_bg   = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BG_DARK);
    ZephioAttr  border_attr = ZEPHIO_ATTR_BOLD;

    zephio_screen_fill(widget->ctx, widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ", text_fg, text_bg, ZEPHIO_ATTR_NONE);

    zephio_screen_box_double(widget->ctx, widget->abs_y, widget->abs_x,
                          widget->width, widget->height,
                          border_fg, border_bg, border_attr);

    if (dialog->title) {
        int title_len = (int)strlen(dialog->title);
        int max_title = widget->width - 4;
        if (max_title < 0) max_title = 0;
        int write_len = title_len < max_title ? title_len : max_title;
        int col = widget->abs_x + (widget->width - write_len) / 2;

        char buf[256];
        int copy_len = write_len < (int)sizeof(buf) - 1 ? write_len : (int)sizeof(buf) - 1;
        memcpy(buf, dialog->title, (size_t)copy_len);
        buf[copy_len] = '\0';
        zephio_screen_write(widget->ctx, widget->abs_y, col, buf, title_fg, border_bg,
                         border_attr | ZEPHIO_ATTR_BOLD);
    }

    if (dialog->message) {
        int row = widget->abs_y + 2;
        int max_width = widget->width - 4;
        if (max_width < 0) max_width = 0;
        const char *p = dialog->message;

        while (*p && row < widget->abs_y + widget->height - 2) {
            const char *eol = strchr(p, '\n');
            int line_len = eol ? (int)(eol - p) : (int)strlen(p);

            int write_len = line_len < max_width ? line_len : max_width;
            char buf[512];
            int copy_len = write_len < (int)sizeof(buf) - 1
                           ? write_len : (int)sizeof(buf) - 1;
            memcpy(buf, p, (size_t)copy_len);
            buf[copy_len] = '\0';

            zephio_screen_write(widget->ctx, row, widget->abs_x + 2, buf, text_fg, text_bg,
                             ZEPHIO_ATTR_NONE);
            row++;

            if (eol)
                p = eol + 1;
            else
                break;
        }
    }

    if (dialog->button_count > 0) {
        int btn_row = widget->abs_y + widget->height - 2;

        int total_btn_width = 0;
        for (int i = 0; i < dialog->button_count; i++) {
            int label_len = (int)strlen(dialog->button_labels[i]);
            total_btn_width += label_len + 4;
        }
        total_btn_width += (dialog->button_count - 1) * 2;

        int btn_col = widget->abs_x + (widget->width - total_btn_width) / 2;
        if (btn_col < widget->abs_x + 1) btn_col = widget->abs_x + 1;

        for (int i = 0; i < dialog->button_count; i++) {
            int label_len = (int)strlen(dialog->button_labels[i]);
            int btn_width = label_len + 4;

            ZephioColor btn_fg = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_WHITE);
            ZephioColor btn_bg = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BLUE);
            ZephioAttr  btn_attr = ZEPHIO_ATTR_NONE;

            if (i == dialog->selected_button) {
                if (widget->focused) {
                    btn_fg = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BLACK);
                    btn_bg = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_CYAN);
                    btn_attr = ZEPHIO_ATTR_BOLD | ZEPHIO_ATTR_REVERSE;
                } else {
                    btn_fg = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BRIGHT_CYAN);
                    btn_bg = ZEPHIO_COLOR_INDEX(ZEPHIO_COLOR_BG_MID);
                    btn_attr = ZEPHIO_ATTR_BOLD;
                }
            }

            zephio_screen_write(widget->ctx, btn_row, btn_col, "[",
                             btn_fg, btn_bg, btn_attr);
            zephio_screen_write(widget->ctx, btn_row, btn_col + 1, dialog->button_labels[i],
                             btn_fg, btn_bg, btn_attr);
            zephio_screen_write(widget->ctx, btn_row, btn_col + 1 + label_len, "]",
                             btn_fg, btn_bg, btn_attr);

            btn_col += btn_width + 2;
        }
    }
}

static int dialog_handle_input(ZephioWidget *widget, const ZephioEvent *event)
{
    ZephioDialog *dialog = (ZephioDialog *)widget;

    if (event->key == ZEPHIO_KEY_RIGHT) {
        if (dialog->selected_button > 0) {
            dialog->selected_button--;
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == ZEPHIO_KEY_LEFT) {
        if (dialog->selected_button < dialog->button_count - 1) {
            dialog->selected_button++;
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == ZEPHIO_KEY_TAB) {
        if (dialog->button_count > 0) {
            dialog->selected_button = (dialog->selected_button + 1)
                                       % dialog->button_count;
            widget->dirty = 1;
        }
        return 1;
    }

    if (event->key == ZEPHIO_KEY_ENTER || event->codepoint == ' ') {
        if (dialog->on_button && dialog->button_count > 0) {
            dialog->on_button(dialog, dialog->selected_button,
                              dialog->user_data);
        }
        return 1;
    }

    if (event->key == ZEPHIO_KEY_ESCAPE) {
        if (dialog->on_button) {
            dialog->on_button(dialog, -1, dialog->user_data);
        }
        return 1;
    }

    return 1;
}

static int dialog_handle_mouse(ZephioWidget *widget, const ZephioMouseEvent *mouse)
{
    ZephioDialog *dialog = (ZephioDialog *)widget;

    if (mouse->action != ZEPHIO_MOUSE_PRESS || mouse->button != ZEPHIO_MOUSE_BTN_LEFT)
        return 1;

    if (dialog->button_count == 0) return 1;

    int btn_row = widget->abs_y + widget->height - 2;

    int total_btn_width = 0;
    for (int i = 0; i < dialog->button_count; i++) {
        int label_len = (int)strlen(dialog->button_labels[i]);
        total_btn_width += label_len + 4;
    }
    total_btn_width += (dialog->button_count - 1) * 2;

    int btn_col = widget->abs_x + (widget->width - total_btn_width) / 2;
    if (btn_col < widget->abs_x + 1) btn_col = widget->abs_x + 1;

    if (mouse->row == btn_row) {
        for (int i = 0; i < dialog->button_count; i++) {
            int label_len = (int)strlen(dialog->button_labels[i]);
            int btn_width = label_len + 4;
            if (mouse->col >= btn_col && mouse->col < btn_col + btn_width) {
                dialog->selected_button = i;
                widget->dirty = 1;
                if (dialog->on_button) {
                    dialog->on_button(dialog, i, dialog->user_data);
                }
                return 1;
            }
            btn_col += btn_width + 2;
        }
    }

    return 1;
}

static void dialog_destroy(ZephioWidget *widget)
{
    ZephioDialog *dialog = (ZephioDialog *)widget;
    free(dialog->title);
    dialog->title = NULL;
    free(dialog->message);
    dialog->message = NULL;
}

static ZephioWidgetVTable dialog_vtable = {
    .render       = dialog_render,
    .handle_input = dialog_handle_input,
    .handle_mouse = dialog_handle_mouse,
    .destroy      = dialog_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

ZephioResult zephio_dialog_init_ctx(ZephioDialog *dialog, ZephioContext *ctx, const char *title,
                              const char *message)
{
    if (!dialog) return TUI_ERR_MEMORY;

    int msg_lines = 0, max_line = 0;
    if (message) {
        msg_lines = count_lines(message, 80, &max_line);
    }

    int title_len = title ? (int)strlen(title) : 0;
    int w = max_line + 4;
    if (title_len + 4 > w) w = title_len + 4;
    if (w < 20) w = 20;
    if (w > 76) w = 76;

    int h = 3 + msg_lines + 2 + 1;
    if (title) h++;
    if (h < 7) h = 7;
    if (h > 23) h = 23;

    ZephioResult res = zephio_widget_init_ctx(&dialog->base, 0, 0, w, h,
                                        &dialog_vtable, ctx, NULL);
    if (res != ZEPHIO_OK) return res;

    dialog->base.focusable = 1;

    dialog->title = title ? strdup(title) : NULL;
    dialog->message = message ? strdup(message) : NULL;
    dialog->button_count = 0;
    dialog->selected_button = 0;
    dialog->on_button = NULL;
    dialog->user_data = NULL;

    memset(dialog->button_labels, 0, sizeof(dialog->button_labels));

    return ZEPHIO_OK;
}

int zephio_dialog_add_button(ZephioDialog *dialog, const char *label)
{
    if (!dialog || !label) return -1;
    if (dialog->button_count >= ZEPHIO_DIALOG_MAX_BUTTONS) return -1;

    int idx = dialog->button_count++;
    strncpy(dialog->button_labels[idx], label, ZEPHIO_DIALOG_MAX_BUTTON_LABEL - 1);
    dialog->button_labels[idx][ZEPHIO_DIALOG_MAX_BUTTON_LABEL - 1] = '\0';
    dialog->base.dirty = 1;

    return idx;
}

void zephio_dialog_set_on_button(ZephioDialog *dialog, ZephioDialogCallback callback,
                              void *user_data)
{
    if (!dialog) return;
    dialog->on_button = callback;
    dialog->user_data = user_data;
}

void zephio_dialog_center(ZephioContext *ctx, ZephioDialog *dialog)
{
    if (!dialog) return;

    ZephioSize size = zephio_screen_size(ctx);
    int x = (size.cols - dialog->base.width) / 2;
    int y = (size.rows - dialog->base.height) / 2;
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    zephio_widget_set_position(&dialog->base, x, y);
}

int zephio_dialog_get_selected(ZephioDialog *dialog)
{
    if (!dialog) return -1;
    return dialog->selected_button;
}
