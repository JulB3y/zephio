/**
 * @file tui_statusbar.h
 * @brief Status bar widget for displaying contextual information.
 *
 * Renders a single-row bar at the bottom of the screen with three
 * text segments: left, center, and right. Supports temporary messages
 * that auto-expire after a configurable tick count.
 *
 * Usage:
 *   1. tui_statusbar_init() with position and width.
 *   2. tui_statusbar_set_text() for each segment.
 *   3. tui_statusbar_set_message() for temporary notifications.
 *   4. Add to widget tree via tui_widget_add_child().
 *   5. Call tui_statusbar_tick() each frame to expire messages.
 */

#ifndef TUI_STATUSBAR_H
#define TUI_STATUSBAR_H

#include "tui_widget.h"

#define TUI_STATUSBAR_MSG_TICKS 300

typedef struct TuiStatusBar TuiStatusBar;

struct TuiStatusBar {
    TuiWidget base;

    char *text_left;
    char *text_center;
    char *text_right;

    char *message;
    int   message_ticks;

    TuiColor fg;
    TuiColor bg;
    TuiColor fg_message;
    TuiColor bg_message;
    TuiAttr  attr;
};

TuiResult tui_statusbar_init_ctx(TuiStatusBar *statusbar, TuiContext *ctx, int x, int y, int width);

void tui_statusbar_set_text(TuiStatusBar *statusbar,
                            const char *left, const char *center,
                            const char *right);

void tui_statusbar_set_message(TuiStatusBar *statusbar,
                               const char *message, int ticks);

void tui_statusbar_set_colors(TuiStatusBar *statusbar,
                              TuiColor fg, TuiColor bg);

void tui_statusbar_set_message_colors(TuiStatusBar *statusbar,
                                      TuiColor fg, TuiColor bg);

void tui_statusbar_tick(TuiStatusBar *statusbar);

#endif
