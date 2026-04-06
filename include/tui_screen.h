/**
 * @file tui_screen.h
 * @brief Double-buffered screen for flicker-free rendering.
 *
 * The screen module maintains two cell buffers (front and back). Widgets
 * write to the back buffer; tui_screen_render() diffs the two buffers
 * and emits only the changed cells to the terminal.
 *
 * Usage:
 *   1. tui_screen_init(rows, cols)  — once after tui_init()
 *   2. tui_screen_clear()           — clear the back buffer
 *   3. tui_screen_write / set_cell  — draw into back buffer
 *   4. tui_screen_render()          — flush diffs to terminal
 *
 * On resize, call tui_screen_resize(new_rows, new_cols).
 */

#ifndef TUI_SCREEN_H
#define TUI_SCREEN_H

#include "tui.h"
#include <stdint.h>

/**
 * @brief Text attributes bitmask (bold, underline, etc.).
 *
 * Combine with bitwise OR: `TUI_ATTR_BOLD | TUI_ATTR_UNDERLINE`.
 */
typedef uint16_t TuiAttr;

#define TUI_ATTR_NONE          0x0000
#define TUI_ATTR_BOLD          0x0001
#define TUI_ATTR_DIM           0x0002
#define TUI_ATTR_ITALIC        0x0004
#define TUI_ATTR_UNDERLINE     0x0008
#define TUI_ATTR_BLINK         0x0010
#define TUI_ATTR_REVERSE       0x0020
#define TUI_ATTR_STRIKETHROUGH 0x0040

/**
 * @brief A single terminal cell (character + colors + attributes).
 */
typedef struct {
    char     ch[4];
    uint8_t  fg;
    uint8_t  bg;
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
 * @param rows  Number of terminal rows.
 * @param cols  Number of terminal columns.
 * @return TUI_OK on success, TUI_ERR_MEMORY on allocation failure.
 */
TuiResult tui_screen_init(int rows, int cols);

/**
 * @brief Free both buffers.
 */
void tui_screen_free(void);

/**
 * @brief Resize both buffers, preserving existing content where possible.
 *
 * @param rows  New row count.
 * @param cols  New column count.
 * @return TUI_OK on success, TUI_ERR_MEMORY on allocation failure.
 */
TuiResult tui_screen_resize(int rows, int cols);

/**
 * @brief Clear the back buffer (fill with spaces, default colors).
 */
void tui_screen_clear(void);

/**
 * @brief Diff front vs. back and write changed cells to the terminal.
 *
 * After rendering, front and back are swapped.
 */
void tui_screen_render(void);

/**
 * @brief Set a single cell in the back buffer.
 *
 * @param row   0-based row.
 * @param col   0-based column.
 * @param ch    UTF-8 character string (up to 4 bytes).
 * @param fg    Foreground 256-color index.
 * @param bg    Background 256-color index.
 * @param attr  Attribute bitmask.
 */
void tui_screen_set_cell(int row, int col, const char *ch, uint8_t fg, uint8_t bg, TuiAttr attr);

/**
 * @brief Write a UTF-8 string into the back buffer (one row).
 *
 * Advances column by column for each character. Out-of-bounds cells
 * are silently clipped.
 *
 * @param row   0-based row.
 * @param col   0-based starting column.
 * @param text  UTF-8 text to write.
 * @param fg    Foreground color.
 * @param bg    Background color.
 * @param attr  Attribute bitmask.
 */
void tui_screen_write(int row, int col, const char *text, uint8_t fg, uint8_t bg, TuiAttr attr);

/**
 * @brief Fill a rectangular region with a single character + style.
 *
 * @param row     Top-left row.
 * @param col     Top-left column.
 * @param width   Region width.
 * @param height  Region height.
 * @param ch      Fill character.
 * @param fg      Foreground color.
 * @param bg      Background color.
 * @param attr    Attribute bitmask.
 */
void tui_screen_fill(int row, int col, int width, int height, const char *ch, uint8_t fg, uint8_t bg, TuiAttr attr);

/**
 * @brief Draw a single-line Unicode box border.
 *
 * Draws the border outline only; interior is not cleared.
 *
 * @param row     Top-left row.
 * @param col     Top-left column.
 * @param width   Box width (>= 2).
 * @param height  Box height (>= 2).
 * @param fg      Foreground color.
 * @param bg      Background color.
 * @param attr    Attribute bitmask.
 */
void tui_screen_box_single(int row, int col, int width, int height, uint8_t fg, uint8_t bg, TuiAttr attr);

/**
 * @brief Draw a double-line Unicode box border.
 *
 * Same as tui_screen_box_single but uses double-line box characters.
 */
void tui_screen_box_double(int row, int col, int width, int height, uint8_t fg, uint8_t bg, TuiAttr attr);

/**
 * @brief Return the current screen dimensions.
 */
TuiSize tui_screen_size(void);

extern TuiScreen g_screen;

#endif
