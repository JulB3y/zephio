/**
 * @file zephio_label.h
 * @brief Static text label widget.
 *
 * Displays a single line of text with configurable colors and attributes.
 * The label is not focusable and does not handle input.
 */

#ifndef ZEPHIO_LABEL_H
#define ZEPHIO_LABEL_H

#include "zephio_widget.h"

/**
 * @brief Label widget data.
 */
typedef struct {
    ZephioWidget base;
    char    *text;
    ZephioColor fg;
    ZephioColor bg;
    ZephioAttr  attr;
} ZephioLabel;

/**
 * @brief Initialize a label widget with context.
 *
 * @param label   Label struct to initialize.
 * @param ctx     TUI context.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param width   Width in columns.
 * @param height  Height in rows.
 * @param text    Initial text (copied; may be NULL).
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_label_init_ctx(ZephioLabel *label, ZephioContext *ctx, int x, int y, int width, int height,
                             const char *text);

/**
 * @brief Update label text.
 *
 * @param label  Label widget.
 * @param text   New text (copied; may be NULL).
 */
void zephio_label_set_text(ZephioLabel *label, const char *text);

/**
 * @brief Set foreground and background colors.
 */
void zephio_label_set_colors(ZephioLabel *label, ZephioColor fg, ZephioColor bg);

/**
 * @brief Set text attributes (bold, underline, etc.).
 */
void zephio_label_set_attr(ZephioLabel *label, ZephioAttr attr);

#endif
