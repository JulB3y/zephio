/**
 * @file tui_ansi.h
 * @brief ANSI escape-sequence helpers.
 *
 * Provides both preprocessor macros for compile-time sequence generation
 * and runtime functions for dynamic sequences (e.g., cursor positioning
 * with integer coordinates).
 *
 * The runtime functions write directly to the global terminal fd via
 * terminal_write_seq(). They require tui_init() to have been called.
 */

#ifndef TUI_ANSI_H
#define TUI_ANSI_H

#include "tui_context.h"
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
void ansi_move_cursor(TuiContext *ctx, int row, int col);
/** @brief Move the cursor up by n rows. */
void ansi_move_up(TuiContext *ctx, int n);
/** @brief Move the cursor down by n rows. */
void ansi_move_down(TuiContext *ctx, int n);
/** @brief Move the cursor left by n columns. */
void ansi_move_left(TuiContext *ctx, int n);
/** @brief Move the cursor right by n columns. */
void ansi_move_right(TuiContext *ctx, int n);
/** @brief Save cursor position (DEC private). */
void ansi_save_cursor(TuiContext *ctx);
/** @brief Restore previously saved cursor position. */
void ansi_restore_cursor(TuiContext *ctx);

/** @brief Clear the entire screen. */
void ansi_clear_screen(TuiContext *ctx);
/** @brief Clear the entire current line. */
void ansi_clear_line(TuiContext *ctx);
/** @brief Clear from cursor to end of line. */
void ansi_clear_line_right(TuiContext *ctx);
/** @brief Clear from start of line to cursor. */
void ansi_clear_line_left(TuiContext *ctx);
/** @brief Clear from start of screen to cursor. */
void ansi_clear_screen_above(TuiContext *ctx);
/** @brief Clear from cursor to end of screen. */
void ansi_clear_screen_below(TuiContext *ctx);

/** @brief Set foreground to 256-color index. */
void ansi_set_fg(TuiContext *ctx, int color);
/** @brief Set background to 256-color index. */
void ansi_set_bg(TuiContext *ctx, int color);
/** @brief Set foreground to 24-bit RGB. */
void ansi_set_fg_rgb(TuiContext *ctx, int r, int g, int b);
/** @brief Set background to 24-bit RGB. */
void ansi_set_bg_rgb(TuiContext *ctx, int r, int g, int b);
/** @brief Reset all attributes and colors. */
void ansi_reset(TuiContext *ctx);
/** @brief Enable bold attribute. */
void ansi_set_bold(TuiContext *ctx);
/** @brief Enable dim/faint attribute. */
void ansi_set_dim(TuiContext *ctx);
/** @brief Enable italic attribute. */
void ansi_set_italic(TuiContext *ctx);
/** @brief Enable underline attribute. */
void ansi_set_underline(TuiContext *ctx);
/** @brief Enable blink attribute. */
void ansi_set_blink(TuiContext *ctx);
/** @brief Enable reverse video attribute. */
void ansi_set_reverse(TuiContext *ctx);
/** @brief Enable strikethrough attribute. */
void ansi_set_strikethrough(TuiContext *ctx);

/** @brief Write raw bytes to the terminal. */
void ansi_write(TuiContext *ctx, const char *text, size_t len);
/** @brief Write raw bytes at a specific position (1-based). */
void ansi_write_at(TuiContext *ctx, int row, int col, const char *text, size_t len);

#endif
