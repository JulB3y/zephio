/**
 * @file tui_input_field.h
 * @brief Single-line text input field widget.
 *
 * Provides a text field with cursor display, horizontal scrolling,
 * and callbacks for text changes and submission (Enter key).
 *
 * Currently supports ASCII input only (codepoints 32-126).
 * The field allocates a text buffer lazily on first character input.
 */

#ifndef ZEPHIO_INPUT_FIELD_H
#define ZEPHIO_INPUT_FIELD_H

#include "tui_widget.h"

/** @brief Callback for text changes and submissions. */
typedef void (*TuiInputFieldCallback)(TuiWidget *widget, const char *text,
                                      void *user_data);

/**
 * @brief Input field widget data.
 */
typedef struct {
    TuiWidget            base;
    char                *text;
    int                  text_capacity;
    int                  cursor_pos;
    int                  scroll_offset;
    TuiColor             fg;
    TuiColor             bg;
    TuiColor             cursor_fg;
    TuiColor             cursor_bg;
    TuiAttr              attr;
    TuiInputFieldCallback on_change;
    TuiInputFieldCallback on_submit;
    void                *user_data;
} TuiInputField;

/**
 * @brief Initialize an input field widget with context.
 *
 * @param field     Field struct to initialize.
 * @param ctx       TUI context.
 * @param x         Column offset.
 * @param y         Row offset.
 * @param width     Visible width in columns.
 * @param capacity  Maximum text buffer size (bytes).
 * @return TUI_OK on success.
 */
TuiResult tui_input_field_init_ctx(TuiInputField *field, TuiContext *ctx, int x, int y, int width,
                                  int capacity);

/**
 * @brief Set the field text programmatically.
 */
void tui_input_field_set_text(TuiInputField *field, const char *text);

/**
 * @brief Return the current field text.
 *
 * @return Internal buffer pointer (valid until next set_text or destroy).
 */
const char *tui_input_field_get_text(TuiInputField *field);

/** @brief Set normal and cursor color pairs. */
void tui_input_field_set_colors(TuiInputField *field, TuiColor fg, TuiColor bg,
                                TuiColor cursor_fg, TuiColor cursor_bg);

/** @brief Set the text-change callback. */
void tui_input_field_set_on_change(TuiInputField *field,
                                   TuiInputFieldCallback callback,
                                   void *user_data);

/** @brief Set the submit callback (Enter key). */
void tui_input_field_set_on_submit(TuiInputField *field,
                                   TuiInputFieldCallback callback,
                                   void *user_data);

#endif
