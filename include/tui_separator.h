/**
 * @file tui_separator.h
 * @brief Horizontal or vertical divider line widget.
 *
 * Draws a single Unicode horizontal (─) or vertical (│) line
 * across the widget's full width or height.
 */

#ifndef TUI_SEPARATOR_H
#define TUI_SEPARATOR_H

#include "tui_widget.h"

/**
 * @brief Separator widget data.
 */
typedef struct {
    TuiWidget base;
    int       horizontal;
    TuiColor  fg;
    TuiColor  bg;
    TuiAttr   attr;
} TuiSeparator;

/**
 * @brief Initialize a horizontal separator.
 *
 * Height is set to 1.
 *
 * @param sep    Separator struct to initialize.
 * @param x      Column offset.
 * @param y      Row offset.
 * @param width  Line width in columns.
 * @return TUI_OK on success.
 */
TuiResult tui_separator_init_h(TuiSeparator *sep, int x, int y, int width);

/**
 * @brief Initialize a vertical separator.
 *
 * Width is set to 1.
 *
 * @param sep     Separator struct to initialize.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param height  Line height in rows.
 * @return TUI_OK on success.
 */
TuiResult tui_separator_init_v(TuiSeparator *sep, int x, int y, int height);

/** @brief Set separator colors. */
void tui_separator_set_colors(TuiSeparator *sep, TuiColor fg, TuiColor bg);

/** @brief Set separator text attributes. */
void tui_separator_set_attr(TuiSeparator *sep, TuiAttr attr);

#endif
