/**
 * @file tui_progress.h
 * @brief Horizontal progress bar widget (0-100%).
 *
 * Renders a configurable progress bar with fill/empty characters,
 * optional label text, and percentage display. Not focusable.
 */

#ifndef TUI_PROGRESS_H
#define TUI_PROGRESS_H

#include "tui_widget.h"

typedef struct {
    TuiWidget base;
    int       value;
    char      fill_char[4];
    char      empty_char[4];
    char     *label;
    int       show_percent;
    TuiColor  fg_fill;
    TuiColor  bg_fill;
    TuiColor  fg_empty;
    TuiColor  bg_empty;
    TuiColor  fg_label;
    TuiColor  bg_label;
    TuiAttr   attr;
} TuiProgress;

/**
 * @brief Initialize a progress bar widget with context.
 *
 * @param progress  Progress struct to initialize.
 * @param ctx       TUI context.
 * @param x         Column offset.
 * @param y         Row offset.
 * @param width     Width in columns (minimum 4 for "[%%]").
 * @param height    Height in rows (typically 1).
 * @return TUI_OK on success.
 */
TuiResult tui_progress_init_ctx(TuiProgress *progress, TuiContext *ctx, int x, int y, int width, int height);

/**
 * @brief Set the progress value (clamped to 0-100).
 */
void tui_progress_set_value(TuiProgress *progress, int value);

/**
 * @brief Get the current progress value (0-100).
 */
int tui_progress_get_value(TuiProgress *progress);

/**
 * @brief Set the fill and empty characters (UTF-8, up to 4 bytes each).
 */
void tui_progress_set_chars(TuiProgress *progress, const char *fill, const char *empty);

/**
 * @brief Set an optional label shown before the bar.
 *
 * @param label  Label text (copied; may be NULL to clear).
 */
void tui_progress_set_label(TuiProgress *progress, const char *label);

/**
 * @brief Show or hide the percentage text after the bar.
 */
void tui_progress_set_show_percent(TuiProgress *progress, int show);

/**
 * @brief Set colors for the filled and empty portions of the bar.
 */
void tui_progress_set_colors(TuiProgress *progress,
                             TuiColor fg_fill, TuiColor bg_fill,
                             TuiColor fg_empty, TuiColor bg_empty);

/**
 * @brief Set colors for the label text.
 */
void tui_progress_set_label_colors(TuiProgress *progress,
                                   TuiColor fg, TuiColor bg);

/**
 * @brief Set text attributes for the entire bar.
 */
void tui_progress_set_attr(TuiProgress *progress, TuiAttr attr);

#endif
