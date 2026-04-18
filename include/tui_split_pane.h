/**
 * @file tui_split_pane.h
 * @brief Split pane container widget with draggable divider.
 *
 * Divides its area into two panes (left/right or top/bottom)
 * separated by a draggable divider. Supports:
 *   - Horizontal (side-by-side) and vertical (stacked) orientations
 *   - Mouse drag on the separator bar
 *   - Keyboard resize (Ctrl+Left/Right or Ctrl+Up/Down)
 *   - Configurable minimum pane sizes
 *   - Nested split panes
 *
 * The two child panes are managed as regular children. The split
 * pane sets manages_children = 1 so it controls child positioning
 * and rendering.
 */

#ifndef TUI_SPLIT_PANE_H
#define TUI_SPLIT_PANE_H

#include "tui_widget.h"

#define TUI_SPLIT_MIN_PANE 5

/**
 * @brief Split orientation.
 */
typedef enum {
    TUI_SPLIT_HORIZONTAL = 0,
    TUI_SPLIT_VERTICAL   = 1
} TuiSplitOrientation;

/**
 * @brief Split pane widget data.
 */
typedef struct {
    TuiWidget base;

    TuiSplitOrientation orientation;
    int     split_pos;
    int     min_pane1;
    int     min_pane2;
    int     separator_size;
    int     dragging;
    int     drag_origin;
    int     drag_split_origin;

    TuiColor sep_fg;
    TuiColor sep_bg;
    TuiAttr  sep_attr;
} TuiSplitPane;

/**
 * @brief Initialize a split pane widget with context.
 *
 * Starts with an even 50/50 split.
 *
 * @param pane         Split pane struct to initialize.
 * @param ctx          TUI context.
 * @param x            Column offset.
 * @param y            Row offset.
 * @param width        Total width.
 * @param height       Total height.
 * @param orientation  TUI_SPLIT_HORIZONTAL or TUI_SPLIT_VERTICAL.
 * @return TUI_OK on success.
 */
TuiResult tui_split_pane_init_ctx(TuiSplitPane *pane, TuiContext *ctx, int x, int y,
                                  int width, int height,
                                  TuiSplitOrientation orientation);

/**
 * @brief Set the split position (in cells from the left/top edge).
 *
 * Clamped to respect minimum pane sizes.
 *
 * @param pane  Split pane.
 * @param pos   New split position.
 */
void tui_split_pane_set_position(TuiSplitPane *pane, int pos);

/**
 * @brief Get the current split position.
 *
 * @param pane  Split pane.
 * @return Split position in cells.
 */
int tui_split_pane_get_position(const TuiSplitPane *pane);

/**
 * @brief Set minimum sizes for pane1 and pane2.
 *
 * @param pane       Split pane.
 * @param min_pane1  Minimum size for the first pane (left/top).
 * @param min_pane2  Minimum size for the second pane (right/bottom).
 */
void tui_split_pane_set_min_sizes(TuiSplitPane *pane, int min_pane1,
                                  int min_pane2);

/**
 * @brief Set the separator colors and attributes.
 *
 * @param pane  Split pane.
 * @param fg    Separator foreground color.
 * @param bg    Separator background color.
 * @param attr  Separator text attributes.
 */
void tui_split_pane_set_separator_style(TuiSplitPane *pane, TuiColor fg,
                                        TuiColor bg, TuiAttr attr);

/**
 * @brief Set pane1 and pane2 as the two children.
 *
 * Convenience: removes any existing children and adds both.
 *
 * @param pane   Split pane.
 * @param pane1  Widget for the first pane (left or top).
 * @param pane2  Widget for the second pane (right or bottom).
 */
void tui_split_pane_set_panes(TuiSplitPane *pane, TuiWidget *pane1,
                              TuiWidget *pane2);

/**
 * @brief Get the rectangle for pane1.
 *
 * @param pane   Split pane.
 * @param[out] x Column offset.
 * @param[out] y Row offset.
 * @param[out] w Width.
 * @param[out] h Height.
 */
void tui_split_pane_get_pane1_rect(const TuiSplitPane *pane,
                                   int *x, int *y, int *w, int *h);

/**
 * @brief Get the rectangle for pane2.
 *
 * @param pane   Split pane.
 * @param[out] x Column offset.
 * @param[out] y Row offset.
 * @param[out] w Width.
 * @param[out] h Height.
 */
void tui_split_pane_get_pane2_rect(const TuiSplitPane *pane,
                                   int *x, int *y, int *w, int *h);

#endif
