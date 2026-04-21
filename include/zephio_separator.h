/**
 * @file zephio_separator.h
 * @brief Horizontal or vertical divider line widget.
 *
 * Draws a single Unicode horizontal (─) or vertical (│) line
 * across the widget's full width or height.
 */

#ifndef ZEPHIO_SEPARATOR_H
#define ZEPHIO_SEPARATOR_H

#include "zephio_widget.h"

/**
 * @brief Separator widget data.
 */
typedef struct {
    ZephioWidget base;
    int       horizontal;
    ZephioColor  fg;
    ZephioColor  bg;
    ZephioAttr   attr;
} ZephioSeparator;

/**
 * @brief Initialize a horizontal separator with context.
 *
 * Height is set to 1.
 *
 * @param sep    Separator struct to initialize.
 * @param ctx    TUI context.
 * @param x      Column offset.
 * @param y      Row offset.
 * @param width  Line width in columns.
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_separator_init_h_ctx(ZephioSeparator *sep, ZephioContext *ctx, int x, int y, int width);

/**
 * @brief Initialize a vertical separator with context.
 *
 * Width is set to 1.
 *
 * @param sep     Separator struct to initialize.
 * @param ctx     TUI context.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param height  Line height in rows.
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_separator_init_v_ctx(ZephioSeparator *sep, ZephioContext *ctx, int x, int y, int height);

/** @brief Set separator colors. */
void zephio_separator_set_colors(ZephioSeparator *sep, ZephioColor fg, ZephioColor bg);

/** @brief Set separator text attributes. */
void zephio_separator_set_attr(ZephioSeparator *sep, ZephioAttr attr);

#endif
