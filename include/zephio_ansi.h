/**
 * @file zephio_ansi.h
 * @brief ANSI escape-sequence helpers.
 *
 * Provides both preprocessor macros for compile-time sequence generation
 * and runtime functions for dynamic sequences (e.g., cursor positioning
 * with integer coordinates).
 *
 * The runtime functions write directly to the global terminal fd via
 * terminal_write_seq(). They require zephio_init() to have been called.
 */

#ifndef ZEPHIO_ANSI_H
#define ZEPHIO_ANSI_H

#include "zephio_context.h"
#include <stddef.h>

#define ANSI_ESC        "\033"
#define ANSI_CSI        "\033["

#define ANSI_CLEAR_SCREEN       ANSI_CSI "2J"
#define ANSI_CLEAR_LINE         ANSI_CSI "2K"
#define ANSI_CLEAR_LINE_RIGHT   ANSI_CSI "K"
#define ANSI_CLEAR_LINE_LEFT    ANSI_CSI "1K"
#define ANSI_CLEAR_SCREEN_ABOVE ANSI_CSI "1J"
#define ANSI_CLEAR_SCREEN_BELOW ANSI_CSI "0J"
#define ANSI_CURSOR_HOME        ANSI_CSI "H"
#define ANSI_ALT_SCREEN_ON      ANSI_CSI "?1049h"
#define ANSI_ALT_SCREEN_OFF     ANSI_CSI "?1049l"
#define ANSI_CURSOR_HIDE        ANSI_CSI "?25l"
#define ANSI_CURSOR_SHOW        ANSI_CSI "?25h"
#define ANSI_RESET              ANSI_CSI "0m"
#define ANSI_SAVE_CURSOR        ANSI_CSI "s"
#define ANSI_RESTORE_CURSOR     ANSI_CSI "u"

#define ANSI_MOUSE_ENABLE        ANSI_CSI "?1000h"
#define ANSI_MOUSE_DISABLE       ANSI_CSI "?1000l"
#define ANSI_MOUSE_DRAG_ENABLE   ANSI_CSI "?1002h"
#define ANSI_MOUSE_DRAG_DISABLE  ANSI_CSI "?1002l"
#define ANSI_MOUSE_SGR_ENABLE    ANSI_CSI "?1006h"
#define ANSI_MOUSE_SGR_DISABLE   ANSI_CSI "?1006l"

#define ANSI_CURSOR_POS(row, col) ANSI_CSI row ";" col "H"
#define ANSI_FG(color)     ANSI_CSI "38;5;" color "m"
#define ANSI_BG(color)     ANSI_CSI "48;5;" color "m"
#define ANSI_FG_RGB(r,g,b) ANSI_CSI "38;2;" r ";" g ";" b "m"
#define ANSI_BG_RGB(r,g,b) ANSI_CSI "48;2;" r ";" g ";" b "m"

#define ANSI_BOLD          ANSI_CSI "1m"
#define ANSI_DIM           ANSI_CSI "2m"
#define ANSI_ITALIC        ANSI_CSI "3m"
#define ANSI_UNDERLINE     ANSI_CSI "4m"
#define ANSI_BLINK         ANSI_CSI "5m"
#define ANSI_REVERSE       ANSI_CSI "7m"
#define ANSI_STRIKETHROUGH ANSI_CSI "9m"

/** @brief Move the cursor to (row, col), 1-based. */
void ansi_move_cursor(ZephioContext *ctx, int row, int col);
/** @brief Move the cursor up by n rows. */
void ansi_move_up(ZephioContext *ctx, int n);
/** @brief Move the cursor down by n rows. */
void ansi_move_down(ZephioContext *ctx, int n);
/** @brief Move the cursor left by n columns. */
void ansi_move_left(ZephioContext *ctx, int n);
/** @brief Move the cursor right by n columns. */
void ansi_move_right(ZephioContext *ctx, int n);
/** @brief Save cursor position (DEC private). */
void ansi_save_cursor(ZephioContext *ctx);
/** @brief Restore previously saved cursor position. */
void ansi_restore_cursor(ZephioContext *ctx);

/** @brief Clear the entire screen. */
void ansi_clear_screen(ZephioContext *ctx);
/** @brief Clear the entire current line. */
void ansi_clear_line(ZephioContext *ctx);
/** @brief Clear from cursor to end of line. */
void ansi_clear_line_right(ZephioContext *ctx);
/** @brief Clear from start of line to cursor. */
void ansi_clear_line_left(ZephioContext *ctx);
/** @brief Clear from start of screen to cursor. */
void ansi_clear_screen_above(ZephioContext *ctx);
/** @brief Clear from cursor to end of screen. */
void ansi_clear_screen_below(ZephioContext *ctx);

/** @brief Set foreground to 256-color index. */
void ansi_set_fg(ZephioContext *ctx, int color);
/** @brief Set background to 256-color index. */
void ansi_set_bg(ZephioContext *ctx, int color);
/** @brief Set foreground to 24-bit RGB. */
void ansi_set_fg_rgb(ZephioContext *ctx, int r, int g, int b);
/** @brief Set background to 24-bit RGB. */
void ansi_set_bg_rgb(ZephioContext *ctx, int r, int g, int b);
/** @brief Reset all attributes and colors. */
void ansi_reset(ZephioContext *ctx);
/** @brief Enable bold attribute. */
void ansi_set_bold(ZephioContext *ctx);
/** @brief Enable dim/faint attribute. */
void ansi_set_dim(ZephioContext *ctx);
/** @brief Enable italic attribute. */
void ansi_set_italic(ZephioContext *ctx);
/** @brief Enable underline attribute. */
void ansi_set_underline(ZephioContext *ctx);
/** @brief Enable blink attribute. */
void ansi_set_blink(ZephioContext *ctx);
/** @brief Enable reverse video attribute. */
void ansi_set_reverse(ZephioContext *ctx);
/** @brief Enable strikethrough attribute. */
void ansi_set_strikethrough(ZephioContext *ctx);

/** @brief Write raw bytes to the terminal. */
void ansi_write(ZephioContext *ctx, const char *text, size_t len);
/** @brief Write raw bytes at a specific position (1-based). */
void ansi_write_at(ZephioContext *ctx, int row, int col, const char *text, size_t len);

#endif
