/**
 * @file tui_container.h
 * @brief Simple colored-rectangle container widget.
 *
 * A TuiContainer renders as a filled rectangle with an optional
 * background color. It has no input handling, focus, or mouse
 * interaction. Useful as a grouping/visual-separation widget.
 */

#ifndef TUI_CONTAINER_H
#define TUI_CONTAINER_H

#include "tui_widget.h"

/**
 * @brief Container widget data.
 */
typedef struct {
    TuiWidget base;
    uint8_t   bg;
} TuiContainer;

/**
 * @brief Initialize a container widget.
 *
 * @param container  Container struct to initialize.
 * @param x          Column offset.
 * @param y          Row offset.
 * @param width      Width in columns.
 * @param height     Height in rows.
 * @return TUI_OK on success.
 */
TuiResult tui_container_init(TuiContainer *container, int x, int y,
                             int width, int height);

/**
 * @brief Set the container background color.
 *
 * @param container  Container widget.
 * @param bg         256-color background index.
 */
void tui_container_set_bg(TuiContainer *container, uint8_t bg);

#endif
