/**
 * @file tui_scroll_container.h
 * @brief Scrollable container widget with viewport clipping and scrollbars.
 *
 * TuiScrollContainer displays a virtual content area that can be larger than
 * the widget's visible viewport. Children outside the viewport are clipped.
 * Supports keyboard (arrows, PageUp/Down, Home/End) and mouse-wheel scrolling.
 * Renders proportional scrollbar thumbs when content exceeds the viewport.
 */

#ifndef TUI_SCROLL_CONTAINER_H
#define TUI_SCROLL_CONTAINER_H

#include "tui_widget.h"

/**
 * @brief Scrollable container widget data.
 */
typedef struct {
    TuiWidget base;
    int scroll_x;
    int scroll_y;
    int content_width;
    int content_height;
} TuiScrollContainer;

/**
 * @brief Initialize a scroll container widget.
 *
 * @param sc      ScrollContainer struct to initialize.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param width   Width in columns.
 * @param height  Height in rows.
 * @return TUI_OK on success.
 */
TuiResult tui_scroll_container_init(TuiScrollContainer *sc, int x, int y,
                                    int width, int height);

/**
 * @brief Set the virtual content dimensions.
 *
 * When content exceeds the viewport, scrollbars appear automatically.
 *
 * @param sc      ScrollContainer widget.
 * @param width   Virtual content width.
 * @param height  Virtual content height.
 */
void tui_scroll_container_set_content_size(TuiScrollContainer *sc,
                                           int width, int height);

/**
 * @brief Auto-compute content size from children's bounding box.
 *
 * Sets content_width/height to the union of all children's (x + width)
 * and (y + height).
 *
 * @param sc  ScrollContainer widget.
 */
void tui_scroll_container_auto_content_size(TuiScrollContainer *sc);

/**
 * @brief Scroll to an absolute position.
 *
 * Values are clamped to the valid scroll range.
 *
 * @param sc  ScrollContainer widget.
 * @param x   Horizontal scroll offset.
 * @param y   Vertical scroll offset.
 */
void tui_scroll_container_scroll_to(TuiScrollContainer *sc, int x, int y);

/**
 * @brief Set horizontal scroll offset.
 */
void tui_scroll_container_set_scroll_x(TuiScrollContainer *sc, int x);

/**
 * @brief Set vertical scroll offset.
 */
void tui_scroll_container_set_scroll_y(TuiScrollContainer *sc, int y);

#endif
