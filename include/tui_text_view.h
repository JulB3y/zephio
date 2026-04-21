/**
 * @file tui_text_view.h
 * @brief Multi-line text display widget with word-wrapping and scrolling.
 *
 * TuiTextView displays read-only text with automatic word-wrapping and
 * scroll container integration. Supports keyboard (arrows, PageUp/Down,
 * Home/End) and mouse-wheel scrolling via the embedded TuiScrollContainer.
 */

#ifndef ZEPHIO_TEXT_VIEW_H
#define ZEPHIO_TEXT_VIEW_H

#include "tui_scroll_container.h"

/**
 * @brief Multi-line text display widget.
 */
typedef struct {
    TuiScrollContainer base;
    char  *text;
    int    text_len;
    int   *line_starts;
    int   *line_lengths;
    int    line_count;
    int    line_capacity;
    int    max_line_width;
    TuiColor fg;
    TuiColor bg;
    TuiAttr  attr;
    int    word_wrap;
} TuiTextView;

/**
 * @brief Initialize a text view widget with context.
 *
 * @param tv      TextView struct to initialize.
 * @param ctx     TUI context.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param width   Width in columns.
 * @param height  Height in rows.
 * @return TUI_OK on success.
 */
TuiResult tui_text_view_init_ctx(TuiTextView *tv, TuiContext *ctx, int x, int y,
                               int width, int height);

/**
 * @brief Set the display text (copied).
 *
 * Recomputes word-wrapped lines and updates scroll bounds.
 *
 * @param tv    TextView widget.
 * @param text  New text to display (may be NULL).
 */
void tui_text_view_set_text(TuiTextView *tv, const char *text);

/**
 * @brief Set foreground and background colors.
 */
void tui_text_view_set_colors(TuiTextView *tv, TuiColor fg, TuiColor bg);

/**
 * @brief Set text attributes (bold, underline, etc.).
 */
void tui_text_view_set_attr(TuiTextView *tv, TuiAttr attr);

/**
 * @brief Enable or disable word-wrapping (default: enabled).
 *
 * When disabled, lines are split only at newline characters and
 * horizontal scrolling becomes available for long lines.
 */
void tui_text_view_set_word_wrap(TuiTextView *tv, int enabled);

/**
 * @brief Return the current number of wrapped lines.
 */
int tui_text_view_get_line_count(TuiTextView *tv);

/**
 * @brief Return the current scroll Y offset.
 */
int tui_text_view_get_scroll_y(TuiTextView *tv);

#endif
