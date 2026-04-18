/**
 * @file tui_box.h
 * @brief Bordered box/panel widget with optional title.
 *
 * Draws a Unicode box border (single or double-line) with an optional
 * title string centered in the top border. Interior is filled with the
 * box background color.
 *
 * Supports padding inside the border. If the box has children, they
 * are positioned within the padded interior.
 */

#ifndef TUI_BOX_H
#define TUI_BOX_H

#include "tui_widget.h"

#define TUI_BOX_SINGLE 0
#define TUI_BOX_DOUBLE 1

/**
 * @brief Box widget data.
 */
typedef struct {
    TuiWidget base;
    char     *title;
    TuiColor  fg;
    TuiColor  bg;
    TuiAttr   attr;
    int       border_style;
    int       padding;
} TuiBox;

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
 * @return TUI_OK on success.
 */
TuiResult tui_box_init_ctx(TuiBox *box, TuiContext *ctx, int x, int y, int width, int height,
                           int border_style);

/**
 * @brief Set or update the box title (centered in top border).
 */
void tui_box_set_title(TuiBox *box, const char *title);

/** @brief Set border and interior colors. */
void tui_box_set_colors(TuiBox *box, TuiColor fg, TuiColor bg);

/** @brief Set text attributes for the border. */
void tui_box_set_attr(TuiBox *box, TuiAttr attr);

/** @brief Set inner padding (space between border and children). */
void tui_box_set_padding(TuiBox *box, int padding);

#endif
