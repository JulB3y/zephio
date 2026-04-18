/**
 * @file tui_style.h
 * @brief Themes, colors, and convenience drawing functions.
 *
 * Provides the TuiColor enum (256-color palette), TuiStyle struct
 * (fg + bg + attributes), TuiTheme (per-state styles), and convenience
 * wrappers that combine screen drawing with style application.
 */

#ifndef TUI_STYLE_H
#define TUI_STYLE_H

#include "tui_context.h"
#include <stdint.h>

/**
 * @brief Named 256-color palette entries.
 */
typedef enum {
    TUI_COLOR_BLACK   = 0,
    TUI_COLOR_RED     = 1,
    TUI_COLOR_GREEN   = 2,
    TUI_COLOR_YELLOW  = 3,
    TUI_COLOR_BLUE    = 4,
    TUI_COLOR_MAGENTA = 5,
    TUI_COLOR_CYAN    = 6,
    TUI_COLOR_WHITE   = 7,

    TUI_COLOR_BRIGHT_BLACK   = 8,
    TUI_COLOR_BRIGHT_RED     = 9,
    TUI_COLOR_BRIGHT_GREEN   = 10,
    TUI_COLOR_BRIGHT_YELLOW  = 11,
    TUI_COLOR_BRIGHT_BLUE    = 12,
    TUI_COLOR_BRIGHT_MAGENTA = 13,
    TUI_COLOR_BRIGHT_CYAN    = 14,
    TUI_COLOR_BRIGHT_WHITE   = 15,

    TUI_COLOR_GRAY_DARK  = 232,
    TUI_COLOR_GRAY_MID   = 240,
    TUI_COLOR_GRAY_LIGHT = 248,

    TUI_COLOR_BG_DARK  = 234,
    TUI_COLOR_BG_MID   = 236,
    TUI_COLOR_BG_LIGHT = 238
} TuiPaletteColor;

typedef TuiAttr TuiTextAttributes;

/**
 * @brief Complete style for a single cell (fg, bg, attributes).
 */
typedef struct {
    TuiColor fg;
    TuiColor bg;
    TuiAttr  attr;
} TuiStyle;

#define TUI_STYLE(fg_color, bg_color, attrs) \
    ((TuiStyle){ TUI_COLOR_INDEX(fg_color), TUI_COLOR_INDEX(bg_color), (attrs) })

#define TUI_STYLE_RGB(fg_r,fg_g,fg_b, bg_r,bg_g,bg_b, attrs) \
    ((TuiStyle){ TUI_COLOR_RGB(fg_r,fg_g,fg_b), TUI_COLOR_RGB(bg_r,bg_g,bg_b), (attrs) })

TuiStyle tui_style_make(TuiColor fg, TuiColor bg, TuiAttr attr);

#define TUI_STYLE_NONE TUI_STYLE(0, 0, TUI_ATTR_NONE)

/**
 * @brief Widget visual states (used to index into a TuiTheme).
 */
typedef enum {
    TUI_STATE_NORMAL   = 0,
    TUI_STATE_FOCUSED  = 1,
    TUI_STATE_DISABLED = 2,
    TUI_STATE_ACTIVE   = 3,
    TUI_STATE_HOVER    = 4,
    TUI_STATE_COUNT    = 5
} TuiWidgetState;

/**
 * @brief Per-state style collection for a widget.
 */
typedef struct TuiTheme {
    TuiStyle styles[TUI_STATE_COUNT];
} TuiTheme;

/**
 * @brief Return the built-in default theme.
 *
 * Uses a dark color scheme with cyan/blue accents.
 */
TuiTheme tui_theme_default(void);

/**
 * @brief Create a theme with custom normal, focused, disabled, and active styles.
 *
 * The hover state is derived from the focused style.
 *
 * @param normal   Style for the normal state.
 * @param focused  Style for the focused state.
 * @param disabled Style for the disabled state.
 * @param active   Style for the active/pressed state.
 * @return The assembled TuiTheme.
 */
TuiTheme tui_theme_create(const TuiStyle *normal, const TuiStyle *focused,
                          const TuiStyle *disabled, const TuiStyle *active);

/**
 * @brief Set a single cell with a TuiStyle.
 *
 * @param ctx    TUI context.
 * @param row    0-based row.
 * @param col    0-based column.
 * @param ch     Character.
 * @param style  Style to apply.
 */
void tui_style_set_cell(TuiContext *ctx, int row, int col, const char *ch, const TuiStyle *style);

/**
 * @brief Write text at (row, col) with a TuiStyle.
 *
 * @param ctx    TUI context.
 * @param row    0-based row.
 * @param col    0-based starting column.
 * @param text   UTF-8 text to write.
 * @param style  Style to apply.
 */
void tui_style_write(TuiContext *ctx, int row, int col, const char *text, const TuiStyle *style);

/**
 * @brief Fill a rectangular region with a TuiStyle.
 *
 * @param ctx     TUI context.
 * @param row     Top-left row.
 * @param col     Top-left column.
 * @param width   Region width.
 * @param height  Region height.
 * @param ch      Fill character.
 * @param style   Style to apply.
 */
void tui_style_fill(TuiContext *ctx, int row, int col, int width, int height,
                    const char *ch, const TuiStyle *style);

#endif
