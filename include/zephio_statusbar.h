/**
 * @file zephio_statusbar.h
 * @brief Status bar widget for displaying contextual information.
 *
 * Renders a single-row bar at the bottom of the screen with three
 * text segments: left, center, and right. Supports temporary messages
 * that auto-expire after a configurable tick count.
 *
 * Usage:
 *   1. zephio_statusbar_init() with position and width.
 *   2. zephio_statusbar_set_text() for each segment.
 *   3. zephio_statusbar_set_message() for temporary notifications.
 *   4. Add to widget tree via zephio_widget_add_child().
 *   5. Call zephio_statusbar_tick() each frame to expire messages.
 */

#ifndef ZEPHIO_STATUSBAR_H
#define ZEPHIO_STATUSBAR_H

#include "zephio_widget.h"

#define ZEPHIO_STATUSBAR_MSG_TICKS 300

typedef struct ZephioStatusBar ZephioStatusBar;

struct ZephioStatusBar {
    ZephioWidget base;

    char *text_left;
    char *text_center;
    char *text_right;

    char *message;
    int   message_ticks;

    ZephioColor fg;
    ZephioColor bg;
    ZephioColor fg_message;
    ZephioColor bg_message;
    ZephioAttr  attr;
};

ZephioResult zephio_statusbar_init_ctx(ZephioStatusBar *statusbar, ZephioContext *ctx, int x, int y, int width);

void zephio_statusbar_set_text(ZephioStatusBar *statusbar,
                            const char *left, const char *center,
                            const char *right);

void zephio_statusbar_set_message(ZephioStatusBar *statusbar,
                               const char *message, int ticks);

void zephio_statusbar_set_colors(ZephioStatusBar *statusbar,
                              ZephioColor fg, ZephioColor bg);

void zephio_statusbar_set_message_colors(ZephioStatusBar *statusbar,
                                      ZephioColor fg, ZephioColor bg);

void zephio_statusbar_tick(ZephioStatusBar *statusbar);

#endif
