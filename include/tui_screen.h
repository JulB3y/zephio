/**
 * @file tui_screen.h
 * @brief Double-buffered screen for flicker-free rendering.
 *
 * The screen module maintains two cell buffers (front and back). Widgets
 * write to the back buffer; tui_screen_render() diffs the two buffers
 * and emits only the changed cells to the terminal.
 *
 * Usage:
 *   1. tui_screen_init(ctx, rows, cols)  — once after tui_init()
 *   2. tui_screen_clear(ctx)             — clear the back buffer
 *   3. tui_screen_write / set_cell       — draw into back buffer
 *   4. tui_screen_render(ctx)            — flush diffs to terminal
 *
 * On resize, call tui_screen_resize(ctx, new_rows, new_cols).
 */

#ifndef ZEPHIO_SCREEN_H
#define ZEPHIO_SCREEN_H

#include "tui.h"
#include <stdint.h>

typedef struct TuiContext TuiContext;

/**
 * @brief Text attributes bitmask (bold, underline, etc.).
 *
 * Combine with bitwise OR: `TUI_ATTR_BOLD | TUI_ATTR_UNDERLINE`.
 */
typedef uint8_t TuiAttr;

#define ZEPHIO_ATTR_NONE          0x00
#define ZEPHIO_ATTR_BOLD          0x01
#define ZEPHIO_ATTR_DIM           0x02
#define ZEPHIO_ATTR_ITALIC        0x04
#define ZEPHIO_ATTR_UNDERLINE     0x08
#define ZEPHIO_ATTR_BLINK         0x10
#define ZEPHIO_ATTR_REVERSE       0x20
#define ZEPHIO_ATTR_STRIKETHROUGH 0x40

enum {
    TUI_COLOR_TYPE_INDEX = 0,
    TUI_COLOR_TYPE_RGB   = 1,
    TUI_COLOR_TYPE_NONE  = -1
};

typedef struct TuiColor {
    int8_t type;
    union {
        uint8_t index;
        struct { uint8_t r, g, b; } rgb;
    };
} TuiColor;

#define ZEPHIO_COLOR_INDEX(i)   ((TuiColor){ .type = TUI_COLOR_TYPE_INDEX, .index = (uint8_t)(i) })
#define ZEPHIO_COLOR_RGB(r,g,b) ((TuiColor){ .type = TUI_COLOR_TYPE_RGB, .rgb = { (uint8_t)(r), (uint8_t)(g), (uint8_t)(b) } })
#define ZEPHIO_COLOR_NONE       ((TuiColor){ .type = TUI_COLOR_TYPE_NONE, .index = 0 })

static inline int tui_color_eq(TuiColor a, TuiColor b) {
    if (a.type != b.type) return 0;
    switch (a.type) {
    case TUI_COLOR_TYPE_INDEX: return a.index == b.index;
    case TUI_COLOR_TYPE_RGB:   return a.rgb.r == b.rgb.r && a.rgb.g == b.rgb.g && a.rgb.b == b.rgb.b;
    default: return 1;
    }
}

/**
 * @brief A single terminal cell (character + colors + attributes).
 */
typedef struct {
    char     ch[4];
    uint8_t  width;
    TuiColor fg;
    TuiColor bg;
    TuiAttr  attr;
} TuiCell;

/**
 * @brief Internal screen state (front + back buffer).
 */
typedef struct {
    TuiCell *front;
    TuiCell *back;
    int      rows;
    int      cols;
    int      initialized;
} TuiScreen;

/**
 * @brief Allocate and initialize both buffers for the given dimensions.
 *
 * @param ctx   TUI context.
 * @param rows  Number of terminal rows.
 * @param cols  Number of terminal columns.
 * @return TUI_OK on success, TUI_ERR_MEMORY on allocation failure.
 */
TuiResult tui_screen_init(TuiContext *ctx, int rows, int cols);

/**
 * @brief Free both buffers.
 *
 * @param ctx  TUI context.
 */
void tui_screen_free(TuiContext *ctx);

/**
 * @brief Resize both buffers, preserving existing content where possible.
 *
 * @param ctx   TUI context.
 * @param rows  New row count.
 * @param cols  New column count.
 * @return TUI_OK on success, TUI_ERR_MEMORY on allocation failure.
 */
TuiResult tui_screen_resize(TuiContext *ctx, int rows, int cols);

/**
 * @brief Clear the back buffer (fill with spaces, default colors).
 *
 * @param ctx  TUI context.
 */
void tui_screen_clear(TuiContext *ctx);

/**
 * @brief Diff front vs. back and write changed cells to the terminal.
 *
 * After rendering, front and back are swapped.
 *
 * @param ctx  TUI context.
 */
void tui_screen_render(TuiContext *ctx);

/**
 * @brief Set a single cell in the back buffer.
 *
 * @param ctx   TUI context.
 * @param row   0-based row.
 * @param col   0-based column.
 * @param ch    UTF-8 character string (up to 4 bytes).
 * @param fg    Foreground 256-color index.
 * @param bg    Background 256-color index.
 * @param attr  Attribute bitmask.
 */
void tui_screen_set_cell(TuiContext *ctx, int row, int col, const char *ch, TuiColor fg, TuiColor bg, TuiAttr attr);

/**
 * @brief Write a UTF-8 string into the back buffer (one row).
 *
 * Advances column by column for each character. Out-of-bounds cells
 * are silently clipped.
 *
 * @param ctx   TUI context.
 * @param row   0-based row.
 * @param col   0-based starting column.
 * @param text  UTF-8 text to write.
 * @param fg    Foreground color.
 * @param bg    Background color.
 * @param attr  Attribute bitmask.
 */
void tui_screen_write(TuiContext *ctx, int row, int col, const char *text, TuiColor fg, TuiColor bg, TuiAttr attr);

/**
 * @brief Fill a rectangular region with a single character + style.
 *
 * @param ctx     TUI context.
 * @param row     Top-left row.
 * @param col     Top-left column.
 * @param width   Region width.
 * @param height  Region height.
 * @param ch      Fill character.
 * @param fg      Foreground color.
 * @param bg      Background color.
 * @param attr    Attribute bitmask.
 */
void tui_screen_fill(TuiContext *ctx, int row, int col, int width, int height, const char *ch, TuiColor fg, TuiColor bg, TuiAttr attr);

/**
 * @brief Draw a single-line Unicode box border.
 *
 * Draws the border outline only; interior is not cleared.
 *
 * @param ctx     TUI context.
 * @param row     Top-left row.
 * @param col     Top-left column.
 * @param width   Box width (>= 2).
 * @param height  Box height (>= 2).
 * @param fg      Foreground color.
 * @param bg      Background color.
 * @param attr    Attribute bitmask.
 */
void tui_screen_box_single(TuiContext *ctx, int row, int col, int width, int height, TuiColor fg, TuiColor bg, TuiAttr attr);

/**
 * @brief Draw a double-line Unicode box border.
 *
 * Same as tui_screen_box_single but uses double-line box characters.
 *
 * @param ctx     TUI context.
 * @param row     Top-left row.
 * @param col     Top-left column.
 * @param width   Box width (>= 2).
 * @param height  Box height (>= 2).
 * @param fg      Foreground color.
 * @param bg      Background color.
 * @param attr    Attribute bitmask.
 */
void tui_screen_box_double(TuiContext *ctx, int row, int col, int width, int height, TuiColor fg, TuiColor bg, TuiAttr attr);

/**
 * @brief Return the current screen dimensions.
 *
 * @param ctx  TUI context.
 */
TuiSize tui_screen_size(TuiContext *ctx);

/**
 * @brief Invert a cell's foreground and background colors.
 *
 * Used for cursor rendering: after drawing text, invert the cell at the
 * cursor position to create a block-cursor effect.
 *
 * @param ctx  TUI context.
 * @param row  0-based row.
 * @param col  0-based column.
 */
void tui_screen_invert_cell(TuiContext *ctx, int row, int col);

#endif
