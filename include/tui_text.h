/**
 * @file tui_text.h
 * @brief UTF-8 string utilities and text measurement.
 *
 * Provides UTF-8 validation, character width calculation (approximation
 * of wcwidth), text clipping, truncation with ellipsis, word wrapping,
 * and tab expansion.
 *
 * All functions are pure (no global state, no I/O).
 */

#ifndef ZEPHIO_TEXT_H
#define ZEPHIO_TEXT_H

#include <stddef.h>
#include <stdint.h>

#define ZEPHIO_TAB_SIZE 8

/**
 * @brief Validate a UTF-8 byte sequence.
 *
 * @param str  Byte sequence to validate.
 * @param len  Number of bytes.
 * @return 1 if valid UTF-8, 0 otherwise.
 */
int tui_utf8_valid(const char *str, size_t len);

/**
 * @brief Encode a Unicode codepoint as UTF-8 bytes.
 *
 * @param codepoint  Unicode codepoint (U+0000 .. U+10FFFF).
 * @param buf        Output buffer for the encoded bytes.
 * @param buf_size   Number of available bytes in buf.
 * @return Number of bytes written (1-4), or 0 on error.
 */
int tui_utf8_encode(uint32_t codepoint, char *buf, size_t buf_size);

/**
 * @brief Return the expected byte length of a UTF-8 character from its lead byte.
 *
 * @param c  The first byte of a potential UTF-8 character.
 * @return Number of expected continuation bytes (1-4), or 1 if not a valid lead byte.
 */
int tui_utf8_char_len(unsigned char c);

/**
 * @brief Decode the next UTF-8 character from a byte sequence.
 *
 * @param str        Input bytes.
 * @param len        Number of available bytes.
 * @param codepoint  [out] Decoded Unicode codepoint.
 * @return Number of bytes consumed, or 0 on error / end of input.
 */
int tui_utf8_next(const char *str, size_t len, uint32_t *codepoint);

/**
 * @brief Return the display column width of a Unicode codepoint.
 *
 * Approximation: returns 0 for control/format characters, 2 for CJK
 * and emoji ranges, 1 for everything else.
 *
 * @param codepoint  Unicode codepoint.
 * @return Column width (0, 1, or 2).
 */
int tui_utf8_char_width(uint32_t codepoint);

/**
 * @brief Measure the total display width of a UTF-8 string.
 *
 * @param text  Input bytes.
 * @param len   Number of bytes.
 * @return Total column width.
 */
int tui_text_width(const char *text, size_t len);

/**
 * @brief Measure the total display width of a null-terminated UTF-8 string.
 *
 * @param text  Null-terminated UTF-8 string.
 * @return Total column width.
 */
int tui_text_str_width(const char *text);

/**
 * @brief Clip a UTF-8 string to a maximum display width.
 *
 * Does not copy; returns the byte length of the longest prefix that
 * fits within max_width columns.
 *
 * @param text      Input bytes.
 * @param len       Number of bytes.
 * @param max_width Maximum display columns.
 * @param[out] out_len  Byte length of the clipped prefix.
 * @return 0 on success.
 */
int tui_text_clip(const char *text, size_t len, int max_width, size_t *out_len);

/**
 * @brief Truncate a UTF-8 string with an ellipsis to fit a display width.
 *
 * @param text      Input bytes.
 * @param len       Number of bytes.
 * @param max_width Maximum display columns.
 * @param ellipsis  Ellipsis string (e.g., "~" or "...").
 * @param out       Output buffer.
 * @param out_size  Output buffer size.
 */
void tui_text_truncate(const char *text, size_t len, int max_width,
                       const char *ellipsis, char *out, size_t out_size);

/**
 * @brief Calculate word-wrap break positions.
 *
 * @param text       Input bytes.
 * @param len        Number of bytes.
 * @param max_width  Maximum display columns per line.
 * @param breaks     [out] Array of byte offsets where line breaks occur.
 * @param max_breaks Maximum number of breaks to store.
 * @return Number of breaks stored.
 */
int tui_text_word_wrap(const char *text, size_t len, int max_width,
                       int *breaks, int max_breaks);

/**
 * @brief Convert a byte index to a display column.
 *
 * @param text  Input bytes.
 * @param len   Number of bytes.
 * @param index Byte index into text.
 * @return Display column.
 */
int tui_text_index_to_col(const char *text, size_t len, size_t index);

/**
 * @brief Convert a display column to a byte index.
 *
 * @param text  Input bytes.
 * @param len   Number of bytes.
 * @param col   Display column.
 * @return Byte index.
 */
int tui_text_col_to_index(const char *text, size_t len, int col);

/**
 * @brief Calculate tab expansion for a given column.
 *
 * @param col       Current column (0-based).
 * @param tab_size  Tab stop interval.
 * @return Number of columns to advance.
 */
int tui_text_expand_tab(int col, int tab_size);

/**
 * @brief Expand tabs in a string to spaces.
 *
 * @param text      Input bytes.
 * @param len       Number of bytes.
 * @param tab_size  Tab stop interval.
 * @param out       Output buffer.
 * @param out_size  Output buffer size.
 * @return Number of bytes written (excluding null terminator).
 */
int tui_text_expand_tabs(const char *text, size_t len, int tab_size,
                         char *out, size_t out_size);

#endif
