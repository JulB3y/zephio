/**
 * @file zephio_container.h
 * @brief Simple colored-rectangle container widget.
 *
 * A ZephioContainer renders as a filled rectangle with an optional
 * background color. It has no input handling, focus, or mouse
 * interaction. Useful as a grouping/visual-separation widget.
 */

#ifndef ZEPHIO_CONTAINER_H
#define ZEPHIO_CONTAINER_H

#include "zephio_widget.h"

/**
 * @brief Container widget data.
 */
typedef struct {
    ZephioWidget base;
    ZephioColor  bg;
} ZephioContainer;

/**
 * @brief Initialize a container widget with context.
 *
 * @param container  Container struct to initialize.
 * @param ctx        TUI context.
 * @param x          Column offset.
 * @param y          Row offset.
 * @param width      Width in columns.
 * @param height     Height in rows.
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_container_init_ctx(ZephioContainer *container, ZephioContext *ctx, int x, int y,
                               int width, int height);

/**
 * @brief Set the container background color.
 *
 * @param container  Container widget.
 * @param bg         256-color background index.
 */
void zephio_container_set_bg(ZephioContainer *container, ZephioColor bg);

#endif
