/**
 * @file zephio_progress.h
 * @brief Horizontal progress bar widget (0-100%).
 *
 * Renders a configurable progress bar with fill/empty characters,
 * optional label text, and percentage display. Not focusable.
 */

#ifndef ZEPHIO_PROGRESS_H
#define ZEPHIO_PROGRESS_H

#include "zephio_widget.h"

typedef struct {
    ZephioWidget base;
    int       value;
    char      fill_char[4];
    char      empty_char[4];
    char     *label;
    int       show_percent;
    ZephioColor  fg_fill;
    ZephioColor  bg_fill;
    ZephioColor  fg_empty;
    ZephioColor  bg_empty;
    ZephioColor  fg_label;
    ZephioColor  bg_label;
    ZephioAttr   attr;
} ZephioProgress;

/**
 * @brief Initialize a progress bar widget with context.
 *
 * @param progress  Progress struct to initialize.
 * @param ctx       TUI context.
 * @param x         Column offset.
 * @param y         Row offset.
 * @param width     Width in columns (minimum 4 for "[%%]").
 * @param height    Height in rows (typically 1).
 * @return ZEPHIO_OK on success.
 */
ZephioResult zephio_progress_init_ctx(ZephioProgress *progress, ZephioContext *ctx, int x, int y, int width, int height);

/**
 * @brief Set the progress value (clamped to 0-100).
 */
void zephio_progress_set_value(ZephioProgress *progress, int value);

/**
 * @brief Get the current progress value (0-100).
 */
int zephio_progress_get_value(ZephioProgress *progress);

/**
 * @brief Set the fill and empty characters (UTF-8, up to 4 bytes each).
 */
void zephio_progress_set_chars(ZephioProgress *progress, const char *fill, const char *empty);

/**
 * @brief Set an optional label shown before the bar.
 *
 * @param label  Label text (copied; may be NULL to clear).
 */
void zephio_progress_set_label(ZephioProgress *progress, const char *label);

/**
 * @brief Show or hide the percentage text after the bar.
 */
void zephio_progress_set_show_percent(ZephioProgress *progress, int show);

/**
 * @brief Set colors for the filled and empty portions of the bar.
 */
void zephio_progress_set_colors(ZephioProgress *progress,
                             ZephioColor fg_fill, ZephioColor bg_fill,
                             ZephioColor fg_empty, ZephioColor bg_empty);

/**
 * @brief Set colors for the label text.
 */
void zephio_progress_set_label_colors(ZephioProgress *progress,
                                   ZephioColor fg, ZephioColor bg);

/**
 * @brief Set text attributes for the entire bar.
 */
void zephio_progress_set_attr(ZephioProgress *progress, ZephioAttr attr);

#endif
