/**
 * @file tui_textarea.h
 * @brief Multi-line editable text area widget.
 *
 * TuiTextArea provides a multi-line text editor with cursor navigation,
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

#ifndef TUI_TEXTAREA_H
#define TUI_TEXTAREA_H

#include "tui_widget.h"

/**
 * @brief Multi-line editable text area widget.
 */
typedef struct {
    TuiWidget base;
    char   **lines;
    int      line_count;
    int      line_capacity;
    int      cursor_row;
    int      cursor_col;
    int      scroll_y;
    int      scroll_x;
    int      preferred_col;
    int      has_preferred_col;
    TuiColor fg;
    TuiColor bg;
    TuiAttr  attr;
} TuiTextArea;

/**
 * @brief Initialize a text area widget with context.
 *
 * @param ta      TextArea struct to initialize.
 * @param ctx     TUI context.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param width   Width in columns.
 * @param height  Height in rows.
 * @return TUI_OK on success.
 */
TuiResult tui_textarea_init_ctx(TuiTextArea *ta, TuiContext *ctx, int x, int y,
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
void tui_textarea_set_text(TuiTextArea *ta, const char *text);

/**
 * @brief Return the current text content as a newly allocated string.
 *
 * Lines are joined with '\n'. Caller must free() the result.
 *
 * @param ta  TextArea widget.
 * @return Heap-allocated string, or NULL on error.
 */
char *tui_textarea_get_text(TuiTextArea *ta);

/**
 * @brief Set foreground, background, and attribute colors.
 */
void tui_textarea_set_colors(TuiTextArea *ta, TuiColor fg, TuiColor bg,
                              TuiAttr attr);

/**
 * @brief Return the current cursor row (0-based).
 */
int tui_textarea_get_cursor_row(TuiTextArea *ta);

/**
 * @brief Return the current cursor column (byte offset, 0-based).
 */
int tui_textarea_get_cursor_col(TuiTextArea *ta);

/**
 * @brief Return the current scroll Y offset.
 */
int tui_textarea_get_scroll_y(TuiTextArea *ta);

/**
 * @brief Return the current number of lines.
 */
int tui_textarea_get_line_count(TuiTextArea *ta);

#endif
