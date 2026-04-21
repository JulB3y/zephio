/**
 * @file zephio_style.h
 * @brief Themes, colors, and convenience drawing functions.
 *
 * Provides the ZephioColor enum (256-color palette), ZephioStyle struct
 * (fg + bg + attributes), ZephioTheme (per-state styles), and convenience
 * wrappers that combine screen drawing with style application.
 */

#ifndef ZEPHIO_STYLE_H
#define ZEPHIO_STYLE_H

#include "zephio_context.h"
#include <stdint.h>

/**
 * @brief Named 256-color palette entries.
 */
typedef enum {
    ZEPHIO_COLOR_BLACK   = 0,
    ZEPHIO_COLOR_RED     = 1,
    ZEPHIO_COLOR_GREEN   = 2,
    ZEPHIO_COLOR_YELLOW  = 3,
    ZEPHIO_COLOR_BLUE    = 4,
    ZEPHIO_COLOR_MAGENTA = 5,
    ZEPHIO_COLOR_CYAN    = 6,
    ZEPHIO_COLOR_WHITE   = 7,

    ZEPHIO_COLOR_BRIGHT_BLACK   = 8,
    ZEPHIO_COLOR_BRIGHT_RED     = 9,
    ZEPHIO_COLOR_BRIGHT_GREEN   = 10,
    ZEPHIO_COLOR_BRIGHT_YELLOW  = 11,
    ZEPHIO_COLOR_BRIGHT_BLUE    = 12,
    ZEPHIO_COLOR_BRIGHT_MAGENTA = 13,
    ZEPHIO_COLOR_BRIGHT_CYAN    = 14,
    ZEPHIO_COLOR_BRIGHT_WHITE   = 15,

    ZEPHIO_COLOR_GRAY_DARK  = 232,
    ZEPHIO_COLOR_GRAY_MID   = 240,
    ZEPHIO_COLOR_GRAY_LIGHT = 248,

    ZEPHIO_COLOR_BG_DARK  = 234,
    ZEPHIO_COLOR_BG_MID   = 236,
    ZEPHIO_COLOR_BG_LIGHT = 238
} ZephioPaletteColor;

typedef ZephioAttr ZephioTextAttributes;

/**
 * @brief Complete style for a single cell (fg, bg, attributes).
 */
typedef struct {
    ZephioColor fg;
    ZephioColor bg;
    ZephioAttr  attr;
} ZephioStyle;

#define ZEPHIO_STYLE(fg_color, bg_color, attrs) \
    ((ZephioStyle){ ZEPHIO_COLOR_INDEX(fg_color), ZEPHIO_COLOR_INDEX(bg_color), (attrs) })

#define ZEPHIO_STYLE_RGB(fg_r,fg_g,fg_b, bg_r,bg_g,bg_b, attrs) \
    ((ZephioStyle){ ZEPHIO_COLOR_RGB(fg_r,fg_g,fg_b), ZEPHIO_COLOR_RGB(bg_r,bg_g,bg_b), (attrs) })

ZephioStyle zephio_style_make(ZephioColor fg, ZephioColor bg, ZephioAttr attr);

#define ZEPHIO_STYLE_NONE ZEPHIO_STYLE(0, 0, ZEPHIO_ATTR_NONE)

/**
 * @brief Widget visual states (used to index into a ZephioTheme).
 */
typedef enum {
    ZEPHIO_STATE_NORMAL   = 0,
    ZEPHIO_STATE_FOCUSED  = 1,
    ZEPHIO_STATE_DISABLED = 2,
    ZEPHIO_STATE_ACTIVE   = 3,
    ZEPHIO_STATE_HOVER    = 4,
    ZEPHIO_STATE_COUNT    = 5
} ZephioWidgetState;

/**
 * @brief Per-state style collection for a widget.
 */
typedef struct ZephioTheme {
    ZephioStyle styles[ZEPHIO_STATE_COUNT];
} ZephioTheme;

/**
 * @brief Return the built-in default theme.
 *
 * Uses a dark color scheme with cyan/blue accents.
 */
ZephioTheme zephio_theme_default(void);

/**
 * @brief Create a theme with custom normal, focused, disabled, and active styles.
 *
 * The hover state is derived from the focused style.
 *
 * @param normal   Style for the normal state.
 * @param focused  Style for the focused state.
 * @param disabled Style for the disabled state.
 * @param active   Style for the active/pressed state.
 * @return The assembled ZephioTheme.
 */
ZephioTheme zephio_theme_create(const ZephioStyle *normal, const ZephioStyle *focused,
                          const ZephioStyle *disabled, const ZephioStyle *active);

/**
 * @brief Set a single cell with a ZephioStyle.
 *
 * @param ctx    TUI context.
 * @param row    0-based row.
 * @param col    0-based column.
 * @param ch     Character.
 * @param style  Style to apply.
 */
void zephio_style_set_cell(ZephioContext *ctx, int row, int col, const char *ch, const ZephioStyle *style);

/**
 * @brief Write text at (row, col) with a ZephioStyle.
 *
 * @param ctx    TUI context.
 * @param row    0-based row.
 * @param col    0-based starting column.
 * @param text   UTF-8 text to write.
 * @param style  Style to apply.
 */
void zephio_style_write(ZephioContext *ctx, int row, int col, const char *text, const ZephioStyle *style);

/**
 * @brief Fill a rectangular region with a ZephioStyle.
 *
 * @param ctx     TUI context.
 * @param row     Top-left row.
 * @param col     Top-left column.
 * @param width   Region width.
 * @param height  Region height.
 * @param ch      Fill character.
 * @param style   Style to apply.
 */
void zephio_style_fill(ZephioContext *ctx, int row, int col, int width, int height,
                    const char *ch, const ZephioStyle *style);

#endif
