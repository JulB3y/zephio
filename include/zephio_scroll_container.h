/**
 * @file zephio_scroll_container.h
 * @brief Scrollable container widget with viewport clipping and scrollbars.
 *
 * ZephioScrollContainer displays a virtual content area that can be larger than
 * the widget's visible viewport. Children outside the viewport are clipped.
 * Supports keyboard (arrows, PageUp/Down, Home/End) and mouse-wheel scrolling.
 * Renders proportional scrollbar thumbs when content exceeds the viewport.
 */

#ifndef ZEPHIO_SCROLL_CONTAINER_H
#define ZEPHIO_SCROLL_CONTAINER_H

#include "zephio_widget.h"

/**
 * @brief Scrollable container widget data.
 */
typedef struct {
    ZephioWidget base;
    int scroll_x;
    int scroll_y;
    int content_width;
    int content_height;
    int dragging;
} ZephioScrollContainer;

/**
 * @brief Initialize a scroll container widget.
 *
 * @param sc      ScrollContainer struct to initialize.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param width   Width in columns.
 * @param height  Height in rows.
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_scroll_container_init(ZephioScrollContainer *sc, int x, int y,
                                    int width, int height);

/**
 * @brief Initialize a scroll container widget with context.
 *
 * @param sc      ScrollContainer struct to initialize.
 * @param ctx     TUI context.
 * @param x       Column offset.
 * @param y       Row offset.
 * @param width   Width in columns.
 * @param height  Height in rows.
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_scroll_container_init_ctx(ZephioScrollContainer *sc, ZephioContext *ctx, int x, int y,
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
void zephio_scroll_container_set_content_size(ZephioScrollContainer *sc,
                                           int width, int height);

/**
 * @brief Auto-compute content size from children's bounding box.
 *
 * Sets content_width/height to the union of all children's (x + width)
 * and (y + height).
 *
 * @param sc  ScrollContainer widget.
 */
void zephio_scroll_container_auto_content_size(ZephioScrollContainer *sc);

/**
 * @brief Scroll to an absolute position.
 *
 * Values are clamped to the valid scroll range.
 *
 * @param sc  ScrollContainer widget.
 * @param x   Horizontal scroll offset.
 * @param y   Vertical scroll offset.
 */
void zephio_scroll_container_scroll_to(ZephioScrollContainer *sc, int x, int y);

/**
 * @brief Set horizontal scroll offset.
 */
void zephio_scroll_container_set_scroll_x(ZephioScrollContainer *sc, int x);

/**
 * @brief Set vertical scroll offset.
 */
void zephio_scroll_container_set_scroll_y(ZephioScrollContainer *sc, int y);

/**
 * @brief Clamp scroll offsets to valid range.
 *
 * @param sc        ScrollContainer widget.
 * @param cv_width  Content viewport width (widget width minus scrollbar).
 * @param cv_height Content viewport height (widget height minus scrollbar).
 */
void zephio_scroll_container_clamp_scroll(ZephioScrollContainer *sc,
                                        int cv_width, int cv_height);

/**
 * @brief Render scrollbar indicators for a scroll container.
 *
 * Call this after rendering the content area.
 *
 * @param widget     Widget (for abs_x/abs_y and dimensions).
 * @param sc         ScrollContainer with scroll/content state.
 * @param has_vscroll  Whether vertical scrollbar is needed.
 * @param has_hscroll  Whether horizontal scrollbar is needed.
 * @param cv_width    Content viewport width.
 * @param cv_height   Content viewport height.
 */
void zephio_scroll_container_render_scrollbars(ZephioWidget *widget,
                                             ZephioScrollContainer *sc,
                                             int has_vscroll, int has_hscroll,
                                             int cv_width, int cv_height);

/**
 * @brief Default keyboard scroll handler (arrows, PageUp/Down, Home/End).
 *
 * Can be reused in compound widget vtables that embed ZephioScrollContainer.
 *
 * @return 1 if the event was consumed, 0 otherwise.
 */
int zephio_scroll_container_handle_input(ZephioWidget *widget, const ZephioEvent *event);

/**
 * @brief Default mouse scroll handler (wheel, scrollbar click).
 *
 * Can be reused in compound widget vtables that embed ZephioScrollContainer.
 *
 * @return 1 if the event was consumed, 0 otherwise.
 */
int zephio_scroll_container_handle_mouse(ZephioWidget *widget, const ZephioMouseEvent *mouse);

#endif
