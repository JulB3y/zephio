/**
 * @file zephio_input_field.h
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

#include "zephio_widget.h"

/** @brief Callback for text changes and submissions. */
typedef void (*ZephioInputFieldCallback)(ZephioWidget *widget, const char *text,
                                      void *user_data);

/**
 * @brief Input field widget data.
 */
typedef struct {
    ZephioWidget            base;
    char                *text;
    int                  text_capacity;
    int                  cursor_pos;
    int                  scroll_offset;
    ZephioColor             fg;
    ZephioColor             bg;
    ZephioColor             cursor_fg;
    ZephioColor             cursor_bg;
    ZephioAttr              attr;
    ZephioInputFieldCallback on_change;
    ZephioInputFieldCallback on_submit;
    void                *user_data;
} ZephioInputField;

/**
 * @brief Initialize an input field widget with context.
 *
 * @param field     Field struct to initialize.
 * @param ctx       TUI context.
 * @param x         Column offset.
 * @param y         Row offset.
 * @param width     Visible width in columns.
 * @param capacity  Maximum text buffer size (bytes).
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_input_field_init_ctx(ZephioInputField *field, ZephioContext *ctx, int x, int y, int width,
                                  int capacity);

/**
 * @brief Set the field text programmatically.
 */
void zephio_input_field_set_text(ZephioInputField *field, const char *text);

/**
 * @brief Return the current field text.
 *
 * @return Internal buffer pointer (valid until next set_text or destroy).
 */
const char *zephio_input_field_get_text(ZephioInputField *field);

/** @brief Set normal and cursor color pairs. */
void zephio_input_field_set_colors(ZephioInputField *field, ZephioColor fg, ZephioColor bg,
                                ZephioColor cursor_fg, ZephioColor cursor_bg);

/** @brief Set the text-change callback. */
void zephio_input_field_set_on_change(ZephioInputField *field,
                                   ZephioInputFieldCallback callback,
                                   void *user_data);

/** @brief Set the submit callback (Enter key). */
void zephio_input_field_set_on_submit(ZephioInputField *field,
                                   ZephioInputFieldCallback callback,
                                   void *user_data);

#endif
