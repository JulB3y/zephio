#define _POSIX_C_SOURCE 200809L

#include "tui_statusbar.h"
#include "tui_context.h"

#include <stdlib.h>
#include <string.h>

static void statusbar_render(TuiWidget *widget)
{
    TuiStatusBar *sb = (TuiStatusBar *)widget;

    TuiColor fg = sb->fg;
    TuiColor bg = sb->bg;
    TuiAttr  at = sb->attr;

    if (sb->message && sb->message_ticks > 0) {
        fg = sb->fg_message;
        bg = sb->bg_message;
    }

    tui_screen_fill(tui_current_ctx, widget->abs_y, widget->abs_x,
                    widget->width, widget->height, " ", fg, bg, at);

    if (sb->message && sb->message_ticks > 0) {
        tui_screen_write(tui_current_ctx, widget->abs_y, widget->abs_x,
                         sb->message, fg, bg, at);
        return;
    }

    int left_len = sb->text_left ? (int)strlen(sb->text_left) : 0;
    int center_len = sb->text_center ? (int)strlen(sb->text_center) : 0;
    int right_len = sb->text_right ? (int)strlen(sb->text_right) : 0;

    if (left_len > 0) {
        int maxw = widget->width / 3;
        int wlen = left_len < maxw ? left_len : maxw;
        char buf[256];
        int clen = wlen < (int)sizeof(buf) - 1 ? wlen : (int)sizeof(buf) - 1;
        memcpy(buf, sb->text_left, (size_t)clen);
        buf[clen] = '\0';
        tui_screen_write(tui_current_ctx, widget->abs_y, widget->abs_x + 1, buf, fg, bg, at);
    }

    if (center_len > 0) {
        int maxw = widget->width / 3;
        int wlen = center_len < maxw ? center_len : maxw;
        char buf[256];
        int clen = wlen < (int)sizeof(buf) - 1 ? wlen : (int)sizeof(buf) - 1;
        memcpy(buf, sb->text_center, (size_t)clen);
        buf[clen] = '\0';
        int cx = (widget->width - wlen) / 2;
        if (cx < 0) cx = 0;
        tui_screen_write(tui_current_ctx, widget->abs_y, widget->abs_x + cx, buf, fg, bg, at);
    }

    if (right_len > 0) {
        int maxw = widget->width / 3;
        int wlen = right_len < maxw ? right_len : maxw;
        char buf[256];
        int clen = wlen < (int)sizeof(buf) - 1 ? wlen : (int)sizeof(buf) - 1;
        memcpy(buf, sb->text_right, (size_t)clen);
        buf[clen] = '\0';
        int rx = widget->width - wlen - 1;
        if (rx < 0) rx = 0;
        tui_screen_write(tui_current_ctx, widget->abs_y, widget->abs_x + rx, buf, fg, bg, at);
    }
}

static void statusbar_destroy(TuiWidget *widget)
{
    TuiStatusBar *sb = (TuiStatusBar *)widget;
    free(sb->text_left);
    free(sb->text_center);
    free(sb->text_right);
    free(sb->message);
    sb->text_left   = NULL;
    sb->text_center = NULL;
    sb->text_right  = NULL;
    sb->message     = NULL;
}

static TuiWidgetVTable statusbar_vtable = {
    .render       = statusbar_render,
    .handle_input = NULL,
    .handle_mouse = NULL,
    .destroy      = statusbar_destroy,
    .on_resize    = NULL,
    .on_focus     = NULL,
    .on_blur      = NULL
};

TuiResult tui_statusbar_init(TuiStatusBar *statusbar, int x, int y, int width)
{
    if (!statusbar) return TUI_ERR_MEMORY;

    TuiResult res = tui_widget_init(&statusbar->base, x, y, width, 1,
                                    &statusbar_vtable, NULL);
    if (res != TUI_OK) return res;

    statusbar->base.focusable = 0;

    statusbar->text_left   = NULL;
    statusbar->text_center = NULL;
    statusbar->text_right  = NULL;
    statusbar->message     = NULL;
    statusbar->message_ticks = 0;

    statusbar->fg          = TUI_COLOR_INDEX(15);
    statusbar->bg          = TUI_COLOR_INDEX(4);
    statusbar->fg_message  = TUI_COLOR_INDEX(0);
    statusbar->bg_message  = TUI_COLOR_INDEX(11);
    statusbar->attr        = TUI_ATTR_BOLD;

    return TUI_OK;
}

void tui_statusbar_set_text(TuiStatusBar *statusbar,
                            const char *left, const char *center,
                            const char *right)
{
    if (!statusbar) return;

    free(statusbar->text_left);
    free(statusbar->text_center);
    free(statusbar->text_right);

    statusbar->text_left   = left ? strdup(left) : NULL;
    statusbar->text_center = center ? strdup(center) : NULL;
    statusbar->text_right  = right ? strdup(right) : NULL;
    statusbar->base.dirty  = 1;
}

void tui_statusbar_set_message(TuiStatusBar *statusbar,
                               const char *message, int ticks)
{
    if (!statusbar) return;

    free(statusbar->message);
    statusbar->message = message ? strdup(message) : NULL;
    statusbar->message_ticks = ticks > 0 ? ticks : 0;
    statusbar->base.dirty = 1;
}

void tui_statusbar_set_colors(TuiStatusBar *statusbar,
                              TuiColor fg, TuiColor bg)
{
    if (!statusbar) return;
    statusbar->fg = fg;
    statusbar->bg = bg;
    statusbar->base.dirty = 1;
}

void tui_statusbar_set_message_colors(TuiStatusBar *statusbar,
                                      TuiColor fg, TuiColor bg)
{
    if (!statusbar) return;
    statusbar->fg_message = fg;
    statusbar->bg_message = bg;
    statusbar->base.dirty = 1;
}

void tui_statusbar_tick(TuiStatusBar *statusbar)
{
    if (!statusbar) return;
    if (statusbar->message_ticks > 0) {
        statusbar->message_ticks--;
        if (statusbar->message_ticks == 0) {
            free(statusbar->message);
            statusbar->message = NULL;
            statusbar->base.dirty = 1;
        }
    }
}
