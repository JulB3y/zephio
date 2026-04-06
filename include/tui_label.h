/**
 * @file tui_label.h
 * @brief Static text label widget.
 *
 * Displays a single line of text with configurable colors and attributes.
 * The label is not focusable and does not handle input.
 */

#ifndef TUI_LABEL_H
#define TUI_LABEL_H

#include "tui_widget.h"

/**
 * @brief Label widget data.
 */
typedef struct {
    TuiWidget base;
    char    *text;
    uint8_t  fg;
    uint8_t  bg;
    TuiAttr  attr;
} TuiLabel;

/**
 * @brief Initialize a label widget.
 *
 * @param label   Label struct to initialize.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param width   Width in columns.
 * @param height  Height in rows.
 * @param text    Initial text (copied; may be NULL).
 * @return TUI_OK on success.
 */
TuiResult tui_label_init(TuiLabel *label, int x, int y, int width, int height,
                         const char *text);

/**
 * @brief Update the label text.
 *
 * @param label  Label widget.
 * @param text   New text (copied; may be NULL).
 */
void tui_label_set_text(TuiLabel *label, const char *text);

/**
 * @brief Set foreground and background colors.
 */
void tui_label_set_colors(TuiLabel *label, uint8_t fg, uint8_t bg);

/**
 * @brief Set text attributes (bold, underline, etc.).
 */
void tui_label_set_attr(TuiLabel *label, TuiAttr attr);

#endif
