/**
 * @file zephio_box.h
 * @brief Bordered box/panel widget with optional title.
 *
 * Draws a Unicode box border (single or double-line) with an optional
 * title string centered in the top border. Interior is filled with the
 * box background color.
 *
 * Supports padding inside the border. If the box has children, they
 * are positioned within the padded interior.
 */

#ifndef ZEPHIO_BOX_H
#define ZEPHIO_BOX_H

#include "zephio_widget.h"

#define ZEPHIO_BOX_SINGLE 0
#define ZEPHIO_BOX_DOUBLE 1

/**
 * @brief Box widget data.
 */
typedef struct {
    ZephioWidget base;
    char     *title;
    ZephioColor  fg;
    ZephioColor  bg;
    ZephioAttr   attr;
    int       border_style;
    int       padding;
} ZephioBox;

/**
 * @brief Initialize a box widget with context.
 *
 * @param box           Box struct to initialize.
 * @param ctx           TUI context.
 * @param x             Column offset.
 * @param y             Row offset.
 * @param width         Width in columns.
 * @param height        Height in rows.
 * @param border_style  TUI_BOX_SINGLE or TUI_BOX_DOUBLE.
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_box_init_ctx(ZephioBox *box, ZephioContext *ctx, int x, int y, int width, int height,
                           int border_style);

/**
 * @brief Set or update the box title (centered in top border).
 */
void zephio_box_set_title(ZephioBox *box, const char *title);

/** @brief Set border and interior colors. */
void zephio_box_set_colors(ZephioBox *box, ZephioColor fg, ZephioColor bg);

/** @brief Set text attributes for the border. */
void zephio_box_set_attr(ZephioBox *box, ZephioAttr attr);

/** @brief Set inner padding (space between border and children). */
void zephio_box_set_padding(ZephioBox *box, int padding);

#endif
