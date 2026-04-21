/**
 * @file zephio_textarea.h
 * @brief Multi-line editable text area widget.
 *
 * ZephioTextArea provides a multi-line text editor with cursor navigation,
 * text insertion/deletion, line splitting (Enter), horizontal scrolling
 * for long lines, and cursor rendering via cell inversion.
 *
 * Supports:
 *   - Arrow keys, Home/End, PageUp/PageDown for navigation
 *   - Printable character insertion (UTF-8)
 *   - Backspace / Delete for character removal
 *   - Enter for line splitting
 *   - Mouse click to position cursor, mouse wheel to scroll
 */

#ifndef ZEPHIO_TEXTAREA_H
#define ZEPHIO_TEXTAREA_H

#include "zephio_widget.h"

/**
 * @brief Multi-line editable text area widget.
 */
typedef struct {
    ZephioWidget base;
    char   **lines;
    int      line_count;
    int      line_capacity;
    int      cursor_row;
    int      cursor_col;
    int      scroll_y;
    int      scroll_x;
    int      preferred_col;
    int      has_preferred_col;
    ZephioColor fg;
    ZephioColor bg;
    ZephioAttr  attr;
} ZephioTextArea;

/**
 * @brief Initialize a text area widget with context.
 *
 * @param ta      TextArea struct to initialize.
 * @param ctx     TUI context.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param width   Width in columns.
 * @param height  Height in rows.
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_textarea_init_ctx(ZephioTextArea *ta, ZephioContext *ctx, int x, int y,
                               int width, int height);

/**
 * @brief Set the text area content (copied).
 *
 * Splits the input at newline characters into internal lines.
 * Resets cursor and scroll position.
 *
 * @param ta    TextArea widget.
 * @param text  New text (may be NULL for empty).
 */
void zephio_textarea_set_text(ZephioTextArea *ta, const char *text);

/**
 * @brief Return the current text content as a newly allocated string.
 *
 * Lines are joined with '\n'. Caller must free() the result.
 *
 * @param ta  TextArea widget.
 * @return Heap-allocated string, or NULL on error.
 */
char *zephio_textarea_get_text(ZephioTextArea *ta);

/**
 * @brief Set foreground, background, and attribute colors.
 */
void zephio_textarea_set_colors(ZephioTextArea *ta, ZephioColor fg, ZephioColor bg,
                              ZephioAttr attr);

/**
 * @brief Return the current cursor row (0-based).
 */
int zephio_textarea_get_cursor_row(ZephioTextArea *ta);

/**
 * @brief Return the current cursor column (byte offset, 0-based).
 */
int zephio_textarea_get_cursor_col(ZephioTextArea *ta);

/**
 * @brief Return the current scroll Y offset.
 */
int zephio_textarea_get_scroll_y(ZephioTextArea *ta);

/**
 * @brief Return the current number of lines.
 */
int zephio_textarea_get_line_count(ZephioTextArea *ta);

#endif
